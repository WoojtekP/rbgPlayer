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
    virtual uint get_unvisited_child_index(const uint&, const uint&) override;
    virtual void play(reasoner::game_state&, simulation_result&) override;
    virtual void mcts(reasoner::game_state&, uint, simulation_result&) override;
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);
};

#endif
