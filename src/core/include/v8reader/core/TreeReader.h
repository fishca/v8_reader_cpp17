#pragma once

#include "Types.h"
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace v8::core {

enum class TreeNodeType {
    Empty,
    String,
    Number,
    Guid,
    List
};

struct TreeNode {
    String value;
    TreeNodeType type{TreeNodeType::Empty};
    TreeNode* parent{nullptr};
    size_t sibling_index{0};
    std::vector<std::unique_ptr<TreeNode>> children;

    TreeNode* subnode(int index) const;
    int subnodeCount() const;
    TreeNode* next() const;
};

struct MetadataBootstrapResult {
    bool ok{false};
    int version{0};
    String metadata_guid;
    String error;
    std::unique_ptr<TreeNode> metadata_tree;
};

std::unique_ptr<TreeNode> parse1CText(const String& text);
TreeNode* findMetadataNodeByGuid(TreeNode* root, const String& target_guid);
TreeNode* getNodeByPath(TreeNode* root, const std::vector<int>& path);
std::vector<String> collectSectionObjectGuids(TreeNode* metadata_tree, const String& section_guid);
int getSectionDeclaredCount(TreeNode* metadata_tree, const String& section_guid);

MetadataBootstrapResult bootstrapMetadataTree(
    const std::function<std::optional<String>(const String&)>& getTextByName);

} // namespace v8::core
