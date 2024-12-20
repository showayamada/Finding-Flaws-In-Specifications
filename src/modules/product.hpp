#ifndef PRODUCT_HPP
#define PRODUCT_HPP

#include <vector>
#include <string>
#include <spot/twa/fwd.hh>
#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/tl/formula.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/minimize.hh>
#include <spot/twaalgos/complement.hh>
#include <spot/twa/twa.hh>
#include <spot/twa/bdddict.hh>
#include <spot/twa/formula2bdd.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twa/twagraph.hh>
#include <spot/misc/bddlt.hh>
#include <spot/twaalgos/copy.hh>
#include <spot/graph/graph.hh>
#include <spot/graph/ngraph.hh>
#include <spot/parseaut/public.hh>
#include <spot/gen/automata.hh>
#include <spot/gen/formulas.hh>
#include <spot/ta/ta.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/dot.hh>
#include <spot/twa/bddprint.hh>
#include <spot/twaalgos/hoa.hh>
#include <bddx.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <map>
#include <spot/twaalgos/toparity.hh>
#include <spot/twaalgos/totgba.hh>
#include <spot/twaalgos/remfin.hh>
#include <spot/parseaut/public.hh>
#include <spot/tl/defaultenv.hh>
#include <spot/tl/environment.hh>
#include <spot/twaalgos/sccinfo.hh>
#include <spot/twa/twaproduct.hh>
#include <numeric>
#include <spot/twaalgos/product.hh>
#include <boost/array.hpp>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <unordered_map>
#include <boost/utility.hpp>
#include <boost/foreach.hpp>
#include <queue>
#include <boost/config.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace std;
using namespace spot;

/**
 * @fn synchronous_product
 * @brief 複数の同期積を合成する
 * @param automatons オートマトンのリスト
 * @param shared 共有オートマトン
 * @param name 同期積合成したオートマトンの状態名のリスト
 * @return 合成したオートマトン
 */
twa_graph_ptr synchronous_product(vector<twa_graph_ptr> automatons, twa_graph_ptr shared, vector<string>& name);

/**
 * @fn product_aut
 * @brief 2つのオートマトンの同期積を取る
 * @param left オートマトン
 * @param right オートマトン
 * @param shared 共有オートマトン
 * @param name 状態名のリスト
 * @return 積を取ったオートマトン
 */
twa_graph_ptr product_aut(twa_graph_ptr left, twa_graph_ptr right, twa_graph_ptr shared, vector<string>& name);

#endif