#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace lz {

enum class Algorithm : uint8_t {
    LZ77 = 1,
    Huffman = 2,
    Deflate = 3,
};

// Streaming compress/decompress API.
// Reads entire input, compresses/decompresses, writes to output.
// Returns the number of bytes written.
size_t stream_compress(std::istream& in, std::ostream& out, Algorithm algo);
size_t stream_decompress(std::istream& in, std::ostream& out);

// Utility to read all bytes from a stream.
std::vector<uint8_t> read_all(std::istream& in);

} // namespace lz
