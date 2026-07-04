#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lz {

class Huffman {
public:
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) const;

private:
    struct Node {
        uint8_t byte = 0;
        uint32_t freq = 0;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;

        bool is_leaf() const { return !left && !right; }
    };

    struct NodeCmp {
        bool operator()(const std::shared_ptr<Node>& a,
                        const std::shared_ptr<Node>& b) const {
            return a->freq > b->freq;
        }
    };

    static std::shared_ptr<Node> build_tree(
        const std::unordered_map<uint8_t, uint32_t>& freq);

    static void build_codes(const std::shared_ptr<Node>& node,
                            const std::string& prefix,
                            std::unordered_map<uint8_t, std::string>& codes);

    // Serialization helpers
    static void serialize_tree(const std::shared_ptr<Node>& node,
                               std::vector<uint8_t>& out);
    static std::shared_ptr<Node> deserialize_tree(const std::vector<uint8_t>& data,
                                                   size_t& pos);
};

} // namespace lz
