#include "deflate.h"
#include "huffman.h"
#include "lz77.h"

namespace lz {

std::vector<uint8_t> Deflate::compress(const std::vector<uint8_t>& data) const {
    // Stage 1: LZ77 compression
    LZ77 lz;
    auto lz_compressed = lz.compress(data);

    // Stage 2: Huffman coding on the LZ77 output
    Huffman huff;
    return huff.compress(lz_compressed);
}

std::vector<uint8_t> Deflate::decompress(const std::vector<uint8_t>& data) const {
    // Stage 1: Huffman decode
    Huffman huff;
    auto lz_compressed = huff.decompress(data);

    // Stage 2: LZ77 decode
    LZ77 lz;
    return lz.decompress(lz_compressed);
}

} // namespace lz
