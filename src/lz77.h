#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace lz {

struct LZ77Token {
    uint16_t offset;   // distance back in the window
    uint16_t length;   // match length
    uint8_t next_char; // literal following the match
};

class LZ77 {
public:
    explicit LZ77(uint16_t window_size = 4096, uint16_t lookahead_size = 18);

    std::vector<LZ77Token> encode(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decode(const std::vector<LZ77Token>& tokens) const;

    // Serialize tokens to bytes and back
    static std::vector<uint8_t> serialize(const std::vector<LZ77Token>& tokens);
    static std::vector<LZ77Token> deserialize(const std::vector<uint8_t>& data);

    // Convenience: compress/decompress raw byte buffers
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) const;

private:
    uint16_t window_size_;
    uint16_t lookahead_size_;
};

} // namespace lz
