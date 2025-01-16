#include "input.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;


Input::Input(
    vector<string> ltl_formula_str_list,
    vector<string> response_events,
    string counterexample
){
    this->ltl_formula_str_list = ltl_formula_str_list;
    this->response_events = response_events;
    this->counterexample = counterexample;
}

Input Input::load_input(ifstream& ifs) {
    vector<string> ltl_formula_str_list;
    vector<string> response_events;
    string counterexample;
    if (!ifs) {
        cout << "Failed to open input.txt" << endl;
        exit(1);
    }
    ptree pt;
    try {
        read_json(ifs, pt);
        if (pt.get_child_optional("guarantees")) {
            for (const auto& formula : pt.get_child("guarantees")) {
                ltl_formula_str_list.push_back(formula.second.get_value<string>());
            }
        }
        if (pt.get_child_optional("outputs")) {
            for (const auto& response_event : pt.get_child("outputs")) {
                response_events.push_back(response_event.second.get_value<string>());
            }
        }
        if (pt.get_child_optional("counterexample")) {
            counterexample = pt.get_child("counterexample").get_value<string>();
        }
    } catch (json_parser_error& e) {
        cout << "Failed to parse input.json" << endl;
        exit(1);
    }

    return {
        ltl_formula_str_list,
        response_events,
        counterexample
    };
}