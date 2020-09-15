#ifndef TREE
#define TREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "mcts_tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "move_chooser.hpp"


class Tree final : public MctsTree {
private:
    uint create_node(reasoner::game_state&);
    void create_children(const uint, reasoner::game_state&);
public:
    Tree(void) = delete;
    Tree(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = default;
    ~Tree(void) = default;
    Tree(const reasoner::game_state&);
    uint perform_simulation();
    void reparent_along_move(const reasoner::move&);
    reasoner::move choose_best_move();
    game_status_indication get_status(const int);
};

#endif
