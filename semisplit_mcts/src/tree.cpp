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
        print_action(children[child_index].action);
    }
    std::cout << "]" << std::endl;
}
#endif

void Tree::choose_best_rolledup_move(std::vector<uint>& best_move_path, const uint node_index) {
    static std::vector<uint> move_path;
    static uint max_sim = 0;
    static double max_score = 0;
    bool all_children_below_limit = true;
    if (!nodes[node_index].is_nodal || node_index == 0) {
        const auto [fst, lst] = nodes[node_index].children_range;
        for (auto i = fst; i < lst; ++i) {
            if (children[i].index > 0) {
                if (children[i].sim_count > ROLLUP_THRESHOLD) {
                    move_path.push_back(i);
                    choose_best_rolledup_move(best_move_path, children[i].index);
                    move_path.pop_back();
                    all_children_below_limit = false;
                }
            }
        }
    }
    if (all_children_below_limit) {
        if (node_index > 0) {
            uint child_index = move_path.back();
            assert(children[child_index].sim_count > 0);
            double score = static_cast<double>(children[child_index].total_score) / children[child_index].sim_count;
            // if (children[child_index].sim_count > max_sim || (children[child_index].sim_count == max_sim && score > max_score)) {
            if (score > max_score || (score == max_score && children[child_index].sim_count > max_sim)) {
                max_score = score;
                max_sim = children[child_index].sim_count;
                best_move_path = move_path;
            }
            #if STATS
            print_node_stats(children[child_index]);
            print_rolledup_move(move_path);
            #endif
        }
    }
    if (node_index == 0) {
        max_sim = 0;
        max_score = 0;
        move_path.clear();
    }
}

reasoner::move Tree::choose_best_move() {
    #if STATS
    std::cout << "turn number " << turn_number / 2 + 1 << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    #endif
    static std::vector<uint> children_indices;
    children_indices.clear();
    if constexpr (!GREEDY_CHOICE) {
        choose_best_rolledup_move(children_indices);
        #if STATS
        std::cout << std::endl << "chosen move:" << std::endl;
        print_node_stats(children[children_indices.back()]);
        print_rolledup_move(children_indices);
        std::cout << std::endl;
        #endif
    }
    choose_best_greedy_move(children_indices);
    return get_move_from_saved_path_with_random_suffix(children_indices);
}
