#ifndef COUNTEREXAMPLE_HPP
#define COUNTEREXAMPLE_HPP

#include <vector>
#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/fusion/include/std_pair.hpp>

using namespace std;
using namespace boost;

/**
 * @struct ParseCounterexample
 * @brief 解析済みの反例
 * @param events イベントのリスト
 * @param w_events ωイベントのリスト
 */
typedef struct {
    vector<string> events;
    vector<string> w_events;
} ParseCounterexample;
BOOST_FUSION_ADAPT_STRUCT(ParseCounterexample, events, w_events);

/**
 * @struct CounterexampleGrammar
 * @brief 反例のパーサ
 */
template <typename Iterator>
struct CounterexampleGrammar : spirit::qi::grammar<Iterator, ParseCounterexample(), spirit::qi::space_type> {
    CounterexampleGrammar() : CounterexampleGrammar::base_type(expr) {
        using spirit::qi::lit;
        using spirit::qi::char_;
        using spirit::qi::lexeme;
        using spirit::qi::alpha;
        using spirit::qi::alnum;
        using spirit::qi::space;
        using spirit::qi::repeat;
        using spirit::qi::int_;
        using spirit::qi::attr;
        using spirit::qi::eps;
        expr = events >> w_events;        
        single_expr = '{' >> lexeme[(char_("a-zA-Z") >> *char_("a-zA-Z0-9"))] >> *lexeme[(char_(','))] >> *lexeme[(char_("a-zA-Z") >> *char_("a-zA-Z0-9"))] >> '}';
        w_events = '(' >> +single_expr >> ')';
        events %= +single_expr;
    }
    spirit::qi::rule<Iterator, ParseCounterexample(), spirit::qi::space_type> expr;
    spirit::qi::rule<Iterator, vector<string>(), spirit::qi::space_type> events;
    spirit::qi::rule<Iterator, vector<string>(), spirit::qi::space_type> w_events;
    spirit::qi::rule<Iterator, string(), spirit::qi::space_type> single_expr;
};

//! edge property
struct EdgeProperty {
    string label;
};
//! vertex property
struct VertexProperty {
    string label;
};

//! 反例のグラフ
typedef boost::adjacency_list<vecS, vecS, directedS, no_property, EdgeProperty> CEGraph;
typedef boost::graph_traits<CEGraph>::edge_descriptor EdgeDescriptor;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;

/**
 * @class Counterexample
 * @brief 反例を解析する
 */
class Counterexample {
public:
    Counterexample() = default; 

    /**
     * @fn parse
     * @brief 反例を解析する
     * @param string 反例の文字列
     * @return counterexample_parsed 解析済みの反例
     */
    static ParseCounterexample parse(string counterexample);

    /**
     * @fn createCounterExampleGraph
     * @brief 反例のグラフを作成する
     * @param counterexample_parsed 解析済みの反例
     * @return 反例のグラフ
     */
    static CEGraph createCounterExampleGraph(ParseCounterexample& counterexample_parsed);
};


#endif