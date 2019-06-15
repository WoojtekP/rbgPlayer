#ifndef CONSTANTS
#define CONSTANTS

#include"types.hpp"

constexpr int KEEPER = 0;
constexpr double EXPECTED_MAX_SCORE = 100.0;
constexpr double EXPLORATION_CONSTANT = 1.0; // TODO: find value
constexpr double INF = 1000000000.0;
constexpr uint MIN_REQUESTS_IN_CHANNEL = 10;
constexpr uint MAX_NEW_REQUESTS_PER_ITERATION = 20;
constexpr uint MAX_TRIES_FOR_NON_TERMINAL_STATE = 20;
constexpr uint MAX_SIMULATION_RESPONSES = 10;
constexpr uint MILISECONDS_PER_MOVE = 1000;
constexpr uint BUFFER_TIME = 10;
constexpr uint SOCKET_BUFFER_SIZE = 1024;

#endif
