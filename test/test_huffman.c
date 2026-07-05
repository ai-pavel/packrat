#include "huffman.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Original tests ---- */

static void test_roundtrip_simple(void) {
    const char *str = "hello world, this is a huffman test!";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_single_byte(void) {
    uint8_t input[] = {'A'};

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 1, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 1);
    assert(decompressed[0] == 'A');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_byte\n");
}

static void test_roundtrip_repeated_char(void) {
    uint8_t input[200];
    memset(input, 'Q', 200);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 200, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 200);
    assert(memcmp(decompressed, input, 200) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_repeated_char\n");
}

static void test_roundtrip_binary(void) {
    uint8_t input[256];
    for (int i = 0; i < 256; i++) input[i] = (uint8_t)i;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 256, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 256);
    assert(memcmp(decompressed, input, 256) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary\n");
}

static void test_compresses_skewed(void) {
    uint8_t input[1000];
    memset(input, 'a', 1000);
    input[100] = 'b';
    input[500] = 'c';

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 1000, &compressed, &comp_len) == 0);
    assert(comp_len < 1000);

    free(compressed);
    printf("  PASS: compresses_skewed\n");
}

/* ---- New tests: error handling ---- */

static void test_compress_null_out(void) {
    uint8_t data[] = {1, 2, 3};
    size_t out_len = 0;
    assert(huffman_compress(data, 3, NULL, &out_len) == -1);
    printf("  PASS: compress_null_out\n");
}

static void test_compress_null_out_len(void) {
    uint8_t data[] = {1, 2, 3};
    uint8_t *out = NULL;
    assert(huffman_compress(data, 3, &out, NULL) == -1);
    printf("  PASS: compress_null_out_len\n");
}

static void test_decompress_null_out(void) {
    uint8_t data[10] = {0};
    size_t out_len = 0;
    assert(huffman_decompress(data, 10, NULL, &out_len) == -1);
    printf("  PASS: decompress_null_out\n");
}

static void test_decompress_null_out_len(void) {
    uint8_t data[10] = {0};
    uint8_t *out = NULL;
    assert(huffman_decompress(data, 10, &out, NULL) == -1);
    printf("  PASS: decompress_null_out_len\n");
}

static void test_decompress_too_short(void) {
    uint8_t data[3] = {1, 2, 3};
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(huffman_decompress(data, 3, &out, &out_len) == -1);
    printf("  PASS: decompress_too_short\n");
}

static void test_decompress_header_only(void) {
    /* 4 bytes for orig_size but not enough for padding+tree_size (needs 7 total) */
    uint8_t data[5] = {0};
    uint32_t orig = 100;
    memcpy(data, &orig, 4);
    data[4] = 0; /* padding */
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(huffman_decompress(data, 5, &out, &out_len) == -1);
    printf("  PASS: decompress_header_only\n");
}

static void test_decompress_zero_orig_size(void) {
    /* Original size is 0 in header */
    uint8_t data[4] = {0, 0, 0, 0};
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(huffman_decompress(data, 4, &out, &out_len) == 0);
    assert(out_len == 0);
    assert(out == NULL);
    printf("  PASS: decompress_zero_orig_size\n");
}

/* ---- New tests: two bytes ---- */

static void test_roundtrip_two_bytes(void) {
    uint8_t input[] = {'X', 'Y'};

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 2, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 2);
    assert(decompressed[0] == 'X');
    assert(decompressed[1] == 'Y');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_two_bytes\n");
}

/* ---- New tests: two distinct bytes ---- */

static void test_roundtrip_two_distinct(void) {
    uint8_t input[] = {'A', 'B', 'A', 'B', 'A'};
    size_t input_len = 5;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_two_distinct\n");
}

/* ---- New tests: large data with many distinct bytes ---- */

static void test_roundtrip_large_diverse(void) {
    size_t len = 5000;
    uint8_t *input = malloc(len);
    assert(input);
    for (size_t i = 0; i < len; i++) input[i] = (uint8_t)(i * 37 + 13);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == len);
    assert(memcmp(decompressed, input, len) == 0);

    free(input);
    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_large_diverse\n");
}

/* ---- New tests: data with all 256 byte values repeated ---- */

static void test_roundtrip_all_256_repeated(void) {
    size_t len = 256 * 10;
    uint8_t *input = malloc(len);
    assert(input);
    for (size_t i = 0; i < len; i++) input[i] = (uint8_t)(i % 256);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == len);
    assert(memcmp(decompressed, input, len) == 0);

    free(input);
    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_all_256_repeated\n");
}

/* ---- New tests: binary data with nulls ---- */

static void test_roundtrip_binary_nulls(void) {
    uint8_t input[] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    size_t input_len = sizeof(input);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary_nulls\n");
}

/* ---- New tests: compression header format validation ---- */

static void test_compress_empty_produces_minimal_header(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(NULL, 0, &compressed, &comp_len) == 0);
    assert(comp_len == 4);
    /* Should be 4 zero bytes */
    uint32_t orig = 0;
    assert(memcmp(compressed, &orig, 4) == 0);

    free(compressed);
    printf("  PASS: compress_empty_produces_minimal_header\n");
}

/* ---- New tests: highly skewed distribution (exercises tree building) ---- */

static void test_roundtrip_highly_skewed(void) {
    /* 999 'a's and 1 'b' */
    size_t len = 1000;
    uint8_t *input = malloc(len);
    assert(input);
    memset(input, 'a', len);
    input[500] = 'b';

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == len);
    assert(memcmp(decompressed, input, len) == 0);

    /* Should achieve compression */
    assert(comp_len < len);

    free(input);
    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_highly_skewed\n");
}

/* ---- New tests: three distinct bytes ---- */

static void test_roundtrip_three_bytes(void) {
    const char *str = "aabbcc";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_three_bytes\n");
}

int main(void) {
    printf("Huffman tests:\n");
    /* Original tests */
    test_roundtrip_simple();
    test_roundtrip_empty();
    test_roundtrip_single_byte();
    test_roundtrip_repeated_char();
    test_roundtrip_binary();
    test_compresses_skewed();

    /* New error-handling tests */
    test_compress_null_out();
    test_compress_null_out_len();
    test_decompress_null_out();
    test_decompress_null_out_len();
    test_decompress_too_short();
    test_decompress_header_only();
    test_decompress_zero_orig_size();

    /* New roundtrip tests */
    test_roundtrip_two_bytes();
    test_roundtrip_two_distinct();
    test_roundtrip_large_diverse();
    test_roundtrip_all_256_repeated();
    test_roundtrip_binary_nulls();
    test_compress_empty_produces_minimal_header();
    test_roundtrip_highly_skewed();
    test_roundtrip_three_bytes();

    printf("All Huffman tests passed.\n\n");
    return 0;
}
