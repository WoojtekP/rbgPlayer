#ifndef MCTSTREE
#define MCTSTREE

#include <utility>
#include <tuple>
#include <vector>

#include "game_state.hpp"
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
    GameState root_state;
    reasoner::resettable_bitarray_stack cache;
    std::vector<node> nodes;
    std::vector<child> children;
    std::vector<std::pair<uint, int>> children_stack;
    MoveChooser<simulation_move_type> move_chooser;
    #if RAVE > 0
    RaveTree moves_tree[reasoner::NUMBER_OF_PLAYERS - 1];
    #endif
    #if RAVE == 3
    RaveTree moves_tree_base[reasoner::NUMBER_OF_PLAYERS - 1];
    #endif
    uint root_sim_count = 0;
    #if STATS
    uint turn_number = 0;
    #endif

    bool is_node_fully_expanded(const uint);
    void complete_turn(GameState&);
    void get_scores_from_state(GameState&, simulation_result&);
    uint get_unvisited_child_index(std::vector<child>&, const node&, const uint, const int);
    void get_amaf_scores(std::vector<std::tuple<amaf_score, uint, bool>>&, uint, uint);
    uint get_best_uct_child_index(const uint, const uint);
    uint get_top_ranked_child_index(const uint);
    void root_at_index(const uint);
    uint fix_tree(std::vector<node>&, std::vector<child>&, const uint);
public:
    MctsTree(void);
    MctsTree(const MctsTree&) = delete;
    MctsTree(MctsTree&&) = default;
    MctsTree& operator=(const MctsTree&) = delete;
    MctsTree& operator=(MctsTree&&) = default;
    ~MctsTree(void) = default;
};

#endif
