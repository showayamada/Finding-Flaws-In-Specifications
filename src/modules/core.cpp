#include "core.hpp"

/**
 * @fn get_strongly_unsatisfiable_core
 * @brief 充足不能コアを取得する
 * @param non_accepting_indices 受理状態でないインデックス
 * @param n インデックスの数
 * @return 充足不能コア
 */
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
