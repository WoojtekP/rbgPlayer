#ifndef RAVE_MOVES_SET
#define RAVE_MOVES_SET

#include "node.hpp"
#include "types.hpp"
#include "reasoner.hpp"
#include "simulator.hpp"
#include "hashset_context_entry.hpp"
#include "move_hash_context.hpp"
#include "moves_hashset.hpp"


class RaveContextMovesSet {
private:
    std::vector<node>& nodes;
    std::vector<child>& children;
    MovesHashset<HashsetContextEntry, move_hash_context<HashsetContextEntry>> hashset[reasoner::NUMBER_OF_PLAYERS - 1];

public:
    RaveContextMovesSet() = delete;
    RaveContextMovesSet(std::vector<node>& _nodes, std::vector<child>& _children)
        : nodes(_nodes)
        , children(_children) {}

    int get_context(const reasoner::action_representation action, const int player, const int context) {
        return hashset[player].get_context(action, context);
    }

    int get_context(const reasoner::move& move, const int player, const int context) {
        auto ctx = context;
        for (const auto action : move.mr) {
            ctx = get_context(action, player, ctx);
        }
        return ctx;
    }

    int insert(const reasoner::action_representation action, const int player, const int context) {
        const auto new_context = hashset[player].insert(action, context);
        if (action.index > 0) {
            return reasoner::is_switch(action.index) ? 0 : new_context;
        }
        return context;
    }

    int insert(const reasoner::move& move, const int player, const int context) {
        auto ctx = context;
        for (const auto action : move.mr) {
            ctx = insert(action, player, ctx);
        }
        return ctx;
    }

    void update_amaf_scores(const uint node_index, const uint child_index, const int player, const simulation_result& results, const int context) {
        insert(children[child_index].get_edge(), player, context);
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (hashset[player].find(children[i].get_edge(), context)) {
                children[i].amaf.score += results[player];
                ++children[i].amaf.count;
            }
        }
    }

    void reset_moves() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            hashset[i - 1].reset();
        }
    }
};

#endif
