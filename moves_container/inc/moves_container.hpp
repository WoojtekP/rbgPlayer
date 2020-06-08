#include <unordered_map>

#include "reasoner.hpp"

struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

class moves_container {
private:
    std::unordered_map<reasoner::move, std::pair<double, double>, move_hash> map;
    const double default_value = 100.0;
    const double decay_factor = 0.4;  // TODO użyć stałej z config.hpp
    double cumulative_decay_factor = decay_factor;
public:
    void insert_or_update(const reasoner::move&, const uint&, const uint&);
    double get_score_or_default_value(const reasoner::move&);
    void apply_decay_factor();
};