#ifndef MOVECHOOSER
#define MOVECHOOSER

#include <random>
#include <vector>

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "random_generator.hpp"

#include <iostream>

template <typename T>
class MoveChooser {
private:
    std::vector<std::pair<T, int>> path;
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

    const reasoner::move_representation extract_actions(const T& move) {
        return move.get_actions();
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, const int) {
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        return rand_gen.uniform_choice(legal_moves.size());
    }

    uint get_unvisited_child_index(std::vector<Child>& children, const Node& node, const int) {
        auto [fst, lst] = node.children_range;
        assert(fst < lst);
        auto lower = std::min(fst + node.sim_count, lst - 1);
        while (lower > fst && children[lower - 1].index == 0) {
            --lower;
        }
        for (auto i = fst; i < lower; ++i) {
            if (children[i].index == 0) {
                std::cerr << "indices: ";
                for (auto j = fst; j < lst; ++j) {
                    std::cerr << children[j].index << " ";
                }
                std::cerr << std::endl << "chosen " << lower - fst;
                std::cerr << std::endl << "sim count " << node.sim_count << std::endl;
                assert(false);
            }
        }
        for (auto i = lower; i < lst; ++i) {
            if (children[i].index > 0) {
                std::cerr << "indices: ";
                for (auto j = fst; j < lst; ++j) {
                    std::cerr << children[j].index << " ";
                }
                std::cerr << std::endl << "chosen " << lower - fst;
                std::cerr << std::endl << "sim count " << node.sim_count << std::endl;
                assert(false);
            }
        }
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        uint chosen_child = lower + rand_gen.uniform_choice(lst - lower);
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
};

template<>
const reasoner::move_representation MoveChooser<reasoner::move>::extract_actions(const reasoner::move&);

#endif
