#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <cstdint>
#include <memory> // 🔑 Нужно для shared_ptr
#include <algorithm>

namespace v8::core {

    using String = std::wstring;
    using ByteArray = std::vector<uint8_t>;
    using PropertyMap = std::map<String, String>;

    // 🔑 ПЕРЕНЕСЕНО СЮДА: Структура элемента метаданных
    struct MetadataItem {
        String id;
        String name;
        String type;
        String uuid;
        bool is_folder{ false };
        std::vector<std::shared_ptr<MetadataItem>> children;

        mutable std::optional<String> module_text;
        mutable std::optional<ByteArray> binary_data;
        mutable std::optional<PropertyMap> properties;
    };

    struct Guid {
        uint32_t data1{};
        uint16_t data2{}, data3{};
        uint8_t data4[8]{};

        [[nodiscard]] String toString() const;
        [[nodiscard]] static Guid parse(const String& str);

        // ✅ Явная реализация для C++17 (вместо = default)
        [[nodiscard]] bool operator==(const Guid& other) const {
            return data1 == other.data1 &&
                data2 == other.data2 &&
                data3 == other.data3 &&
                std::equal(std::begin(data4), std::end(data4), std::begin(other.data4));
        }
        [[nodiscard]] bool operator!=(const Guid& other) const {
            return !(*this == other);
        }
    };

} // namespace v8::core