#ifndef RAVE_MIX_MOVES_SET
#define RAVE_MIX_MOVES_SET

#include "actions_set.hpp"
#include "hashset_context_entry.hpp"
#include "move_hash_context.hpp"
#include "moves_hashset.hpp"
#include "node.hpp"
#include "reasoner.hpp"


class RaveMixMovesSet {
private:
    std::vector<node>& nodes;
    std::vector<child>& children;
    ActionsSet actions[reasoner::NUMBER_OF_PLAYERS - 1];
    MovesHashset<HashsetContextEntry, move_hash_context<HashsetContextEntry>> hashset[reasoner::NUMBER_OF_PLAYERS - 1];

public:
    RaveMixMovesSet() = delete;
    RaveMixMovesSet(std::vector<node>& _nodes, std::vector<child>& _children)
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
        actions[player].insert(action);
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

    void update_amaf_scores(const uint node_index, const uint child_index, const int player, const simulation_result& results, const int context = 0) {
        actions[player].insert(children[child_index].get_edge());
        hashset[player].insert(children[child_index].get_edge(), context);
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (actions[player].find(children[i].get_edge())) {
                children[i].amaf.score_base += results[player];
                ++children[i].amaf.count_base;
            }
            if (hashset[player].find(children[i].get_edge(), context)) {
                children[i].amaf.score += results[player];
                ++children[i].amaf.count;
            }
        }
    }

    void reset_moves() {
        for (int i = 1; i < reasoner::NUMBER_OF_PLAYERS; ++i) {
            actions[i - 1].reset();
            hashset[i - 1].reset();
        }
    }
};

#endif
