#pragma once

#include <cstdint>
#include <vector>

namespace lz {

// DEFLATE-style compression: LZ77 followed by Huffman coding.
class Deflate {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) const;
};

} // namespace lz
