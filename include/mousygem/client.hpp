#ifndef MOUSYGEM__CLIENT_HPP
#define MOUSYGEM__CLIENT_HPP

#include <string>
#include <memory>

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
        
        Client();
        ~Client();
        
    private:
        std::unique_ptr<SocketAddress> socket_address;
        std::unique_ptr<Socket> socket;
    };
}

#endif
