#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <cstdint>

namespace v8::core {
    using String = std::wstring;
    using ByteArray = std::vector<uint8_t>;
    using PropertyMap = std::map<String, String>;

    struct Guid {
        uint32_t data1{};
        uint16_t data2{}, data3{};
        uint8_t data4[8]{};
        
        // ✅ Ручная реализация для C++17
        [[nodiscard]] bool operator==(const Guid& other) const {
            return data1 == other.data1 
                && data2 == other.data2 
                && data3 == other.data3 
                && std::equal(std::begin(data4), std::end(data4), std::begin(other.data4));
        }
        
        [[nodiscard]] bool operator!=(const Guid& other) const {
            return !(*this == other);
        }
    };
}