#ifndef TREE
#define TREE

#include<random>
#include<vector>

#include"reasoner.hpp"
#include"types.hpp"
#include"node.hpp"

typedef std::vector<int> simulation_result;

class Tree {
private:
    reasoner::game_state root_state;
    int root_index = 0;
    std::vector<Node> nodes;
    std::vector<int> children;
    std::mt19937 random_numbers_generator;

    int create_node(reasoner::game_state&);
    void play(reasoner::game_state&, simulation_result&);
    void mcts(reasoner::game_state&, int, simulation_result&);
    void complete_turn(reasoner::game_state&) const;
    int get_best_uct_and_change_state(const Node&, reasoner::game_state&) const;
    int get_random_child_and_change_state(Node&, reasoner::game_state&);
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state& initial_state);
    game_status_indication get_status(int player_index) const;
    int get_simulation_counter();
    reasoner::move choose_best_move();
    void reparent_along_move(const reasoner::move& move);
    void perform_simulation();
};

#endif
