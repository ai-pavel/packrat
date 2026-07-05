#include "lz77.h"
#include <stdlib.h>
#include <string.h>

LZ77Config lz77_default_config(void) {
    LZ77Config cfg = {
        .window_size = LZ77_DEFAULT_WINDOW_SIZE,
        .lookahead_size = LZ77_DEFAULT_LOOKAHEAD_SIZE
    };
    return cfg;
}

int lz77_encode(const LZ77Config *cfg, const uint8_t *data, size_t data_len,
                LZ77Token **out_tokens, size_t *out_count) {
    if (!out_tokens || !out_count) return -1;

    *out_tokens = NULL;
    *out_count = 0;

    if (data_len == 0) return 0;

    /* Allocate a generous initial buffer for tokens. */
    size_t capacity = data_len; /* worst case: one token per byte */
    LZ77Token *tokens = malloc(capacity * sizeof(LZ77Token));
    if (!tokens) return -1;

    size_t count = 0;
    size_t pos = 0;

    uint16_t window_size = cfg ? cfg->window_size : LZ77_DEFAULT_WINDOW_SIZE;
    uint16_t lookahead_size = cfg ? cfg->lookahead_size : LZ77_DEFAULT_LOOKAHEAD_SIZE;

    while (pos < data_len) {
        uint16_t best_offset = 0;
        uint16_t best_length = 0;

        size_t search_start = (pos > window_size) ? pos - window_size : 0;
        size_t lookahead_end = pos + lookahead_size;
        if (lookahead_end > data_len) lookahead_end = data_len;

        for (size_t i = search_start; i < pos; i++) {
            uint16_t match_len = 0;
            while (pos + match_len < lookahead_end &&
                   data[i + match_len] == data[pos + match_len]) {
                match_len++;
                if (match_len >= (uint16_t)(lookahead_size - 1)) break;
            }
            if (match_len > best_length) {
                best_length = match_len;
                best_offset = (uint16_t)(pos - i);
            }
        }

        /* Ensure there is always a valid next_char byte after the match.
           If the match extends to the very end of data, shorten it by one
           so the last byte becomes next_char instead of being lost. */
        if (best_length > 0 && pos + best_length >= data_len) {
            best_length = (uint16_t)(data_len - pos - 1);
        }

        uint8_t next = 0;
        if (pos + best_length < data_len) {
            next = data[pos + best_length];
        }

        tokens[count].offset = best_offset;
        tokens[count].length = best_length;
        tokens[count].next_char = next;
        count++;

        pos += best_length + 1;
    }

    *out_tokens = tokens;
    *out_count = count;
    return 0;
}

int lz77_decode(const LZ77Config *cfg, const LZ77Token *tokens, size_t count,
                uint8_t **out_data, size_t *out_len) {
    (void)cfg;
    if (!out_data || !out_len) return -1;

    *out_data = NULL;
    *out_len = 0;

    if (count == 0) return 0;

    size_t capacity = 256;
    size_t len = 0;
    uint8_t *result = malloc(capacity);
    if (!result) return -1;

    for (size_t i = 0; i < count; i++) {
        /* Ensure capacity */
        size_t needed = len + tokens[i].length + 1;
        if (needed > capacity) {
            while (capacity < needed) capacity *= 2;
            uint8_t *tmp = realloc(result, capacity);
            if (!tmp) { free(result); return -1; }
            result = tmp;
        }

        if (tokens[i].length > 0) {
            if (tokens[i].offset > len) {
                free(result);
                return -1; /* invalid back-reference */
            }
            size_t start = len - tokens[i].offset;
            for (uint16_t j = 0; j < tokens[i].length; j++) {
                result[len++] = result[start + j];
            }
        }
        result[len++] = tokens[i].next_char;
    }

    *out_data = result;
    *out_len = len;
    return 0;
}

int lz77_serialize(const LZ77Token *tokens, size_t count,
                   uint8_t **out, size_t *out_len) {
    if (!out || !out_len) return -1;

    /* Format: [4-byte token count][5 bytes per token: offset(2) + length(2) + next(1)] */
    size_t total = 4 + count * 5;
    uint8_t *buf = malloc(total);
    if (!buf) return -1;

    uint32_t cnt = (uint32_t)count;
    memcpy(buf, &cnt, 4);
    for (size_t i = 0; i < count; i++) {
        size_t base = 4 + i * 5;
        memcpy(buf + base, &tokens[i].offset, 2);
        memcpy(buf + base + 2, &tokens[i].length, 2);
        buf[base + 4] = tokens[i].next_char;
    }

    *out = buf;
    *out_len = total;
    return 0;
}

int lz77_deserialize(const uint8_t *data, size_t data_len,
                     LZ77Token **out_tokens, size_t *out_count) {
    if (!out_tokens || !out_count) return -1;
    if (data_len < 4) return -1;

    uint32_t count;
    memcpy(&count, data, 4);

    if (data_len < 4 + (size_t)count * 5) return -1;

    LZ77Token *tokens = malloc(count * sizeof(LZ77Token));
    if (!tokens && count > 0) return -1;

    for (uint32_t i = 0; i < count; i++) {
        size_t base = 4 + i * 5;
        memcpy(&tokens[i].offset, data + base, 2);
        memcpy(&tokens[i].length, data + base + 2, 2);
        tokens[i].next_char = data[base + 4];
    }

    *out_tokens = tokens;
    *out_count = count;
    return 0;
}

int lz77_compress(const uint8_t *data, size_t data_len,
                  uint8_t **out, size_t *out_len) {
    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    size_t count = 0;

    if (lz77_encode(&cfg, data, data_len, &tokens, &count) != 0)
        return -1;

    int rc = lz77_serialize(tokens, count, out, out_len);
    free(tokens);
    return rc;
}

int lz77_decompress(const uint8_t *data, size_t data_len,
                    uint8_t **out, size_t *out_len) {
    LZ77Token *tokens = NULL;
    size_t count = 0;

    if (lz77_deserialize(data, data_len, &tokens, &count) != 0)
        return -1;

    LZ77Config cfg = lz77_default_config();
    int rc = lz77_decode(&cfg, tokens, count, out, out_len);
    free(tokens);
    return rc;
}
