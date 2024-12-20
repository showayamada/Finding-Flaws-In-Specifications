#ifndef AUTOMATON_HPP
#define AUTOMATON_HPP

#include <vector>
#include <string>
#include <spot/twa/fwd.hh>
#include <spot/twa/twa.hh>
#include <bddx.h>

using namespace std;
using namespace spot;

/**
 * @class HandleAutomaton
 * @brief オートマトンを扱うクラス
 */
class HandleAutomaton {

public:
    /** shared */
    twa_graph_ptr shared;
    /** 共有BDD辞書 */
    bdd_dict_ptr dict;
    HandleAutomaton(twa_graph_ptr shared, bdd_dict_ptr dict);
    
    /**
     * @fn make_shared_dict
     * @brief 共有BDD辞書を生成する
     * @param ltl_formula_str_list LTL式のリスト
     * @return HandleAutomaton 共有BDD辞書を持つオートマトン
     */
    static HandleAutomaton make_shared_dict(vector<string> ltl_formula_str_list);

    /**
     * @fn ltl_list_to_automaton
     * @brief LTL式を解析し、オートマトンを生成する
     * @param ltl_formula_str_list LTL式のリスト
     * @return vector<twa_graph_ptr> LTL式を解析して生成したオートマトンのリスト
     */
    static twa_graph_ptr projection_of_request_events(twa_graph_ptr automaton_producted, bdd_dict_ptr dict, vector<string> response_events);
    
    /**
     * @fn ltl_list_to_automaton
     * @brief 複数LTL式を解析し、それぞれオートマトンを生成する
     * @param ltl_formula_str_list LTL式のリスト
     * @return vector<twa_graph_ptr> LTL式を解析して生成したオートマトンのリスト
     */
    vector<twa_graph_ptr> ltl_list_to_automaton(vector<string> ltl_formula_str_list);
};

#endif
