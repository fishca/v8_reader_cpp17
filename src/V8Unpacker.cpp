#include "v8reader/core/V8Unpacker.h"

namespace v8::core {

bool V8Unpacker::loadFromFile(const String& path) {
    root_ = std::make_shared<MetadataItem>();
    root_->name = L"Конфигурация";
    root_->type = L"Root";
    root_->is_folder = true;
    
    auto catalog = std::make_shared<MetadataItem>();
    catalog->id = L"cat_1";
    catalog->name = L"Справочники";
    catalog->type = L"CatalogFolder";
    catalog->is_folder = true;
    root_->children.push_back(catalog);
    
    auto item = std::make_shared<MetadataItem>();
    item->id = L"item_1";
    item->name = L"Номенклатура";
    item->type = L"Catalog";
    item->uuid = L"{12345678-1234-5678-9ABC-123456789ABC}";
    catalog->children.push_back(item);
    
    if (callback_) callback_(true, L"");
    return true;
}

const std::shared_ptr<MetadataItem>& V8Unpacker::getRoot() const { return root_; }

std::optional<String> V8Unpacker::getModuleText(const String& itemId) const {
    if (itemId == L"item_1") {
        return L"&НаКлиенте\nПроцедура ОбработкаПроведения()\n  // TODO\nКонецПроцедуры";
    }
    return std::nullopt;
}

std::optional<ByteArray> V8Unpacker::getBinaryData(const String&) const { return std::nullopt; }

std::optional<PropertyMap> V8Unpacker::getProperties(const String& itemId) const {
    if (itemId == L"item_1") {
        return PropertyMap{{L"Имя", L"Номенклатура"}, {L"Тип", L"Справочник"}, {L"Иерархия", L"Да"}};
    }
    return std::nullopt;
}

void V8Unpacker::setLoadCallback(LoadCallback cb) { callback_ = std::move(cb); }
String V8Unpacker::getLastError() const { return last_error_; }

std::unique_ptr<IV8Repository> createV8Repository() {
    return std::make_unique<V8Unpacker>();
}

}