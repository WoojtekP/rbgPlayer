#ifndef MCTSTREE
#define MCTSTREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "move_chooser.hpp"
#include "simulator.hpp"
#if RAVE
#include "rave_tree.hpp"
#endif


class MctsTree {
protected:
    reasoner::game_state root_state;
    reasoner::resettable_bitarray_stack cache;
    std::vector<Node> nodes;
    std::vector<Child> children;
    std::vector<std::pair<uint,int>> children_stack;
    MoveChooser<move_type> move_chooser;
    #if RAVE
    RaveTree moves_tree[reasoner::NUMBER_OF_PLAYERS - 1];
    #endif
    uint root_sim_count = 0;
    #if STATS
    uint turn_number = 0;
    #endif

    bool is_node_fully_expanded(const uint);
    void complete_turn(reasoner::game_state&);
    void get_scores_from_state(reasoner::game_state&, simulation_result&);
    uint get_unvisited_child_index(std::vector<Child>&, const Node&, const uint, const int);
    uint get_best_uct_child_index(const uint, const uint);
    uint get_top_ranked_child_index(const uint);
    void root_at_index(const uint);
    uint fix_tree(std::vector<Node>&, std::vector<Child>&, const uint);
public:
    MctsTree(void)=delete;
    MctsTree(const MctsTree&)=delete;
    MctsTree(MctsTree&&)=default;
    MctsTree& operator=(const MctsTree&)=delete;
    MctsTree& operator=(MctsTree&&)=default;
    ~MctsTree(void)=default;
    MctsTree(const reasoner::game_state&);
};

#endif
