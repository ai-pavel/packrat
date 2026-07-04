#include <catch2/catch_test_macros.hpp>
#include "deflate.h"
#include <string>

using namespace lz;

static std::vector<uint8_t> to_bytes(const std::string& s) {
    return {s.begin(), s.end()};
}

TEST_CASE("Deflate roundtrip on simple string", "[deflate]") {
    Deflate defl;
    auto input = to_bytes("ABCABCABCABCABC");
    auto compressed = defl.compress(input);
    auto decompressed = defl.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Deflate roundtrip on empty input", "[deflate]") {
    Deflate defl;
    std::vector<uint8_t> input;
    auto compressed = defl.compress(input);
    auto decompressed = defl.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Deflate roundtrip on longer text", "[deflate]") {
    Deflate defl;
    std::string text =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "Pack my box with five dozen liquor jugs.";
    auto input = to_bytes(text);
    auto compressed = defl.compress(input);
    auto decompressed = defl.decompress(compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("Deflate compresses repeated data", "[deflate]") {
    Deflate defl;
    std::string repeated(2000, 'X');
    auto input = to_bytes(repeated);
    auto compressed = defl.compress(input);
    REQUIRE(compressed.size() < input.size());
}
