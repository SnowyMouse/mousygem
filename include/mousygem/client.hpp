#ifndef MOUSYGEM__CLIENT_HPP
#define MOUSYGEM__CLIENT_HPP

#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace Mousygem {
    class Server;
    
    struct Socket;
    struct SocketAddress;
    
    /**
     * Client information
     */
    class Client {
        friend class Server;
        
    public:
        /**
         * Get the IP address of the client in string format
         * @return IP address
         */
        std::string ip_address() const;
        
        /**
         * Get the DER-encoded certificate received from the client if one was received
         * @return certificate
         */
        const std::optional<std::vector<std::byte>> &get_certificate() const noexcept {
            return this->certificate;
        }
        
        ~Client();
        
    private:
        std::unique_ptr<SocketAddress> socket_address;
        std::unique_ptr<Socket> socket;
        
        std::optional<std::vector<std::byte>> certificate;
        
        Client();
    };
}

#endif
