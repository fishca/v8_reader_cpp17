#include "v8reader/core/V8Catalog.h"

namespace v8reader::core {

V8CatalogItem::V8CatalogItem(const QString& name, const QString& uuid)
    : m_name(name)
    , m_uuid(uuid)
{
}

void V8CatalogItem::addChild(std::shared_ptr<V8CatalogItem> child) {
    if (child) {
        child->setParent(shared_from_this());
        m_children.append(child);
    }
}

void V8CatalogItem::setParent(std::weak_ptr<V8CatalogItem> parent) {
    m_parent = std::move(parent);
}

std::shared_ptr<V8CatalogItem> V8CatalogItem::findChild(const QString& name) const {
    for (const auto& child : m_children) {
        if (child->getName() == name) {
            return child;
        }
    }
    return nullptr;
}

std::shared_ptr<V8CatalogItem> V8CatalogItem::findByPath(const QString& path) const {
    const auto parts = path.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return nullptr;
    }

    std::shared_ptr<V8CatalogItem> current = const_cast<V8CatalogItem*>(this)->shared_from_this();
    
    for (const auto& part : parts) {
        if (!current) {
            return nullptr;
        }
        current = current->findChild(part);
    }
    
    return current;
}

} // namespace v8reader::core
