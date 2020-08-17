#ifndef SEMISPLITTREE
#define SEMISPLITTREE

#include <vector>

#include "mcts_tree.hpp"
#include "semisplit_node.hpp"
#include "reasoner.hpp"
#include "types.hpp"


class SemisplitTree : public MctsTree {
protected:
    #if STATS
    void print_node_stats(const SemisplitChild&);
    void print_actions(const reasoner::move_representation&);
    void print_move(const reasoner::move_representation&);
    #endif
    uint create_node(reasoner::game_state&, const node_status = node_status::unknown);
    void create_children(const uint, reasoner::game_state&);
    bool has_nodal_successor(reasoner::game_state&, uint = 0);
    bool save_path_to_nodal_state(reasoner::game_state&, std::vector<reasoner::semimove>&, uint = 0);
    reasoner::move choose_best_greedy_move();
public:
    SemisplitTree(void)=delete;
    SemisplitTree(const SemisplitTree&)=delete;
    SemisplitTree(SemisplitTree&&)=default;
    SemisplitTree& operator=(const SemisplitTree&)=delete;
    SemisplitTree& operator=(SemisplitTree&&)=default;
    ~SemisplitTree(void)=default;
    SemisplitTree(const reasoner::game_state&);
    uint perform_simulation();
    void reparent_along_move(const reasoner::move&);
    game_status_indication get_status(const int);
};

#endif
