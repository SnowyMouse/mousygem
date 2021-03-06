#ifndef MOUSYGEM__RESPONSE_HPP
#define MOUSYGEM__RESPONSE_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <istream>
#include <fstream>
#include <variant>
#include <optional>

namespace Mousygem {
    class Server;
    
    /**
     * Response class
     */
    class Response {
        friend class Server;
    public:
        /**
         * Input codes
         */
        enum ResponseCode {
            /** Request input - meta is a user readable prompt */
            Input = 10,
            
            /** Request input - meta is a user readable prompt - client should not display password onscreen */
            SensitiveInput = 11,
            
            /** Send data - type and language are in meta, and data can be sent after the meta */
            Success = 20,
            
            /** Redirect to a given page - URL is in the response */
            Redirect = 30,
            
            /** Redirect to a given page - URL is in the response */
            RedirectPermanent = 31,
            
            /** An error occurred - meta is a user readable error */
            TemporaryFailure = 40,
            
            /** Server is not available - meta is a user readable error */
            ServerUnavailable = 41,
            
            /** Dynamic page generation error - meta is a user readable error */
            CGIError = 42,
            
            /** Proxy request failed - meta is a user-readable error */
            ProxyError = 43,
            
            /** Rate limited - meta is an integer of the seconds the user should wait before trying again */
            SlowDown = 44,
            
            /** An error ocurred - meta is a user readable error */
            PermanentFailure = 50,
            
            /** File not found - meta is a user readable error */
            NotFound = 51,
            
            /** File not found and will not be available again - meta is a user readable error */
            Gone = 52,
            
            /** Server does not accept proxy requests - meta is a user readable error */
            ProxyRequestRefused = 53,
            
            /** Malformed client request - meta is a user readable error */
            BadRequest = 59,
            
            /** The client needs a certificate - meta is a user readable error */
            CertificateRequired = 60,
            
            /** The client's certificate is not authorized - meta is a user readable error */
            CertificateNotAuthorised = 61,
            
            /** The client's certificate is invalid - meta is a user readable error */
            CertificateNotValid = 62
        };
        
        /**
         * Construct a response without data.
         * @param code response code to send
         * @param meta meta to send
         */
        Response(ResponseCode code, const std::string &meta) :
            code(code), meta(meta) {}
        
        /**
         * Construct a response with data. This should only be used with response 2X codes.
         * @param code response code to send
         * @param meta meta to send
         * @param data data to send after the response
         */
        Response(ResponseCode code, const std::string &meta, const std::vector<std::byte> &data) :
            code(code), meta(meta), data(data) {}
        
        /**
         * Construct a response with a string. This should only be used with response 2X codes.
         * @param code response code to send
         * @param meta meta to send
         * @param data data to send after the response
         */
        Response(ResponseCode code, const std::string &meta, const std::string &data) :
            Response(code, meta, std::vector<std::byte>(reinterpret_cast<const std::byte *>(data.data()), reinterpret_cast<const std::byte *>(data.data()) + data.size())) {}
        
        /**
         * Construct a response, moving a byte vector of data to prevent copying. This should only be used with response 2X codes.
         * @param code response code to send
         * @param meta meta to send
         * @param data data to send after the response
         */
        Response(ResponseCode code, const std::string &meta, std::vector<std::byte> &&data) :
            code(code), meta(meta), data(std::move(data)) {}
        
        /**
         * Construct a response, using a file stream. This should only be used with response 2X codes.
         * @param code response code to send
         * @param meta meta to send
         * @param data data to stream
         */
        Response(ResponseCode code, const std::string &meta, std::ifstream &&data) :
            code(code), meta(meta), data(std::move(data)) {}
        
        /**
         * Set the response code
         * @param code response code to send
         */
        void set_code(ResponseCode code) noexcept {
            this->code = code;
        }
        
        /**
         * Get the response code
         * @return response code
         */
        ResponseCode get_code() const noexcept {
            return this->code;
        }
        
        /**
         * Set the meta
         * @param meta meta to send
         */
        void set_meta(const std::string &meta) {
            this->meta = meta;
        }
        
        /**
         * Get the meta
         * @return meta to send
         */
        const std::string &get_meta() const noexcept {
            return this->meta;
        }
        
        /**
         * Set the data. This should only be used with response 2X codes.
         * @param data data to send
         */
        void set_data(const std::vector<std::byte> &data) {
            this->data = data;
        }
        
        /**
         * Set the data, moving a byte vector of data to prevent copying. This should only be used with response 2X codes.
         * @param data data to send
         */
        void set_data(std::vector<std::byte> &&data) {
            this->data = std::move(data);
        }
        
        /**
         * Set the data to a file stream. This should only be used with response 2X codes.
         * @param data data to send
         */
        void set_data(std::ifstream &&data) {
            this->data = std::move(data);
        }
        
        /**
         * Clear the data
         */
        void clear_data() noexcept {
            this->data = std::nullopt;
        }
        
        /**
         * Check if we're sending data
         * @return true if we have data
         */
        bool has_data() const noexcept {
            return data.has_value();
        }
        
    private:
        /** Response code */
        ResponseCode code;
        
        /** Meta */
        std::string meta;
        
        /** Data we're sending */
        std::optional<std::variant<std::vector<std::byte>, std::ifstream>> data;
    };
}

#endif
