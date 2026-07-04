#include "stream.h"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

static void print_usage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " compress   [--lz77|--huffman|--deflate] <input> <output>\n"
              << "  " << prog << " decompress <input> <output>\n"
              << "\nDefault algorithm: --deflate\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "compress") {
        lz::Algorithm algo = lz::Algorithm::Deflate;
        int file_arg_start = 2;

        if (argc >= 5) {
            std::string algo_str = argv[2];
            if (algo_str == "--lz77") {
                algo = lz::Algorithm::LZ77;
                file_arg_start = 3;
            } else if (algo_str == "--huffman") {
                algo = lz::Algorithm::Huffman;
                file_arg_start = 3;
            } else if (algo_str == "--deflate") {
                algo = lz::Algorithm::Deflate;
                file_arg_start = 3;
            }
        }

        if (file_arg_start + 1 >= argc) {
            print_usage(argv[0]);
            return 1;
        }

        const char* input_file = argv[file_arg_start];
        const char* output_file = argv[file_arg_start + 1];

        std::ifstream fin(input_file, std::ios::binary);
        if (!fin) {
            std::cerr << "Error: cannot open input file: " << input_file << "\n";
            return 1;
        }

        // Get input size
        fin.seekg(0, std::ios::end);
        auto input_size = fin.tellg();
        fin.seekg(0, std::ios::beg);

        std::ofstream fout(output_file, std::ios::binary);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << "\n";
            return 1;
        }

        try {
            size_t output_size = lz::stream_compress(fin, fout, algo);
            double ratio = (input_size > 0)
                ? static_cast<double>(output_size) / static_cast<double>(input_size) * 100.0
                : 0.0;

            std::cout << "Compressed " << input_size << " -> " << output_size
                      << " bytes (" << std::fixed << std::setprecision(1)
                      << ratio << "% of original)\n";
        } catch (const std::exception& e) {
            std::cerr << "Compression error: " << e.what() << "\n";
            return 1;
        }

    } else if (mode == "decompress") {
        if (argc < 4) {
            print_usage(argv[0]);
            return 1;
        }

        const char* input_file = argv[2];
        const char* output_file = argv[3];

        std::ifstream fin(input_file, std::ios::binary);
        if (!fin) {
            std::cerr << "Error: cannot open input file: " << input_file << "\n";
            return 1;
        }

        fin.seekg(0, std::ios::end);
        auto input_size = fin.tellg();
        fin.seekg(0, std::ios::beg);

        std::ofstream fout(output_file, std::ios::binary);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << "\n";
            return 1;
        }

        try {
            size_t output_size = lz::stream_decompress(fin, fout);
            std::cout << "Decompressed " << input_size << " -> " << output_size
                      << " bytes\n";
        } catch (const std::exception& e) {
            std::cerr << "Decompression error: " << e.what() << "\n";
            return 1;
        }

    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
