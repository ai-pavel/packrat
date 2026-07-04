#include "huffman.h"
#include <algorithm>
#include <cstring>
#include <queue>
#include <stdexcept>

namespace lz {

std::shared_ptr<Huffman::Node> Huffman::build_tree(
    const std::unordered_map<uint8_t, uint32_t>& freq) {
    std::priority_queue<std::shared_ptr<Node>,
                        std::vector<std::shared_ptr<Node>>, NodeCmp>
        pq;

    for (auto& [byte, count] : freq) {
        auto node = std::make_shared<Node>();
        node->byte = byte;
        node->freq = count;
        pq.push(node);
    }

    if (pq.empty()) return nullptr;

    // Handle single-symbol case
    if (pq.size() == 1) {
        auto parent = std::make_shared<Node>();
        parent->freq = pq.top()->freq;
        parent->left = pq.top();
        pq.pop();
        return parent;
    }

    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto parent = std::make_shared<Node>();
        parent->freq = left->freq + right->freq;
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }

    return pq.top();
}

void Huffman::build_codes(const std::shared_ptr<Node>& node,
                          const std::string& prefix,
                          std::unordered_map<uint8_t, std::string>& codes) {
    if (!node) return;
    if (node->is_leaf()) {
        codes[node->byte] = prefix.empty() ? "0" : prefix;
        return;
    }
    build_codes(node->left, prefix + "0", codes);
    build_codes(node->right, prefix + "1", codes);
}

void Huffman::serialize_tree(const std::shared_ptr<Node>& node,
                             std::vector<uint8_t>& out) {
    if (!node) return;
    if (node->is_leaf()) {
        out.push_back(1); // leaf marker
        out.push_back(node->byte);
    } else {
        out.push_back(0); // internal node marker
        serialize_tree(node->left, out);
        serialize_tree(node->right, out);
    }
}

std::shared_ptr<Huffman::Node> Huffman::deserialize_tree(
    const std::vector<uint8_t>& data, size_t& pos) {
    if (pos >= data.size()) {
        throw std::runtime_error("Huffman: truncated tree data");
    }
    auto node = std::make_shared<Node>();
    uint8_t marker = data[pos++];
    if (marker == 1) {
        // Leaf
        if (pos >= data.size()) {
            throw std::runtime_error("Huffman: truncated tree data");
        }
        node->byte = data[pos++];
    } else {
        // Internal
        node->left = deserialize_tree(data, pos);
        node->right = deserialize_tree(data, pos);
    }
    return node;
}

std::vector<uint8_t> Huffman::compress(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        // Return a minimal header: 0 original size
        std::vector<uint8_t> out(4, 0);
        return out;
    }

    // Build frequency table
    std::unordered_map<uint8_t, uint32_t> freq;
    for (auto b : data) freq[b]++;

    auto root = build_tree(freq);

    std::unordered_map<uint8_t, std::string> codes;
    build_codes(root, "", codes);

    // Encode data as bit string
    std::string bits;
    bits.reserve(data.size() * 4);
    for (auto b : data) {
        bits += codes[b];
    }

    // Pack bits into bytes
    std::vector<uint8_t> encoded_bytes;
    uint8_t padding = (8 - (bits.size() % 8)) % 8;
    for (uint8_t i = 0; i < padding; ++i) bits += '0';

    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; ++j) {
            byte = (byte << 1) | (bits[i + j] - '0');
        }
        encoded_bytes.push_back(byte);
    }

    // Output format:
    //   [4 bytes: original size]
    //   [1 byte: padding bits]
    //   [2 bytes: tree data size]
    //   [tree data]
    //   [encoded bytes]
    std::vector<uint8_t> tree_data;
    serialize_tree(root, tree_data);

    std::vector<uint8_t> result;
    uint32_t orig_size = static_cast<uint32_t>(data.size());
    uint16_t tree_size = static_cast<uint16_t>(tree_data.size());

    result.resize(4 + 1 + 2 + tree_data.size() + encoded_bytes.size());
    size_t pos = 0;

    std::memcpy(result.data() + pos, &orig_size, 4); pos += 4;
    result[pos++] = padding;
    std::memcpy(result.data() + pos, &tree_size, 2); pos += 2;
    std::memcpy(result.data() + pos, tree_data.data(), tree_data.size());
    pos += tree_data.size();
    std::memcpy(result.data() + pos, encoded_bytes.data(), encoded_bytes.size());

    return result;
}

std::vector<uint8_t> Huffman::decompress(const std::vector<uint8_t>& data) const {
    if (data.size() < 4) {
        throw std::runtime_error("Huffman decompress: data too short");
    }

    uint32_t orig_size;
    std::memcpy(&orig_size, data.data(), 4);
    if (orig_size == 0) return {};

    if (data.size() < 7) {
        throw std::runtime_error("Huffman decompress: data too short");
    }

    uint8_t padding = data[4];
    uint16_t tree_size;
    std::memcpy(&tree_size, data.data() + 5, 2);

    size_t pos = 7;
    auto root = deserialize_tree(data, pos);

    // Decode bits
    size_t encoded_start = 7 + tree_size;
    if (encoded_start > data.size()) {
        throw std::runtime_error("Huffman decompress: truncated data");
    }

    // Convert encoded bytes to bit string
    std::string bits;
    for (size_t i = encoded_start; i < data.size(); ++i) {
        for (int j = 7; j >= 0; --j) {
            bits += ((data[i] >> j) & 1) ? '1' : '0';
        }
    }

    // Remove padding
    if (padding > 0 && bits.size() >= padding) {
        bits.resize(bits.size() - padding);
    }

    // Traverse tree to decode
    std::vector<uint8_t> result;
    result.reserve(orig_size);
    auto node = root;

    for (char c : bits) {
        if (c == '0') {
            node = node->left;
        } else {
            node = node->right;
        }

        if (node->is_leaf()) {
            result.push_back(node->byte);
            node = root;
            if (result.size() == orig_size) break;
        }
    }

    return result;
}

} // namespace lz
