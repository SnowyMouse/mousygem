#include <iostream>
#include <sstream>
#include <mousygem/uri.hpp>

using namespace Mousygem;

#define test_str(a,b) { \
    if(a != b) { \
        std::cerr << __FILE__ ":" << __LINE__ << " - failed test: expected " << b << ", got " << a << "\n"; \
        std::exit(EXIT_FAILURE); \
    } \
}

#define test_if_opt_is_nullopt(a) { \
    if(a.has_value()) { \
        std::cerr << __FILE__ ":" << __LINE__ << " - failed test: expected nullopt, got "; \
        if(a == std::nullopt) { \
            std::cerr << "nullopt"; \
        } \
        else { \
            std::cerr << *a; \
        } \
        std::cerr << "\n"; \
        std::exit(EXIT_FAILURE); \
    } \
}

#define test_if_opt_is_value(a, b) { \
    if(!a.has_value()) { \
        std::cerr << __FILE__ ":" << __LINE__ << " - failed test: expected " << b << ", got "; \
        if(a == std::nullopt) { \
            std::cerr << "nullopt"; \
        } \
        else { \
            std::cerr << *a; \
        } \
        std::cerr << "\n"; \
        std::exit(EXIT_FAILURE); \
    } \
}

#define test_if_exceptions(...) try { \
    __VA_ARGS__; \
    std::cerr << __FILE__ ":" << __LINE__ << " - expected exception error\n"; \
    std::exit(EXIT_FAILURE); \
} \
catch(std::exception &) {} \

int main() {
    ////////////////////////////////////////////////////////////////////////////
    // Basic tests
    ////////////////////////////////////////////////////////////////////////////
    
    // Basic URI
    auto uri_simple = URI("gemini://snowymouse.com");
    test_str(uri_simple.protocol(), "gemini");
    test_str(uri_simple.hostname(), "snowymouse.com");
    test_if_opt_is_nullopt(uri_simple.port());
    test_str(uri_simple.path(), "");
    test_if_opt_is_nullopt(uri_simple.input());
    
    // Basic URI with a port
    auto uri_with_port = URI("gemini://snowymouse.com:1965/post/9-this-site-is-now-live-on-geminispace");
    test_str(uri_with_port.protocol(), "gemini");
    test_str(uri_with_port.hostname(), "snowymouse.com");
    test_if_opt_is_value(uri_with_port.port(), 1965);
    test_str(uri_with_port.path(), "/post/9-this-site-is-now-live-on-geminispace");
    test_if_opt_is_nullopt(uri_with_port.input());
    
    // Test input
    auto uri_with_port_and_input = URI("gemini://snowymouse.com:1965/some/form?test%20value");
    test_str(uri_with_port_and_input.protocol(), "gemini");
    test_str(uri_with_port_and_input.hostname(), "snowymouse.com");
    test_if_opt_is_value(uri_with_port_and_input.port(), 1965);
    test_str(uri_with_port_and_input.path(), "/some/form");
    test_if_opt_is_value(uri_with_port_and_input.input(), "test%20value");
    
    // File path (no hostname)
    auto uri_file_root = URI("file:///");
    test_str(uri_file_root.protocol(), "file");
    test_str(uri_file_root.hostname(), "");
    test_if_opt_is_nullopt(uri_file_root.port());
    test_str(uri_file_root.path(), "/");
    test_if_opt_is_nullopt(uri_file_root.input());
    
    ////////////////////////////////////////////////////////////////////////////
    // IPv6 tests
    ////////////////////////////////////////////////////////////////////////////
    
    // Basic IPv6 URI
    auto uri_ipv6 = URI("gemini://[::1]");
    test_str(uri_ipv6.protocol(), "gemini");
    test_str(uri_ipv6.hostname(), "[::1]");
    test_if_opt_is_nullopt(uri_ipv6.port());
    test_str(uri_ipv6.path(), "");
    test_if_opt_is_nullopt(uri_ipv6.input());
    
    // Basic IPv6 URI but with a port
    auto uri_ipv6_with_port = URI("gemini://[::1]:1965");
    test_str(uri_ipv6_with_port.protocol(), "gemini");
    test_str(uri_ipv6_with_port.hostname(), "[::1]");
    test_if_opt_is_value(uri_ipv6_with_port.port(), 1965);
    test_str(uri_ipv6_with_port.path(), "");
    test_if_opt_is_nullopt(uri_ipv6_with_port.input());
    
    ////////////////////////////////////////////////////////////////////////////
    // Error tests
    ////////////////////////////////////////////////////////////////////////////
    
    // Random data
    test_if_exceptions(URI("asdf"));
    
    // Forgot a slash
    test_if_exceptions(URI("gemini:/snowymouse.com"));
    
    // Two colons after port
    test_if_exceptions(URI("gemini://snowymouse.com::1965"));
    
    // Missing last square bracket
    test_if_exceptions(URI("gemini://[::1"));
    test_if_exceptions(URI("gemini://[::1:1965"));
    
    // Invalid ports
    test_if_exceptions(URI("gemini://snowymouse.com:65536")); // 65536 > 65535
    test_if_exceptions(URI("gemini://snowymouse.com:-1234")); // port contains non-numeric characters
    test_if_exceptions(URI("gemini://snowymouse.com:notarealport")); // port contains non-numeric characters
    test_if_exceptions(URI("gemini://snowymouse.com:1234notarealport")); // port contains non-numeric characters
    
    ////////////////////////////////////////////////////////////////////////////
    // Other functions
    ////////////////////////////////////////////////////////////////////////////
    
    // Comparing with a string
    test_str(uri_simple, "gemini://snowymouse.com");
    
    // Copy constructor
    auto uri_simple_copy = uri_simple;
    test_str(uri_simple_copy, uri_simple);
    
    // Move constructor
    auto uri_simple_moved = std::move(uri_simple_copy);
    test_str(uri_simple_moved, uri_simple);
    
    // Set to string
    uri_simple = "gemini://snowymouse.com";
    test_str(uri_simple, "gemini://snowymouse.com");
    
    // String stream
    std::stringstream ss;
    ss << uri_simple;
    test_str(ss.str(), uri_simple.string());
}
