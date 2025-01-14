#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <string>
#include <spot/twa/fwd.hh>
#include <spot/twa/twa.hh>
#include <spot/twa/bdddict.hh>
#include <spot/twa/formula2bdd.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twa/twagraph.hh>
#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/tl/formula.hh>
#include <unordered_map>
#include <boost/utility.hpp>
#include <boost/foreach.hpp>
#include <queue>
#include <boost/config.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graph_utility.hpp>
#include <bddx.h>
#include "counterexample.hpp"



using namespace std;
using namespace spot;
using namespace boost;

/**
 * @struct DCG
 * @brief DCG
 */
struct DCG {
    Graph graph;
    map<pair<int, string>, VertexDescriptor> node_map;
};

/**
 * @struct V
 * @brief 頂点
 */
struct V {
    string label;
    int ce;
    bool operator<(const V& right) const {
        if (ce < right.ce) {
            return true;
        }
        if (ce > right.ce) {
            return false;
        }
        if (label.length() < right.label.length()) {
            return true;
        }
        if (label.length() > right.label.length()) {
            return false;
        }
        return false;
    }
};

/**
 * @struct E
 * @brief エッジ
 */
struct E {
    string label;
    V source;
    V target;
};

/**
 * @fn compare_V_List
 * @brief Vリストを比較する
 * @param left 左辺
 * @param right 右辺
 * @return 左辺と右辺が異なる場合はtrue
 */
bool compare_V_List(vector<V> left, vector<V> right);

/**
 * @fn compare_E_List
 * @brief Eリストを比較する
 * @param left 左辺
 * @param right 右辺
 * @return 左辺と右辺が異なる場合はtrue
 */
bool compare_E_List(vector<E> left, vector<E> right);

/**
 * @struct Head
 * @brief 反例の第一要素
 */
struct Head {
    //! BDD
    string label;
};

/**
 * @fn head
 * @brief 反例の第一要素を取得する
 * @param g 反例のグラフ
 * @param pointer ポインタ
 * @return 反例の第一要素
 */
Head head(CEGraph& g, int pointer);

typedef graph_traits<CEGraph>::edge_iterator edge_iter;

/**
 * @fn tail
 * @brief 反例の第二要素以降の列を返す
 * @param g グラフ
 * @param head ヘッド
 * @return tail
 */
int tail(CEGraph& g , int head);

/**
 * @fn filleter_keys
 * @brief キーを除外する
 * @param input 入力
 * @param keys_to_exclude 除外するキー
 * @return 除外後のマップ
 */
template <typename K, typename V>
map<K, V> filleter_keys(const map<K, V>& input, const vector<K>& keys_to_exclude);

/** 
 * @fn get_index_by_name
 * @brief 名前からインデックスを取得する
 * @param name 名前
 * @param target ターゲット
 * @return インデックス
 */
int get_index_by_name(vector<string> name, string target);

/**
 * @fn get_edge_BDD
 * @brief エッジのBDDを取得する
 * @param source ソース
 * @param target ターゲット
 * @param automaton オートマトン
 * @return BDD
 */
bdd get_edge_BDD(int source, int target, twa_graph_ptr automaton);

/**
 * @fn vector_exists
 * @brief ベクトルに要素が存在するかどうかを判定する
 * @param vec ベクトル
 * @param v 要素
 * @return 存在する場合はtrue
 */
bool vector_exists(vector<V> vec, V v);

/**
 * @fn find_vertex
 * @brief 頂点を検索する
 * @param vec 頂点リスト
 * @param v 頂点
 * @return 頂点が見つかった場合はインデックス、見つからなかった場合は-1
 */
int find_vertex(vector<V> vec, V v);

/**
 * @fn replace_commas_with_ampersands
 * @brief ,を&に置換する
 * @param s 置換対象の文字列
 * @return 置換後の文字列
 */
string replace_commas_with_ampersands(const string& s);

/**
 * @fn extract_variables
 * @brief 変数を抽出する
 * @param s 抽出対象の文字列
 * @return vector<string> 抽出した変数のリスト
 */
vector<string> extract_variables(const string& s);

/**
 * @fn createDCG
 * @brief DCGを作成する
 * @param automaton オートマトン
 * @param aut_name オートマトン各ノードの名前
 * @param dcg_name DCG各ノードの名前
 * @param ce_graph 反例のグラフ
 * @param response_events 応答イベント
 */
Graph createDCG(twa_graph_ptr& automaton, vector<string> aut_name, vector<string>& dcg_name, CEGraph ce_graph, vector<string> response_events);

/**
 * @struct Mscc
 * @brief 極大強連結成分
 * @param component 極大強連結成分
 * @param num_scc 極大強連結成分の数
 */
struct Mscc {
    map<int, vector<int>> component;
    int num_scc;
};

/**
 * @fn searchSCC
 * @brief 強連結成分を検索する
 * @param dcg DCG
 * @return 極大強連結成分
 */
Mscc searchSCC (Graph& dcg);

/**
 * @fn find_non_accepting_indices
 * @brief 受理状態でないインデックスを検索する
 * @param mscc 極大強連結成分
 * @param dcg_name DCGの名前
 * @return 受理状態でないインデックス
 */
map<int, string> find_non_accepting_indices(Mscc mscc, vector<string> dcg_name);

#endif
