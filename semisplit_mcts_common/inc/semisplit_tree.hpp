#ifndef SEMISPLITTREE
#define SEMISPLITTREE

#if STATS
#include <cmath>
#include <iostream>
#include <iomanip>
#endif

#include <vector>

#include "game_state.hpp"
#include "mcts_tree.hpp"
#include "semisplit_node.hpp"
#include "reasoner.hpp"
#include "types.hpp"


class SemisplitTree : public MctsTree {
protected:
    #if STATS
    void print_node_stats(const child&);
    void print_move(const reasoner::move&);
    void print_move(const reasoner::action_representation);
    #endif
    uint create_node(GameState&, const node_status = node_status::unknown);
    uint create_node(const bool, const node_status = node_status::unknown);
    void create_children(const uint, GameState&);
    void create_children(const uint, const std::vector<semimove>&);
    bool has_nodal_successor(GameState&, uint = 0);
    bool save_path_to_nodal_state(GameState&, std::vector<semimove>&, uint = 0);
    bool random_walk_to_nodal(GameState&, std::vector<semimove>&, uint = 0);
    void choose_best_greedy_move(std::vector<uint>&);
public:
    SemisplitTree(void);
    SemisplitTree(const SemisplitTree&) = delete;
    SemisplitTree(SemisplitTree&&) = default;
    SemisplitTree& operator=(const SemisplitTree&) = delete;
    SemisplitTree& operator=(SemisplitTree&&) = default;
    ~SemisplitTree(void) = default;
    uint perform_simulation();
    game_status_indication get_status(const int);

    template <
        typename T = decltype(child::semimove),
        typename std::enable_if<std::is_same<T, reasoner::action_representation>::value, T>::type* = nullptr>
    reasoner::move get_move_from_saved_path_with_random_suffix(std::vector<uint>& children_indices) {
        static GameState state;
        state = root_state;
        reasoner::move move;
        for (const auto child_index : children_indices) {
            const auto action = children[child_index].get_action();
            if (action.index > 0) {
                move.mr.emplace_back(action);
            }
        }
        state.apply(move);
        if (!state.is_nodal()) {
            #if STATS
            std::cout << "random continuation..." << std::endl << std::endl;
            #endif
            static std::vector<T> move_suffix;
            move_suffix.clear();
            random_walk_to_nodal(state, move_suffix);
            assert(!move_suffix.empty());
            for (const auto action : move_suffix) {
                if (action.index > 0) {
                    move.mr.push_back(action);
                }
            }
        }
        return move;
    }

    template <
        typename T = decltype(child::semimove),
        typename std::enable_if<std::is_same<T, reasoner::move>::value, T>::type* = nullptr>
    reasoner::move get_move_from_saved_path_with_random_suffix(std::vector<uint>& children_indices) {
        static GameState state;
        state = root_state;
        reasoner::move move;

        static std::vector<semimove> semimoves;

        for (const auto child_index : children_indices) {
            if (!move.mr.empty() && move.mr.back().index <= 0) {
                move.mr.pop_back();
            }
            const T& mv = children[child_index].semimove;
            move.mr.insert(move.mr.end(), mv.mr.begin(), mv.mr.end());
        }
        state.apply(move);
        if (!state.is_nodal()) {
            #if STATS
            std::cout << "random continuation..." << std::endl << std::endl;
            #endif
            static std::vector<T> move_suffix;
            move_suffix.clear();
            random_walk_to_nodal(state, move_suffix);
            assert(!move_suffix.empty());
            for (const auto semimove : move_suffix) {
                if (!move.mr.empty() && move.mr.back().index <= 0) {
                    move.mr.pop_back();
                }
                move.mr.insert(move.mr.end(), semimove.mr.begin(), semimove.mr.end());
            }
        }
        return move;
    }

    template <
        typename T = decltype(child::semimove),
        typename std::enable_if<std::is_same<T, reasoner::action_representation>::value, T>::type* = nullptr>
    void reparent_along_move(const reasoner::move& move) {
        #if STATS
        ++turn_number;
        #endif
        root_state.apply(move);
        complete_turn(root_state);
        uint root_index = 0;
        static std::vector<std::tuple<uint, uint, uint>> stack;
        stack.clear();
        const auto& mr = move.mr;
        const uint size = mr.size();
        uint i = 0;
        auto [fst, lst] = nodes.front().children_range;
        while (i < size) {
            if (!nodes[root_index].is_expanded()) {
                root_index = 0;
                break;
            }
            while (fst < lst) {
                if (children[fst].get_action().index <= 0) {
                    stack.emplace_back(root_index, fst, root_sim_count);
                    root_index = children[fst].index;
                    root_sim_count = children[fst].sim_count;
                    break;
                }
                else if (children[fst].get_action() == mr[i]) {
                    root_index = children[fst].index;
                    root_sim_count = children[fst].sim_count;
                    ++i;
                    stack.clear();
                    break;
                }
                ++fst;
            }
            if (fst == lst || root_index == 0) {
                if (!stack.empty()) {
                    std::tie(root_index, fst, root_sim_count) = stack.back();
                    stack.pop_back();
                    ++fst;
                    lst = nodes[root_index].children_range.second;
                    continue;
                }
                else {
                    root_index = 0;
                    break;
                }
            }
            std::tie(fst, lst) = nodes[root_index].children_range;
        }
        if (root_index == 0) {
            root_sim_count = 0;
            nodes.clear();
            children.clear();
            const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
            create_node(root_state, status);
        }
        else {
            root_at_index(root_index);
            if (nodes.front().status == node_status::unknown) {
                if (has_nodal_successor(root_state)) {
                    nodes.front().status = node_status::nonterminal;
                }
                else {
                    nodes.front().status = node_status::terminal;
                    nodes.front().children_range = {0, 0};
                }
            }
        }
        if (!nodes.front().is_expanded()) {
            create_children(0, root_state);
        }
        #if MAST > 0
        move_chooser.complete_turn();
        #endif
    }

    template <
        typename T = decltype(child::semimove),
        typename std::enable_if<std::is_same<T, reasoner::move>::value, T>::type* = nullptr>
    void reparent_along_move(const reasoner::move& move) {
        #if STATS
        ++turn_number;
        #endif
        root_state.apply(move);
        complete_turn(root_state);
        uint root_index = 0;
        static std::vector<std::tuple<uint, uint, uint>> stack;
        stack.clear();
        const auto& mr = move.mr;
        const uint size = mr.size();
        uint i = 0;
        auto [fst, lst] = nodes.front().children_range;
        while (i < size) {
            if (!nodes[root_index].is_expanded()) {
                root_index = 0;
                break;
            }
            while (fst < lst) {
                if (static_cast<T>(children[fst].semimove).mr.size() == 1 && children[fst].get_action().index <= 0) {
                    stack.emplace_back(root_index, fst, root_sim_count);
                    root_index = children[fst].index;
                    root_sim_count = children[fst].sim_count;
                    break;
                }
                bool matched = true;
                uint j = 0;
                for (const auto action : static_cast<T>(children[fst].semimove).mr) {
                    if (!(action == mr[i + j])) {
                        matched = false;
                        break;
                    }
                    ++j;
                }
                if (matched) {
                    root_index = children[fst].index;
                    root_sim_count = children[fst].sim_count;
                    i += j;
                    stack.clear();
                    break;
                }
                ++fst;
            }
            if (fst == lst || root_index == 0) {
                if (!stack.empty()) {
                    std::tie(root_index, fst, root_sim_count) = stack.back();
                    stack.pop_back();
                    ++fst;
                    lst = nodes[root_index].children_range.second;
                    continue;
                }
                else {
                    root_index = 0;
                    break;
                }
            }
            std::tie(fst, lst) = nodes[root_index].children_range;
        }
        if (root_index == 0) {
            root_sim_count = 0;
            nodes.clear();
            children.clear();
            const auto status = has_nodal_successor(root_state) ? node_status::nonterminal : node_status::terminal;
            create_node(root_state, status);
        }
        else {
            root_at_index(root_index);
            if (nodes.front().status == node_status::unknown) {
                if (has_nodal_successor(root_state)) {
                    nodes.front().status = node_status::nonterminal;
                }
                else {
                    nodes.front().status = node_status::terminal;
                    nodes.front().children_range = {0, 0};
                }
            }
        }
        if (!nodes.front().is_expanded()) {
            create_children(0, root_state);
        }
        #if MAST > 0
        move_chooser.complete_turn();
        #endif
    }
};

#endif
