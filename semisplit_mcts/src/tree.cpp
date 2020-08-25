#if STATS
#include <iostream>
#include <iomanip>
#endif

#include "tree.hpp"
#include "semisplit_tree.hpp"
#include "node.hpp"
#include "simulator.hpp"
#include "constants.hpp"


#if STATS
void Tree::print_rolledup_move(const std::vector<uint>& children_indices) {
    std::cout << "[";
    for (const auto child_index : children_indices) {
        print_actions(children[child_index].semimove.get_actions());
    }
    std::cout << "]" << std::endl;
}
#endif

Tree::Tree(const reasoner::game_state& initial_state) : SemisplitTree(initial_state) {}

void Tree::choose_best_rolledup_move(const uint node_index, std::vector<uint>& best_move_path) {
    static std::vector<uint> move_path;
    static uint max_sim = 0;
    static double max_score = 0;
    const auto [fst, lst] = nodes[node_index].children_range;
    for (auto i = fst; i < lst; ++i) {
        auto index = children[i].index;
        if (index > 0) {
            move_path.push_back(i);
            if (nodes[index].is_nodal) {
                #if STATS
                print_node_stats(children[i]);
                print_rolledup_move(move_path);
                #endif
                if (children[i].sim_count > max_sim) {
                    max_sim = children[i].sim_count;
                    max_score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                    best_move_path = move_path;
                }
                else if (children[i].sim_count == max_sim) {
                    auto score = static_cast<double>(children[i].total_score) / children[i].sim_count;
                    if (score > max_score) {
                        max_score = score;
                        best_move_path = move_path;
                    }
                }
            }
            else {
                choose_best_rolledup_move(index, best_move_path);
            }
            move_path.pop_back();
        }
    }
    if (node_index == 0) {
        max_sim = 0;
        max_score = 0;
        move_path.clear();
    }
}

reasoner::move Tree::choose_best_move() {
    if constexpr (GREEDY_CHOICE) {
        return choose_best_greedy_move();
    }
    static std::vector<uint> children_indices;
    children_indices.clear();
    choose_best_rolledup_move(0, children_indices);
    reasoner::move move;
    for (const auto child_index : children_indices) {
        const auto& semimove = children[child_index].semimove.get_actions();
        move.mr.insert(move.mr.end(), semimove.begin(), semimove.end());
    }
    #if STATS
    std::cout << std::endl << "chosen move:" << std::endl;
    print_node_stats(children[children_indices.back()]);
    print_rolledup_move(children_indices);
    std::cout << std::endl;
    #endif
    return move;
}
