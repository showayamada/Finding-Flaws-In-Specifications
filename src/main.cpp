#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/tl/formula.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/minimize.hh>
#include <spot/twaalgos/complement.hh>
#include <spot/twa/twa.hh>
#include <spot/twa/bdddict.hh>
#include <spot/twa/formula2bdd.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twa/twagraph.hh>
#include <spot/misc/bddlt.hh>
#include <spot/twaalgos/copy.hh>
#include <spot/graph/graph.hh>
#include <spot/graph/ngraph.hh>
#include <spot/parseaut/public.hh>
#include <spot/gen/automata.hh>
#include <spot/gen/formulas.hh>
#include <spot/ta/ta.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twaalgos/hoa.hh>
#include <bddx.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <map>
#include <spot/twaalgos/toparity.hh>
#include <spot/twaalgos/totgba.hh>
#include <spot/twaalgos/remfin.hh>
#include <spot/parseaut/public.hh>
#include <spot/tl/defaultenv.hh>
#include <spot/tl/environment.hh>
#include <spot/twaalgos/sccinfo.hh>
#include <spot/twa/twaproduct.hh>
#include <numeric>
#include <spot/twaalgos/product.hh>
#include <boost/array.hpp>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <unordered_map>
#include <boost/utility.hpp>
#include <boost/foreach.hpp>
#include <queue>
#include <boost/config.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace std;
using namespace spot;
using namespace boost;

/**
 * @fn replace_commas_with_ampersands
 * @brief ,を&に置換する
 * @param s 置換対象の文字列
 * @return 置換後の文字列
 */
string replace_commas_with_ampersands(const string& s) {
    string result = s;
    replace(result.begin(), result.end(), ',', '&');
    return result;
}

/**
 * @fn extract_variables
 * @brief 変数を抽出する
 * @param s 抽出対象の文字列
 * @return vector<string> 抽出した変数のリスト
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
 * @fn filleter_keys
 * @brief キーを除外する
 * @param input 入力
 * @param keys_to_exclude 除外するキー
 * @return 除外後のマップ
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

//! edge property
struct EdgeProperty {
    string label;
};
//! vertex property
struct VertexProperty {
    string label;
};

//! 反例のグラフ
typedef boost::adjacency_list<vecS, vecS, directedS, no_property, EdgeProperty> CEGraph;
typedef boost::graph_traits<CEGraph>::edge_descriptor EdgeDescriptor;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;

/**
 * @struct DCG
 * @brief DCG
 */
struct DCG {
    Graph graph;
    map<pair<int, string>, VertexDescriptor> node_map;
};

struct Node_Map {
    string name;
    int ce;
    int index;
};

/**
 * @struct V
 * @brief 頂点
 */
struct V {
    string label;
    int ce;
    bool operator<(const V& right) const {
        if (ce < right.ce) {
            return true;
        }
        if (ce > right.ce) {
            return false;
        }
        if (label.length() < right.label.length()) {
            return true;
        }
        if (label.length() > right.label.length()) {
            return false;
        }
        return false;
    }
};

/**
 * @struct E
 * @brief エッジ
 */
struct E {
    string label;
    V source;
    V target;
};

/**
 * @fn vector_exists
 * @brief ベクトルに要素が存在するかどうかを判定する
 * @param vec ベクトル
 * @param v 要素
 * @return 存在する場合はtrue
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
 * @fn find_vertex
 * @brief 頂点を検索する
 * @param vec 頂点リスト
 * @param v 頂点
 * @return 頂点が見つかった場合はインデックス、見つからなかった場合は-1
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
 * @fn compare_V_List
 * @brief Vリストを比較する
 * @param left 左辺
 * @param right 右辺
 * @return 左辺と右辺が異なる場合はtrue
 */
bool compare_V_List(vector<V> left, vector<V> right) {
    if (left.size() != right.size()) {
        return true;
    }
    return false;
}

/**
 * @fn compare_E_List
 * @brief Eリストを比較する
 * @param left 左辺
 * @param right 右辺
 * @return 左辺と右辺が異なる場合はtrue
 */
bool compare_E_List(vector<E> left, vector<E> right) {
    if (left.size() != right.size()) {
        return true;
    }
    return false;
}

typedef graph_traits<CEGraph>::edge_iterator edge_iter;

/**
 * @fn tail
 * @brief 反例の第二要素以降の列を返す
 * @param g グラフ
 * @param head ヘッド
 * @return tail
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
 * @struct Head
 * @brief 反例の第一要素
 */
struct Head {
    //! BDD
    string label;
};

/**
 * @fn head
 * @brief 反例の第一要素を取得する
 * @param g 反例のグラフ
 * @param pointer ポインタ
 * @return 反例の第一要素
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
 * @fn get_edge_BDD
 * @brief エッジのBDDを取得する
 * @param source ソース
 * @param target ターゲット
 * @param automaton オートマトン
 * @return BDD
 */
bdd get_edge_BDD(int source, int target, twa_graph_ptr automaton) {
    for (auto& edge: automaton->edges()) {
        if (edge.src == source && edge.dst == target) return edge.cond;
    }
    return bddfalse;
}

/** 
 * @fn get_index_by_name
 * @brief 名前からインデックスを取得する
 * @param name 名前
 * @param target ターゲット
 * @return インデックス
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
 * @fn createDCG
 * @brief DCGを作成する
 * @param automaton オートマトン
 * @param aut_name オートマトン各ノードの名前
 * @param dcg_name DCG各ノードの名前
 * @param ce_graph 反例のグラフ
 * @param response_events 応答イベント
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
 * @struct ParseCounterexample
 * @brief 解析済みの反例
 * @param events イベントのリスト
 * @param w_events ωイベントのリスト
 */
struct ParseCounterexample {
    vector<string> events;
    vector<string> w_events;
};
BOOST_FUSION_ADAPT_STRUCT(ParseCounterexample, events, w_events);

/**
 * @struct CounterexampleGrammar
 * @brief 反例のパーサ
 */
template <typename Iterator>
struct CounterexampleGrammar : spirit::qi::grammar<Iterator, ParseCounterexample(), spirit::qi::space_type> {
    CounterexampleGrammar() : CounterexampleGrammar::base_type(expr) {
        using spirit::qi::lit;
        using spirit::qi::char_;
        using spirit::qi::lexeme;
        using spirit::qi::alpha;
        using spirit::qi::alnum;
        using spirit::qi::space;
        using spirit::qi::repeat;
        using spirit::qi::int_;
        using spirit::qi::attr;
        using spirit::qi::eps;
        expr = events >> w_events;        
        single_expr = '{' >> lexeme[(char_("a-zA-Z") >> *char_("a-zA-Z0-9"))] >> *lexeme[(char_(','))] >> *lexeme[(char_("a-zA-Z") >> *char_("a-zA-Z0-9"))] >> '}';
        w_events = '(' >> +single_expr >> ')';
        events %= +single_expr;
    }
    spirit::qi::rule<Iterator, ParseCounterexample(), spirit::qi::space_type> expr;
    spirit::qi::rule<Iterator, vector<string>(), spirit::qi::space_type> events;
    spirit::qi::rule<Iterator, vector<string>(), spirit::qi::space_type> w_events;
    spirit::qi::rule<Iterator, string(), spirit::qi::space_type> single_expr;
};

/**
 * @fn product_aut
 * @brief 2つのオートマトンの同期積を取る
 * @param left オートマトン
 * @param right オートマトン
 * @param shared 共有オートマトン
 * @param name 状態名のリスト
 * @return 積を取ったオートマトン
 */
twa_graph_ptr product_aut(twa_graph_ptr left, twa_graph_ptr right, twa_graph_ptr shared, vector<string>& name) {
    twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    producted->copy_ap_of(shared);
    vector<string> state_names;
    if (0 == name.size()) {
        for (size_t i = 0; i < left->num_states(); i++) {
            for (size_t j = 0; j < right->num_states(); j++) {
                unsigned new_state = producted->new_state();
                string nname = to_string(i) + to_string(j);
                state_names.push_back(nname);
                name.push_back(nname);
            }
        }
    } else {
        vector<string> tmp;
        for (size_t i = 0; i < left->num_states(); i++) {
            for (size_t j = 0; j < right->num_states(); j++) {
                unsigned new_state = producted->new_state();
                state_names.push_back(to_string(i) + to_string(j));
                tmp.push_back(name[i]+to_string(j));
            }
        }
        name = tmp;
    }

    for (size_t i = 0; i < state_names.size(); i++) {
        int l_state_src = stoi(to_string(state_names[i][0]));
        int r_state_src = stoi(to_string(state_names[i][1]));
        for (size_t j = 0; j < state_names.size(); j++) {
            int l_state_dst = stoi(to_string(state_names[j][0]));
            int r_state_dst = stoi(to_string(state_names[j][1]));
            for (auto l_edge: left->out(l_state_src)) {
                for (auto r_edge: right->out(r_state_src)) {
                    if (l_edge.dst == l_state_dst && r_edge.dst == r_state_dst) {
                        bdd cond = bdd_and(l_edge.cond, r_edge.cond);
                        producted->new_edge(i, j, cond);
                    }
                }
            }
        }
    }

    return producted;
}

/**
 * @fn synchronous_product
 * @brief 複数の同期積を合成する
 * @param automatons オートマトンのリスト
 * @param shared 共有オートマトン
 * @param name 同期積合成したオートマトンの状態名のリスト
 * @return 合成したオートマトン
 */
twa_graph_ptr synchronous_product(vector<twa_graph_ptr> automatons, twa_graph_ptr shared, vector<string>& name) {
    vector<int> state_names;
    twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    producted->copy_ap_of(shared);

    for (size_t i = 0; i < automatons.size(); i++) {
        if (producted->num_states() == 0) {
            producted = automatons[i];
            continue;
        }
        producted = product_aut(producted, automatons[i], shared, name);
        // --- DEBUG FILE ---
        ofstream processing("processing"+to_string(i)+".dot");
        print_dot(processing, producted);
    }

    return producted;
}

/**
 * @fn createCounterExampleGraph
 * @brief 反例のグラフを作成する
 * @param graph グラフ
 * @param counterexample_parsed 解析済みの反例
 * @return 反例のグラフ
 */
CEGraph createCounterExampleGraph(CEGraph& graph, ParseCounterexample& counterexample_parsed) {
    vector<size_t> vertex_map;
    size_t num_of_ce_node = counterexample_parsed.events.size() + counterexample_parsed.w_events.size();
    for (size_t i = 0; i < num_of_ce_node; i++) {
        vertex_map.push_back(add_vertex(graph));
    }
    for (size_t i = 0; i < vertex_map.size()-1; i++) {
        EdgeDescriptor e;
        if (i < counterexample_parsed.events.size()) {
            e = add_edge(vertex_map[i], vertex_map[i+1], graph).first;
            graph[e].label = counterexample_parsed.events[i];
        } else {
            e = add_edge(vertex_map[i], vertex_map[i+1], graph).first;
            graph[e].label = counterexample_parsed.w_events[i-counterexample_parsed.events.size()];
        }
    }
    EdgeDescriptor e;
    e = add_edge(vertex_map[vertex_map.size()-1], vertex_map[counterexample_parsed.events.size()], graph).first;
    graph[e].label = counterexample_parsed.w_events[counterexample_parsed.w_events.size()-1];

    // --- DEBUG FILE ---
    ofstream cegfile("ceg.dot");
    write_graphviz(cegfile, graph, default_writer(), make_label_writer(get(&EdgeProperty::label, graph)));

    return graph;
}

/**
 * @struct Mscc
 * @brief 極大強連結成分
 * @param component 極大強連結成分
 * @param num_scc 極大強連結成分の数
 */
struct Mscc {
    map<int, vector<int>> component;
    int num_scc;
};

/**
 * @fn searchSCC
 * @brief 強連結成分を検索する
 * @param dcg DCG
 * @return 極大強連結成分
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

/**
 * @fn get_strongly_unsatisfiable_core
 * @brief 充足不能コアを取得する
 * @param non_accepting_indices 受理状態でないインデックス
 * @param n インデックスの数
 * @return 充足不能コア
 */
vector<vector<int>> get_strongly_unsatisfiable_core(map<int, string> non_accepting_indices, int n) {    
    vector<vector<int>> core;
    vector<int> all;
    for (int i = 1; i <= n; i++) {
        all.push_back(i);
    }
    vector<vector<int>> subsets;
    for (int i = 0; i < (1 << n); i++) {
        vector<int> subset;
        for (int j = 0; j < n; j++) {
            if (i & (1 << j)) {
                subset.push_back(all[j]);
            }
        }
        subsets.push_back(subset);
    }

    for (auto subset: subsets) {
        bool is_included = true;
        for (auto pair: non_accepting_indices) {
            string non_accepting = pair.second;
            bool flg = false;
            for (auto s: subset) {
                if (non_accepting.find(to_string(s)) != string::npos) {
                    flg = true;
                }
            }
            is_included = flg;
            if (!is_included) {
                break;
            }
        }
        vector<int> core_subset;
        if (is_included) {
            for (auto s: subset) {
                core_subset.push_back(s);
            }
            core.push_back(core_subset);
        }
    }

    return core;    
}

/**
 * @fn
 * @brief LTL式を解析し、オートマトンを生成する
 * @param ltl_formula_str_list LTL式のリスト(string[])
 * @return twa_graph_ptr 全てのLTL式を&で結合して生成したオートマトン
 */
twa_graph_ptr make_shared_dict(vector<string> ltl_formula_str_list) {
    string ltl_formula_str = std::accumulate(
        ltl_formula_str_list.begin(), ltl_formula_str_list.end(), string(),
        [](const string& a, const string& b) {
            return a.empty() ? b : a + " & " + b;
        }
    );

    formula f = parse_infix_psl(ltl_formula_str).f;
    translator trans;

    return trans.run(f);
}

/**
 * @fn
 * @brief 複数LTL式を解析し、それぞれオートマトンを生成する
 * @param ltl_formula_str_list LTL式のリスト(string[])
 * @param dict 共有BDD辞書
 * @return vector<twa_graph_ptr> LTL式を解析して生成したオートマトンのリスト
 */
vector<twa_graph_ptr> ltl_list_to_automaton(vector<string> ltl_formula_str_list, bdd_dict_ptr dict) {
    vector<twa_graph_ptr> automaton_list;
    for (size_t i = 0; i < ltl_formula_str_list.size(); i++) {
        parsed_formula parsed = parse_infix_psl(ltl_formula_str_list[i]);
        formula formula = parsed.f;
        translator translator(dict);
        twa_graph_ptr automaton = translator.run(formula);
        // オートマトンの各状態からある状態にtrueで遷移できるようにし、その状態からその状態にtrueの遷移を追加する
        unsigned newState = automaton->new_state();
        for (size_t i = 0; i < automaton->num_states(); i++) {
            automaton->new_edge(i, newState, bddtrue);
        }
        automaton_list.push_back(automaton);

        // --- DEBUG FILE ---
        ofstream f("automaton" + to_string(i) + ".dot");
        print_dot(f, automaton);
        f.close();
    }

    return automaton_list;
}

/**
 * @fn projection_of_request_events
 * @brief 要求イベントのみを含むオートマトンを生成する
 * @param automaton_producted 同期積合成されたオートマトン
 * @param dict 共有BDD辞書
 * @param response_events 応答イベントのリスト
 * @return twa_graph_ptr 要求イベントのみを含むオートマトン
 */
twa_graph_ptr projection_of_request_events(twa_graph_ptr automaton_producted, bdd_dict_ptr dict, vector<string> response_events) {
    auto p_dict = automaton_producted->get_dict();
    for (string res: response_events) {
        parsed_formula res_parsed = parse_infix_psl(res);
        int res_BDD_index = dict->var_map[res_parsed.f];
        for (auto& t: automaton_producted->edges()) {
            bdd res_BDD = bdd_ithvar(res_BDD_index);
            t.cond = bdd_exist(t.cond, res_BDD);
        }
    }
    // --- DEBUG FILE ---
    ofstream projected("projected.dot");
    print_dot(projected, automaton_producted);
    projected.close();

    return automaton_producted;
}

int main() {
    //! LTL式
    vector<string> ltl_formula_str_list = {
        "G(x1 -> F(y))",
        "G(x2 -> !y)",
        "G((x3 & y) -> (y U x2))"
    };

    //! 応答イベント
    vector<string> response_events = {"y"};
    //! 反例
    string counterexample = "{x2}{x2,x3}({x1,x2}{x2,x3})";

    // --- DEBUG LOG ---
    cout << "LTL formulas: " << endl;
    for (auto ltl_formula_str: ltl_formula_str_list) {
        cout << ltl_formula_str << endl;
    }
    cout << "Response Events:" << endl;
    for (auto responseEvent: response_events) {
        cout << responseEvent << endl;
    }

    //! sharedオートマトン
    twa_graph_ptr shared = make_shared_dict(ltl_formula_str_list);
    //! 共有BDD辞書
    bdd_dict_ptr dict = shared->get_dict(); 

    // 反例の解析

    //! 解析済みの反例
    ParseCounterexample counterexample_parsed;
    CounterexampleGrammar<string::iterator> grammar;
    bool success = spirit::qi::phrase_parse(
        counterexample.begin(),
        counterexample.end(),
        grammar,
        spirit::qi::space,
        counterexample_parsed
    );

    // --- DEBUG LOG ---
    if (success) {
        cout << "Parsing successful! : " << endl;
        for (auto e: counterexample_parsed.events) {
            cout << "event: " << e << endl;
        }
        for (auto w: counterexample_parsed.w_events) {
            cout << "w_event: " << w << endl;
        }
    } else {
        cout << "Parsing failed." << endl;
    }

    // それぞれのLTL式からオートマトンを生成
    vector<twa_graph_ptr> automaton_list = ltl_list_to_automaton(ltl_formula_str_list, dict);

    // オートマトンの同期積合成

    //! 同期積合成されたオートマトン 
    twa_graph_ptr automaton_producted = make_twa_graph(dict);
    //! 同期積合成されたオートマトンの各状態の名前
    vector<string> aut_name;
    cout << "Starting synchronous producting." << endl;
    automaton_producted = synchronous_product(automaton_list, shared, aut_name);
    cout << "Producting successful!" << endl;
    
    // --- DEBUG FILE ---
    ofstream producted_ofs("producted.dot");
    print_dot(producted_ofs, automaton_producted);
    producted_ofs.close();

    // --- DEBUG LOG ---
    // cout<< "autmaton_name" << endl;
    // for (size_t i = 0; i < name.size(); i++) {
    //     cout << i << " : " << name[i] << endl;
    // }
    // automaton_producted->get_dict()->dump(cout);
    // cout << "producted" << endl;
    // print_dot(cout, automaton_producted);

    // 命題変数を要求イベントだけに制限
    cout << "Creating projected automaton." << endl;
    //! 要求イベントのみを含むオートマトン
    twa_graph_ptr automaton_projected = projection_of_request_events(automaton_producted, dict, response_events);
    cout << "Creating projected automaton successful!" << endl;

    // 反例をグラフにする
    cout << "Creating counterexample graph." << endl;
    //! 反例のグラフ
    CEGraph ceg;
    ceg = createCounterExampleGraph(ceg, counterexample_parsed);
    cout << "Creating counterexample graph successful!" << endl;

    // 有向閉路グラフ(DCG)の作成
    cout << "Creating DCG." << endl;
    //! DCGのノードの名前
    vector<string> dcg_name;
    //! DCG
    Graph dcg = createDCG(automaton_projected, aut_name, dcg_name, ceg, response_events);
    cout << "Creating DCG successful!" << endl;

    // --- DEBUG LOG ---    
    // cout << "dcg_name: " << endl;
    // for (size_t i = 0; i < dcg_name.size(); i++) {
    //     cout << i << " : " << dcg_name[i] << endl;
    // }

    // 極大強連結成分の探索
    cout << "Starting search of maximal strongly connected components." << endl;
    //! 極大強連結成分
    Mscc mscc = searchSCC(dcg);
    cout << "Search of maximal strongly connected components successful!" << endl;

    // 受理条件を満たしていないインデックスの導出
    auto non_accepting_indices = find_non_accepting_indices(mscc, dcg_name);

    // LTL式の数
    int num_of_ltl = dcg_name[0].length() - 1;
    //! 強充足不能コア
    vector<vector<int>> core = get_strongly_unsatisfiable_core(non_accepting_indices, num_of_ltl);
    cout << "強充足不能コア: " << endl;
    for (auto c: core) {
        for (auto i: c) {
            cout << i << " ";
        }
        cout << endl;
    }
    // 強充足不能コアのsizeの一番小さいものを取得
    vector<int> min_core = core[0];
    for (auto c: core) {
        if (c.size() < min_core.size()) {
            min_core = c;
        }
    }
    cout << "極小の強充足不能コア: " << endl;
    for (auto i: min_core) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
