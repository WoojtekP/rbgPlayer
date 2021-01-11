#ifndef TREE
#define TREE

#include <vector>

#include "reasoner.hpp"
#include "semisplit_tree.hpp"
#include "types.hpp"


class Tree final : public SemisplitTree {
private:
    void choose_children_for_rolling_up(const uint, std::vector<uint>&);
    void roll_up(const uint, std::vector<uint>&);
    reasoner::move get_move_from_saved_path_with_random_suffix(std::vector<uint>&);
public:
    Tree(void) = default;
    Tree(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = default;
    ~Tree(void) = default;
    uint perform_simulation();
    void reparent_along_move(const reasoner::move&);
    reasoner::move choose_best_move();
};

#endif
