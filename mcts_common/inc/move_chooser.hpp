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

    const std::vector<std::pair<T, int>> get_path() const {
        return path;
    }

    const reasoner::move_representation extract_actions(const T& move)  {
        return move.get_actions();
    }

    uint get_random_move(const std::vector<T>& legal_moves, [[maybe_unused]] const int current_player) {
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        uint chosen_move;
        #if MAST > 0
        static std::uniform_real_distribution<double> prob(0.0, 1.0);
        static std::mt19937 random_numbers_generator(std::random_device{}());
        static std::vector<uint> indices;
        if (prob(random_numbers_generator) < EPSILON) {
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
            std::uniform_int_distribution<uint> dist(0, indices.size() - 1);
            chosen_move = indices[dist(random_numbers_generator)];
        }
        else {
            chosen_move = rand_gen.uniform_choice(legal_moves.size());
        }
        if constexpr (not TREE_ONLY) {
            path.emplace_back(legal_moves[chosen_move], current_player);
        }
        #else
        chosen_move = rand_gen.uniform_choice(legal_moves.size());
        #endif
        #if MAST == 0 && RAVE > 0
        path.emplace_back(legal_moves[chosen_move]);
        #endif
        return chosen_move;
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
