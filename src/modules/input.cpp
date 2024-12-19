#include "input.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


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
    string line;
    string section;
    while (getline(ifs, line)){
        if (line.empty()) continue;
        if (line == "LTL_FORMULAS:") {
            section = "ltl";
        } else if (line == "RESPONSE_EVENTS:") {
            section = "response";
        } else if (line == "COUNTEREXAMPLE:") {
            section = "counterexample";
        } else {
            if (section == "ltl") {
                ltl_formula_str_list.push_back(line);
            } else if (section == "response") {
                response_events.push_back(line);
            } else if (section == "counterexample") {
                counterexample = line;
            }
        }
    }
    return {
        ltl_formula_str_list,
        response_events,
        counterexample
    };
}