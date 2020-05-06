#ifndef TREE
#define TREE

#include<random>
#include<vector>

#include"reasoner.hpp"
#include"types.hpp"
#include"node.hpp"

typedef int simulation_result;

class tree {
private:
    node root;
    std::mt19937 random_numbers_generator;
    node* traverse(node* state);
    simulation_result play(const node* state);
    void backpropagate(node* state, const simulation_result& winner);
    simulation_result mcts(node* state);
public:
    tree(void)=delete;
    tree(const tree&)=delete;
    tree(tree&&)=default;
    tree& operator=(const tree&)=delete;
    tree& operator=(tree&&)=default;
    ~tree(void)=default;
    tree(const reasoner::game_state& initial_state);
    game_status_indication get_status(int player_index) const;
    int get_simulations_count();
    reasoner::move choose_best_move();
    void reparent_along_move(const reasoner::move& move);
    void perform_simulation();
};

#endif
