#ifndef SEMISPLITTREE
#define SEMISPLITTREE

#include <vector>

#include "game_state.hpp"
#include "mcts_tree.hpp"
#include "semisplit_node.hpp"
#include "reasoner.hpp"
#include "types.hpp"


class SemisplitTree : public MctsTree {
protected:
    #if STATS
    void print_node_stats(const semisplit_child&);
    void print_actions(const reasoner::move_representation&);
    void print_move(const reasoner::move_representation&);
    #endif
    uint create_node(GameState&, const node_status = node_status::unknown);
    void create_children(const uint, GameState&);
    bool has_nodal_successor(GameState&, uint = 0);
    bool save_path_to_nodal_state(GameState&, std::vector<reasoner::semimove>&, uint = 0);
    bool random_walk_to_nodal(GameState&, std::vector<reasoner::semimove>&, uint = 0);
    void choose_best_greedy_move(std::vector<uint>&);
    reasoner::move get_move_from_saved_path_with_random_suffix(std::vector<uint>&);
public:
    SemisplitTree(void);
    SemisplitTree(const SemisplitTree&) = delete;
    SemisplitTree(SemisplitTree&&) = default;
    SemisplitTree& operator=(const SemisplitTree&) = delete;
    SemisplitTree& operator=(SemisplitTree&&) = default;
    ~SemisplitTree(void) = default;
    uint perform_simulation();
    void reparent_along_move(const reasoner::move&);
    game_status_indication get_status(const int);
};

#endif
