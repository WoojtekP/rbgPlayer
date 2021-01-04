#ifndef RAVE_MOVES_SET
#define RAVE_MOVES_SET

#include "node.hpp"
#include "types.hpp"
#include "reasoner.hpp"
#include "simulator.hpp"


template <typename T>
class RaveMovesSet {
private:
    std::vector<node>& nodes;
    std::vector<child>& children;
    T moves_set[reasoner::NUMBER_OF_PLAYERS - 1];

public:
    RaveMovesSet() = delete;
    RaveMovesSet(std::vector<node>& _nodes, std::vector<child>& _children)
        : nodes(_nodes)
        , children(_children) {}

    template <typename M>
    int get_context(const M& move, const int player, const int context) {
        #if RAVE >= 2
        return moves_set[player].get_context(move, context);
        #endif
        assert(context == 0);
        return 0;
    }

    template <typename M>
    int insert(const M& move, const int player, const int context = 0) {
        #if RAVE >= 2
        const auto new_context = moves_set[player].insert(move, context);
        if (move.index > 0) {
            return reasoner::is_switch(move.index) ? 0 : new_context;
        }
        return context;
        #endif
        assert(context == 0);
        moves_set[player].insert(move, context);
        return 0;
    }

    void update_amaf_scores(const uint node_index, const uint child_index, const int player, const simulation_result& results, const int context = 0) {
        #if RAVE < 2
        assert(context == 0);
        #endif
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (moves_set[player].find(children[i].get_edge(), context)) {
                children[i].amaf.score += results[player];
                ++children[i].amaf.count;
            }
        }
        moves_set[player].insert(children[child_index].get_edge(), context);
    }

    void reset_moves() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            moves_set[i - 1].reset();
        }
    }
};

#endif
