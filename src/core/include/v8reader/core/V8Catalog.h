#pragma once

#include "v8reader/types/v8_types.h"
#include <QString>
#include <QVector>
#include <memory>

namespace v8reader::core {

/**
 * @brief Элемент каталога метаданных 1С
 * 
 * Представляет узел в иерархии метаданных конфигурации.
 * Используется для навигации по дереву объектов.
 */
class V8CatalogItem {
public:
    explicit V8CatalogItem(const QString& name, const QString& uuid = {});
    virtual ~V8CatalogItem() = default;

    [[nodiscard]] const QString& getName() const { return m_name; }
    [[nodiscard]] const QString& getUuid() const { return m_uuid; }
    [[nodiscard]] const QVector<std::shared_ptr<V8CatalogItem>>& getChildren() const { return m_children; }
    [[nodiscard]] std::shared_ptr<V8CatalogItem> getParent() const { return m_parent.lock(); }

    void addChild(std::shared_ptr<V8CatalogItem> child);
    void setParent(std::weak_ptr<V8CatalogItem> parent);

    /**
     * @brief Найти дочерний элемент по имени
     * @param name Имя элемента
     * @return Умный указатель на найденный элемент или nullptr
     */
    [[nodiscard]] std::shared_ptr<V8CatalogItem> findChild(const QString& name) const;

    /**
     * @brief Найти элемент по полному пути (например, "Catalogs/Products")
     * @param path Путь к элементу
     * @return Умный указатель на найденный элемент или nullptr
     */
    [[nodiscard]] std::shared_ptr<V8CatalogItem> findByPath(const QString& path) const;

private:
    QString m_name;
    QString m_uuid;
    QVector<std::shared_ptr<V8CatalogItem>> m_children;
    std::weak_ptr<V8CatalogItem> m_parent;
};

} // namespace v8reader::core
