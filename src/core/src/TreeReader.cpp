#include "v8reader/core/TreeReader.h"
#include "v8reader/core/metadata/MetadataFactory.h"
#include <algorithm>
#include <cwctype>
#include <regex>
#include <unordered_set>

namespace v8::core {
namespace {
String toLowerCopy(String value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return value;
}

bool isGuidLike(const String& value) {
    if (value.size() != 36) return false;
    for (size_t i = 0; i < value.size(); ++i) {
        const bool dash = (i == 8 || i == 13 || i == 18 || i == 23);
        if (dash) {
            if (value[i] != L'-') return false;
            continue;
        }
        const wchar_t c = value[i];
        const bool hex = (c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f') || (c >= L'A' && c <= L'F');
        if (!hex) return false;
    }
    return true;
}

bool isNumberLike(const String& value) {
    if (value.empty()) return false;
    size_t i = 0;
    if (value[0] == L'-') i = 1;
    if (i >= value.size()) return false;
    for (; i < value.size(); ++i) {
        if (!(value[i] >= L'0' && value[i] <= L'9')) return false;
    }
    return true;
}
} // namespace

TreeNode* TreeNode::subnode(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= children.size()) return nullptr;
    return children[static_cast<size_t>(index)].get();
}

int TreeNode::subnodeCount() const {
    return static_cast<int>(children.size());
}

TreeNode* TreeNode::next() const {
    if (!parent) return nullptr;
    const size_t next_idx = sibling_index + 1;
    if (next_idx >= parent->children.size()) return nullptr;
    return parent->children[next_idx].get();
}

// Реализация методов TreeReader
TreeReader::TreeReader(std::unique_ptr<TreeNode> root) : m_root(std::move(root)) {}

TreeNode* TreeReader::findNodeByGuid(const String& guid) {
    return findMetadataNodeByGuid(m_root.get(), guid);
}

std::vector<TreeNode*> TreeReader::getChildNodes(TreeNode* parent) {
    std::vector<TreeNode*> result;
    if (!parent) return result;
    
    for (const auto& child : parent->children) {
        result.push_back(child.get());
    }
    return result;
}

std::unique_ptr<v8reader::core::TMDO> TreeReader::buildObjectFromNode(TreeNode* node) {
    if (!node || node->type != TreeNodeType::List || node->subnodeCount() < 2) {
        return nullptr;
    }
    
    // Первый элемент - GUID типа
    TreeNode* typeNode = node->subnode(0);
    if (!typeNode || typeNode->type != TreeNodeType::Guid) {
        return nullptr;
    }
    
    // Второй элемент - имя объекта
    TreeNode* nameNode = node->subnode(1);
    if (!nameNode) {
        return nullptr;
    }
    
    // Создаем объект через фабрику
    // В реальной реализации здесь будет маппинг GUID на тип
    auto obj = v8reader::core::MetadataFactory::createObject(typeNode->value);
    if (obj) {
        obj->setName(nameNode->value);
        obj->setGuid(typeNode->value);
    }
    
    return obj;
}

std::unique_ptr<v8reader::core::TMDO> TreeReader::buildMetadataTree() {
    if (!m_root) {
        return nullptr;
    }
    
    return buildObjectFromNode(m_root.get());
}

std::unique_ptr<TreeNode> parse1CText(const String& text) {
    auto skipWs = [&](size_t& i) {
        while (i < text.size()) {
            wchar_t ch = text[i];
            if (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' || ch == 0xFEFF) ++i;
            else break;
        }
    };

    std::function<std::unique_ptr<TreeNode>(size_t&, TreeNode*)> parseValue;
    parseValue = [&](size_t& i, TreeNode* parent) -> std::unique_ptr<TreeNode> {
        skipWs(i);
        if (i >= text.size()) return nullptr;

        if (text[i] == L'{') {
            ++i;
            auto list = std::make_unique<TreeNode>();
            list->type = TreeNodeType::List;
            list->parent = parent;
            skipWs(i);
            while (i < text.size() && text[i] != L'}') {
                auto child = parseValue(i, list.get());
                if (child) {
                    child->sibling_index = list->children.size();
                    list->children.push_back(std::move(child));
                }
                skipWs(i);
                if (i < text.size() && text[i] == L',') {
                    ++i;
                    skipWs(i);
                }
            }
            if (i < text.size() && text[i] == L'}') ++i;
            return list;
        }

        auto node = std::make_unique<TreeNode>();
        node->parent = parent;
        if (text[i] == L'"') {
            ++i;
            while (i < text.size()) {
                wchar_t ch = text[i++];
                if (ch == L'"') {
                    if (i < text.size() && text[i] == L'"') {
                        node->value.push_back(L'"');
                        ++i;
                        continue;
                    }
                    break;
                }
                node->value.push_back(ch);
            }
        } else {
            size_t start = i;
            while (i < text.size()) {
                wchar_t ch = text[i];
                if (ch == L',' || ch == L'{' || ch == L'}' || ch == L'\r' || ch == L'\n' || ch == L'\t') break;
                ++i;
            }
            while (i > start && text[i - 1] == L' ') --i;
            node->value = text.substr(start, i - start);
        }

        if (isGuidLike(node->value)) node->type = TreeNodeType::Guid;
        else if (isNumberLike(node->value)) node->type = TreeNodeType::Number;
        else node->type = TreeNodeType::String;
        return node;
    };

    size_t pos = 0;
    return parseValue(pos, nullptr);
}

TreeNode* getNodeByPath(TreeNode* root, const std::vector<int>& path) {
    TreeNode* node = root;
    for (int idx : path) {
        if (!node) return nullptr;
        node = node->subnode(idx);
    }
    return node;
}

TreeNode* findMetadataNodeByGuid(TreeNode* root, const String& target_guid) {
    if (!root) return nullptr;
    const String low_target = toLowerCopy(target_guid);

    std::function<TreeNode*(TreeNode*)> walk = [&](TreeNode* node) -> TreeNode* {
        if (!node) return nullptr;
        if (node->type == TreeNodeType::List && node->subnodeCount() >= 2) {
            TreeNode* n0 = node->subnode(0);
            TreeNode* n1 = node->subnode(1);
            if (n0 && n1 && n0->type == TreeNodeType::Guid && n1->type == TreeNodeType::Number) {
                if (toLowerCopy(n0->value) == low_target) return n0;
            }
        }
        for (int i = 0; i < node->subnodeCount(); ++i) {
            if (TreeNode* res = walk(node->subnode(i))) return res;
        }
        return nullptr;
    };

    return walk(root);
}

std::vector<String> collectSectionObjectGuids(TreeNode* metadata_tree, const String& section_guid) {
    std::vector<String> out;
    TreeNode* node_md = findMetadataNodeByGuid(metadata_tree, section_guid);
    if (!node_md) return out;

    TreeNode* countNode = node_md->next();
    if (!countNode || countNode->type != TreeNodeType::Number) return out;
    int count = 0;
    try { count = std::stoi(countNode->value); } catch (...) { return out; }

    std::unordered_set<String> uniq;
    TreeNode* cur = countNode;
    for (int i = 0; i < count; ++i) {
        cur = cur ? cur->next() : nullptr;
        if (!cur) break;
        if (cur->type == TreeNodeType::Guid && isGuidLike(cur->value)) {
            String g = toLowerCopy(cur->value);
            if (uniq.insert(g).second) out.push_back(g);
        }
    }
    return out;
}

int getSectionDeclaredCount(TreeNode* metadata_tree, const String& section_guid) {
    TreeNode* node_md = findMetadataNodeByGuid(metadata_tree, section_guid);
    if (!node_md) return 0;
    TreeNode* countNode = node_md->next();
    if (!countNode || countNode->type != TreeNodeType::Number) return 0;
    try {
        return std::max(0, std::stoi(countNode->value));
    } catch (...) {
        return 0;
    }
}

MetadataBootstrapResult bootstrapMetadataTree(
    const std::function<std::optional<String>(const String&)>& getTextByName) {
    MetadataBootstrapResult out;
    // Version is optional for metadata bootstrap in this port.
    // We still try to parse it for diagnostics, but never fail bootstrap on it.
    const auto versionText = getTextByName(L"version");
    if (versionText) {
        auto versionTree = parse1CText(*versionText);
        TreeNode* verNode = versionTree ? getNodeByPath(versionTree.get(), {0, 0, 0}) : nullptr;
        if (verNode && verNode->type == TreeNodeType::Number) {
            try { out.version = std::stoi(verNode->value); }
            catch (...) { out.version = 0; }
        }
        if (out.version <= 0) {
            static const std::wregex rgxNum(LR"((-?\d+))");
            std::wsmatch m;
            if (std::regex_search(*versionText, m, rgxNum) && m.size() >= 2) {
                try { out.version = std::stoi(m[1].str()); }
                catch (...) { out.version = 0; }
            }
        }
    }

    const auto rootText = getTextByName(L"root");
    if (!rootText) {
        out.error = L"root text not found";
        return out;
    }

    // Original v8_reader root format is usually: {2,<metadata-guid>,}
    // Parse this form first, then fallback to a generic GUID search.
    static const std::wregex rootGuidPattern(
        LR"(\{\s*-?\d+\s*,\s*([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\s*,)",
        std::regex::ECMAScript);
    std::wsmatch m;
    if (std::regex_search(*rootText, m, rootGuidPattern) && m.size() >= 2) {
        out.metadata_guid = toLowerCopy(m[1].str());
    } else {
        static const std::wregex guidPattern(
            LR"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))",
            std::regex::ECMAScript);
        if (std::regex_search(*rootText, m, guidPattern) && m.size() >= 2) {
            out.metadata_guid = toLowerCopy(m[1].str());
        }
    }

    if (out.metadata_guid.empty()) {
        out.error = L"root metadata guid not found";
        return out;
    }
    if (!isGuidLike(out.metadata_guid)) {
        out.error = L"root metadata guid invalid";
        return out;
    }

    const auto metaText = getTextByName(out.metadata_guid);
    if (!metaText) {
        out.error = L"metadata file text not found";
        return out;
    }
    out.metadata_tree = parse1CText(*metaText);
    if (!out.metadata_tree) {
        out.error = L"metadata parse failed";
        return out;
    }
    out.ok = true;
    return out;
}

} // namespace v8::core
