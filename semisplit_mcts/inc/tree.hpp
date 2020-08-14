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
    uint create_node(reasoner::game_state&, const node_status = node_status::unknown);
    void create_children(const uint, reasoner::game_state&);
    bool has_nodal_successor(reasoner::game_state&, uint = 0);
    bool save_path_to_nodal_state(reasoner::game_state&, std::vector<reasoner::semimove>&, uint = 0);
    void choose_best_move(const uint, std::vector<uint>&);
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);
    uint perform_simulation();
    void reparent_along_move(const reasoner::move&);
    reasoner::move choose_best_move();
    game_status_indication get_status(const int);
};

#endif
