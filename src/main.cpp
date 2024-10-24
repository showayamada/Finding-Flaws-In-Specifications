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

using namespace std;
using namespace spot;

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

    // if (left->num_states() == 0) {
    //     return right;
    // }
    // if (right->num_states() == 0) {
    //     return left;
    // }
    // twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    // producted->copy_ap_of(shared);
    // auto namer = producted->create_namer<string>();

    // // 同期積の状態を作成
    // for (size_t left_state = 0; left_state < left->num_states(); left_state++) {
    //     for (size_t right_state = 0; right_state < right->num_states(); right_state++) {
    //         unsigned new_state = producted->new_state();
    //         string name = to_string(left_state) + to_string(right_state);
    //         cout << "name: " << name << endl;
    //         cout << namer->alias_state(new_state, name) << endl;
    //     }
    // }
    // auto namer_producted = producted->create_namer<string>();
    // print_hoa(cout, producted);

    // namer_producted->get_name(45);

    // for (auto left_edge: left->edges()) {
    //     for (auto right_edge: right->edges()) {
    //         if (left_edge.src == right_edge.src && left_edge.dst == right_edge.dst) {
    //             bdd cond = bdd_and(left_edge.cond, right_edge.cond);
    //             producted->new_edge(left_edge.src, left_edge.dst, cond);
    //         }
    //     }
    // }

    // unsigned pre_l;
    // for (auto left_edge: left->edges()) {
    //     cout << "LEFT:: from: " << left_edge.src << " to: " << left_edge.dst << " label: " << left_edge.cond << endl;
    //     unsigned pre_r;
    //     for (auto right_edge: right->edges()) {
    //         cout << "RIGHT:: from: " << right_edge.src << " to: " << right_edge.dst << " label: " << right_edge.cond << endl;
    //         if (pre_l != left_edge.src && pre_r != right_edge.src) {
    //             unsigned state = producted->new_state();
    //             cout << left->create_namer<string>()->get_name(left_edge.src) << endl;
    //             string name = to_string(left_edge.src) + to_string(right_edge.src);
    //             cout << "name: " << name << endl;
    //             namer->alias_state(state, name);
    //         }                
    //         pre_r = right_edge.src;
    //     }
    //     pre_l = left_edge.src;
    // }

    // for (auto left_edge: left->edges()) {
    //     cout << "LEFT:: from: " << left_edge.src << " to: " << left_edge.dst << " label: " << left_edge.cond << endl;
    //     for (auto right_edge: right->edges()) {
    //         cout << "RIGHT:: from: " << right_edge.src << " to: " << right_edge.dst << " label: " << right_edge.cond << endl;
    //         if (left_edge.src == right_edge.src && left_edge.dst == right_edge.dst) {
    //             bdd cond = bdd_and(left_edge.cond, right_edge.cond);
    //             producted->new_edge(left_edge.src, right_edge.dst, cond);
    //         }
    //     }
    // }

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
    dict->dump(cout);

    vector<string> responseEvents = {"y"};
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

    cout << "##############################################################" << endl;
    // オートマトンの同期積合成
    twa_graph_ptr automaton_producted = make_twa_graph(dict);
    vector<string> name;
    automaton_producted = synchronous_product(automaton_list, shared, name);
    cout<< "なまえ" << endl;
    for (size_t i = 0; i < name.size(); i++) {
        cout << i << " : " << name[i] << endl;
    }

    print_hoa(cout, automaton_producted);
    ofstream producted_ofs("producted.hoa");
    print_hoa(producted_ofs, automaton_producted);
    producted_ofs.close();

    automaton_producted->get_dict()->dump(cout);

    return 0;

    vector<bdd> bdd_list = {};    
    for (auto automaton: automaton_list) {
        // オートマトンの出力
        cout << "##############################################################" << endl;
        print_hoa(cout, automaton);
        cout << endl;

        for (auto edge: automaton->edges()) {
            cout << "from: " << edge.src << " to: " << edge.dst << " label: " << edge.cond << endl;
            bdd_list.push_back(edge.cond);
        }
    }
    bdd tmp = bdd_and(bdd_list[0], bdd_list[4]);
    bdd zero = bdd_and(tmp, bdd_list[5]);
    cout << zero << endl;

    // parsed_formula parsed = parse_infix_psl(ltl_formula_str);
    // formula formula = parsed.f;
    // cout << "入力 LTL : " << str_psl(formula) << endl;
    cout << "応答イベント : " << endl;
    for (auto res: responseEvents) {
        cout << res << endl;
    }

    // オートマトンの出力
    //print_dot(cout, automaton_producted);
}
