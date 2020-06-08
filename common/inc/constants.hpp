#ifndef CONSTANTS
#define CONSTANTS

#include"types.hpp"
#include"config.hpp"

constexpr int KEEPER = 0;
constexpr double EXPECTED_MAX_SCORE = 100.0;
constexpr double UNEXPLORED_SCORE = 10.0;
constexpr double EXPLORATION_CONSTANT = 0.4; // TODO: find value
constexpr double DECAY_FACTOR = 0.2;
constexpr double EPSILON = 0.4;
constexpr double INF = 100.0;
constexpr uint MIN_REQUESTS_IN_CHANNEL = 10;
constexpr uint MAX_NEW_REQUESTS_PER_ITERATION = 20;
constexpr uint MAX_TRIES_FOR_NON_TERMINAL_STATE = 2000;
constexpr uint MAX_SIMULATION_RESPONSES = 10;
constexpr uint MILISECONDS_TIME_GRANULATION = 10;
constexpr uint BUFFER_TIME = 100;
constexpr uint SOCKET_BUFFER_SIZE = 1024;
constexpr uint CHARGES = 0;
constexpr uint MEBIBYTE = 1048576;

#endif
