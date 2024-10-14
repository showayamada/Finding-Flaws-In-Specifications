#include <iostream>
#include <vector>
#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>

using namespace std;
using namespace spot;

int main() {
    // LTL式の入力
    string ltl_formula_str = "G((x1 -> F(y)) & (x2 -> !y))";
    vector<string> responseEvents = {"y"};
    parsed_formula parsed = parse_infix_psl(ltl_formula_str);
    formula formula = parsed.f;
    cout << "入力 LTL : " << str_psl(formula) << endl;
    cout << "応答イベント : ";
    for (auto res: responseEvents) {
        cout << res << endl;
    }
}
