#include "deflate.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Original tests ---- */

static void test_roundtrip_simple(void) {
    const char *str = "ABCABCABCABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_longer_text(void) {
    const char *str =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "Pack my box with five dozen liquor jugs.";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_longer_text\n");
}

static void test_compresses_repeated(void) {
    uint8_t input[2000];
    memset(input, 'X', 2000);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 2000, &compressed, &comp_len) == 0);
    assert(comp_len < 2000);

    free(compressed);
    printf("  PASS: compresses_repeated\n");
}

/* ---- New tests: error handling ---- */

static void test_compress_null_out(void) {
    uint8_t data[] = {1, 2, 3};
    size_t out_len = 0;
    assert(deflate_compress(data, 3, NULL, &out_len) == -1);
    printf("  PASS: compress_null_out\n");
}

static void test_compress_null_out_len(void) {
    uint8_t data[] = {1, 2, 3};
    uint8_t *out = NULL;
    assert(deflate_compress(data, 3, &out, NULL) == -1);
    printf("  PASS: compress_null_out_len\n");
}

static void test_decompress_null_out(void) {
    uint8_t data[10] = {0};
    size_t out_len = 0;
    assert(deflate_decompress(data, 10, NULL, &out_len) == -1);
    printf("  PASS: decompress_null_out\n");
}

static void test_decompress_null_out_len(void) {
    uint8_t data[10] = {0};
    uint8_t *out = NULL;
    assert(deflate_decompress(data, 10, &out, NULL) == -1);
    printf("  PASS: decompress_null_out_len\n");
}

static void test_decompress_invalid_data(void) {
    /* Feed garbage data */
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t *out = NULL;
    size_t out_len = 0;
    /* This should fail because huffman_decompress will fail on garbage */
    assert(deflate_decompress(data, 4, &out, &out_len) == -1);
    printf("  PASS: decompress_invalid_data\n");
}

/* ---- New tests: single byte ---- */

static void test_roundtrip_single_byte(void) {
    uint8_t input[] = {'Z'};

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 1, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 1);
    assert(decompressed[0] == 'Z');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_byte\n");
}

/* ---- New tests: two bytes ---- */

static void test_roundtrip_two_bytes(void) {
    uint8_t input[] = {'H', 'i'};

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 2, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 2);
    assert(decompressed[0] == 'H');
    assert(decompressed[1] == 'i');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_two_bytes\n");
}

/* ---- New tests: binary data ---- */

static void test_roundtrip_binary(void) {
    uint8_t input[256];
    for (int i = 0; i < 256; i++) input[i] = (uint8_t)i;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 256, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 256);
    assert(memcmp(decompressed, input, 256) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary\n");
}

/* ---- New tests: binary data with nulls ---- */

static void test_roundtrip_binary_nulls(void) {
    uint8_t input[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};
    size_t input_len = sizeof(input);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary_nulls\n");
}

/* ---- New tests: highly repetitive ---- */

static void test_roundtrip_highly_repetitive(void) {
    size_t len = 5000;
    uint8_t *input = malloc(len);
    assert(input);
    memset(input, 'A', len);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, len, &compressed, &comp_len) == 0);
    assert(comp_len < len);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == len);
    assert(memcmp(decompressed, input, len) == 0);

    free(input);
    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_highly_repetitive\n");
}

/* ---- New tests: alternating pattern ---- */

static void test_roundtrip_alternating(void) {
    uint8_t input[200];
    for (int i = 0; i < 200; i++) input[i] = (uint8_t)('A' + (i % 3));

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 200, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 200);
    assert(memcmp(decompressed, input, 200) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_alternating\n");
}

/* ---- New tests: all same char repeated char (exercises single-symbol tree) ---- */

static void test_roundtrip_single_char_repeated(void) {
    uint8_t input[100];
    memset(input, 0xFF, 100);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 100, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 100);
    assert(memcmp(decompressed, input, 100) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_char_repeated\n");
}

/* ---- New tests: realistic text with sentence structure ---- */

static void test_roundtrip_english_text(void) {
    const char *str =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_english_text\n");
}

int main(void) {
    printf("Deflate tests:\n");
    /* Original tests */
    test_roundtrip_simple();
    test_roundtrip_empty();
    test_roundtrip_longer_text();
    test_compresses_repeated();

    /* New error-handling tests */
    test_compress_null_out();
    test_compress_null_out_len();
    test_decompress_null_out();
    test_decompress_null_out_len();
    test_decompress_invalid_data();

    /* New roundtrip tests */
    test_roundtrip_single_byte();
    test_roundtrip_two_bytes();
    test_roundtrip_binary();
    test_roundtrip_binary_nulls();
    test_roundtrip_highly_repetitive();
    test_roundtrip_alternating();
    test_roundtrip_single_char_repeated();
    test_roundtrip_english_text();

    printf("All Deflate tests passed.\n\n");
    return 0;
}
