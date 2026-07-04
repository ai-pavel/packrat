#include "stream.h"
#include "deflate.h"
#include "huffman.h"
#include "lz77.h"
#include <cstring>
#include <stdexcept>

namespace lz {

static const uint8_t MAGIC[4] = {'L', 'Z', 'C', 'M'};

std::vector<uint8_t> read_all(std::istream& in) {
    std::vector<uint8_t> data;
    char buf[8192];
    while (in.read(buf, sizeof(buf))) {
        data.insert(data.end(), buf, buf + in.gcount());
    }
    if (in.gcount() > 0) {
        data.insert(data.end(), buf, buf + in.gcount());
    }
    return data;
}

size_t stream_compress(std::istream& in, std::ostream& out, Algorithm algo) {
    auto input = read_all(in);

    std::vector<uint8_t> compressed;
    switch (algo) {
        case Algorithm::LZ77: {
            LZ77 lz;
            compressed = lz.compress(input);
            break;
        }
        case Algorithm::Huffman: {
            Huffman huff;
            compressed = huff.compress(input);
            break;
        }
        case Algorithm::Deflate: {
            Deflate defl;
            compressed = defl.compress(input);
            break;
        }
        default:
            throw std::runtime_error("Unknown algorithm");
    }

    // Write header: magic(4) + algorithm(1) + original_size(4)
    out.write(reinterpret_cast<const char*>(MAGIC), 4);
    uint8_t algo_byte = static_cast<uint8_t>(algo);
    out.write(reinterpret_cast<const char*>(&algo_byte), 1);
    uint32_t orig_size = static_cast<uint32_t>(input.size());
    out.write(reinterpret_cast<const char*>(&orig_size), 4);

    // Write compressed data
    out.write(reinterpret_cast<const char*>(compressed.data()),
              static_cast<std::streamsize>(compressed.size()));

    return 9 + compressed.size();
}

size_t stream_decompress(std::istream& in, std::ostream& out) {
    auto data = read_all(in);

    if (data.size() < 9) {
        throw std::runtime_error("Invalid compressed file: too short");
    }

    if (std::memcmp(data.data(), MAGIC, 4) != 0) {
        throw std::runtime_error("Invalid compressed file: bad magic");
    }

    auto algo = static_cast<Algorithm>(data[4]);
    uint32_t orig_size;
    std::memcpy(&orig_size, data.data() + 5, 4);

    std::vector<uint8_t> compressed(data.begin() + 9, data.end());
    std::vector<uint8_t> decompressed;

    switch (algo) {
        case Algorithm::LZ77: {
            LZ77 lz;
            decompressed = lz.decompress(compressed);
            break;
        }
        case Algorithm::Huffman: {
            Huffman huff;
            decompressed = huff.decompress(compressed);
            break;
        }
        case Algorithm::Deflate: {
            Deflate defl;
            decompressed = defl.decompress(compressed);
            break;
        }
        default:
            throw std::runtime_error("Unknown algorithm in file");
    }

    out.write(reinterpret_cast<const char*>(decompressed.data()),
              static_cast<std::streamsize>(decompressed.size()));

    return decompressed.size();
}

} // namespace lz
