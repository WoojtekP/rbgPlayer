#ifndef MCTSTREE
#define MCTSTREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"


class MctsTree {
protected:
    reasoner::game_state root_state;
    reasoner::resettable_bitarray_stack cache;
    std::vector<Node> nodes;
    std::vector<Child> children;

    bool is_node_fully_expanded(const uint);
    void complete_turn(reasoner::game_state&);
    uint get_best_uct_child_index(const uint);
    uint get_unvisited_child_index(const uint);
    uint fix_tree(std::vector<Node>&, std::vector<Child>&, const uint);
public:
    MctsTree(void)=delete;
    MctsTree(const MctsTree&)=delete;
    MctsTree(MctsTree&&)=default;
    MctsTree& operator=(const MctsTree&)=delete;
    MctsTree& operator=(MctsTree&&)=default;
    ~MctsTree(void)=default;
    MctsTree(const reasoner::game_state&);
    game_status_indication get_status(const int) const;
};

#endif
