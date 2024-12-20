#include "counterexample.hpp"

using namespace std;
using namespace boost;

/**
 * @inheritDoc
 */
ParseCounterexample Counterexample::parse(string counterexample) {
    ParseCounterexample result;
    CounterexampleGrammar<string::const_iterator> grammar;
    string::const_iterator iter = counterexample.begin();
    string::const_iterator end = counterexample.end();
    bool r = spirit::qi::phrase_parse(iter, end, grammar, spirit::qi::space, result);
    if (!r || iter != end) {
        cout << "parse failed" << endl;
        exit(1);
    }
    return result;
}

/**
 * @inheritDoc
 */
CEGraph Counterexample::createCounterExampleGraph(ParseCounterexample& counterexample_parsed) {
    CEGraph graph;
    vector<size_t> vertex_map;
    size_t num_of_ce_node = counterexample_parsed.events.size() + counterexample_parsed.w_events.size();
    for (size_t i = 0; i < num_of_ce_node; i++) {
        vertex_map.push_back(add_vertex(graph));
    }
    for (size_t i = 0; i < vertex_map.size()-1; i++) {
        EdgeDescriptor e;
        if (i < counterexample_parsed.events.size()) {
            e = add_edge(vertex_map[i], vertex_map[i+1], graph).first;
            graph[e].label = counterexample_parsed.events[i];
        } else {
            e = add_edge(vertex_map[i], vertex_map[i+1], graph).first;
            graph[e].label = counterexample_parsed.w_events[i-counterexample_parsed.events.size()];
        }
    }
    EdgeDescriptor e;
    e = add_edge(vertex_map[vertex_map.size()-1], vertex_map[counterexample_parsed.events.size()], graph).first;
    graph[e].label = counterexample_parsed.w_events[counterexample_parsed.w_events.size()-1];

    // --- DEBUG FILE ---
    ofstream cegfile("ceg.dot");
    write_graphviz(cegfile, graph, default_writer(), make_label_writer(get(&EdgeProperty::label, graph)));

    return graph;
}