#ifndef INPUT_HPP
#define INPUT_HPP

#include <vector>
#include <string>

using namespace std;

/**
 * @class Input
 * @brief 入力
 */
class Input {
public:
    Input(
        vector<string> ltl_formula_str_list,
        vector<string> response_events,
        string counterexample
    );
    vector<string> ltl_formula_str_list;
    vector<string> response_events;
    string counterexample;
static Input load_input(ifstream& ifs);
};

#endif
