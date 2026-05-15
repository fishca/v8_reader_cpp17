#pragma once
#include "Types.h"
#include "V8Format.h"
#include "V8Element.h"
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>

namespace v8::core {

    class V8Container {
    public:
        explicit V8Container(const String& filepath);
        ~V8Container() = default;

        [[nodiscard]] int load();
        [[nodiscard]] bool isFormat16() const { return is_format16_; }
        [[nodiscard]] const String& getLastError() const { return last_error_; }

        [[nodiscard]] const std::vector<V8Element>& getElements() const { return elements_; }
        [[nodiscard]] const V8Element* findElement(const String& name) const;
        [[nodiscard]] std::shared_ptr<MetadataItem> buildMetadataTree() const;
        [[nodiscard]] std::optional<std::vector<uint8_t>> extractData(const String& name) const;
        [[nodiscard]] std::optional<String> getModuleText(const String& name) const;

    private:
        template<typename Format>
        [[nodiscard]] int loadImpl();

        [[nodiscard]] bool detectFormat(std::ifstream& file);
        [[nodiscard]] std::vector<uint8_t> readBlockData(std::ifstream& file, uint64_t start_addr, bool compressed) const;
        [[nodiscard]] std::vector<uint8_t> decompressZlib(const std::vector<uint8_t>& src) const;

        String filepath_;
        bool is_format16_ = false;
        bool is_loaded_ = false;
        String last_error_;

        std::vector<V8Element> elements_;
        std::unordered_map<std::wstring, size_t> name_index_;
    };

} // namespace v8::core