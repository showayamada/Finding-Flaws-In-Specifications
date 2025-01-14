#include "graph.hpp"

using namespace std;
using namespace spot;
using namespace boost;

/**
 * @inheritDoc
 */
bool compare_V_List(vector<V> left, vector<V> right) {
    if (left.size() != right.size()) {
        return true;
    }
    return false;
}

/**
 * @inheritDoc
 */
bool compare_E_List(vector<E> left, vector<E> right) {
    if (left.size() != right.size()) {
        return true;
    }
    return false;
}

/**
 * @inheritDoc
 */
Head head(CEGraph& g, int pointer) {
    edge_iter e, eend;
    for (tie(e, eend) = edges(g); e != eend; e++) {
        if (source(*e, g) == pointer) {
            EdgeProperty ceg = g[*e];
            Head head = {ceg.label};
            return head;
        }
    }

    return {"error"};
}

/**
 * @inheritDoc
 */
int tail(CEGraph& g , int head) {
    edge_iter e, eend;
    for (tie(e, eend) = edges(g); e != eend; e++) {
        if (source(*e, g) == head) {
            return target(*e, g);
        }
    }
    return -1;
}

/**
 * @inheritDoc
 */
template <typename K, typename V>
map<K, V> filleter_keys(const map<K, V>& input, const vector<K>& keys_to_exclude) {
    map<K, V> result;
    // 除外キーをセットで保持（高速な検索のため）
    set<K> exclude_set(keys_to_exclude.begin(), keys_to_exclude.end());
    for (const auto& pair : input) {
        if (exclude_set.find(pair.first) == exclude_set.end()) {
            result.insert(pair);
        }
    }

    return result;
}

/**
 * @inheritDoc
 */
int get_index_by_name(vector<string> name, string target) {
    for (size_t i = 0; i < name.size(); i++) {
        if (name[i] == target) {
            return i;
        }
    }
    return -1;
}

/**
 * @inheritDoc
 */
bdd get_edge_BDD(int source, int target, twa_graph_ptr automaton) {
    for (auto& edge: automaton->edges()) {
        if (edge.src == source && edge.dst == target) return edge.cond;
    }
    return bddfalse;
}

/**
 * @inheritDoc
 */
bool vector_exists(vector<V> vec, V v) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].label == v.label && vec[i].ce == v.ce) {
            return true;
        }
    }
    return false;
}

/**
 * @inheritDoc
 */
int find_vertex(vector<V> vec, V v) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].label == v.label && vec[i].ce == v.ce) {
            return i;
        }
    }
    return -1;
}

/**
 * @inheritDoc
 */
string replace_commas_with_ampersands(const string& s) {
    string result = s;
    replace(result.begin(), result.end(), ',', '&');
    return result;
}

/**
 * @inheritDoc
 */
vector<string> extract_variables(const string& s) {
    vector<string> result;
    string::size_type start = 0;
    string::size_type end = 0;
    while (end != string::npos) {
        end = s.find_first_of("&", start);
        result.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    return result;
}

/**
 * @inheritDoc
 */
Graph createDCG(twa_graph_ptr& automaton, vector<string> aut_name, vector<string>& dcg_name, CEGraph ce_graph, vector<string> response_events) {
    vector<V> V_list;
    vector<E> E_list;
    vector<V> V_prime_list;
    vector<E> E_prime_list;
    vector<V> V_skip_list;

    auto dict = automaton->get_dict(); 
    V_list.push_back({aut_name[0], 0});
    dcg_name.push_back(aut_name[0] + "0");

    while (compare_V_List(V_list, V_prime_list) || compare_E_List(E_list, E_prime_list)) {
        V_prime_list = V_list;
        E_prime_list = E_list;
        for (auto v: V_prime_list) {
            Head h = head(ce_graph, v.ce);
            string head_string = replace_commas_with_ampersands(h.label);
            vector<bdd> vars;
            vector<formula> head_formulas;
            for (const auto& var: extract_variables(head_string)) {
                formula f = parse_infix_psl(var).f;
                int bdd_index = dict->var_map[f];
                bdd var_b = bdd_ithvar(bdd_index);
                vars.push_back(var_b);
                head_formulas.push_back(f);
            }
            bdd head_bdd = bddtrue;
            for (const auto& var: vars) {
                head_bdd &= var;
            }
            map<formula, int> head_fmap = filleter_keys(dict->var_map, head_formulas);
            for (const auto& pair: head_fmap) {
                // 応答イベントの場合はスキップする
                if (find(response_events.begin(), response_events.end(), str_psl(pair.first)) != response_events.end()) {
                    continue;
                }
                head_bdd &= bdd_nithvar(pair.second);
            }
            // skip listに含まれていたら、処理しない
            if (! vector_exists(V_skip_list, v)) {    
                for (int i = 0; i < automaton->num_states(); i++) {// vから遷移できるnode
                    V new_v = {aut_name[i], tail(ce_graph, v.ce)};
                    V target_v = new_v;
                    // オートマトンvからのi番目の状態に遷移できるか
                    // オートマトンのV(000,,,)からi(0123)へのエッジを取得
                    // b = get_edgeBDD(0, 1, automaton) -> 0から1へのエッジを取得
                    int v_index = get_index_by_name(aut_name, v.label);
                    bdd b = get_edge_BDD(v_index, i, automaton);
                    bdd restricted = bdd_restrict(b, head_bdd);

                    if (! vector_exists(V_list, new_v)) {
                        dcg_name.push_back(new_v.label + to_string(new_v.ce));
                        V_list.push_back(new_v);
                    }
                    if (bddtrue == bdd_satone(restricted)) { // head(z) |= b
                        E_list.push_back({"label", v, new_v});
                    }
                }
            }
            V_skip_list.push_back(v);   
        }
    }

    // DCGの作成
    Graph dcg;
    map<V, int> map;
    for (auto v: V_list) {
        add_vertex(dcg);
    }
    for (auto e: E_list) {
        add_edge(find_vertex(V_list, e.source), find_vertex(V_list, e.target), dcg);
    }

    // --- DEBUG FILE ---
    ofstream file("dcg.dot");
    write_graphviz(file, dcg);

    return dcg;
}

/**
 * @inheritDoc
 */
Mscc searchSCC (Graph& dcg) {
    dynamic_properties dp;
    typedef graph_traits<Graph>::vertex_descriptor Dcg_vertex;
    vector<int> component(num_vertices(dcg)), discover_time(num_vertices(dcg));
    vector<default_color_type> color(num_vertices(dcg));
    vector<Dcg_vertex> root(num_vertices(dcg));
    auto params = (
        root_map(make_iterator_property_map(root.begin(), get(vertex_index, dcg)))
        .color_map(make_iterator_property_map(color.begin(), get(vertex_index, dcg)))
        .discover_time_map(make_iterator_property_map(discover_time.begin(), get(vertex_index, dcg)))
    );
    int num_scc = strong_components(dcg,
        make_iterator_property_map(component.begin(), get(vertex_index, dcg)),
        params
    );

    map<int, vector<int>> mscc_map;
    for (size_t i = 0; i < component.size(); i++) {
        if (is_reachable(0, i, dcg, color.data())) {
            mscc_map[component[i]].push_back(i);
        }
    }

    return {mscc_map, num_scc};
}

/**
 * @inheritDoc
 */
/**


/**
 * @fn find_non_accepting_indices
 * @brief 受理状態でないインデックスを検索する
 * @param mscc 極大強連結成分
 * @param dcg_name DCGの名前
 * @return 受理状態でないインデックス
 */
map<int, string> find_non_accepting_indices(Mscc mscc, vector<string> dcg_name) {
    map<int, string> non_accepting_indices;
    string tmp_index_name = "";
    for(int i = 1; i < dcg_name[0].length(); i++) {
        tmp_index_name += to_string(i);
    }
    for (size_t i = 0; i < mscc.num_scc; i++) {
        non_accepting_indices[i] = tmp_index_name;
    }
    for (auto pair: mscc.component) {
        if (pair.second.size() == 1) continue;
        for (auto v: pair.second) {
            // dcg_name[v]の文字列の何番目に0があるか
            for (size_t j = 0; j < dcg_name[v].length(); j++) {
                if (dcg_name[v][j] == '0') {
                    // 既に消えている場合はスキップ 0があった番目を消す
                    if (non_accepting_indices[pair.first].find(to_string(j+1)) != string::npos) {
                        non_accepting_indices[pair.first].erase(non_accepting_indices[pair.first].find(to_string(j+1)), 1);
                    }
                }
            }
        }
    }

    return non_accepting_indices;
}