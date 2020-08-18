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
    void choose_best_rolledup_move(const uint, std::vector<uint>&);
public:
    Tree(void)=delete;
    Tree(const Tree&)=delete;
    Tree(Tree&&)=default;
    Tree& operator=(const Tree&)=delete;
    Tree& operator=(Tree&&)=default;
    ~Tree(void)=default;
    Tree(const reasoner::game_state&);
    reasoner::move choose_best_move();
};

#endif
