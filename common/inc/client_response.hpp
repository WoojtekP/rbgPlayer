#ifndef CLIENT_RESPONSE
#define CLIENT_RESPONSE

#include <variant>

#include "reasoner.hpp"
#include "types.hpp"


struct client_response{
    std::variant<reasoner::move, game_status_indication> content;
};

#endif
