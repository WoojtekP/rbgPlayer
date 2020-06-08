#include "reasoner.hpp"

class moves_container {
private:
    static constexpr uint size = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
    std::pair<double, double> map[moves_container::size];
    const double default_value = 100.0;
    const double decay_factor = 0.8;  // TODO użyć stałej z config.hpp
public:
    moves_container();
    void insert_or_update(const reasoner::move&, const uint&, const uint&);
    double get_score_or_default_value(const reasoner::move&);
    void apply_decay_factor();
};