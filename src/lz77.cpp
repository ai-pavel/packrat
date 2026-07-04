#include "lz77.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace lz {

LZ77::LZ77(uint16_t window_size, uint16_t lookahead_size)
    : window_size_(window_size), lookahead_size_(lookahead_size) {}

std::vector<LZ77Token> LZ77::encode(const std::vector<uint8_t>& data) const {
    std::vector<LZ77Token> tokens;
    if (data.empty()) return tokens;

    size_t pos = 0;
    while (pos < data.size()) {
        uint16_t best_offset = 0;
        uint16_t best_length = 0;

        size_t search_start = (pos > window_size_) ? pos - window_size_ : 0;
        size_t lookahead_end = std::min(pos + lookahead_size_, data.size());

        for (size_t i = search_start; i < pos; ++i) {
            uint16_t match_len = 0;
            while (pos + match_len < lookahead_end &&
                   data[i + match_len] == data[pos + match_len]) {
                ++match_len;
                if (match_len >= lookahead_size_ - 1) break;
            }
            if (match_len > best_length) {
                best_length = match_len;
                best_offset = static_cast<uint16_t>(pos - i);
            }
        }

        uint8_t next = 0;
        if (pos + best_length < data.size()) {
            next = data[pos + best_length];
        }

        tokens.push_back({best_offset, best_length, next});
        pos += best_length + 1;
    }

    return tokens;
}

std::vector<uint8_t> LZ77::decode(const std::vector<LZ77Token>& tokens) const {
    std::vector<uint8_t> result;

    for (const auto& token : tokens) {
        if (token.length > 0) {
            if (token.offset > result.size()) {
                throw std::runtime_error("LZ77 decode: invalid back-reference offset");
            }
            size_t start = result.size() - token.offset;
            for (uint16_t i = 0; i < token.length; ++i) {
                result.push_back(result[start + i]);
            }
        }
        // Append the literal (unless we've consumed the entire input
        // and the token was the final one with length covering the rest)
        result.push_back(token.next_char);
    }

    return result;
}

std::vector<uint8_t> LZ77::serialize(const std::vector<LZ77Token>& tokens) {
    // Format: [4-byte token count][5 bytes per token: offset(2) + length(2) + next(1)]
    std::vector<uint8_t> out;
    uint32_t count = static_cast<uint32_t>(tokens.size());
    out.resize(4 + count * 5);

    std::memcpy(out.data(), &count, 4);
    for (uint32_t i = 0; i < count; ++i) {
        size_t base = 4 + i * 5;
        std::memcpy(out.data() + base, &tokens[i].offset, 2);
        std::memcpy(out.data() + base + 2, &tokens[i].length, 2);
        out[base + 4] = tokens[i].next_char;
    }
    return out;
}

std::vector<LZ77Token> LZ77::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        throw std::runtime_error("LZ77 deserialize: data too short");
    }
    uint32_t count;
    std::memcpy(&count, data.data(), 4);

    if (data.size() < 4 + count * 5) {
        throw std::runtime_error("LZ77 deserialize: truncated data");
    }

    std::vector<LZ77Token> tokens(count);
    for (uint32_t i = 0; i < count; ++i) {
        size_t base = 4 + i * 5;
        std::memcpy(&tokens[i].offset, data.data() + base, 2);
        std::memcpy(&tokens[i].length, data.data() + base + 2, 2);
        tokens[i].next_char = data[base + 4];
    }
    return tokens;
}

std::vector<uint8_t> LZ77::compress(const std::vector<uint8_t>& data) const {
    auto tokens = encode(data);
    return serialize(tokens);
}

std::vector<uint8_t> LZ77::decompress(const std::vector<uint8_t>& data) const {
    auto tokens = deserialize(data);
    return decode(tokens);
}

} // namespace lz
