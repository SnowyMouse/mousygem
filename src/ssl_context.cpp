#include <stdexcept>

#include "ssl_context.hpp"

namespace Mousygem {
    SSLContext::SSLContext() {
        // Initialize the context
        this->context = SSL_CTX_new(SSLv23_server_method());
        
        // Check if successful
        if(this->context == nullptr) {
            throw std::runtime_error("failed to create SSL context");
        }
    }
    
    SSLContext::SSLContext(SSLContext &&other) noexcept {
        this->context = other.context;
        other.context = nullptr; // set to nullptr so the destructor knows to not free it
    }
    
    SSLContext::~SSLContext() noexcept {
        if(this->context) {
            SSL_CTX_free(this->context);
        }
    }
}
