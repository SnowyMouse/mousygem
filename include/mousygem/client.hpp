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
         * Get the certificate received from the client if one was received
         * @return certificate
         */
        const std::optional<std::vector<std::byte>> &get_certificate() const noexcept {
            return this->certificate;
        }
        
        /**
         * Get whether or not the certificate could be verified
         * @return verified
         */
        bool is_certificate_verified() const noexcept {
            return this->certificate_verified;
        }
        
        ~Client();
        
    private:
        std::unique_ptr<SocketAddress> socket_address;
        std::unique_ptr<Socket> socket;
        
        std::optional<std::vector<std::byte>> certificate;
        bool certificate_verified = false;
        
        Client();
    };
}

#endif
