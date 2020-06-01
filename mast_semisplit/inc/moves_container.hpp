#include "reasoner.hpp"

class moves_container {
private:
    std::pair<uint, uint> map[reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS];
    double default_value = 100.0;
public:
    moves_container() {
        uint size = reasoner::BOARD_SIZE * reasoner::NUMBER_OF_MODIFIERS;
        for (uint i = 0; i < size; ++i) {
            map[i].first = 0;
            map[i].second = 0;
        }
    }
    void insert_or_update(const reasoner::move&, const uint&);
    double get_score_or_default_value(const reasoner::move&);
};