#ifndef CORE_HPP
#define CORE_HPP

#include <vector>
#include <string>
#include <map>

using namespace std;

/**
 * @fn get_strongly_unsatisfiable_core
 * @brief 充足不能コアを取得する
 * @param non_accepting_indices 受理状態でないインデックス
 * @param n インデックスの数
 * @return 充足不能コア
 */
vector<vector<int>> get_strongly_unsatisfiable_core(map<int, string> non_accepting_indices, int n);


#endif
