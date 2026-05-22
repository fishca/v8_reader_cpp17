// src/core/src/V8Unpacker.cpp
#include "v8reader/core/V8Unpacker.h"
#include "v8reader/core/V8Container.h"
#include <filesystem>
#include <cstring>

namespace v8::core {

    bool V8Unpacker::loadFromFile(const String& path) {
        try {
            const auto ext = std::filesystem::path(path).extension().wstring();
            if (ext != EXT_CF && ext != EXT_CFU && ext != EXT_CFE &&
                ext != EXT_EPF && ext != EXT_ERF) {
                last_error_ = L"���������������� ������: " + ext;
                if (callback_) callback_(false, last_error_);
                return false;
            }

            V8Container container(path);
            int result = container.load();
            if (result != V8_OK) {
                last_error_ = container.getLastError();
                if (callback_) callback_(false, last_error_);
                return false;
            }

            root_ = container.buildMetadataTree();
            container_ = std::make_unique<V8Container>(std::move(container));

            if (callback_) callback_(true, L"");
            return true;

        }
        catch (const std::exception& e) {
            last_error_ = String(e.what(), e.what() + std::strlen(e.what()));
            if (callback_) callback_(false, last_error_);
            return false;
        }
    }

    const std::shared_ptr<MetadataItem>& V8Unpacker::getRoot() const {
        return root_;
    }

    std::optional<String> V8Unpacker::getModuleText(const String& itemId) const {
        if (!container_) return std::nullopt;
        return container_->getModuleText(itemId);
    }

    std::optional<ByteArray> V8Unpacker::getBinaryData(const String& itemId) const {
        if (!container_) return std::nullopt;
        return container_->extractData(itemId);
    }

    std::optional<PropertyMap> V8Unpacker::getProperties(const String&) const {
        // ��������: ������� ������� ������� ������� ���������� �������� 1�
        return std::nullopt;
    }

    void V8Unpacker::setLoadCallback(LoadCallback cb) { callback_ = std::move(cb); }
    String V8Unpacker::getLastError() const { return last_error_; }

    std::unique_ptr<IV8Repository> createV8Repository() {
        return std::make_unique<V8Unpacker>();
    }

} // namespace v8::core