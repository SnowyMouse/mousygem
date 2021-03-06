#ifndef MOUSYGEM__URI_HPP
#define MOUSYGEM__URI_HPP

#include <optional>
#include <string>

namespace Mousygem {
    class URI {
    public:
        /**
         * Construct a URI
         * @param uri_string input string
         * @throws invalid_argument if URI is invalid
         */
        URI(const std::string &uri_string);
        
        /**
         * Construct a URI
         * @param uri_string input string
         * @return reference to URI
         * @throws invalid_argument if URI is invalid
         */
        URI &operator =(const std::string &uri_string);
        
        /**
         * Convert the URI to a C string. The string will only be valid for as long as the URI exists and is unmodified. If the string must exist longer than this instance, use URI::string() instead.
         * @return C string
         */
        const char *c_str() const noexcept {
            return this->data.c_str();
        }
        
        /**
         * Convert the URI to a C++ string.
         * @return string
         */
        std::string string() const {
            return this->data;
        }
        
        /**
         * Get the hostname
         * @return hostname
         */
        std::string hostname() const;
        
        /**
         * Get the port
         * @return port
         */
        std::optional<std::uint16_t> port() const;
        
        /**
         * Get the protocol
         * @return protocol
         */
        std::string protocol() const;
        
        /**
         * Get the input
         * @return input string
         */
        std::optional<std::string> input() const;
        
        /**
         * Get the path
         * @return path string
         */
        std::string path() const;
        
        URI(const URI &) = default;
        URI &operator =(const URI &) = default;
        URI(URI &&) = default;
        
        bool operator ==(const URI &other) const noexcept {
            return this->data == other.data;
        }
        
        bool operator !=(const URI &other) const noexcept {
            return !(*this == other);
        }
        
        bool operator ==(const std::string &other) const noexcept {
            return this->data == other;
        }
        
        bool operator !=(const std::string &other) const noexcept {
            return !(*this == other);
        }
        
    private:
        std::string data;
        
        /** Find the hostname */
        std::size_t hostname_offset() const;
        
        /** Find the port if it exists. Throw if the port is out of range or contains non-numerical characters */
        std::optional<std::size_t> port_offset() const;
        
        /** Find the path */
        std::size_t path_offset() const;
        
        /** Find the input */
        std::optional<std::size_t> input_offset() const;
        
        void validate();
    };
    
    static inline std::ostream &operator<<(std::ostream &stream, const URI &uri) { 
        return stream << uri.string();
    }
}

#endif
