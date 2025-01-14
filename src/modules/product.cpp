#include "product.hpp"

using namespace std;
using namespace spot;
using namespace boost;

/**
 * @inheritDoc
 */
twa_graph_ptr product_aut(twa_graph_ptr left, twa_graph_ptr right, twa_graph_ptr shared, vector<string>& name) {
    twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    producted->copy_ap_of(shared);
    vector<string> state_names;
    if (0 == name.size()) {
        for (size_t i = 0; i < left->num_states(); i++) {
            for (size_t j = 0; j < right->num_states(); j++) {
                unsigned new_state = producted->new_state();
                string nname = to_string(i) + to_string(j);
                state_names.push_back(nname);
                name.push_back(nname);
            }
        }
    } else {
        vector<string> tmp;
        for (size_t i = 0; i < left->num_states(); i++) {
            for (size_t j = 0; j < right->num_states(); j++) {
                unsigned new_state = producted->new_state();
                state_names.push_back(to_string(i) + to_string(j));
                tmp.push_back(name[i]+to_string(j));
            }
        }
        name = tmp;
    }
    cout << "state_names.size(): " << state_names.size() << endl;
    for (size_t i = 0; i < state_names.size(); i++) {
        int l_state_src = stoi(to_string(state_names[i][0]));
        int r_state_src = stoi(to_string(state_names[i][1]));
        for (size_t j = 0; j < state_names.size(); j++) {
            int l_state_dst = stoi(to_string(state_names[j][0]));
            int r_state_dst = stoi(to_string(state_names[j][1]));
            for (auto l_edge: left->out(l_state_src)) {
                for (auto r_edge: right->out(r_state_src)) {
                    if (l_edge.dst == l_state_dst && r_edge.dst == r_state_dst) {
                        bdd cond = bdd_and(l_edge.cond, r_edge.cond);
                        producted->new_edge(i, j, cond);
                    }
                }
            }
        }
    }

    return producted;
}

/**
 * @inheritDoc
 */
twa_graph_ptr synchronous_product(vector<twa_graph_ptr> automatons, twa_graph_ptr shared, vector<string>& name) {
    vector<int> state_names;
    twa_graph_ptr producted = make_twa_graph(shared->get_dict());
    producted->copy_ap_of(shared);
    cout << "automatons.size(): " << automatons.size() << endl;

    for (size_t i = 0; i < automatons.size(); i++) {
        if (producted->num_states() == 0) {
            producted = automatons[i];
            cout << "1st" << endl;
            cout << "producted->num_states(): " << producted->num_states() << endl;
            continue;
        }
        cout << "product_aut start" << endl;
        producted = product_aut(producted, automatons[i], shared, name);
        cout << "producted->num_states(): " << producted->num_states() << endl;
        // --- DEBUG FILE ---
        ofstream processing("processing"+to_string(i)+".dot");
        print_dot(processing, producted);
    }

    return producted;
}