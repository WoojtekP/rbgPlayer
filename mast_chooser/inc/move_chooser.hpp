#ifndef MOVECHOOSER
#define MOVECHOOSER

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
    moves_container moves[reasoner::NUMBER_OF_PLAYERS - 1];
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
    const reasoner::move_representation& extract_actions(const M& move) {
        return move.get_actions();
    }

    const reasoner::move_representation& extract_actions(const reasoner::move& move) {
        return move.mr;
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, const int current_player) {
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        if (rand_gen.random_real_number() < EPSILON) {
            double best_score = moves[current_player - 1].get_score_or_default_value(extract_actions(legal_moves.front()));
            indices.resize(1);
            indices[0] = 0;
            const uint size = legal_moves.size();
            for (uint i = 1; i < size; ++i) {
                double score = moves[current_player - 1].get_score_or_default_value(extract_actions(legal_moves[i]));
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

    uint get_unvisited_child_index(std::vector<Child>& children, const Node& node, const int current_player) {
        auto [fst, lst] = node.children_range;
        assert(fst < lst);
        auto lower = std::min(fst + node.sim_count, lst - 1);
        while (lower > fst && children[lower - 1].index == 0) {
            --lower;
        }
        for (auto i = fst; i < lower; ++i) {
            assert(children[i].index > 0);
        }
        for (auto i = lower; i < lst; ++i) {
            assert(children[i].index == 0);
        }
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        uint chosen_child;
        if (rand_gen.random_real_number() < EPSILON) {
            indices.clear();
            indices.reserve(lst - lower);
            double best_score = 0.0;
            for (uint i = lower; i < lst; ++i) {
                assert(children[i].index == 0);
                double score = moves[current_player - 1].get_score_or_default_value(children[i].get_actions());
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
        path.emplace_back(move, current_player);
    }

    void revert_move() {
        if (!path.empty()) {
            path.pop_back();
        }
    }

    void clear_path() {
        path.clear();
    }

    void complete_turn() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            moves[i - 1].apply_decay_factor();
        }
    }

    void update_move(const reasoner::move_representation& actions, const simulation_result& results, const int player, const uint depth) {
        moves[player - 1].insert_or_update(actions, results[player - 1], depth);
    }

    void update_all_moves(const simulation_result& results, const uint depth) {
        for (const auto& [move, player] : path) {
            moves[player - 1].insert_or_update(extract_actions(move), results[player - 1], depth);
        }
    }
};

#endif