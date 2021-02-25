#ifndef MASTCHOOSER
#define MASTCHOOSER

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "random_generator.hpp"
#include "moves_container.hpp"


template <typename T>
class MastChooser {
protected:
    std::vector<std::pair<T, int>> path;
    std::vector<uint> indices;
    MovesContainer moves[reasoner::NUMBER_OF_PLAYERS - 1];
    int context = 0;

public:
    MastChooser(void) = default;
    MastChooser(const MastChooser&) = delete;
    MastChooser(MastChooser&&) = default;
    MastChooser& operator=(const MastChooser&) = delete;
    MastChooser& operator=(MastChooser&&) = default;
    ~MastChooser(void) = default;

    int get_context() {
        return context;
    }

    const std::vector<std::pair<T, int>>& get_path() const {
        return path;
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, const int current_player, const bool greedy_choice = RBGRandomGenerator::get_instance().random_real_number() >= EPSILON) {
        assert(current_player != KEEPER);
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        if (greedy_choice) {
            double best_score = moves[current_player - 1].get_score_or_default_value(legal_moves.front(), context).get_score();
            indices.resize(1);
            indices[0] = 0;
            const uint size = legal_moves.size();
            for (uint i = 1; i < size; ++i) {
                double score = moves[current_player - 1].get_score_or_default_value(legal_moves[i], context).get_score();
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

    uint get_unvisited_child_index(std::vector<child>& children, const node& node, const uint node_sim_count, const int current_player, const bool greedy_choice = RBGRandomGenerator::get_instance().random_real_number() >= EPSILON) {
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
        if (greedy_choice) {
            indices.clear();
            indices.reserve(lst - lower);
            double best_score = 0.0;
            for (uint i = lower; i < lst; ++i) {
                assert(children[i].index == 0);
                double score = moves[current_player - 1].get_score_or_default_value(children[i].get_edge(), context).get_score();
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

    void complete_turn() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            moves[i - 1].apply_decay_factor();
        }
    }
};

#endif
