# Mousygem
Mousygem is a simple C++ based library for writing Gemini servers. It handles
creating sockets, (most) OpenSSL functions, accepting clients, receiving/parsing
URIs, and formatting/sending responses to the client.

## Requirements
- C++17 compiler
- OpenSSL (latest recommended)
- CMake 3.12 or newer

## Example implementation
Here is a very simple implementation of a working server in 31 lines of C++
code.

```cpp
#include <cstdint>
#include <openssl/ssl.h>
#include <mousygem/mousygem.hpp>

using namespace Mousygem;
using namespace std;

// Inherit the abstract class, Mousygem::Server
class MyServer : public Server {
public:
    MyServer(const char *ip, uint16_t port) : Server(ip, port) {} // just pass the ip/port

private:
    // Override respond(). Respond with "Hello world" and the path requested by the client.
    Response respond(const URI &uri, const Client &client) override {
        return Response(Response::Success, "text/gemini", string("# Hello world\r\n\r\nPath: ") + uri.path());
    }
};

int main() {
    // Initialize OpenSSL (required before we can create a server, since gemini mandates TLS)
    OpenSSL_add_ssl_algorithms();
    
    // Create server. Specify IP, port, key, and certificate.
    MyServer server(nullptr, 1965); // Passing nullptr as the IP binds to all addresses.
    server.use_certificate_file("cert.pem");
    server.use_private_key_file("key.pem");
    
    // Start the server (sets up sockets, calls bind() and listen(), begins accepting clients)
    server.accept_clients();
}
```

Use a gemini client to connect to it.

```
$ gmnlm gemini://localhost/path/to/resource?someinput
#  Hello world
   
   Path: /path/to/resource

text/gemini at gemini://localhost/path/to/resource?someinput
[N]: follow Nth link; [q]uit; [?]; or type a URL
=>
```
