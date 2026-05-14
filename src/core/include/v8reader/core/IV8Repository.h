#pragma once
#include "Types.h"
#include <functional>
#include <memory>

namespace v8::core {
    struct MetadataItem {
        String id;
        String name;
        String type;
        String uuid;
        bool is_folder{false};
        std::vector<std::shared_ptr<MetadataItem>> children;
        
        mutable std::optional<String> module_text;
        mutable std::optional<ByteArray> binary_data;
        mutable std::optional<PropertyMap> properties;
    };

    class IV8Repository {
    public:
        virtual ~IV8Repository() = default;
        
        [[nodiscard]] virtual bool loadFromFile(const String& path) = 0;
        [[nodiscard]] virtual const std::shared_ptr<MetadataItem>& getRoot() const = 0;
        
        [[nodiscard]] virtual std::optional<String> getModuleText(const String& itemId) const = 0;
        [[nodiscard]] virtual std::optional<ByteArray> getBinaryData(const String& itemId) const = 0;
        [[nodiscard]] virtual std::optional<PropertyMap> getProperties(const String& itemId) const = 0;
        
        using LoadCallback = std::function<void(bool success, const String& error)>;
        virtual void setLoadCallback(LoadCallback cb) = 0;
        [[nodiscard]] virtual String getLastError() const = 0;
    };

    std::unique_ptr<IV8Repository> createV8Repository();
}