#ifndef MOUSYGEM__SERVER_HPP
#define MOUSYGEM__SERVER_HPP

#include <filesystem>
#include <cstdint>
#include <memory>
#include <mutex>

namespace Mousygem {
    class URI;
    class Response;
    class Client;
    
    struct Socket;
    class SSLContext;
    
    /**
     * Server instance.
     * 
     * Remarks:
     * - This class does manage its own OpenSSL context, but it does NOT handle initializing OpenSSL.
     * - OpenSSL needs to be initialized before creating a server. You can use the OpenSSL_add_ssl_algorithms() macro in <openssl/ssl.h> (or SSL_library_init() with the equivalent functions for setting up SSL) to do this.
     */
    class Server {
    public:
        /**
         * Default port
         */
        static const constexpr std::uint16_t DEFAULT_GEMINI_PORT = 1965;
        
        /**
         * Set the TLS certificate file (PEM format)
         * @param path path to the file
         */
        void use_tls_certificate(const std::filesystem::path &path);
        
        /**
         * Set the TLS private key file (PEM format)
         * @param path path to the file
         */
        void use_tls_private_key(const std::filesystem::path &path);
        
        /**
         * Begin accepting clients. This blocks until after shutdown() is called and all clients have disconnected. The TLS certificate and key must be set before this is called. This must not be called while clients are connected.
         */
        void accept_clients();
        
        /**
         * Stop accepting clients. Clients still connected will not be immediately dropped. Block until all clients have disconnected. This function is thread-safe, but it will cause a deadlock if called within respond().
         */
        void shutdown();
        
        /**
         * Destroy the server and free resources.
         */
        virtual ~Server() = 0;
        
    protected:
        /**
         * Callback for receiving a request
         * @param url    URL being retrieved
         * @param client client information
         */
        virtual Response respond(const URI &url, const Client &client) = 0;
        
        /**
         * Instantiate a server, binding to the given hostname/ip and port.
         * @param ip_hostname Hostname or IP address to bind to; if nullptr, attempt to bind to any interface
         * @param port        TCP port to bind to
         */
        Server(const char *ip_hostname = nullptr, std::uint16_t port = DEFAULT_GEMINI_PORT);
        
    private:
        /** SSL context */
        std::unique_ptr<SSLContext> ssl_context;
        
        /** Implementation-specific socket address */
        struct SocketAddress;
        std::unique_ptr<SocketAddress> address;
        
        /** Number of currently connected clients */
        unsigned long connected_clients = 0;
        
        /** Mutex for the connected clients count *?
        std::mutex connected_clients_mutex;
        
        /** Server running? */
        bool server_running = false;
        
        /** Shutting down? */
        std::mutex shutdown_mutex;
        
        /** Mutex */
        std::mutex connected_clients_mutex;
        
        /** Serve the client (thread) */
        static void serve_client(Server *server, void *ssl_handle, Socket client_handle) noexcept;
    };
}

#endif
