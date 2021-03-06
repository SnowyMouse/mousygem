#ifndef MOUSYGEM__SSL_CONTEXT_HPP
#define MOUSYGEM__SSL_CONTEXT_HPP

#include <openssl/ssl.h>

namespace Mousygem {
    /**
     * SSL context
     */
    class SSLContext {
    public:
        /**
         * Get the context
         * @return context
         */
        SSL_CTX *get_context() noexcept {
            return context;
        }
        
        SSLContext();
        SSLContext(SSLContext &&) noexcept;
        ~SSLContext() noexcept;
        
    private:
        SSL_CTX *context = nullptr;
    };
}

#endif
