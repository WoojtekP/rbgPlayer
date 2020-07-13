#ifndef MOVECHOOSER
#define MOVECHOOSER

#include <random>
#include <vector>

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"
#include "mcts_tree.hpp"
#include "node.hpp"
#include "random_generator.hpp"
#if MAST > 0
#include "moves_container.hpp"
#endif
#include <iostream>


template <typename T>
class MoveChooser {
private:
    std::vector<std::pair<T, int>> path;
    #if MAST > 0
    moves_container* moves;
    #endif
public:
    MoveChooser(const MoveChooser&)=delete;
    MoveChooser(MoveChooser&&)=default;
    MoveChooser& operator=(const MoveChooser&)=delete;
    MoveChooser& operator=(MoveChooser&&)=default;
    ~MoveChooser(void)=default;
    #if MAST > 0
    MoveChooser(void)=delete;
    MoveChooser(moves_container* moves) : moves(moves) {}
    #else
    MoveChooser(void)=default;
    #endif

    const std::vector<std::pair<T, int>>& get_path() const {
        return path;
    }

    const reasoner::move_representation extract_actions(const T& move) {
        return move.get_actions();
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, [[maybe_unused]] const int current_player) {
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        #if MAST > 0
        static std::vector<uint> indices;
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
        #endif
        return rand_gen.uniform_choice(legal_moves.size());
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
};

template<>
const reasoner::move_representation MoveChooser<reasoner::move>::extract_actions(const reasoner::move&);

#endif
