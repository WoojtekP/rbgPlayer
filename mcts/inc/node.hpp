#ifndef NODE
#define NODE

#include <random>

#include "reasoner.hpp"
#include "types.hpp"

class node {
        node* parent = nullptr;
        reasoner::game_state state;
        reasoner::resettable_bitarray_stack cache;
        std::vector<reasoner::move> moves = {};
        public: std::vector<node> children = {};
        int simulations_counter = 0;
        int wins_counter = 0;
        void complete_turn();
    public:
        node(void)=default;
        node(const node&)=default;
        node(node&&)=default;
        node& operator=(const node&)=default;
        node& operator=(node&&)=default;
        ~node(void)=default;
        node(const reasoner::game_state&, node*, const reasoner::resettable_bitarray_stack&);
        node(const reasoner::game_state&);

        void increment_simulations_counter();
        void increment_wins_counter();
        void set_parent(node*);
        void set_root();
        void reset(const reasoner::game_state&);
        int get_current_player() const;
        int get_simulations_count() const;
        double get_priority() const;
        bool is_root() const;
        bool is_leaf() const;
        bool is_fully_expanded() const;
        node* get_best_uct();
        node* get_random_child(std::mt19937&);
        node* get_parent();
        node&& get_node_by_move(const reasoner::move&);
        reasoner::move choose_best_move();
        reasoner::game_state get_game_state() const;
        reasoner::resettable_bitarray_stack get_cache() const;
        std::vector<reasoner::move> get_move_list() const;
};

#endif
