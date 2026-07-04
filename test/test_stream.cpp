#include <catch2/catch_test_macros.hpp>
#include "stream.h"
#include <sstream>
#include <string>

using namespace lz;

TEST_CASE("Stream compress/decompress LZ77", "[stream]") {
    std::string text = "Hello hello hello world world world!";
    std::istringstream in(text);
    std::ostringstream compressed_out;

    stream_compress(in, compressed_out, Algorithm::LZ77);

    std::istringstream compressed_in(compressed_out.str());
    std::ostringstream decompressed_out;

    stream_decompress(compressed_in, decompressed_out);
    REQUIRE(decompressed_out.str() == text);
}

TEST_CASE("Stream compress/decompress Huffman", "[stream]") {
    std::string text = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::istringstream in(text);
    std::ostringstream compressed_out;

    stream_compress(in, compressed_out, Algorithm::Huffman);

    std::istringstream compressed_in(compressed_out.str());
    std::ostringstream decompressed_out;

    stream_decompress(compressed_in, decompressed_out);
    REQUIRE(decompressed_out.str() == text);
}

TEST_CASE("Stream compress/decompress Deflate", "[stream]") {
    std::string text = "DEFLATE combines LZ77 and Huffman. "
                       "DEFLATE combines LZ77 and Huffman.";
    std::istringstream in(text);
    std::ostringstream compressed_out;

    stream_compress(in, compressed_out, Algorithm::Deflate);

    std::istringstream compressed_in(compressed_out.str());
    std::ostringstream decompressed_out;

    stream_decompress(compressed_in, decompressed_out);
    REQUIRE(decompressed_out.str() == text);
}

TEST_CASE("Stream compress reports size", "[stream]") {
    std::string text = "Test data for size reporting.";
    std::istringstream in(text);
    std::ostringstream out;

    size_t written = stream_compress(in, out, Algorithm::Huffman);
    REQUIRE(written > 0);
    REQUIRE(written == out.str().size());
}

TEST_CASE("read_all helper", "[stream]") {
    std::string text = "some binary data \x00\x01\x02";
    std::istringstream in(text);
    auto data = read_all(in);
    REQUIRE(data.size() == text.size());
}
