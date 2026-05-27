# Предложение по рефакторингу V8Container::buildMetadataTree()

## Текущие проблемы

Функция `buildMetadataTree()` (строки 663-1027 в V8Container.cpp) имеет следующие проблемы:

1. **Слишком большой размер**: ~365 строк кода в одной функции
2. **Множество встроенных лямбда-выражений**: 10+ лямбда-функций внутри метода
3. **Дублирование кода**: логика обработки секций повторяется (строки 940-975 и 986-1017)
4. **Сложная навигация**: трудно понять общий поток выполнения
5. **Низкая тестируемость**: невозможно протестировать отдельные части логики
6. **Лямбда-выражения с префиксом `new_`**: дублируют функции из `getMetadataSummaryText()`

## Предлагаемая структура

### Шаг 1: Создать отдельный класс-билдер

```cpp
// v8reader/core/MetadataTreeBuilder.h
#pragma once
#include "v8reader/core/Types.h"
#include "v8reader/core/V8Element.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace v8reader::core {

    struct MetadataBootstrapResult {
        bool ok;
        String metadata_guid;
        std::unique_ptr<TreeNode> metadata_tree;
        String version;
        String error;
    };

    class MetadataTreeBuilder {
    public:
        explicit MetadataTreeBuilder(const V8Container& container);
        
        [[nodiscard]] std::shared_ptr<MetadataItem> build();

    private:
        // === Утилиты ===
        [[nodiscard]] static String toLower(String value);
        [[nodiscard]] static bool isGuidLike(const String& value);
        
        // === Работа с текстом ===
        [[nodiscard]] std::optional<String> decodeText(const V8Element& elem, bool requireBraces) const;
        [[nodiscard]] std::optional<String> getTextByName(const String& fileName) const;
        [[nodiscard]] std::vector<uint8_t> inflateWith(const std::vector<uint8_t>& src, int windowBits) const;
        
        // === Bootstrap ===
        [[nodiscard]] MetadataBootstrapResult bootstrapMetadata() const;
        [[nodiscard]] String guessMetadataGuid() const;
        [[nodiscard]] bool hasObjectStorage(const String& guid) const;
        
        // === Сбор данных секций ===
        [[nodiscard]] std::vector<String> collectSectionGuidsRegex(const String& sectionGuid, 
                                                                    const String& metadataText,
                                                                    const std::unique_ptr<TreeNode>& tree) const;
        
        // === Построение дерева ===
        [[nodiscard]] std::shared_ptr<MetadataItem> buildRawFallback() const;
        [[nodiscard]] std::shared_ptr<MetadataItem> buildSectionFolder(
            const String& guid, 
            const String& title,
            const std::vector<String>& objectGuids,
            const std::unique_ptr<TreeNode>& metadataTree) const;
        [[nodiscard]] String extractDisplayName(const String& objectGuid, 
                                                 const String& sectionGuid,
                                                 const std::unique_ptr<TreeNode>& metadataTree) const;
        
        // === Данные ===
        const V8Container& container_;
        std::unordered_map<String, const V8Element*> elementsByLowerName_;
        
        // === Статические данные секций ===
        static const std::vector<std::pair<String, String>> kSections;
        static const std::unordered_map<String, std::vector<int>> kNamePaths;
    };

} // namespace v8reader::core
```

### Шаг 2: Разбить логику на отдельные методы

#### 2.1 Утилитные методы (вынести в приватные статические методы)
```cpp
String MetadataTreeBuilder::toLower(String value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return value;
}

bool MetadataTreeBuilder::isGuidLike(const String& value) {
    static const std::wregex re(
        LR"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
    return std::regex_match(value, re);
}
```

#### 2.2 Методы работы с текстом (вынести в приватные методы)
```cpp
std::optional<String> MetadataTreeBuilder::decodeText(const V8Element& elem, bool requireBraces) const {
    // Логика из current new_decodeText (строки 679-737)
    // Использует inflateWith, decompressZlib от container_
}

std::optional<String> MetadataTreeBuilder::getTextByName(const String& fileName) const {
    // Логика из current new_getTextByName (строки 743-763)
}
```

#### 2.3 Bootstrap логика (вынести в отдельные методы)
```cpp
MetadataBootstrapResult MetadataTreeBuilder::bootstrapMetadata() const {
    // Вызов существующей функции bootstrapMetadataTree
    // Строки 799, 889-912
}

String MetadataTreeBuilder::guessMetadataGuid() const {
    // Логика из current new_guessMetadataGuid (строки 841-880)
}
```

#### 2.4 Построение секций (устранить дублирование)
```cpp
std::shared_ptr<MetadataItem> MetadataTreeBuilder::buildSectionFolder(
    const String& guid, 
    const String& title,
    const std::vector<String>& objectGuids,
    const std::unique_ptr<TreeNode>& metadataTree) const 
{
    auto folder = std::make_shared<MetadataItem>();
    folder->id = L"folder_" + toLower(guid);
    folder->name = title;
    folder->type = L"Folder";
    folder->is_folder = true;

    for (const auto& objectGuid : objectGuids) {
        String displayName = extractDisplayName(objectGuid, guid, metadataTree);
        
        auto item = std::make_shared<MetadataItem>();
        item->id = objectGuid;
        item->uuid = objectGuid;
        item->name = displayName.empty() ? objectGuid : displayName;
        item->type = title;
        item->is_folder = false;
        folder->children.push_back(item);
    }
    
    return folder;
}
```

### Шаг 3: Упростить основной метод build()

```cpp
std::shared_ptr<MetadataItem> MetadataTreeBuilder::build() {
    auto root = std::make_shared<MetadataItem>();
    root->name = L"Configuration";
    root->type = L"Root";
    root->is_folder = true;
    
    // Инициализация кэша элементов
    for (const auto& e : container_.getElements()) {
        elementsByLowerName_[toLower(e.getName())] = &e;
    }
    
    // Bootstrap метаданных
    const auto bootstrap = bootstrapMetadata();
    
    String activeMetadataGuid;
    std::unique_ptr<TreeNode> activeMetadataTree;
    String activeMetadataText;
    
    if (bootstrap.ok && bootstrap.metadata_tree) {
        activeMetadataGuid = bootstrap.metadata_guid;
        auto txt = getTextByName(activeMetadataGuid);
        if (txt) {
            activeMetadataText = *txt;
            activeMetadataTree = parse1CText(*txt);
        }
    } else {
        activeMetadataGuid = guessMetadataGuid();
        if (!activeMetadataGuid.empty()) {
            auto txt = getTextByName(activeMetadataGuid);
            if (txt) {
                activeMetadataText = *txt;
                activeMetadataTree = parse1CText(*txt);
            }
        }
    }
    
    if (!activeMetadataTree) {
        return buildRawFallback();
    }
    
    // Построение секций
    int totalSectionsBuilt = 0;
    for (const auto& [guid, title] : kSections) {
        auto objectGuids = collectSectionObjectGuids(activeMetadataTree.get(), guid);
        if (objectGuids.empty()) {
            objectGuids = collectSectionGuidsRegex(guid, activeMetadataText, activeMetadataTree);
        }
        if (objectGuids.empty()) continue;
        
        ++totalSectionsBuilt;
        auto folder = buildSectionFolder(guid, title, objectGuids, activeMetadataTree);
        if (!folder->children.empty()) {
            root->children.push_back(folder);
        }
    }
    
    // Попытка с альтернативными метаданными если секции не построены
    if (totalSectionsBuilt == 0) {
        const String altGuid = guessMetadataGuid();
        if (!altGuid.empty() && altGuid != activeMetadataGuid) {
            // Повторить построение с altGuid (вынести в отдельный метод)
            return rebuildWithAlternativeMetadata(root, altGuid);
        }
    }
    
    if (root->children.empty()) {
        return buildRawFallback();
    }
    
    return root;
}
```

### Шаг 4: Обновить V8Container

```cpp
// В V8Container.h
#include "v8reader/core/MetadataTreeBuilder.h"

std::shared_ptr<MetadataItem> buildMetadataTree() const;

// В V8Container.cpp
std::shared_ptr<MetadataItem> V8Container::buildMetadataTree() const {
    MetadataTreeBuilder builder(*this);
    return builder.build();
}
```

## Преимущества нового подхода

| Критерий | Было | Стало |
|----------|------|-------|
| Размер метода | ~365 строк | ~80 строк (build()) |
| Количество лямбда-выражений | 10+ | 0 |
| Дублирование кода | Есть (секции) | Устранено |
| Тестируемость | Низкая | Высокая (можно тестировать каждый метод отдельно) |
| Читаемость | Низкая | Высокая (понятная структура) |
| Расширяемость | Сложно | Легко добавлять новые методы |

## Структура файлов после рефакторинга

```
src/core/include/v8reader/core/
├── V8Container.h          # Добавить forward declaration MetadataTreeBuilder
├── MetadataTreeBuilder.h  # Новый файл
└── Types.h

src/core/src/
├── V8Container.cpp        # Удалить buildMetadataTree(), оставить вызов билдера
└── MetadataTreeBuilder.cpp # Новый файл (~400 строк)
```

## План внедрения

1. Создать `MetadataTreeBuilder.h` с объявлением класса
2. Создать `MetadataTreeBuilder.cpp` с реализацией
3. Обновить `V8Container.cpp` - удалить текущую реализацию, вызвать билдер
4. Написать unit-тесты для отдельных методов билдера
5. Проверить сборку и функциональность
6. Удалить старый код из `V8Container.cpp`

## Дополнительные улучшения

### Возможность мокирования
```cpp
// Для тестирования можно создать MockMetadataTreeBuilder
class MockMetadataTreeBuilder : public MetadataTreeBuilder {
    // Переопределить методы для тестирования
};
```

### Ленивая загрузка
Если дерево метаданных нужно не всегда, можно реализовать ленивую загрузку:
```cpp
class V8Container {
    mutable std::weak_ptr<MetadataItem> metadataTreeCache_;
    
    std::shared_ptr<MetadataItem> getMetadataTree() const {
        if (auto cached = metadataTreeCache_.lock()) {
            return cached;
        }
        auto tree = buildMetadataTree();
        metadataTreeCache_ = tree;
        return tree;
    }
};
```

## Заключение

Рефакторинг разобьет монолитную функцию на логические блоки, улучшит читаемость, тестируемость и поддерживаемость кода. Общий объем кода не изменится, но он будет лучше организован.
