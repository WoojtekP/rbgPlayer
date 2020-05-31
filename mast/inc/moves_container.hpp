#include <unordered_map>

#include "reasoner.hpp"

struct move_hash {
    std::size_t operator()(const reasoner::move& move) const noexcept;
};

class moves_container {
private:
    std::unordered_map<reasoner::move, std::pair<uint, uint>, move_hash> map;
    double default_value = 100.0;
public:
    void insert_or_update(const reasoner::move&, const uint&);
    double get_score_or_default_value(const reasoner::move&);
};