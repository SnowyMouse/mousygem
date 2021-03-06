#include <stdexcept>
#include <arpa/inet.h>
#include <mousygem/client.hpp>
#include "socket.hpp"

namespace Mousygem {
    Client::Client() {}
    Client::~Client() {}
    
    std::string Client::ip_address() const {
        // Ensure we have an address
        if(!this->socket_address) {
            throw std::runtime_error("socket address is null");
        }
        
        return this->socket_address->ip_address();
    }
}
