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

using namespace std;
using namespace spot;
using namespace boost;

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

void createDCG(twa_graph_ptr& automaton, vector<string> aut_name ,CEGraph ce_graph) {
    vector<V> V_list;
    vector<E> E_list;
    vector<V> V_prime_list;
    vector<E> E_prime_list;
    vector<V> V_skip_list;
    cout << "name:" << endl;
    for (size_t i = 0; i < aut_name.size(); i++) {
        cout << aut_name[i] << endl;
    }
    V_list.push_back({aut_name[0], 0});
    int c = 0;
    while (compareVList(V_list, V_prime_list) || compareEList(E_list, E_prime_list)) {
        cout << "whileはじまるよ"<< endl;
        V_prime_list = V_list;
        E_prime_list = E_list;
        cout << "V_prime_listのサイズ " << V_prime_list.size() << endl;
        cout << "E_prime_listのサイズ " << E_prime_list.size() << endl;
        cout << "V_listの出力 " << endl;
        for (auto v: V_list) {
            cout << v.label << "," << v.ce << endl;
        }
        cout << "V_skip_listの出力 " << endl;
        for (auto v: V_skip_list) {
            cout << v.label << "," << v.ce << endl;
        }
        for (auto v: V_prime_list) {
            cout << "==========" << endl;
            cout << "v: " << v.label << "," << v.ce << endl;
            if (vector_exists(V_skip_list, v)) {
                cout << "skipリストにあるよ" << endl;
            }

            // skip listに含まれていたら、処理しない
            if (! vector_exists(V_skip_list, v)) {    
                for (int i = 0; i < automaton->num_states(); i++) {// vから遷移できるnode
                    if (true) { // todo head(z) |= b
                        V new_v = {aut_name[i], tail(ce_graph, v.ce)};
                        if (! vector_exists(V_list, new_v)) V_list.push_back(new_v);
                        cout << "追加するedge:" << v.label << ","<< v.ce << " -> " << new_v.label << "," << new_v.ce << endl;   
                        E_list.push_back({"label", v, new_v});
                    }
                }
            }
            V_skip_list.push_back(v);   
        }
        cout << "v_listのサイズ " << V_list.size() << endl;
        cout << "e_listのサイズ " << E_list.size() << endl;

        cout << "whileおわるよ" << endl;
    }

    // V_list, E_listを標準出力
    cout << "最後のV_listの出力 " << endl;
    for (auto v: V_list) {
        cout << v.label << ","<< v.ce << endl;
    }
    cout << "最後のE_listの出力 " << endl;
    for (auto e: E_list) {
        cout << "edge" << " " << e.source.label << "," << e.source.ce << " -> " << e.target.label << "," << e.target.ce << endl;
    }

    // DCGの作成
    Graph dcg;
    map<V, int> map;
    int index = 0;
    for (auto v: V_list) {
        //map[v] = index;
        add_vertex(dcg);
        index++;
    }
    for (auto e: E_list) {
        //add_edge(map[e.source], map[e.target], dcg);
    }
    ofstream file("dcg.dot");
    //write_graphviz(file, dcg);

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


struct Head {
    string label;
    int index;
};

Head head(CEGraph& g, int tail) {
    int next;
    edge_iter e, eend;
    for (tie(e, eend) = edges(g); e != eend; e++) {
        if (source(*e, g) == tail) {
            next = target(*e, g);
        }
    }
    for (tie(e, eend) = edges(g); e != eend; e++) {
        if (source(*e, g) == next) {
            EdgeProperty ceg = g[*e];
            cout << "label: " << ceg.label << endl;
            Head head = {ceg.label, next};
            return head;
        }
    }
    return {"", -1};
}

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
            for (size_t j = 0; j < left->num_states(); j++) {
                unsigned new_state = producted->new_state();
                state_names.push_back(to_string(i) + to_string(j));
                tmp.push_back(name[i]+to_string(j));
            }
        }
        name = tmp;
    }

    for (size_t i = 0; i < state_names.size(); i++) {
        int l_state_src = int(state_names[i][0]-'0');
        int r_state_src = int(state_names[i][1]-'0');
        for (size_t j = 0; j < state_names.size(); j++) {
            int l_state_dst = int(state_names[j][0]-'0');
            int r_state_dst = int(state_names[j][1]-'0');
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
    }
    return producted;
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
        automaton_list.push_back(automaton);
        // hoa形式をファイルに出力する
        ofstream ofs(ltl_formula_str_list[i] + "_automaton.hoa");
        print_hoa(ofs, automaton);
        ofs.close();
    }
    cout << "応答イベント : " << endl;
    for (auto res: responseEvents) {
        cout << res << endl;
    }

    // オートマトンの同期積合成
    cout << "同期積合成を開始します" << endl;
    twa_graph_ptr automaton_producted = make_twa_graph(dict);
    vector<string> name;
    automaton_producted = synchronous_product(automaton_list, shared, name);
    cout << "同期積合成が完了しました" << endl;
    ofstream producted_ofs("producted.hoa");
    print_hoa(producted_ofs, automaton_producted);
    producted_ofs.close();

    // デバッグ用
    // cout << "====================DEBUG====================" << endl;
    // cout<< "なまえ" << endl;
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
    cout << "====================DEBUG====================" << endl;
    print_hoa(cout, automaton_producted);
    ofstream projected("projected.hoa");
    print_hoa(projected, automaton_producted);
    projected.close();

    // 反例をグラフにする
    CEGraph ceg;
    vector<size_t> vertex_map;
    size_t num_of_ce_node = counterexample_parsed.events.size() + counterexample_parsed.w_events.size();
    for (size_t i = 0; i < num_of_ce_node; i++) {
        vertex_map.push_back(add_vertex(ceg));
    }
    for (size_t i = 0; i < vertex_map.size()-1; i++) {
        EdgeDescriptor e;
        if (i < counterexample_parsed.events.size()) {
            e = add_edge(vertex_map[i], vertex_map[i+1], ceg).first;
            ceg[e].label = counterexample_parsed.events[i];
        } else {
            e = add_edge(vertex_map[i], vertex_map[i+1], ceg).first;
            ceg[e].label = counterexample_parsed.w_events[i-counterexample_parsed.events.size()];
        }
    }
    EdgeDescriptor e;
    e = add_edge(vertex_map[vertex_map.size()-1], vertex_map[counterexample_parsed.events.size()], ceg).first;
    ceg[e].label = counterexample_parsed.w_events[counterexample_parsed.w_events.size()-1];

    ofstream cegfile("ceg.dot");
    write_graphviz(cegfile, ceg, default_writer(), make_label_writer(get(&EdgeProperty::label, ceg)));
    

    // 有向閉路グラフ(DCG)の作成
    Graph dcg;

    createDCG(automaton_producted, name, ceg);

    

    // vector<Node_Map> dcg_node_map;
    // auto v = add_vertex(dcg);
    // string initial_node = name[0]+to_string(0);
    // dcg[v].label = initial_node; // 000, 0
    // Node_Map initial_node_map = {name[0], 0, v};
    // dcg_node_map.push_back(initial_node_map);
    // cout << "num_of_ce_node: " << num_of_ce_node << endl;
    // int tail_num = 0;
    // for (size_t i = 0; i < num_of_ce_node; i++) { // {x2}{x2,x3}({x1,x2}{x2,x3})
    //     for (size_t j = 0; j < automaton_producted->num_states(); j++) { // 000, 001, 100, 101
    //         Head h = head(ceg, tail_num);
    //         for () {// state j をsourceとするエッジをループ
    //             tail_num = tail(ceg, h.index);
    //             if (true) { // todo: head(ceg, i) |= b
    //                 if (true) { // ノードが作成されてなかったら作る
    //                     auto v = add_vertex(dcg);
    //                     dcg[v].label = name[k]+to_string(tail_num);
    //                     Node_Map node_map = {name[k], tail_num, v};
    //                     dcg_node_map.push_back(node_map);
    //                 }
    //                 add_edge(h.index, getIndex(dcg_node_map, name[k],tail(ceg,i)), dcg);
    //             }
    //         }
    //     }
    // }

    //ofstream dcgfile("dcg.dot");
    //write_graphviz(dcgfile, dcg, make_label_writer(get(&VertexProperty::label, dcg)));

    // VertexMap vertex_map;



    // map<int, Graph::vertex_descriptor> desc;
    // auto V = automaton_producted->states();
    // vector<spot::internal::distate_storage<unsigned int, spot::internal::boxed_label<spot::twa_graph_state>>> V_prime;
    // twa_graph::edge_storage_t E;
    // twa_graph::edge_storage_t E_prime; 
    // while(V != V_prime || E != E_prime) {
    //     V_prime = V;
    //     E_prime = E;
    //     for (size_t i = 0; i < automaton_producted->states().size(); i++) {
    //         for (auto& t: automaton_producted->out(i)) {
    //             string src = to_string(i);
    //             string dst= to_string(t.dst);
    //             if (head(i)|= b) {
    //                 // 新しいノードとエッジを追加
    //                 desc[i] = add_vertex(dcg); 
    //                 add_edge()
    //             }
    //         }
    //     }

    // }

    ofstream dcg_ofs("projected.dot");
    print_dot(dcg_ofs, automaton_producted);

    return 0;
}
