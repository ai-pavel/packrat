# lz-compressor

A C++17 compression library implementing LZ77, Huffman coding, and DEFLATE (combining both).

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

```cpp
#include "stream.h"
#include <fstream>

// Streaming API
std::ifstream fin("input.txt", std::ios::binary);
std::ofstream fout("output.lzc", std::ios::binary);
lz::stream_compress(fin, fout, lz::Algorithm::Deflate);

// Direct buffer API
#include "deflate.h"
lz::Deflate defl;
auto compressed = defl.compress(data);
auto original = defl.decompress(compressed);
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
  lz77.cpp/.h       - LZ77 encoder/decoder
  huffman.cpp/.h     - Huffman encoder/decoder
  deflate.cpp/.h     - DEFLATE (LZ77 + Huffman)
  stream.cpp/.h      - Streaming API for istream/ostream
  main.cpp           - CLI tool
test/
  test_lz77.cpp      - LZ77 unit tests
  test_huffman.cpp   - Huffman unit tests
  test_deflate.cpp   - DEFLATE unit tests
  test_stream.cpp    - Streaming API tests
```
