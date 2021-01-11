#ifndef TREE
#define TREE

#include <vector>

#include "reasoner.hpp"
#include "semisplit_tree.hpp"
#include "types.hpp"


class Tree final : public SemisplitTree {
private:
    #if STATS
    void print_rolledup_move(const std::vector<uint>&);
    #endif
    void choose_best_rolledup_move(std::vector<uint>&, const uint = 0);
    reasoner::move get_move_from_saved_path_with_random_suffix(std::vector<uint>&);
public:
    Tree(void) = default;
    Tree(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = default;
    ~Tree(void) = default;
    void reparent_along_move(const reasoner::move&);
    reasoner::move choose_best_move();
};

#endif
