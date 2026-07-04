#include <catch2/catch_test_macros.hpp>
#include "lz77.h"
#include <string>

using namespace lz;

static std::vector<uint8_t> to_bytes(const std::string& s) {
    return {s.begin(), s.end()};
}

static std::string to_string(const std::vector<uint8_t>& v) {
    return {v.begin(), v.end()};
}

TEST_CASE("LZ77 roundtrip on simple string", "[lz77]") {
    LZ77 lz;
    auto input = to_bytes("AABABCABCABC");
    auto compressed = lz.compress(input);
    auto decompressed = lz.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("LZ77 roundtrip on repeated data", "[lz77]") {
    LZ77 lz;
    std::string repeated(500, 'X');
    auto input = to_bytes(repeated);
    auto compressed = lz.compress(input);
    auto decompressed = lz.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("LZ77 roundtrip on empty input", "[lz77]") {
    LZ77 lz;
    std::vector<uint8_t> input;
    auto compressed = lz.compress(input);
    auto decompressed = lz.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("LZ77 roundtrip on single byte", "[lz77]") {
    LZ77 lz;
    auto input = to_bytes("A");
    auto compressed = lz.compress(input);
    auto decompressed = lz.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("LZ77 compression actually reduces repeated data", "[lz77]") {
    LZ77 lz;
    std::string repeated(1000, 'Z');
    auto input = to_bytes(repeated);
    auto compressed = lz.compress(input);
    REQUIRE(compressed.size() < input.size());
}

TEST_CASE("LZ77 encode/decode token level", "[lz77]") {
    LZ77 lz;
    auto input = to_bytes("ABCABC");
    auto tokens = lz.encode(input);
    auto decoded = lz.decode(tokens);
    REQUIRE(to_string(decoded) == "ABCABC");
}
