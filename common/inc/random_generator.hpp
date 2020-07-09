#ifndef RANDGEN
#define RANDGEN

#include <random>

struct RBGRandomGenerator {
public:
    static RBGRandomGenerator& get_instance();
    unsigned int uniform_choice(const uint32_t);

private:
    RBGRandomGenerator();
    RBGRandomGenerator(RBGRandomGenerator const&)=delete;
    void operator=(RBGRandomGenerator const&)=delete;

    std::mt19937 random_generator;
};

#endif
