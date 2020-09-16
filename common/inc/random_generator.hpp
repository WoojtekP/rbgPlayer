#ifndef RANDGEN
#define RANDGEN

#include <random>

struct RBGRandomGenerator {
public:
    static RBGRandomGenerator& get_instance();
    unsigned int uniform_choice(const uint32_t);
    #if MAST > 0
    double random_real_number();
    #endif
    RBGRandomGenerator(RBGRandomGenerator const&) = delete;
    void operator=(RBGRandomGenerator const&) = delete;
private:
    RBGRandomGenerator();
    std::mt19937 random_generator;
    #if MAST > 0
    std::uniform_real_distribution<> dist{0.0, 1.0};
    #endif
};

#endif
