#ifndef RANDGEN
#define RANDGEN

#include <random>


struct RBGRandomGenerator {
public:
    static RBGRandomGenerator& get_instance();
    unsigned int uniform_choice(const uint32_t);
    double random_real_number();
    RBGRandomGenerator(RBGRandomGenerator const&) = delete;
    void operator=(RBGRandomGenerator const&) = delete;
private:
    RBGRandomGenerator();
    std::mt19937 random_generator;
    std::uniform_real_distribution<> dist {0.0, 1.0};
};

#endif
