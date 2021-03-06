#ifndef MOUSYGEM__SOCKET_HPP
#define MOUSYGEM__SOCKET_HPP

#include <optional>
#include <cstddef>
#include <openssl/ssl.h>
#include <unistd.h>

namespace Mousygem {
    struct Socket {
        std::optional<int> socket;
        
        void destroy() {
            if(socket.has_value()) {
                close(*this->socket);
                this->socket = std::nullopt;
            }
        }
        
        Socket() = default;
        Socket(const Socket &) = default;
        Socket(Socket &&) = default;
        
        Socket(int s) : socket(s) {}
    };
}

#endif
