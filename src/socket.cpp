#include <stdexcept>
#include <arpa/inet.h>
#include "socket.hpp"

namespace Mousygem {
    std::string SocketAddress::ip_address() const {
        // Do it!
        char ip_address[1024];
        const char *pt = nullptr;
        switch(this->ss.ss_family) {
            case AF_INET:
                pt = inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in *>(&this->ss)->sin_addr, ip_address, sizeof(ip_address));
                break;
            case AF_INET6:
                pt = inet_ntop(AF_INET6, &reinterpret_cast<const sockaddr_in6 *>(&this->ss)->sin6_addr, ip_address, sizeof(ip_address));
                break;
        }
        
        if(pt == nullptr) {
            throw std::runtime_error("inet_ntop() failed");
        }
        return std::string(pt);
    }
}
