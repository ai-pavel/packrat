# lz-compressor

[![CI](https://github.com/ai-pavel/lz-compressor/actions/workflows/ci.yml/badge.svg)](https://github.com/ai-pavel/lz-compressor/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/ai-pavel/lz-compressor/branch/main/graph/badge.svg)](https://codecov.io/gh/ai-pavel/lz-compressor)

A C17 compression library implementing LZ77, Huffman coding, and DEFLATE (combining both).

## Building

```bash
mkdir -p build && cd build
cmake ..
make
```

## Usage

### CLI Tool

```bash
# Compress a file (default: DEFLATE)
./lz-compressor compress input.txt output.lzc

# Compress with a specific algorithm
./lz-compressor compress --lz77 input.txt output.lzc
./lz-compressor compress --huffman input.txt output.lzc
./lz-compressor compress --deflate input.txt output.lzc

# Decompress (algorithm is auto-detected from the file header)
./lz-compressor decompress output.lzc restored.txt
```

### Library API

```c
#include "stream.h"

/* Streaming API using FILE* */
FILE *fin = fopen("input.txt", "rb");
FILE *fout = fopen("output.lzc", "wb");
stream_compress(fin, fout, ALGO_DEFLATE);
fclose(fin);
fclose(fout);

/* Direct buffer API */
#include "deflate.h"
uint8_t *compressed = NULL;
size_t comp_len = 0;
deflate_compress(data, data_len, &compressed, &comp_len);

uint8_t *original = NULL;
size_t orig_len = 0;
deflate_decompress(compressed, comp_len, &original, &orig_len);

free(compressed);
free(original);
```

## Algorithms

- **LZ77**: Sliding-window dictionary compression. Finds repeated byte sequences and encodes them as back-references (offset, length, next literal).
- **Huffman**: Entropy coding that assigns shorter bit sequences to more frequent bytes.
- **DEFLATE**: Two-stage pipeline that first applies LZ77, then Huffman codes the result.

## Testing

```bash
cd build
ctest --output-on-failure
```

## Project Structure

```
src/
  lz77.c/.h       - LZ77 encoder/decoder
  huffman.c/.h     - Huffman encoder/decoder
  deflate.c/.h     - DEFLATE (LZ77 + Huffman)
  stream.c/.h      - Streaming API for FILE*
  main.c           - CLI tool
test/
  test_lz77.c      - LZ77 unit tests
  test_huffman.c   - Huffman unit tests
  test_deflate.c   - DEFLATE unit tests
  test_stream.c    - Streaming API tests
```
