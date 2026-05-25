#pragma once
#include "v8reader/core/Types.h" // Подключаем структуру
#include <functional>

namespace v8reader::core {

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

} // namespace v8reader::core