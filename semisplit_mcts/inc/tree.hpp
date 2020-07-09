#ifndef TREE
#define TREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "mcts_tree.hpp"
#include "node.hpp"

typedef std::vector<uint> simulation_result;

class Tree final : public MctsTree {
private:
    bool reset_path = false;
    uint create_node(reasoner::game_state&);
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);
    void perform_simulation();
    void reparent_along_move(const reasoner::move&);
    reasoner::move choose_best_move();
};

#endif
