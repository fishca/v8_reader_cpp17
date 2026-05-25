#pragma once

#include "v8reader/core/Types.h"
#include "v8reader/core/metadata/TMDO.h"
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace v8reader::core {

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

/**
 * @brief Улучшенный парсер дерева метаданных с поддержкой умных указателей
 */
class TreeReader {
public:
    explicit TreeReader(std::unique_ptr<TreeNode> root);
    
    /**
     * @brief Построить иерархию объектов метаданных
     * @return Корневой объект метаданных
     */
    std::unique_ptr<v8reader::core::TMDO> buildMetadataTree();
    
    /**
     * @brief Найти узел по GUID
     */
    TreeNode* findNodeByGuid(const String& guid);
    
    /**
     * @brief Получить все дочерние узлы
     */
    std::vector<TreeNode*> getChildNodes(TreeNode* parent);
    
private:
    std::unique_ptr<TreeNode> m_root;
    
    /**
     * @brief Рекурсивное построение объекта из узла
     */
    std::unique_ptr<v8reader::core::TMDO> buildObjectFromNode(TreeNode* node);
};

} // namespace v8reader::core
