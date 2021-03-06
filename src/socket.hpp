#ifndef MOUSYGEM__SOCKET_HPP
#define MOUSYGEM__SOCKET_HPP

#include <string>
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
    
    struct SocketAddress {
        sockaddr_storage ss;
        socklen_t ss_size;
        bool any = false;
        
        std::string ip_address() const;
        
        SocketAddress() = default;
        SocketAddress(const SocketAddress &) = default;
        SocketAddress(SocketAddress &&) = default;
        
        SocketAddress(const sockaddr_storage &ss, socklen_t ss_size) : ss(ss), ss_size(ss_size) {}
        SocketAddress(const sockaddr_in &sin) : ss_size(sizeof(sin)) {
            *reinterpret_cast<sockaddr_in *>(&this->ss) = sin;
        }
        SocketAddress(const sockaddr_in6 &sin6) : ss_size(sizeof(sin6)) {
            *reinterpret_cast<sockaddr_in6 *>(&this->ss) = sin6;
        }
    };
}

#endif
