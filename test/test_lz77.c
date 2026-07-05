#include "lz77.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Roundtrip tests ---- */

static void test_roundtrip_simple(void) {
    const char *str = "AABABCABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_repeated(void) {
    uint8_t input[500];
    memset(input, 'X', 500);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 500, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == 500);
    assert(memcmp(decompressed, input, 500) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_repeated\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_single_byte(void) {
    uint8_t input[] = {'A'};
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 1, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 1);
    assert(decompressed[0] == 'A');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_byte\n");
}

static void test_compression_reduces_size(void) {
    uint8_t input[1000];
    memset(input, 'Z', 1000);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 1000, &compressed, &comp_len) == 0);
    assert(comp_len < 1000);

    free(compressed);
    printf("  PASS: compression_reduces_size\n");
}

static void test_encode_decode_tokens(void) {
    const char *str = "ABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, input, input_len, &tokens, &count) == 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(&cfg, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == input_len);
    assert(memcmp(decoded, input, input_len) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: encode_decode_tokens\n");
}

/* ---- New tests: error handling ---- */

static void test_encode_null_out_tokens(void) {
    uint8_t data[] = {1, 2, 3};
    LZ77Config cfg = lz77_default_config();
    size_t count = 0;
    assert(lz77_encode(&cfg, data, 3, NULL, &count) == -1);
    printf("  PASS: encode_null_out_tokens\n");
}

static void test_encode_null_out_count(void) {
    uint8_t data[] = {1, 2, 3};
    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    assert(lz77_encode(&cfg, data, 3, &tokens, NULL) == -1);
    printf("  PASS: encode_null_out_count\n");
}

static void test_decode_null_out_data(void) {
    LZ77Config cfg = lz77_default_config();
    LZ77Token tokens[1] = {{0, 0, 'A'}};
    size_t out_len = 0;
    assert(lz77_decode(&cfg, tokens, 1, NULL, &out_len) == -1);
    printf("  PASS: decode_null_out_data\n");
}

static void test_decode_null_out_len(void) {
    LZ77Config cfg = lz77_default_config();
    LZ77Token tokens[1] = {{0, 0, 'A'}};
    uint8_t *out = NULL;
    assert(lz77_decode(&cfg, tokens, 1, &out, NULL) == -1);
    printf("  PASS: decode_null_out_len\n");
}

static void test_decode_invalid_backref(void) {
    LZ77Config cfg = lz77_default_config();
    /* Token references offset=10 but buffer only has 1 byte */
    LZ77Token tokens[2] = {
        {0, 0, 'A'},
        {10, 3, 'B'}
    };
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(lz77_decode(&cfg, tokens, 2, &out, &out_len) == -1);
    printf("  PASS: decode_invalid_backref\n");
}

static void test_decode_empty_tokens(void) {
    LZ77Config cfg = lz77_default_config();
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(lz77_decode(&cfg, NULL, 0, &out, &out_len) == 0);
    assert(out_len == 0);
    printf("  PASS: decode_empty_tokens\n");
}

static void test_encode_empty_data(void) {
    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, NULL, 0, &tokens, &count) == 0);
    assert(count == 0);
    printf("  PASS: encode_empty_data\n");
}

/* ---- New tests: encode with NULL config (uses defaults) ---- */

static void test_encode_null_config(void) {
    const char *str = "ABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(NULL, input, input_len, &tokens, &count) == 0);
    assert(count > 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(NULL, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == input_len);
    assert(memcmp(decoded, input, input_len) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: encode_null_config\n");
}

/* ---- New tests: serialize/deserialize ---- */

static void test_serialize_null_out(void) {
    LZ77Token tokens[1] = {{0, 0, 'A'}};
    size_t out_len = 0;
    assert(lz77_serialize(tokens, 1, NULL, &out_len) == -1);
    printf("  PASS: serialize_null_out\n");
}

static void test_serialize_null_out_len(void) {
    LZ77Token tokens[1] = {{0, 0, 'A'}};
    uint8_t *out = NULL;
    assert(lz77_serialize(tokens, 1, &out, NULL) == -1);
    printf("  PASS: serialize_null_out_len\n");
}

static void test_deserialize_null_out_tokens(void) {
    uint8_t data[4] = {0};
    size_t count = 0;
    assert(lz77_deserialize(data, 4, NULL, &count) == -1);
    printf("  PASS: deserialize_null_out_tokens\n");
}

static void test_deserialize_null_out_count(void) {
    uint8_t data[4] = {0};
    LZ77Token *tokens = NULL;
    assert(lz77_deserialize(data, 4, &tokens, NULL) == -1);
    printf("  PASS: deserialize_null_out_count\n");
}

static void test_deserialize_too_short(void) {
    uint8_t data[3] = {1, 2, 3};
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_deserialize(data, 3, &tokens, &count) == -1);
    printf("  PASS: deserialize_too_short\n");
}

static void test_deserialize_truncated_tokens(void) {
    /* Header says 2 tokens (requires 4 + 2*5 = 14 bytes), but only provide 10 */
    uint8_t data[10] = {0};
    uint32_t cnt = 2;
    memcpy(data, &cnt, 4);
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_deserialize(data, 10, &tokens, &count) == -1);
    printf("  PASS: deserialize_truncated_tokens\n");
}

static void test_serialize_deserialize_roundtrip(void) {
    LZ77Token tokens[3] = {
        {0, 0, 'H'},
        {0, 0, 'i'},
        {2, 1, '!'}
    };

    uint8_t *serialized = NULL;
    size_t ser_len = 0;
    assert(lz77_serialize(tokens, 3, &serialized, &ser_len) == 0);
    assert(ser_len == 4 + 3 * 5);

    LZ77Token *deser_tokens = NULL;
    size_t deser_count = 0;
    assert(lz77_deserialize(serialized, ser_len, &deser_tokens, &deser_count) == 0);
    assert(deser_count == 3);

    for (size_t i = 0; i < 3; i++) {
        assert(deser_tokens[i].offset == tokens[i].offset);
        assert(deser_tokens[i].length == tokens[i].length);
        assert(deser_tokens[i].next_char == tokens[i].next_char);
    }

    free(serialized);
    free(deser_tokens);
    printf("  PASS: serialize_deserialize_roundtrip\n");
}

static void test_serialize_empty(void) {
    uint8_t *serialized = NULL;
    size_t ser_len = 0;
    assert(lz77_serialize(NULL, 0, &serialized, &ser_len) == 0);
    assert(ser_len == 4);

    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_deserialize(serialized, ser_len, &tokens, &count) == 0);
    assert(count == 0);

    free(serialized);
    free(tokens);
    printf("  PASS: serialize_empty\n");
}

/* ---- New tests: compress/decompress wrappers ---- */

static void test_compress_null_out(void) {
    uint8_t data[] = {1, 2, 3};
    size_t out_len = 0;
    assert(lz77_compress(data, 3, NULL, &out_len) == -1);
    printf("  PASS: compress_null_out\n");
}

static void test_decompress_null_out(void) {
    uint8_t data[] = {1, 2, 3, 4};
    size_t out_len = 0;
    assert(lz77_decompress(data, 4, NULL, &out_len) == -1);
    printf("  PASS: decompress_null_out\n");
}

static void test_decompress_invalid_data(void) {
    /* Feed garbage that looks like it has many tokens but is too short */
    uint8_t data[4] = {0};
    uint32_t cnt = 999;
    memcpy(data, &cnt, 4);
    uint8_t *out = NULL;
    size_t out_len = 0;
    assert(lz77_decompress(data, 4, &out, &out_len) == -1);
    printf("  PASS: decompress_invalid_data\n");
}

/* ---- New tests: default config ---- */

static void test_default_config(void) {
    LZ77Config cfg = lz77_default_config();
    assert(cfg.window_size == LZ77_DEFAULT_WINDOW_SIZE);
    assert(cfg.lookahead_size == LZ77_DEFAULT_LOOKAHEAD_SIZE);
    printf("  PASS: default_config\n");
}

/* ---- New tests: custom config with small window ---- */

static void test_custom_config_small_window(void) {
    const char *str = "ABCDEFABCDEFABCDEF";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    LZ77Config cfg = {.window_size = 8, .lookahead_size = 4};
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, input, input_len, &tokens, &count) == 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(&cfg, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == input_len);
    assert(memcmp(decoded, input, input_len) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: custom_config_small_window\n");
}

/* ---- New tests: roundtrip with two bytes ---- */

static void test_roundtrip_two_bytes(void) {
    uint8_t input[] = {'A', 'B'};
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 2, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 2);
    assert(decompressed[0] == 'A');
    assert(decompressed[1] == 'B');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_two_bytes\n");
}

/* ---- New tests: binary data with nulls ---- */

static void test_roundtrip_binary_with_nulls(void) {
    uint8_t input[] = {0, 0, 0, 1, 2, 0, 0, 0, 1, 2, 0, 0, 0, 1, 2};
    size_t input_len = sizeof(input);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary_with_nulls\n");
}

/* ---- New tests: larger data with patterns ---- */

static void test_roundtrip_alternating_pattern(void) {
    uint8_t input[256];
    for (int i = 0; i < 256; i++) input[i] = (uint8_t)(i % 4);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 256, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 256);
    assert(memcmp(decompressed, input, 256) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_alternating_pattern\n");
}

/* ---- New tests: all distinct bytes (no matches) ---- */

static void test_roundtrip_no_matches(void) {
    uint8_t input[256];
    for (int i = 0; i < 256; i++) input[i] = (uint8_t)i;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 256, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 256);
    assert(memcmp(decompressed, input, 256) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_no_matches\n");
}

/* ---- New tests: decode growing buffer (exercises realloc path) ---- */

static void test_decode_realloc(void) {
    /* Create enough tokens to force the decode buffer to realloc multiple times.
       Start capacity is 256 in lz77_decode. */
    uint8_t input[1024];
    for (int i = 0; i < 1024; i++) input[i] = (uint8_t)('A' + (i % 26));

    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, input, 1024, &tokens, &count) == 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(&cfg, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == 1024);
    assert(memcmp(decoded, input, 1024) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: decode_realloc\n");
}

/* ---- New test: large highly-compressible data ---- */

static void test_roundtrip_large_repeated(void) {
    size_t len = 10000;
    uint8_t *input = malloc(len);
    assert(input);
    memset(input, 'Q', len);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, len, &compressed, &comp_len) == 0);
    /* Should compress significantly */
    assert(comp_len < len);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == len);
    assert(memcmp(decompressed, input, len) == 0);

    free(input);
    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_large_repeated\n");
}

/* ---- New test: data that exactly fills lookahead ---- */

static void test_exact_lookahead_boundary(void) {
    /* Create data where match length equals lookahead_size - 1 */
    LZ77Config cfg = {.window_size = 64, .lookahead_size = 8};
    uint8_t input[] = "AAAAAAAAAAAAAAAA"; /* 16 A's */
    size_t input_len = 16;

    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, input, input_len, &tokens, &count) == 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(&cfg, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == input_len);
    assert(memcmp(decoded, input, input_len) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: exact_lookahead_boundary\n");
}

int main(void) {
    printf("LZ77 tests:\n");
    /* Original tests */
    test_roundtrip_simple();
    test_roundtrip_repeated();
    test_roundtrip_empty();
    test_roundtrip_single_byte();
    test_compression_reduces_size();
    test_encode_decode_tokens();

    /* New error-handling tests */
    test_encode_null_out_tokens();
    test_encode_null_out_count();
    test_decode_null_out_data();
    test_decode_null_out_len();
    test_decode_invalid_backref();
    test_decode_empty_tokens();
    test_encode_empty_data();

    /* New config tests */
    test_encode_null_config();
    test_default_config();
    test_custom_config_small_window();

    /* New serialize/deserialize tests */
    test_serialize_null_out();
    test_serialize_null_out_len();
    test_deserialize_null_out_tokens();
    test_deserialize_null_out_count();
    test_deserialize_too_short();
    test_deserialize_truncated_tokens();
    test_serialize_deserialize_roundtrip();
    test_serialize_empty();

    /* New compress/decompress wrapper tests */
    test_compress_null_out();
    test_decompress_null_out();
    test_decompress_invalid_data();

    /* New roundtrip tests */
    test_roundtrip_two_bytes();
    test_roundtrip_binary_with_nulls();
    test_roundtrip_alternating_pattern();
    test_roundtrip_no_matches();
    test_decode_realloc();
    test_roundtrip_large_repeated();
    test_exact_lookahead_boundary();

    printf("All LZ77 tests passed.\n\n");
    return 0;
}
