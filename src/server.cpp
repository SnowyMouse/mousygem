#include <mousygem/server.hpp>
#include <mousygem/response.hpp>
#include <mousygem/client.hpp>
#include <mousygem/uri.hpp>
#include <thread>
#include <cstring>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ssl_context.hpp"
#include "socket.hpp"

namespace Mousygem {
    static std::runtime_error except_latest_error(const std::string &message) {
        auto error_number = errno;
        return std::runtime_error(message + ": " + strerrorname_np(error_number) + " - " + strerrordesc_np(error_number));
    }
    
    Server::Server(const char *ip_hostname, std::uint16_t port) {
        this->ssl_context = std::make_unique<SSLContext>();
        
        // I hate BSD sockets. Let's begin.
        sockaddr_storage address = {};
        socklen_t address_size;
        
        // First let's look up the address if we need to
        if(ip_hostname) {
            addrinfo addrinfo_hints = {};
            addrinfo *addrinfo_result;
            addrinfo_hints.ai_family = AF_UNSPEC;
            addrinfo_hints.ai_socktype = SOCK_STREAM;
            
            auto ai = getaddrinfo(ip_hostname, std::to_string(port).c_str(), &addrinfo_hints, &addrinfo_result);
            if(ai != 0) {
                throw std::runtime_error(std::string("could not resolve ") + ip_hostname + ":" + std::to_string(port) + " to an address");
            }
            
            // Copy what we got
            address_size = addrinfo_result->ai_addrlen;
            std::memcpy(&address, addrinfo_result->ai_addr, address_size);
            
            // Free the result
            freeaddrinfo(addrinfo_result);
        }
        
        // If ip_hostname is null, do IPv6 on all
        else {
            auto &new_address = *reinterpret_cast<sockaddr_in6 *>(&address);
            address_size = sizeof(new_address);
            new_address.sin6_family = AF_INET6;
            new_address.sin6_addr = IN6ADDR_ANY_INIT;
            new_address.sin6_port = htons(port);
        }
        
        // Set it
        this->address = std::make_unique<SocketAddress>(address, address_size);
        this->address->any = ip_hostname == nullptr;
    }
    
    void Server::use_tls_certificate(const std::filesystem::path &path) {
        SSL_CTX_use_certificate_file(this->ssl_context->get_context(), path.string().c_str(), SSL_FILETYPE_PEM);
    }
        
    void Server::use_tls_private_key(const std::filesystem::path &path) {
        SSL_CTX_use_PrivateKey_file(this->ssl_context->get_context(), path.string().c_str(), SSL_FILETYPE_PEM);
    }
    
    void Server::serve_client(Server *server, void *ssl_handle, Client *client) noexcept {
        // Assign to a unique_ptr to avoid leakage
        std::unique_ptr<Client> client_unique_ptr(client);
        
        // Let's do this
        auto *ssl = reinterpret_cast<SSL *>(ssl_handle);
        auto response = Response(Response::ResponseCode::TemporaryFailure, "error");
        SSL_set_fd(ssl, *client->socket->socket);
        
        bool error = false;
        
        // Try to accept it
        if(!SSL_accept(ssl)) {
            goto ssl_cleanup_spaghetti;
        }
        
        // Get the URL
        {
            char uri_input[1027] = {};
            int offset = 0;
            
            // Build the URL
            while(true) {
                int new_offset = SSL_read(ssl, uri_input + offset, (sizeof(uri_input) - 1) - offset);
                if(new_offset <= 0) {
                    error = true;
                    break;
                }
                
                offset += new_offset;
                if(offset > 2 && uri_input[offset - 2] == '\r' && uri_input[offset - 1] == '\n') {
                    break;
                }
            }
            
            // Validate it.
            if(!error) {
                try {
                    URI requested_uri = std::string(uri_input, offset - 2);
                    
                    // Only accept gemini connections
                    if(requested_uri.protocol() != "gemini") {
                        error = true;
                    }
                    
                    response = server->respond(requested_uri, *client);
                }
                catch(std::exception &) {
                    error = true;
                    
                    response = Response(Response::ResponseCode::BadRequest, "invalid uri");
                }
            }
        }
        
        // Can we respond without breaking gemini spec?
        {
            auto code = response.get_code();
            if((code < 20 || code > 29) && response.has_data()) {
                std::fprintf(stderr, "Tried to send a non-successful response with data\n");
                std::terminate();
            }
        
            // Send the meta
            {
                const auto &meta = response.get_meta();
                if(meta.size() == 0) {
                    std::fprintf(stderr, "Tried to send a response without meta\n");
                    std::terminate();
                }
                
                char response_data[1025];
                auto meta_size = std::snprintf(response_data, sizeof(response_data), "%i %s\r\n", code, meta.c_str());
                if(meta_size < 0) {
                    std::fprintf(stderr, "An error occured when encoding the response data\n");
                    std::terminate();
                }
                
                // Limit of 1024 bytes
                if(static_cast<std::size_t>(meta_size) >= sizeof(response_data)) {
                    std::fprintf(stderr, "Response code and meta line is too long (%zu / %zu bytes)\n", static_cast<std::size_t>(meta_size), sizeof(response_data) - 1);
                    std::terminate();
                }
                
                // Send that
                if(SSL_write(ssl, response_data, meta_size) <= 0) {
                    std::fprintf(stderr, "Failed to send response header to a client\n");
                    goto ssl_cleanup_spaghetti;
                }
            }
        }
        
        // Next, send the data if we have it
        if(response.has_data()) {
            auto *data_vector = std::get_if<std::vector<std::byte>>(&*response.data);
            if(data_vector) {
                std::size_t bytes_sent = 0;
                while(bytes_sent < data_vector->size()) {
                    auto data_to_send = data_vector->size() - bytes_sent;
                    if(data_to_send > INT_MAX) {
                        data_to_send = INT_MAX;
                    }
                    if(SSL_write(ssl, data_vector->data() + bytes_sent, data_to_send) <= 0) {
                        std::fprintf(stderr, "Failed to send data bytes to a client\n");
                        goto ssl_cleanup_spaghetti;
                    }
                    bytes_sent += data_to_send;
                }
            }
            
            auto *data_stream = std::get_if<std::ifstream>(&*response.data);
            if(data_stream) {
                while(*data_stream) {
                    char stream_buffer[4096];
                    std::size_t buffer_len = 0;
                    
                    // Go through the file
                    while(true) {
                        // Check if EOF
                        auto next_char = data_stream->get();
                        if(next_char == EOF) {
                            break;
                        }
                        
                        // We didn't. Set the character here.
                        stream_buffer[buffer_len] = next_char;
                        
                        // Check if we hit the max buffer size
                        buffer_len++;
                        if(buffer_len == sizeof(stream_buffer)) {
                            break;
                        }
                    }
                    
                    // Send it if we can
                    if(buffer_len) {
                        if(SSL_write(ssl, stream_buffer, buffer_len) <= 0) {
                            std::fprintf(stderr, "Failed to send data stream to a client\n");
                            goto ssl_cleanup_spaghetti;
                        }
                    }
                }
            }
        }
        
        // Spaghetti goto code
        ssl_cleanup_spaghetti:
        
        // Decrement client count (we're done)
        server->connected_clients_mutex.lock();
        server->connected_clients--;
        server->connected_clients_mutex.unlock();
        
        // Cleanup
        SSL_shutdown(ssl);
        SSL_free(ssl);
        client->socket->destroy();
    }
        
    void Server::accept_clients() {
        // Can we do that?
        if(this->server_running) {
            throw std::runtime_error("Server::accept_clients() called while accepting clients");
        }
        
        // Start
        this->server_running = true;
        
        // Make the actual socket
        int socket_flags = 0;
        auto socket_handle = socket(this->address->ss.ss_family, SOCK_STREAM, socket_flags);
        
        // Failed to make a socket
        if(socket_handle < 0) {
            throw except_latest_error("failed to make socket");
        }
        
        // Windows decided to use a meme DWORD instead of an int here. OK then.
        #ifdef _WIN32
        DWORD sockopt_on = 1, sockopt_off = 0;
        #else
        int sockopt_on = 1, sockopt_off = 0;
        #endif
        
        // Support both IPv6 and IPv4 if we're doing all addresses
        if(this->address->any) {
            if(setsockopt(socket_handle, IPPROTO_IPV6, IPV6_V6ONLY, &sockopt_off, sizeof(sockopt_off)) < 0) {
                throw except_latest_error("setsockopt failed (when disabling IPV6_ONLY)");
            }
        }
        
        // Allow re-binding
        if(setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, &sockopt_on, sizeof(sockopt_on)) < 0) {
            throw except_latest_error("setsockopt failed (when enabling SO_REUSEADDR)");
        }
        
        // We don't want to wait forever (in case we want to arbitrarily close the server)
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;
        setsockopt(socket_handle, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        // Actually bind now
        if(bind(socket_handle, reinterpret_cast<sockaddr *>(&this->address->ss), this->address->ss_size) < 0) {
            close(socket_handle);
            throw except_latest_error("bind failed");
        }
        
        // Hey, listen!
        if(listen(socket_handle, 0) < 0) {
            close(socket_handle);
            throw except_latest_error("listen failed");
        }
        
        // All right. We have a socket and it's bound.
        auto socket = Socket(socket_handle);
        
        // Get clients
        while(true) {
            // Are we shutting down?
            if(!shutdown_mutex.try_lock()) {
                break;
            }
            
            // Nope? Okay.
            shutdown_mutex.unlock();
            
            // Listen for a client
            sockaddr_storage client_address;
            socklen_t client_address_length = sizeof(sockaddr_storage);
            auto client_handle = accept(socket_handle, reinterpret_cast<sockaddr *>(&client_address), &client_address_length);
            if(client_handle < 0) {
                continue;
            }
            
            // Make a new SSL thingy
            auto *ssl = SSL_new(this->ssl_context->get_context());
            this->connected_clients_mutex.lock();
            this->connected_clients++;
            this->connected_clients_mutex.unlock();
            
            auto *client = new Client;
            client->socket = std::make_unique<Socket>(client_handle);
            client->socket_address = std::make_unique<SocketAddress>(client_address, client_address_length);
            
            // Serve the client
            std::thread(serve_client, this, ssl, client).detach();
        }
        
        // Done
        socket.destroy();
        this->server_running = false;
    }
        
    void Server::shutdown() {
        this->shutdown_mutex.lock();
        bool done_shutting_down = false;
        
        // Wait until all clients have disconnected
        while(!done_shutting_down) {
            // Check if we're done yet
            this->connected_clients_mutex.lock();
            done_shutting_down = this->connected_clients == 0;
            this->connected_clients_mutex.unlock();
            
            // Prevent busy waiting
            if(!done_shutting_down) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
        this->shutdown_mutex.unlock();
    }
        
    Server::~Server() {
        this->shutdown();
    }
}
