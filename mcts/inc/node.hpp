#ifndef NODE
#define NODE

#include <random>

#include "reasoner.hpp"
#include "types.hpp"

typedef std::vector<uint> simulation_result;

class Node {
private:
    std::vector<reasoner::move> moves;
    std::vector<uint> simulation_counters;
    std::vector<uint> total_scores;
    std::pair<uint, uint> children;
    uint simulation_counter = 0;
public:
    Node(void)=default;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(reasoner::game_state&, const uint&);

    void update_stats(int, uint, const simulation_result&);
    bool is_terminal() const;
    bool is_fully_expanded() const;
    std::pair<uint, uint> get_children() const;
    uint get_child_index_by_move(const reasoner::move&) const;
    std::pair<reasoner::move, uint> get_best_uct_and_child_index(std::mt19937&);
    std::pair<reasoner::move, uint> get_random_move_and_child_index(std::mt19937&);
    reasoner::move choose_best_move() const;

    static reasoner::resettable_bitarray_stack cache;
};

#endif
