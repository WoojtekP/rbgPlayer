#ifndef TREE
#define TREE

#include <vector>

#include "reasoner.hpp"
#include "semisplit_tree.hpp"
#include "types.hpp"


class Tree final : public SemisplitTree {
private:
    void choose_children_for_rolling_up(const uint, const uint, std::vector<uint>&);
    void roll_up(const uint, std::vector<uint>&);
public:
    Tree(void) = delete;
    Tree(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = default;
    ~Tree(void) = default;
    Tree(const reasoner::game_state&);
    uint perform_simulation();
    reasoner::move choose_best_move();
};

#endif
