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

string replaceCommasWithAmpersands(const string& s) {
    string result = s;
    replace(result.begin(), result.end(), ',', '&');
    return result;
}

vector<string> extractVariables(const string& s) {
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

template <typename K, typename V>
map<K, V> filleterKeys(const map<K, V>& input, const vector<K>& keysToExclude) {
    map<K, V> result;
    // 除外キーをセットで保持（高速な検索のため）
    set<K> excludeSet(keysToExclude.begin(), keysToExclude.end());
    for (const auto& pair : input) {
        if (excludeSet.find(pair.first) == excludeSet.end()) {
            result.insert(pair);
        }
    }
    return result;
}

// edge property
struct EdgeProperty {
    string label;
};
struct VertexProperty {
    string label;
};
typedef boost::adjacency_list<vecS, vecS, directedS, no_property, EdgeProperty> CEGraph;
typedef boost::graph_traits<CEGraph>::edge_descriptor EdgeDescriptor;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;

struct DCG {
    Graph graph;
    map<pair<int, string>, VertexDescriptor> node_map;
};

struct Node_Map {
    string name;
    int ce;
    int index;
};

int countNumCe(const vector<Node_Map>& map, int ce) {
    return count_if(map.begin(), map.end(), [ce](const Node_Map& n) { return n.ce == ce; });
}
int getIndex(const vector<Node_Map>& map, string name, int ce) {
    for (size_t i = 0; i < map.size(); i++) {
        if (map[i].name == name && map[i].ce == ce) {
            return map[i].index;
        }
    }
    return -1;

}


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
struct E {
    string label;
    V source;
    V target;
};

bool vector_exists(vector<V> vec, V v) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].label == v.label && vec[i].ce == v.ce) {
            return true;
        }
    }
    return false;
}

int findVertex(vector<V> vec, V v) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].label == v.label && vec[i].ce == v.ce) {
            return i;
        }
    }
    return -1;
}

bool compareVList(vector<V> left, vector<V> right) {
    if (left.size() != right.size()) {
        return true;
    }
    // for (size_t i = 0; i < left.size(); i++) {
    //     if (left[i].label != right[i].label) {
    //         return true;
    //     }
    // }
    return false;
}
bool compareEList(vector<E> left, vector<E> right) {
    if (left.size() != right.size()) {
        return true;
    }
    // for (size_t i = 0; i < left.size(); i++) {
        // if (left[i].label != right[i].label || left[i].source != right[i].source || left[i].target != right[i].target) {
        //     return false;
        // }
    // }
    return false;
}

typedef graph_traits<CEGraph>::edge_iterator edge_iter;
int tail(CEGraph& g , int head) {
    edge_iter e, eend;
    for (tie(e, eend) = edges(g); e != eend; e++) {
        if (source(*e, g) == head) {
            //cout << "tail: " << target(*e, g) << endl;
            return target(*e, g);
        }
    }
    return -1;
}

struct Head {
    string label;
};

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

bdd getEdgeBDD(int source, int target, twa_graph_ptr automaton) {
    
    for (auto& edge: automaton->edges()) {
        if (edge.src == source && edge.dst == target) return edge.cond;
    }
    return bddfalse;
}

int getIndexByName(vector<string> name, string target) {
    for (size_t i = 0; i < name.size(); i++) {
        if (name[i] == target) {
            return i;
        }
    }
    return -1;
}

Graph createDCG(twa_graph_ptr& automaton, vector<string> aut_name, vector<string>& dcg_name, CEGraph ce_graph, vector<string> responseEvents) {
    vector<V> V_list;
    vector<E> E_list;
    vector<V> V_prime_list;
    vector<E> E_prime_list;
    vector<V> V_skip_list;
    auto dict = automaton->get_dict();
    //cout << "dict: " << endl;
    dict->dump(cout); 

    V_list.push_back({aut_name[0], 0});
    dcg_name.push_back(aut_name[0] + "0");    
    while (compareVList(V_list, V_prime_list) || compareEList(E_list, E_prime_list)) {
        V_prime_list = V_list;
        E_prime_list = E_list;
        for (auto v: V_prime_list) {
            Head h = head(ce_graph, v.ce);
            string head_string = replaceCommasWithAmpersands(h.label);
            vector<bdd> vars;
            vector<formula> head_formulas;
            for (const auto& var: extractVariables(head_string)) {
                //cout << "var: " << var << endl;
                formula f = parse_infix_psl(var).f;
                int bdd_index = dict->var_map[f];
                bdd var_b = bdd_ithvar(bdd_index);
                vars.push_back(var_b);
                head_formulas.push_back(f);
                //cout << "var_b: " << var_b << endl;
            }
            bdd head_bdd = bddtrue;
            for (const auto& var: vars) {
                head_bdd &= var;
            }
            map<formula, int> head_fmap = filleterKeys(dict->var_map, head_formulas);
            for (const auto& pair: head_fmap) {
                // 応答イベントの場合はスキップする
                if (find(responseEvents.begin(), responseEvents.end(), str_psl(pair.first)) != responseEvents.end()) {
                    continue;
                }
                head_bdd &= bdd_nithvar(pair.second);
            }
            //cout << "head_bdd: " << bdd_format_formula(dict,head_bdd) << endl;

            // skip listに含まれていたら、処理しない
            if (! vector_exists(V_skip_list, v)) {    
                for (int i = 0; i < automaton->num_states(); i++) {// vから遷移できるnode

                    V new_v = {aut_name[i], tail(ce_graph, v.ce)};
                    V target_v = new_v;
                    
                    // オートマトンvからのi番目の状態に遷移できるか
                    // オートマトンのV(000,,,)からi(0123)へのエッジを取得
                    // b = get_edgeBDD(0, 1, automaton) -> 0から1へのエッジを取得
                    int v_index = getIndexByName(aut_name, v.label);
                    bdd b = getEdgeBDD(v_index, i, automaton);
                    //cout << "b: " << bdd_format_formula(dict,b) << endl;
                    bdd restricted = bdd_restrict(b, head_bdd);
                    //cout << "satone(edgeを追加するかどうか):" << bdd_satone(restricted) << endl; 
                    //cout << "restricted: " << restricted << endl;

                    if (! vector_exists(V_list, new_v)) {
                        dcg_name.push_back(new_v.label + to_string(new_v.ce));
                        V_list.push_back(new_v);
                    }
                    if (bddtrue == bdd_satone(restricted)) { // todo head(z) |= b
                        //cout << "追加するedge:" << v.label << ","<< v.ce << " -> " << target_v.label << "," << target_v.ce << endl;   
                        E_list.push_back({"label", v, new_v});
                    }
                }
            }
            V_skip_list.push_back(v);   
        }
    }

    // V_list, E_listを標準出力
    // cout << "最後のV_listの出力 " << endl;
    // for (auto v: V_list) {
    //     cout << v.label << ","<< v.ce << endl;
    // }
    // cout << "最後のE_listの出力 " << endl;
    // for (auto e: E_list) {
    //     cout << "edge" << " " << e.source.label << "," << e.source.ce << " -> " << e.target.label << "," << e.target.ce << endl;
    // }

    // DCGの作成
    Graph dcg;
    map<V, int> map;
    for (auto v: V_list) {
        add_vertex(dcg);
    }

    for (auto e: E_list) {
        // cout << "edge:" << findVertex(V_list, e.source)<< " -> " << findVertex(V_list, e.target) << endl;
        add_edge(findVertex(V_list, e.source), findVertex(V_list, e.target), dcg);
    }
    ofstream file("dcg.dot");
    write_graphviz(file, dcg);

    return dcg;
}

struct ParseCounterexample {
    vector<string> events;
    vector<string> w_events;
};
BOOST_FUSION_ADAPT_STRUCT(ParseCounterexample, events, w_events);

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


twa_graph_ptr p(twa_graph_ptr left, twa_graph_ptr right, twa_graph_ptr shared, vector<string>& name) {
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

twa_graph_ptr synchronous_product(vector<twa_graph_ptr> automatons, twa_graph_ptr shared, vector<string>& name) {
    vector<int> state_names;
    twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    producted->copy_ap_of(shared);

    for (size_t i = 0; i < automatons.size(); i++) {
        if (producted->num_states() == 0) {
            producted = automatons[i];
            continue;
        }
        producted = p(producted, automatons[i], shared, name);
        ofstream totyu("途中"+to_string(i)+".dot");
        print_dot(totyu, producted);
    }
    return producted;
}

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

    ofstream cegfile("ceg.dot");
    write_graphviz(cegfile, graph, default_writer(), make_label_writer(get(&EdgeProperty::label, graph)));

    return graph;
}

struct Mscc {
    map<int, vector<int>> component;
    int num_scc;
};

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

    // cout << "num_scc: " << num_scc << endl;
    // for (int i = 0; i != component.size(); i++) {
    //     cout << "vertex: " << i << " is in component " << component[i] << endl;
    // }

    map<int, vector<int>> mscc_map;
    for (size_t i = 0; i < component.size(); i++) {
        if (is_reachable(0, i, dcg, color.data())) {
            mscc_map[component[i]].push_back(i);
        }
    }

    return {mscc_map, num_scc};
}

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
        cout << "component: " << pair.first << endl;
        if (pair.second.size() == 1) continue;
        for (auto v: pair.second) {
            cout << v << endl;
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

    // non_accepting_indicesの出力
    cout << "non_accepting_indices: " << endl;
    for (auto pair: non_accepting_indices) {
        cout << pair.second << endl;
    }

    return non_accepting_indices;
}

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

int main() {
    // LTL式の入力
    vector<string> ltl_formula_str_list = {
        "G(x1 -> F(y))",
        "G(x2 -> !y)",
        "G((x3 & y) -> (y U x2))"
    };
    string ltl_formula_str = std::accumulate(
        ltl_formula_str_list.begin(), ltl_formula_str_list.end(), string(),
        [](const string& a, const string& b) {
            return a.empty() ? b : a + " & " + b;
        }
    );
    // 共有BDD辞書の作成
    formula f = parse_infix_psl(ltl_formula_str).f;
    translator trans;
    auto shared = trans.run(f);
    auto dict = shared->get_dict();

    vector<string> responseEvents = {"y"};
    string counterexample = "{x2}{x2,x3}({x1,x2}{x2,x3})";
    CounterexampleGrammar<string::iterator> grammar;
    ParseCounterexample counterexample_parsed;
    auto iter = counterexample.begin();
    auto end = counterexample.end();
    bool success = spirit::qi::phrase_parse(iter, end, grammar, spirit::qi::space, counterexample_parsed);
    if (success) {
        for (auto e: counterexample_parsed.events) {
            cout << "e: " << e << endl;
        }
        for (auto w: counterexample_parsed.w_events) {
            cout << "w: " << w << endl;
        }
    } else {
        cout << "Parsing failed." << endl;
    }

    vector<twa_graph_ptr> automaton_list = {};
    cout << "入力 LTL : " << endl;
    for (size_t i = 0; i < ltl_formula_str_list.size(); i++) {
        parsed_formula parsed = parse_infix_psl(ltl_formula_str_list[i]);
        formula formula = parsed.f;
        cout << str_psl(formula) << endl;

        translator translator(dict);
        twa_graph_ptr automaton = translator.run(formula);
        // オートマトンの各状態からある状態にtrueで遷移できるようにし、その状態からその状態にtrueの遷移を追加する
        unsigned newState = automaton->new_state();
        for (size_t i = 0; i < automaton->num_states(); i++) {
            automaton->new_edge(i, newState, bddtrue);
        }

        automaton_list.push_back(automaton);
        // hoa形式をファイルに出力する
        //ofstream ofs(ltl_formula_str_list[i] + "_automaton.hoa");
        // print_hoa(ofs, automaton);
        ofstream f("automaton" + to_string(i) + ".dot");
        print_dot(f, automaton);
        //ofs.close();
        f.close();


    }
    cout << "応答イベント : " << endl;
    for (auto res: responseEvents) {
        cout << res << endl;
    }

    // オートマトンの同期積合成
    cout << "同期積合成を開始します" << endl;
    twa_graph_ptr automaton_producted = make_twa_graph(dict);
    vector<string> aut_name;
    automaton_producted = synchronous_product(automaton_list, shared, aut_name);
    cout << "同期積合成が完了しました" << endl;
    ofstream producted_ofs("producted.dot");
    print_dot(producted_ofs, automaton_producted);
    producted_ofs.close();

    // デバッグ用
    // cout << "====================DEBUG====================" << endl;
    // cout<< "autmaton_name" << endl;
    // for (size_t i = 0; i < name.size(); i++) {
    //     cout << i << " : " << name[i] << endl;
    // }
    // automaton_producted->get_dict()->dump(cout);
    // print_hoa(cout, automaton_producted);

    // 命題変数を要求イベントだけに制限
    auto p_dict = automaton_producted->get_dict();
    for (string res: responseEvents) {
        parsed_formula res_parsed = parse_infix_psl(res);
        int res_BDD_index = dict->var_map[res_parsed.f];
        for (auto& t: automaton_producted->edges()) {
            bdd res_BDD = bdd_ithvar(res_BDD_index);
            t.cond = bdd_exist(t.cond, res_BDD);
        }
    }    
    // デバッグ用
    // cout << "====================DEBUG====================" << endl;
    // print_hoa(cout, automaton_producted);
    ofstream projected("projected.dot");
    print_dot(projected, automaton_producted);
    projected.close();

    // 反例をグラフにする
    CEGraph ceg;
    ceg = createCounterExampleGraph(ceg, counterexample_parsed);  

    // 有向閉路グラフ(DCG)の作成
    vector<string> dcg_name;
    Graph dcg = createDCG(automaton_producted, aut_name, dcg_name, ceg, responseEvents);
    cout << "DCGの作成が完了しました" << endl;
    
    // debug log
    // cout << "dcg_name: " << endl;
    // for (size_t i = 0; i < dcg_name.size(); i++) {
    //     cout << i << " : " << dcg_name[i] << endl;
    // }

    // 極大強連結成分の探索
    cout << "極大強連結成分の探索を開始します" << endl;
    Mscc mscc = searchSCC(dcg);

    // 受理条件を満たしていないインデックスの導出
    auto non_accepting_indices = find_non_accepting_indices(mscc, dcg_name);

    // LTL式の数
    int num_of_ltl = dcg_name[0].length() - 1;
    // 強充足不能コアの導出
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
