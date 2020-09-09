#ifndef MOVECHOOSER
#define MOVECHOOSER

#include <deque>
#include <random>
#include <vector>

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "random_generator.hpp"
#include "moves_container.hpp"


template <typename T>
class MoveChooser {
private:
    std::vector<std::pair<T, int>> path;
    std::vector<uint> indices;
    MovesContainer moves[reasoner::NUMBER_OF_PLAYERS - 1];
    int context = 0;
    #if MAST == 2
    std::vector<int> context_stack;

    bool end_of_context(const reasoner::semimove& semimove) const {
        return !semimove.mr.empty() && reasoner::is_switch(semimove.mr.back().index);
    }
    #endif

public:
    MoveChooser(const MoveChooser&)=delete;
    MoveChooser(MoveChooser&&)=default;
    MoveChooser& operator=(const MoveChooser&)=delete;
    MoveChooser& operator=(MoveChooser&&)=default;
    ~MoveChooser(void)=default;
    MoveChooser(void)=default;

    const std::vector<std::pair<T, int>>& get_path() const {
        return path;
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, const int current_player) {
        assert(current_player != KEEPER);
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        if (rand_gen.random_real_number() >= EPSILON) {
            double best_score = moves[current_player - 1].get_score_or_default_value(legal_moves.front(), context);
            indices.resize(1);
            indices[0] = 0;
            const uint size = legal_moves.size();
            for (uint i = 1; i < size; ++i) {
                double score = moves[current_player - 1].get_score_or_default_value(legal_moves[i], context);
                if (score > best_score) {
                    best_score = score;
                    indices.resize(1);
                    indices[0] = i;
                }
                else if (score == best_score) {
                    indices.push_back(i);
                }
            }
            return indices[rand_gen.uniform_choice(indices.size())];
        }
        return rand_gen.uniform_choice(legal_moves.size());
    }

    uint get_unvisited_child_index(std::vector<child>& children, const node& node, const uint node_sim_count, const int current_player) {
        assert(current_player != KEEPER);
        auto [fst, lst] = node.children_range;
        assert(fst < lst);
        auto lower = std::min(fst + node_sim_count, lst - 1);
        while (lower > fst && children[lower - 1].index == 0) {
            --lower;
        }
        #ifndef NDEBUG
        for (auto i = fst; i < lower; ++i) {
            assert(children[i].index > 0);
        }
        for (auto i = lower; i < lst; ++i) {
            assert(children[i].index == 0);
        }
        #endif
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        uint chosen_child;
        if (rand_gen.random_real_number() >= EPSILON) {
            indices.clear();
            indices.reserve(lst - lower);
            double best_score = 0.0;
            for (uint i = lower; i < lst; ++i) {
                assert(children[i].index == 0);
                double score = moves[current_player - 1].get_score_or_default_value(children[i].get_edge(), context);
                if (score > best_score) {
                    best_score = score;
                    indices.resize(1);
                    indices[0] = i;
                }
                else if (score == best_score) {
                    indices.push_back(i);
                }
            }
            chosen_child = indices[rand_gen.uniform_choice(indices.size())];
        }
        else {
            chosen_child = lower + rand_gen.uniform_choice(lst - lower);
        }
        if (chosen_child != lower) {
            std::swap(children[chosen_child], children[lower]);
        }
        return lower;
    }

    void save_move(const T& move, const int current_player) {
        assert(current_player != KEEPER);
        path.emplace_back(move, current_player);
    }

    void revert_move() {
        path.pop_back();
    }

    void clear_path() {
        path.clear();
        reset_context();
    }

    template <typename M>
    void update_move(const M& move, const simulation_result& results, const int player, const uint depth) {
        assert(player != KEEPER);
        [[maybe_unused]] auto new_context = moves[player - 1].insert_or_update(move, results[player - 1], depth, context);
        #if MAST == 2
        context = end_of_context(move) ? 0 : new_context;
        #endif
    }

    void update_all_moves(const simulation_result& results, const uint depth) {
        for (const auto& [move, player] : path) {
            update_move(move, results, player, depth);
        }
        #if MAST == 2
        assert(context == 0);
        #endif
    }

    void complete_turn() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            moves[i - 1].apply_decay_factor();
        }
    }

    template <typename M>
    void switch_context([[maybe_unused]] const M& move, [[maybe_unused]] const int current_player) {
        #if MAST == 2
        if (end_of_context(move)) {
            reset_context();
        }
        else {
            context_stack.push_back(context);
            context = moves[current_player - 1].get_context(move.mr, context);
        }
        #endif
    }

    void revert_context() {
        #if MAST == 2
        context = context_stack.back();
        context_stack.pop_back();
        #endif
    }

    void reset_context() {
        #if MAST == 2
        context = 0;
        context_stack.clear();
        #endif
    }
};

#endif
