#ifndef MOVECHOOSER
#define MOVECHOOSER

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "constants.hpp"
#include "reasoner.hpp"
#include "types.hpp"
#include "node.hpp"
#include "random_generator.hpp"


template <typename T>
class MoveChooser {
private:
    std::vector<std::pair<T, int>> path;
public:
    MoveChooser(void) = default;
    MoveChooser(const MoveChooser&) = delete;
    MoveChooser(MoveChooser&&) = default;
    MoveChooser& operator=(const MoveChooser&) = delete;
    MoveChooser& operator=(MoveChooser&&) = default;
    ~MoveChooser(void) = default;

    int get_context() {
        return 0;
    }

    const std::vector<std::pair<T, int>>& get_path() const {
        return path;
    }

    template <typename M>
    uint get_random_move(const std::vector<M>& legal_moves, [[maybe_unused]] const int current_player) {
        assert(current_player != KEEPER);
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        return rand_gen.uniform_choice(legal_moves.size());
    }

    uint get_unvisited_child_index(std::vector<child>& children, const node& node, const uint node_sim_count, const int) {
        auto [fst, lst] = node.children_range;
        assert(fst < lst);
        auto lower = std::min(fst + node_sim_count, lst - 1);
        while (lower > fst && children[lower - 1].index == 0) {
            --lower;
        }
        for (auto i = fst; i < lower; ++i) {
            assert(children[i].index > 0);
        }
        for (auto i = lower; i < lst; ++i) {
            assert(children[i].index == 0);
        }
        assert(children[lst - 1].index == 0);
        RBGRandomGenerator& rand_gen = RBGRandomGenerator::get_instance();
        uint chosen_child = lower + rand_gen.uniform_choice(lst - lower);
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
        assert(!path.empty());
        path.pop_back();
    }

    void clear_path() {
        path.clear();
    }

    template <typename M>
    void switch_context(const M&, const int) {}

    void revert_context() {}

    void reset_context() {}
};

#endif
