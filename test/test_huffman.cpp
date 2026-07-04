#include <catch2/catch_test_macros.hpp>
#include "huffman.h"
#include <string>

using namespace lz;

static std::vector<uint8_t> to_bytes(const std::string& s) {
    return {s.begin(), s.end()};
}

TEST_CASE("Huffman roundtrip on simple string", "[huffman]") {
    Huffman huff;
    auto input = to_bytes("hello world, this is a huffman test!");
    auto compressed = huff.compress(input);
    auto decompressed = huff.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Huffman roundtrip on empty input", "[huffman]") {
    Huffman huff;
    std::vector<uint8_t> input;
    auto compressed = huff.compress(input);
    auto decompressed = huff.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Huffman roundtrip on single byte", "[huffman]") {
    Huffman huff;
    auto input = to_bytes("A");
    auto compressed = huff.compress(input);
    auto decompressed = huff.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Huffman roundtrip on repeated single char", "[huffman]") {
    Huffman huff;
    std::string repeated(200, 'Q');
    auto input = to_bytes(repeated);
    auto compressed = huff.compress(input);
    auto decompressed = huff.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Huffman roundtrip on binary data", "[huffman]") {
    Huffman huff;
    std::vector<uint8_t> input;
    for (int i = 0; i < 256; ++i) {
        input.push_back(static_cast<uint8_t>(i));
    }
    auto compressed = huff.compress(input);
    auto decompressed = huff.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Huffman compresses skewed data", "[huffman]") {
    Huffman huff;
    // Mostly 'a' with a few other chars - should compress well
    std::string skewed(1000, 'a');
    skewed[100] = 'b';
    skewed[500] = 'c';
    auto input = to_bytes(skewed);
    auto compressed = huff.compress(input);
    REQUIRE(compressed.size() < input.size());
}
