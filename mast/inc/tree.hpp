#ifndef TREE
#define TREE

#include <random>
#include <vector>

#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "moves_container.hpp"

typedef std::vector<uint> simulation_result;

class Tree {
private:
    reasoner::game_state root_state;
    std::vector<Node> nodes;
    std::vector<Child> children;
    double tau = 10.0;
    moves_container moves[reasoner::NUMBER_OF_PLAYERS - 1];
    std::mt19937 random_numbers_generator;

    uint create_node(reasoner::game_state&);
    void play(reasoner::game_state&, simulation_result&);
    void mcts(reasoner::game_state&, uint, simulation_result&, uint&);
    void complete_turn(reasoner::game_state&) const;
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);

    uint get_best_uct_child_index(const uint&);
    uint get_unvisited_child_index(const uint&, const uint&);
    game_status_indication get_status(const uint&) const;
    reasoner::move choose_best_move();
    void reparent_along_move(const reasoner::move&);
    uint fix_tree(std::vector<Node>&, std::vector<Child>&, uint);
    void perform_simulation();
};

#endif
