#ifndef MCTSTREE
#define MCTSTREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"

typedef std::vector<uint> simulation_result;

class MctsTree {
protected:
    reasoner::game_state root_state;
    reasoner::resettable_bitarray_stack cache;
    uint depth;
    std::vector<Node> nodes;
    std::vector<Child> children;
    std::mt19937 random_numbers_generator;

    uint create_node(reasoner::game_state&);
    bool is_node_fully_expanded(const uint);
    uint fix_tree(std::vector<Node>&, std::vector<Child>&, const uint);
    void complete_turn(reasoner::game_state&);
    uint get_best_uct_child_index(const uint);
    void reparent_along_move(const reasoner::move&);
public:
    MctsTree(void)=delete;
    MctsTree(const MctsTree&)=delete;
    MctsTree(MctsTree&&)=default;
    MctsTree& operator=(const MctsTree&)=delete;
    MctsTree& operator=(MctsTree&&)=default;
    ~MctsTree(void)=default;
    MctsTree(const reasoner::game_state&);
    game_status_indication get_status(const int) const;
    reasoner::move choose_best_move();
};

#endif
