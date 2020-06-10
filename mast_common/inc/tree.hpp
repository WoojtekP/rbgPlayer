#ifndef TREE
#define TREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "mcts_tree.hpp"
#include "moves_container.hpp"

typedef std::vector<uint> simulation_result;

class Tree final : public MctsTree {
private:
    moves_container moves[reasoner::NUMBER_OF_PLAYERS - 1];
    std::uniform_real_distribution<double> prob;
    uint depth;
    uint get_unvisited_child_index(const uint, const uint);
    void play(reasoner::game_state&, simulation_result&);
    void mcts(reasoner::game_state&, const uint, simulation_result&);
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);
    void perform_simulation();
    void apply_move(const reasoner::move&);
};

#endif
