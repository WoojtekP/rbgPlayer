#ifndef SEMISPLITNODE
#define SEMISPLITNODE


#include <type_traits>

#include "config.hpp"
#include "node_base.hpp"
#include "reasoner.hpp"
#include "types.hpp"


enum node_status : char {
    terminal,
    nonterminal,
    unknown,
};

struct semisplit_node : node_base {
    const bool is_nodal;
    node_status status;
    semisplit_node(void) = delete;
    semisplit_node(const semisplit_node&) = default;
    semisplit_node(semisplit_node&&) = default;
    semisplit_node& operator=(const semisplit_node&) = default;
    semisplit_node& operator=(semisplit_node&&) = default;
    ~semisplit_node(void) = default;
    semisplit_node(const uint, const uint, const bool, const node_status);
    semisplit_node(const bool, const node_status);
    bool is_terminal() const;
};

template <typename T = semimove>
struct semisplit_child : child_base {
    T semimove;
    semisplit_child(void) = delete;
    semisplit_child(const semisplit_child&) = default;
    semisplit_child(semisplit_child&&) = default;
    semisplit_child& operator=(const semisplit_child&) = default;
    semisplit_child& operator=(semisplit_child&&) = default;
    ~semisplit_child(void) = default;
    semisplit_child(const T& semimove_) : semimove(semimove_) {}

    template <typename M = T>
    typename std::enable_if<std::is_same<M, reasoner::action_representation>::value, reasoner::action_representation>::type get_action() const {
        return semimove;
    }

    template <typename M = T>
    typename std::enable_if<std::is_same<M, reasoner::move>::value, reasoner::action_representation>::type get_action() const {
        assert(semimove.mr.size() == 1);
        return semimove.mr.back();
    }

    template <typename M = T>
    typename std::enable_if<std::is_same<M, reasoner::action_representation>::value, T>::type get_edge() const {
        return semimove;
    }

    template <typename M = T>
    typename std::enable_if<std::is_same<M, reasoner::move>::value, const T&>::type get_edge() const {
        return semimove;
    }
};

#endif
