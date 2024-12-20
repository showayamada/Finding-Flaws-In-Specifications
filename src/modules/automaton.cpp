#include "automaton.hpp"
#include <spot/parseaut/public.hh>
#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/tl/print.hh>
#include <fstream>

using namespace std;
using namespace spot;

HandleAutomaton::HandleAutomaton(twa_graph_ptr shared, bdd_dict_ptr dict){
    this->shared = shared;
    this->dict = dict;
};

/** @inheritDoc */
HandleAutomaton HandleAutomaton::make_shared_dict(vector<string> ltl_formula_str_list){
    string ltl_formula_str = std::accumulate(
        ltl_formula_str_list.begin(), ltl_formula_str_list.end(), string(),
        [](const string& a, const string& b) {
            return a.empty() ? b : a + " & " + b;
        }
    );

    formula f = parse_infix_psl(ltl_formula_str).f;
    translator trans;
    auto shared = trans.run(f);
    auto dict = shared->get_dict();

    return {trans.run(f), dict};
}

/** @inheritDoc */
vector<twa_graph_ptr> HandleAutomaton::ltl_list_to_automaton(vector<string> ltl_formula_str_list) {
    vector<twa_graph_ptr> automaton_list;
    for (size_t i = 0; i < ltl_formula_str_list.size(); i++) {
        parsed_formula parsed = parse_infix_psl(ltl_formula_str_list[i]);
        formula formula = parsed.f;
        translator translator(this->dict);
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

/** @inheritDoc */
twa_graph_ptr HandleAutomaton::projection_of_request_events(twa_graph_ptr automaton_producted, bdd_dict_ptr dict, vector<string> response_events) {
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


