#include "stream.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper: compress a string through FILE streams and decompress, verify roundtrip. */
static void test_stream_roundtrip(const char *label, const char *text, Algorithm algo) {
    size_t text_len = strlen(text);

    /* Write text to a temp file */
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    fwrite(text, 1, text_len, tmp_in);
    rewind(tmp_in);

    /* Compress to another temp file */
    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, algo);
    assert(comp_size > 0);
    fclose(tmp_in);

    /* Decompress */
    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);

    assert(decomp_size == (long)text_len);

    /* Read back and compare */
    rewind(tmp_out);
    char *result = malloc(text_len + 1);
    assert(result);
    size_t n = fread(result, 1, text_len, tmp_out);
    assert(n == text_len);
    result[text_len] = '\0';
    fclose(tmp_out);

    assert(memcmp(result, text, text_len) == 0);
    free(result);

    printf("  PASS: %s\n", label);
}

static void test_read_all_helper(void) {
    const char *text = "some binary data";
    size_t text_len = strlen(text);

    FILE *f = tmpfile();
    assert(f);
    fwrite(text, 1, text_len, f);
    rewind(f);

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(read_all(f, &data, &data_len) == 0);
    fclose(f);

    assert(data_len == text_len);
    assert(memcmp(data, text, text_len) == 0);
    free(data);

    printf("  PASS: read_all_helper\n");
}

/* ---- New tests: read_all error handling ---- */

static void test_read_all_null_out(void) {
    FILE *f = tmpfile();
    assert(f);
    size_t out_len = 0;
    assert(read_all(f, NULL, &out_len) == -1);
    fclose(f);
    printf("  PASS: read_all_null_out\n");
}

static void test_read_all_null_out_len(void) {
    FILE *f = tmpfile();
    assert(f);
    uint8_t *out = NULL;
    assert(read_all(f, &out, NULL) == -1);
    fclose(f);
    printf("  PASS: read_all_null_out_len\n");
}

static void test_read_all_empty_file(void) {
    FILE *f = tmpfile();
    assert(f);
    /* Don't write anything -- empty file */

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(read_all(f, &data, &data_len) == 0);
    assert(data_len == 0);
    fclose(f);
    free(data);
    printf("  PASS: read_all_empty_file\n");
}

static void test_read_all_large_data(void) {
    /* Test with data larger than initial 8192 capacity to trigger realloc */
    size_t len = 20000;
    uint8_t *input = malloc(len);
    assert(input);
    for (size_t i = 0; i < len; i++) input[i] = (uint8_t)(i % 256);

    FILE *f = tmpfile();
    assert(f);
    fwrite(input, 1, len, f);
    rewind(f);

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(read_all(f, &data, &data_len) == 0);
    assert(data_len == len);
    assert(memcmp(data, input, len) == 0);
    fclose(f);

    free(input);
    free(data);
    printf("  PASS: read_all_large_data\n");
}

/* ---- New tests: stream_decompress error handling ---- */

static void test_decompress_too_short(void) {
    /* Less than 9 bytes -- should fail */
    FILE *f = tmpfile();
    assert(f);
    uint8_t data[] = {1, 2, 3, 4, 5};
    fwrite(data, 1, 5, f);
    rewind(f);

    FILE *fout = tmpfile();
    assert(fout);
    long result = stream_decompress(f, fout);
    assert(result == -1);
    fclose(f);
    fclose(fout);
    printf("  PASS: decompress_too_short\n");
}

static void test_decompress_bad_magic(void) {
    /* 9+ bytes but wrong magic */
    FILE *f = tmpfile();
    assert(f);
    uint8_t data[12] = {0};
    data[0] = 'X'; /* wrong magic */
    data[1] = 'Y';
    data[2] = 'Z';
    data[3] = 'W';
    data[4] = 1; /* algo */
    fwrite(data, 1, 12, f);
    rewind(f);

    FILE *fout = tmpfile();
    assert(fout);
    long result = stream_decompress(f, fout);
    assert(result == -1);
    fclose(f);
    fclose(fout);
    printf("  PASS: decompress_bad_magic\n");
}

static void test_decompress_invalid_algo(void) {
    /* Correct magic but invalid algorithm byte */
    FILE *f = tmpfile();
    assert(f);
    uint8_t data[12] = {0};
    data[0] = 'L';
    data[1] = 'Z';
    data[2] = 'C';
    data[3] = 'M';
    data[4] = 99; /* invalid algo */
    fwrite(data, 1, 12, f);
    rewind(f);

    FILE *fout = tmpfile();
    assert(fout);
    long result = stream_decompress(f, fout);
    assert(result == -1);
    fclose(f);
    fclose(fout);
    printf("  PASS: decompress_invalid_algo\n");
}

static void test_compress_invalid_algo(void) {
    FILE *fin = tmpfile();
    assert(fin);
    const char *text = "hello";
    fwrite(text, 1, 5, fin);
    rewind(fin);

    FILE *fout = tmpfile();
    assert(fout);
    long result = stream_compress(fin, fout, (Algorithm)99);
    assert(result == -1);
    fclose(fin);
    fclose(fout);
    printf("  PASS: compress_invalid_algo\n");
}

/* ---- New tests: stream roundtrip with various data types ---- */

static void test_stream_roundtrip_empty_lz77(void) {
    /* Empty input through stream */
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    /* Write nothing */

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_LZ77);
    fclose(tmp_in);
    assert(comp_size > 0);

    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);
    assert(decomp_size == 0);
    fclose(tmp_out);

    printf("  PASS: stream_roundtrip_empty_lz77\n");
}

static void test_stream_roundtrip_empty_huffman(void) {
    FILE *tmp_in = tmpfile();
    assert(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_HUFFMAN);
    fclose(tmp_in);
    assert(comp_size > 0);

    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);
    assert(decomp_size == 0);
    fclose(tmp_out);

    printf("  PASS: stream_roundtrip_empty_huffman\n");
}

static void test_stream_roundtrip_empty_deflate(void) {
    FILE *tmp_in = tmpfile();
    assert(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_DEFLATE);
    fclose(tmp_in);
    assert(comp_size > 0);

    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);
    assert(decomp_size == 0);
    fclose(tmp_out);

    printf("  PASS: stream_roundtrip_empty_deflate\n");
}

static void test_stream_roundtrip_single_byte(void) {
    test_stream_roundtrip("stream_single_byte_lz77", "X", ALGO_LZ77);
    test_stream_roundtrip("stream_single_byte_huffman", "Y", ALGO_HUFFMAN);
    test_stream_roundtrip("stream_single_byte_deflate", "Z", ALGO_DEFLATE);
}

static void test_stream_roundtrip_repeated(void) {
    char input[501];
    memset(input, 'R', 500);
    input[500] = '\0';
    test_stream_roundtrip("stream_repeated_lz77", input, ALGO_LZ77);
    test_stream_roundtrip("stream_repeated_huffman", input, ALGO_HUFFMAN);
    test_stream_roundtrip("stream_repeated_deflate", input, ALGO_DEFLATE);
}

static void test_stream_roundtrip_long_text(void) {
    const char *text =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "Pack my box with five dozen liquor jugs.";
    test_stream_roundtrip("stream_long_lz77", text, ALGO_LZ77);
    test_stream_roundtrip("stream_long_huffman", text, ALGO_HUFFMAN);
    test_stream_roundtrip("stream_long_deflate", text, ALGO_DEFLATE);
}

/* ---- New tests: stream with binary data ---- */

static void test_stream_binary_roundtrip(void) {
    /* Create binary data and write through stream */
    size_t len = 512;
    uint8_t *input = malloc(len);
    assert(input);
    for (size_t i = 0; i < len; i++) input[i] = (uint8_t)(i % 256);

    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    fwrite(input, 1, len, tmp_in);
    rewind(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_DEFLATE);
    assert(comp_size > 0);
    fclose(tmp_in);

    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);
    assert(decomp_size == (long)len);

    rewind(tmp_out);
    uint8_t *result = malloc(len);
    assert(result);
    size_t n = fread(result, 1, len, tmp_out);
    assert(n == len);
    fclose(tmp_out);

    assert(memcmp(result, input, len) == 0);

    free(input);
    free(result);
    printf("  PASS: stream_binary_roundtrip\n");
}

/* ---- New tests: compressed output contains header ---- */

static void test_stream_compress_header_format(void) {
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    const char *text = "test data";
    fwrite(text, 1, strlen(text), tmp_in);
    rewind(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_LZ77);
    assert(comp_size > 0);
    fclose(tmp_in);

    /* Verify header: LZCM magic + algo byte + orig_size */
    rewind(tmp_comp);
    uint8_t header[9];
    size_t n = fread(header, 1, 9, tmp_comp);
    assert(n == 9);
    assert(header[0] == 'L');
    assert(header[1] == 'Z');
    assert(header[2] == 'C');
    assert(header[3] == 'M');
    assert(header[4] == ALGO_LZ77);

    uint32_t orig_size;
    memcpy(&orig_size, header + 5, 4);
    assert(orig_size == strlen(text));

    fclose(tmp_comp);
    printf("  PASS: stream_compress_header_format\n");
}

static void test_stream_compress_header_huffman(void) {
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    const char *text = "huffman header test";
    fwrite(text, 1, strlen(text), tmp_in);
    rewind(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_HUFFMAN);
    assert(comp_size > 0);
    fclose(tmp_in);

    rewind(tmp_comp);
    uint8_t header[9];
    size_t n = fread(header, 1, 9, tmp_comp);
    assert(n == 9);
    assert(header[0] == 'L' && header[1] == 'Z' && header[2] == 'C' && header[3] == 'M');
    assert(header[4] == ALGO_HUFFMAN);

    fclose(tmp_comp);
    printf("  PASS: stream_compress_header_huffman\n");
}

static void test_stream_compress_header_deflate(void) {
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    const char *text = "deflate header test";
    fwrite(text, 1, strlen(text), tmp_in);
    rewind(tmp_in);

    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, ALGO_DEFLATE);
    assert(comp_size > 0);
    fclose(tmp_in);

    rewind(tmp_comp);
    uint8_t header[9];
    size_t n = fread(header, 1, 9, tmp_comp);
    assert(n == 9);
    assert(header[0] == 'L' && header[1] == 'Z' && header[2] == 'C' && header[3] == 'M');
    assert(header[4] == ALGO_DEFLATE);

    fclose(tmp_comp);
    printf("  PASS: stream_compress_header_deflate\n");
}

int main(void) {
    printf("Stream tests:\n");
    /* Original tests */
    test_stream_roundtrip("stream_lz77",
        "Hello hello hello world world world!", ALGO_LZ77);
    test_stream_roundtrip("stream_huffman",
        "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ", ALGO_HUFFMAN);
    test_stream_roundtrip("stream_deflate",
        "DEFLATE combines LZ77 and Huffman. DEFLATE combines LZ77 and Huffman.",
        ALGO_DEFLATE);
    test_read_all_helper();

    /* New read_all tests */
    test_read_all_null_out();
    test_read_all_null_out_len();
    test_read_all_empty_file();
    test_read_all_large_data();

    /* New error-handling tests */
    test_decompress_too_short();
    test_decompress_bad_magic();
    test_decompress_invalid_algo();
    test_compress_invalid_algo();

    /* New roundtrip tests with empty data */
    test_stream_roundtrip_empty_lz77();
    test_stream_roundtrip_empty_huffman();
    test_stream_roundtrip_empty_deflate();

    /* New roundtrip tests with various data */
    test_stream_roundtrip_single_byte();
    test_stream_roundtrip_repeated();
    test_stream_roundtrip_long_text();
    test_stream_binary_roundtrip();

    /* New header format tests */
    test_stream_compress_header_format();
    test_stream_compress_header_huffman();
    test_stream_compress_header_deflate();

    printf("All Stream tests passed.\n\n");
    return 0;
}
