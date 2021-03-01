#include "random_generator.hpp"

RBGRandomGenerator& RBGRandomGenerator::get_instance() {
    static RBGRandomGenerator rand_gen;
    return rand_gen;
}

unsigned int RBGRandomGenerator::uniform_choice(const uint32_t upper_bound) {
    uint32_t x = random_generator();
    uint64_t m = uint64_t(x) * uint64_t(upper_bound);
    uint32_t l = uint32_t(m);
    if (l < upper_bound) {
        uint32_t t = -upper_bound;
        if (t >= upper_bound) {
            t -= upper_bound;
            if (t >= upper_bound)
                t %= upper_bound;
        }
        while (l < t) {
            x = random_generator();
            m = uint64_t(x) * uint64_t(upper_bound);
            l = uint32_t(m);
        }
    }
    return m >> 32;
}

double RBGRandomGenerator::random_real_number() {
    return random_generator() / 4294967296.0;
}

RBGRandomGenerator::RBGRandomGenerator() : random_generator(std::random_device{}()) {}
