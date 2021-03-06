#include <stdexcept>
#include <arpa/inet.h>
#include "socket.hpp"

namespace Mousygem {
    std::string SocketAddress::ip_address() const {
        // Do it!
        char ip_address[1024];
        auto *pt = inet_ntop(this->ss.ss_family, &this->ss, ip_address, sizeof(ip_address));
        if(pt == nullptr) {
            throw std::runtime_error("inet_ntop() failed");
        }
        return std::string(pt);
    }
}
