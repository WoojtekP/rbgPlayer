#ifndef NODE
#define NODE

#include <random>

#include "reasoner.hpp"
#include "types.hpp"

typedef std::vector<int> simulation_result;

class Node {
private:
    std::vector<reasoner::move> moves;  // jeden ruch (którym doszliśmy do tego węzła) zamiast wektora ruchów na dzieci ???
    std::pair<uint, uint> children;
    uint child_counter = 0;  // można obliczać na podstawie simulation_counter ???
    uint simulation_counter = 0;
    double total_score = 0;
public:
    Node(void)=default;
    Node(const Node&)=default;
    Node(Node&&)=default;
    Node& operator=(const Node&)=default;
    Node& operator=(Node&&)=default;
    ~Node(void)=default;
    Node(reasoner::game_state&, const uint&);

    bool is_leaf() const;
    bool is_fully_expanded() const;
    void update_stats(int, const simulation_result&);
    std::pair<uint, uint> get_children() const;
    uint get_simulation_counter() const;
    double get_total_score() const;
    reasoner::move get_move_by_child_index(uint) const;
    std::pair<reasoner::move, uint> get_random_move_and_child_index(std::mt19937&);
    uint get_child_index_by_move(const reasoner::move&) const;
    
    static reasoner::resettable_bitarray_stack cache;
};

#endif
