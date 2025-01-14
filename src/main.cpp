#include "modules/input.hpp"
#include "modules/counterexample.hpp"
#include "modules/automaton.hpp"
#include "modules/product.hpp"
#include "modules/graph.hpp"
#include "modules/core.hpp"

int main() {
    //ifstream input_file("../src/elebater2.txt");
    ifstream input_file("../src/input.txt");
    Input input = Input::load_input(input_file);

    // --- DEBUG LOG ---
    cout << "LTL formulas: " << endl;
    for (auto ltl_formula_str: input.ltl_formula_str_list) {
        cout << ltl_formula_str << endl;
    }
    cout << "Response Events:" << endl;
    for (auto responseEvent: input.response_events) {
        cout << responseEvent << endl;
    }

    HandleAutomaton automaton = HandleAutomaton::make_shared_dict(input.ltl_formula_str_list);
    twa_graph_ptr shared = automaton.shared;
    bdd_dict_ptr dict = automaton.dict;
    // それぞれのLTL式からオートマトンを生成
    vector<twa_graph_ptr> automaton_list = automaton.ltl_list_to_automaton(input.ltl_formula_str_list);

    // 反例の解析してグラフにする
    ParseCounterexample counterexample_parsed = Counterexample::parse(input.counterexample);
    CEGraph ceg = Counterexample::createCounterExampleGraph(counterexample_parsed);

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

    // 命題変数を要求イベントだけに制限
    twa_graph_ptr automaton_projected = HandleAutomaton::projection_of_request_events(automaton_producted, dict, input.response_events);

    // 有向閉路グラフ(DCG)の作成
    cout << "Creating DCG." << endl;
    //! DCGのノードの名前
    vector<string> dcg_name;
    //! DCG
    Graph dcg = createDCG(automaton_projected, aut_name, dcg_name, ceg, input.response_events);
    cout << "Creating DCG successful!" << endl;

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
