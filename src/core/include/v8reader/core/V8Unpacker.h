#pragma once
#include "IV8Repository.h"
#include "v8reader/core/V8Container.h" // 🔑 КРИТИЧЕСКИ ВАЖНО: Подключаем V8Container
#include <memory>

namespace v8reader::core {

    class V8Unpacker : public IV8Repository {
    public:
        bool loadFromFile(const String& path) override;
        const std::shared_ptr<MetadataItem>& getRoot() const override;

        std::optional<String> getModuleText(const String& itemId) const override;
        std::optional<ByteArray> getBinaryData(const String& itemId) const override;
        std::optional<PropertyMap> getProperties(const String& itemId) const override;

        void setLoadCallback(LoadCallback cb) override;
        String getLastError() const override;

    private:
        std::shared_ptr<MetadataItem> root_;
        String last_error_;
        LoadCallback callback_;

        // 🔑 Исправлено: используем V8Container, заголовок которого подключен выше
        mutable std::unique_ptr<V8Container> container_;
    };

} // namespace v8reader::core