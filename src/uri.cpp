#include <mousygem/uri.hpp>
#include <cstring>
#include <stdexcept>

namespace Mousygem {
    URI::URI(const std::string &uri_string) : data(uri_string) {
        try {
            this->validate();
        }
        catch(std::exception &) {
            throw std::invalid_argument(uri_string + " is not a valid URI");
        }
    }
    
    URI &URI::operator =(const std::string &uri_string) {
        return (*this = URI(uri_string));
    }
    
    void URI::validate() {
        // this->hostname_offset(); // called inside port_offset() anyway
        this->port_offset();
    }
    
    static constexpr const char colon_slash_slash[] = "://";
    
    std::size_t URI::hostname_offset() const {
        auto colon_slash_slash_offset = this->data.find(colon_slash_slash);
        if(colon_slash_slash_offset == std::string::npos) {
            throw std::exception();
        }
        return colon_slash_slash_offset + sizeof(colon_slash_slash) - 1;
    }
    
    std::size_t URI::path_offset() const {
        auto forward_slash_offset = this->data.find_first_of('/', this->hostname_offset());
        if(forward_slash_offset == std::string::npos) {
            return this->data.size();
        }
        
        return forward_slash_offset;
    }
    
    std::optional<std::size_t> URI::port_offset() const {
        std::size_t start = this->hostname_offset(), end = this->path_offset();
        
        // Empty hostname
        if(start == end) {
            return std::nullopt;
        }
        
        // We can't just look for a colon right off the bat because IPv6 is a thing >.>
        const auto *str = this->c_str();
        
        if(str[start] == '[') {
            start = this->data.find_first_of(']', start); // find the matching ']'
            if(start == std::string::npos) {
                throw std::exception();
            }
        }
        
        // Now we can look for a colon
        auto colon = this->data.find_first_of(':', start);
        if(colon == std::string::npos || colon >= end) {
            return std::nullopt;
        }
        
        // We have a port
        auto port_start = colon + 1;
        const char *port_start_str = str + port_start;
        
        // Too bad it's invalid
        if(port_start == end) {
            throw std::exception();
        }
        
        // Check the first character
        if(*port_start_str < '0' || *port_start_str > '9') {
            throw std::exception();
        }
        
        // Is it out of range or a valid number?
        const char *port_end_str;
        if(std::strtol(port_start_str, const_cast<char **>(&port_end_str), 10) > UINT16_MAX) {
            throw std::exception();
        }
        
        // Is the end of the number string expected?
        if(port_start_str == port_end_str || port_end_str != str + end) {
            throw std::exception();
        }
        
        return port_start;
    }
    
    std::optional<std::size_t> URI::input_offset() const {
        auto question_mark_offset = this->data.find_first_of('?', this->path_offset());
        if(question_mark_offset == std::string::npos) {
            return std::nullopt;
        }
        
        return question_mark_offset + 1;
    }
    
    std::optional<std::uint16_t> URI::port() const {
        auto port_offset = this->port_offset();
        if(!port_offset.has_value()) {
            return std::nullopt;
        }
        
        return std::strtoul(this->c_str() + *port_offset, nullptr, 10);
    }
    
    std::string URI::protocol() const {
        return this->data.substr(0, this->hostname_offset() - (sizeof(colon_slash_slash) - 1));
    }
    
    static std::string decode_percent_encoding(std::string input) {
        for(std::size_t i = 0; i + 2 < input.size(); i++) {
            if(input[i] == '%') {
                auto to_hex_upper = [](char input) -> std::optional<int> {
                    if(input >= '0' && input <= '9') {
                        return input - '0';
                    }
                    if(input >= 'a' && input <= 'f') {
                        return input - 'a' + 10;
                    }
                    if(input >= 'A' && input <= 'F') {
                        return input - 'A' + 10;
                    }
                    
                    return std::nullopt;
                };
                
                auto d1 = to_hex_upper(input[i + 1]);
                auto d2 = to_hex_upper(input[i + 2]);
                
                // Skip if no hex character for both
                if(!d1.has_value() || !d2.has_value()) {
                    continue;
                }
                
                // Otherwise replace the % with the character and delete the two memes after it
                input[i] = (*d1 * 0x10) | *d2;
                input.erase(i + 1, 2);
            }
        }
        return input;
    }
    
    std::string URI::hostname() const {
        auto port_offset = this->port_offset();
        auto hostname_offset = this->hostname_offset();
        auto path_offset = this->path_offset();
        
        // If we have a port, go until that
        if(port_offset.has_value()) {
            return decode_percent_encoding(this->data.substr(hostname_offset, (*port_offset - 1) - hostname_offset));
        }
        else {
            return decode_percent_encoding(this->data.substr(hostname_offset, path_offset - hostname_offset));
        }
    }
    
    std::optional<std::string> URI::input() const {
        auto input_offset = this->input_offset();
        if(!input_offset.has_value()) {
            return std::nullopt;
        }
        return decode_percent_encoding(this->data.substr(*input_offset));
    }
    
    std::string URI::path() const {
        auto path_offset = this->path_offset();
        auto input_offset = this->input_offset();
        if(input_offset.has_value()) {
            return decode_percent_encoding(this->data.substr(path_offset, (*input_offset - 1) - path_offset));
        }
        
        return decode_percent_encoding(this->data.substr(path_offset));
    }
}
