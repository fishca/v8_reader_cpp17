#pragma once

#include "v8reader/core/Types.h"
#include "v8reader/core/V8Container.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>

namespace v8reader::core {

/**
 * @brief Класс для построения дерева метаданных из сырых данных V8.
 * 
 * Инкапсулирует сложную логику парсинга, декодирования текстов и восстановления
 * иерархии объектов метаданных. Вынесен из V8Container для улучшения читаемости
 * и тестируемости кода.
 */
class MetadataTreeBuilder {
public:
    using String = core::String;
    using MetadataItemPtr = std::shared_ptr<MetadataItem>;
    
    explicit MetadataTreeBuilder(const V8Container* container);
    ~MetadataTreeBuilder() = default;

    /**
     * @brief Построить полное дерево метаданных.
     * @return Уникальный указатель на корневой элемент дерева.
     */
    MetadataItemPtr build();

private:
    // --- Данные ---
    const V8Container* m_container;
    
    // Кэш элементов по имени (lower-case) для быстрого доступа
    std::unordered_map<String, const V8Element*> m_elementsByNameLower;
    
    // Карта GUID -> Элемент метаданных (для связи родителей и детей)
    std::unordered_map<String, MetadataItem*> m_guidMap;
    
    // Текст метаданных (распарсенное дерево)
    std::unique_ptr<TreeNode> m_metadataTree;
    String m_metadataText;
    String m_metadataGuid;

    // --- Структуры данных ---
    struct SectionInfo {
        String guid;
        String title;
    };
    
    struct BootstrapResult {
        bool ok{false};
        String error;
        String metadata_guid;
        String version;
        std::unique_ptr<TreeNode> metadata_tree;
    };

    // --- Основные этапы построения ---
    
    /**
     * @brief Инициализация: индексация элементов, bootstrap метаданных.
     */
    void initialize();

    /**
     * @brief Построение всех секций верхнего уровня.
     */
    void buildSections(MetadataItemPtr root);

    /**
     * @brief Обработка объекта: создание элемента и рекурсивная обработка детей.
     */
    void processObject(
        MetadataItemPtr parent,
        const String& objectGuid,
        const String& sectionGuid);

    // --- Вспомогательные методы ---

    /**
     * @brief Bootstrap: поиск основного объекта метаданных.
     */
    BootstrapResult bootstrapMetadata();

    /**
     * @brief Получить текст по имени файла из контейнера.
     */
    std::optional<String> getTextByName(const String& fileName) const;

    /**
     * @brief Декодировать текст из элемента V8.
     */
    std::optional<String> decodeText(const V8Element& elem, bool requireBraces) const;

    /**
     * @brief Собрать GUID объектов секции через regex (fallback).
     */
    std::vector<String> collectSectionGuidsRegex(const String& sectionGuid) const;

    /**
     * @brief Проверить наличие хранилища объектов для GUID.
     */
    bool hasObjectStorage(const String& guid) const;

    /**
     * @brief Попытка восстановить GUID метаданных перебором.
     */
    String guessMetadataGuid() const;

    /**
     * @brief Создать fallback-дерево из сырых элементов контейнера.
     */
    MetadataItemPtr createRawFallback() const;

    /**
     * @brief Преобразование строки в нижний регистр.
     */
    static String toLower(String value);

    /**
     * @brief Проверка, является ли строка GUID.
     */
    static bool isGuidLike(const String& value);
    
    // --- Константы ---
    static const std::vector<SectionInfo> kSections;
    static const std::unordered_map<String, std::vector<int>> kNamePaths;
};

} // namespace v8reader::core
