#include"node.hpp"
#include"constants.hpp"
#include<cmath>

node::node(reasoner::resettable_bitarray_stack& cache, const reasoner::game_state& state, std::vector<node>& nodes_register)
  : cache(cache)
  , nodes_register(nodes_register)
  , state(state){
    go_to_completion();
}

node::node(reasoner::resettable_bitarray_stack& cache, reasoner::game_state&& state, std::vector<node>& nodes_register)
  : cache(cache)
  , nodes_register(nodes_register)
  , state(std::move(state)){
    go_to_completion();
}

void node::go_to_completion(void){
    while(state.get_current_player() == KEEPER){
        auto any_move = state.get_any_move(cache);
        if(any_move)
            state.apply_move(*any_move);
        else
            break;
    }
}

double node::average_score(uint player)const{
    return sum_of_scores.get_player_score_divided_by(player, number_of_simulations);
}

double node::exploration_value(uint parent_simulations)const{
    return EXPLORATION_CONSTANT*sqrt(log(double(parent_simulations))/double(number_of_simulations));
}

void node::expand_children(void){
    if(not children){
        children = std::vector<edge>();
        auto all_moves = state.get_all_moves(cache);
        for(const auto& m: all_moves)
            children->emplace_back(m, *this, nodes_register);
    }
}

const node& node::get_node_by_address(const node_address& address, uint current_address_position)const{
    if(current_address_position >= address.size())
        return *this;
    else
        return
            (*children)[address[current_address_position]]
                .get_target()
                .get_node_by_address(address, current_address_position+1);
}

void node::apply_simulation_result(const simulation_result& result){
    sum_of_scores += result;
    ++number_of_simulations;
}

void node::apply_simulation_result_for_address(const simulation_result& result, const node_address& address, uint current_address_position){
    if(current_address_position < address.size()){
        apply_simulation_result(result);
        (*children)[address[current_address_position]]
            .get_target()
            .apply_simulation_result_for_address(result, address, current_address_position+1);
    }
}

std::tuple<const reasoner::game_state&, bool> node::choose_state_for_simulation(node_address& current_address){
    if(number_of_attempts==0){
        ++number_of_attempts;
        return std::make_tuple(state, true);
    }
    else{
        ++number_of_attempts;
        expand_children();
        if(children->empty())
            return std::make_pair(state, false);
        else{
            uint choice = children_with_highest_priority();
            (*children)[choice].create_target();
            current_address.push_back(choice);
            return (*children)[choice].get_target().choose_state_for_simulation(current_address);
        }
    }
}

uint node::children_with_highest_priority(void)const{
    std::vector<std::tuple<priority,uint>> candidates;
    uint current_player = state.get_current_player()-1;
    for(uint i = 0; i < children->size(); ++i)
        candidates.emplace_back((*children)[i].get_priority(number_of_simulations, current_player), i);
    return std::get<1>(*(std::max(candidates.begin(), candidates.end())));
}

node node::create_node_after_move(const reasoner::move& m)const{
    reasoner::game_state result = state;
    result.apply_move(m);
    return node(cache, std::move(result), nodes_register);
}

const node& node::get_node_by_address(const node_address& address)const{
    return get_node_by_address(address, 0);
}

const reasoner::game_state& node::get_state(void)const{
    return state;
}

priority node::get_priority(uint parent_simulations, uint parent_player)const{
    if(number_of_simulations == 0)
        return {INF, number_of_attempts};
    else
        return {average_score(parent_player)/EXPECTED_MAX_SCORE+exploration_value(parent_simulations), number_of_attempts};
}

std::optional<std::tuple<node_address, const reasoner::game_state&>> node::choose_state_for_simulation(void){
    for(uint i=0;i<MAX_TRIES_FOR_NON_TERMINAL_STATE;++i){
        node_address result;
        const auto& [state, is_terminal] = choose_state_for_simulation(result);
        if(is_terminal)
            return std::make_tuple(result, state);
        apply_simulation_result(simulation_result(state));
    }
    return std::nullopt;
}

void node::apply_simulation_result_for_address(const simulation_result& result, const node_address& address){
    apply_simulation_result_for_address(result, address, 0);
}
