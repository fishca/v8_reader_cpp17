#include "v8reader/core/V8Container.h"
#include <zlib.h>
#include <algorithm>

namespace v8::core {

    V8Container::V8Container(const String& filepath) : filepath_(filepath) {}

    bool V8Container::detectFormat(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        auto file_size = file.tellg();
        if (file_size < static_cast<std::streamoff>(FORMAT16_BASE_OFFSET + FileHeader16::Size())) {
            is_format16_ = false;
            return true;
        }

        file.seekg(FORMAT16_BASE_OFFSET);
        FileHeader16 hdr16;
        file.read(reinterpret_cast<char*>(&hdr16), hdr16.Size());

        if (file && hdr16.isValid()) {
            BlockHeader16 block_hdr;
            file.read(reinterpret_cast<char*>(&block_hdr), block_hdr.Size());
            if (file && block_hdr.isValid()) {
                is_format16_ = true;
                return true;
            }
        }

        file.clear();
        file.seekg(0);
        FileHeader15 hdr15;
        file.read(reinterpret_cast<char*>(&hdr15), hdr15.Size());

        if (file && hdr15.isValid()) {
            BlockHeader15 block_hdr;
            file.read(reinterpret_cast<char*>(&block_hdr), block_hdr.Size());
            if (file && block_hdr.isValid()) {
                is_format16_ = false;
                return true;
            }
        }

        last_error_ = L"Не распознан формат файла 1С";
        return false;
    }

    int V8Container::load() {
        std::ifstream file(filepath_, std::ios::binary);
        if (!file) {
            last_error_ = L"Не удалось открыть файл: " + filepath_;
            return V8_FILE_NOT_FOUND;
        }

        if (!detectFormat(file)) return V8_NOT_V8_FILE;
        return is_format16_ ? loadImpl<Format16>() : loadImpl<Format15>();
    }

    template<typename Format>
    int V8Container::loadImpl() {
        std::ifstream file(filepath_, std::ios::binary);
        if (!file) return V8_FILE_NOT_FOUND;

        using FileHdr = typename Format::file_header_t;
        using FatEntry = typename Format::fat_entry_t;
        using BlockHdr = typename Format::block_header_t;

        file.seekg(Format::BASE_OFFSET);
        FileHdr file_hdr;
        file.read(reinterpret_cast<char*>(&file_hdr), file_hdr.Size());
        if (!file || !file_hdr.isValid()) {
            last_error_ = L"Ошибка чтения заголовка файла";
            return V8_HEADER_CORRUPT;
        }

        BlockHdr fat_block_hdr;
        file.read(reinterpret_cast<char*>(&fat_block_hdr), fat_block_hdr.Size());
        if (!file || !fat_block_hdr.isValid()) {
            last_error_ = L"Ошибка чтения заголовка блока FAT";
            return V8_HEADER_CORRUPT;
        }

        size_t entry_count = fat_block_hdr.getDataSize() / FatEntry::Size();
        std::vector<FatEntry> fat_entries(entry_count);
        file.read(reinterpret_cast<char*>(fat_entries.data()), fat_block_hdr.getDataSize());
        if (!file) return V8_ERROR;

        elements_.clear();
        name_index_.clear();

        for (size_t i = 0; i < entry_count; ++i) {
            const auto& fat = fat_entries[i];
            if (fat.isTerminator()) break;

            file.seekg(fat.header_addr + Format::BASE_OFFSET);
            BlockHdr elem_hdr;
            file.read(reinterpret_cast<char*>(&elem_hdr), elem_hdr.Size());
            if (!file || !elem_hdr.isValid()) continue;

            auto header_size = elem_hdr.getDataSize();
            std::vector<uint8_t> raw_header(header_size);
            file.read(reinterpret_cast<char*>(raw_header.data()), header_size);
            if (!file) continue;

            V8Element elem;
            elem.setHeader(std::move(raw_header));
            elem.parseHeader();

            if (fat.data_addr != Format::UNDEFINED_VALUE) {
                file.seekg(fat.data_addr + Format::BASE_OFFSET);
                BlockHdr data_hdr;
                file.read(reinterpret_cast<char*>(&data_hdr), data_hdr.Size());
                if (file && data_hdr.isValid()) {
                    auto data = readBlockData(file, fat.data_addr + Format::BASE_OFFSET + BlockHdr::Size(), false);
                    if (!data.empty()) {
                        elem.setData(std::move(data));
                        if (data.size() >= 2 && data[0] == 0x78 && (data[1] == 0x9C || data[1] == 0xDA || data[1] == 0x01)) {
                            elem.setIsCompressed(true);
                        }
                    }
                }
            }

            name_index_[elem.getName()] = elements_.size();
            elements_.push_back(std::move(elem));
        }

        is_loaded_ = true;
        return V8_OK;
    }

    std::vector<uint8_t> V8Container::readBlockData(std::ifstream& file, uint64_t start_addr, bool compressed) const {
        // ✅ ИСПРАВЛЕНО: Убран std::conditional_t. Используем ветвление по флагу формата.
        if (is_format16_) {
            file.seekg(start_addr);
            BlockHeader16 hdr;
            file.read(reinterpret_cast<char*>(&hdr), hdr.Size());
            if (!file || !hdr.isValid()) return {};

            auto size = hdr.getDataSize();
            std::vector<uint8_t> data(size);
            file.read(reinterpret_cast<char*>(data.data()), size);
            return (!file) ? std::vector<uint8_t>{} : (compressed ? decompressZlib(data) : data);
        }
        else {
            file.seekg(start_addr);
            BlockHeader15 hdr;
            file.read(reinterpret_cast<char*>(&hdr), hdr.Size());
            if (!file || !hdr.isValid()) return {};

            auto size = hdr.getDataSize();
            std::vector<uint8_t> data(size);
            file.read(reinterpret_cast<char*>(data.data()), size);
            return (!file) ? std::vector<uint8_t>{} : (compressed ? decompressZlib(data) : data);
        }
    }

    // ... (начало файла без изменений) ...

    std::vector<uint8_t> V8Container::decompressZlib(const std::vector<uint8_t>& src) const {
        z_stream stream{};
        stream.next_in = const_cast<Bytef*>(src.data());
        stream.avail_in = static_cast<uInt>(src.size());

        // ✅ Подавляем [[nodiscard]], так как ошибку обрабатываем ниже
        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) return {};

        std::vector<uint8_t> output(1024 * 1024);
        while (true) {
            stream.next_out = output.data() + stream.total_out;
            stream.avail_out = static_cast<uInt>(output.size() - stream.total_out);

            int ret = inflate(&stream, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) {
                output.resize(stream.total_out);
                break;
            }
            if (ret != Z_OK) {
                (void)inflateEnd(&stream); // ✅ Явно игнорируем, если уже упали
                return {};
            }
            if (stream.avail_out == 0) output.resize(output.size() * 2);
        }

        (void)inflateEnd(&stream); // ✅ Подавляем предупреждение
        return output;
    }

    // ... (остальной код без изменений) ...

    const V8Element* V8Container::findElement(const String& name) const {
        auto it = name_index_.find(name);
        return (it == name_index_.end()) ? nullptr : &elements_[it->second];
    }

    std::optional<String> V8Container::getModuleText(const String& name) const {
        const auto* elem = findElement(name);
        if (!elem) return std::nullopt;

        auto data = elem->getData();
        if (elem->isCompressed()) {
            data = decompressZlib(data);
            if (data.empty()) return std::nullopt;
        }
        if (data.size() < 2 || data.size() % 2 != 0) return std::nullopt;

#ifdef _WIN32
        String result(data.size() / 2, L'\0');
        std::memcpy(result.data(), data.data(), data.size());
        return result;
#else
        try {
            std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>, wchar_t> conv;
            return conv.from_bytes(reinterpret_cast<const char*>(data.data()),
                reinterpret_cast<const char*>(data.data() + data.size()));
        }
        catch (...) { return std::nullopt; }
#endif
    }

    std::optional<std::vector<uint8_t>> V8Container::extractData(const String& name) const {
        const auto* elem = findElement(name);
        if (!elem) return std::nullopt;
        return elem->isCompressed() ? decompressZlib(elem->getData()) : elem->getData();
    }

    std::shared_ptr<MetadataItem> V8Container::buildMetadataTree() const {
        auto root = std::make_shared<MetadataItem>();
        root->name = L"Конфигурация"; root->type = L"Root"; root->is_folder = true;

        std::unordered_map<String, std::shared_ptr<MetadataItem>> folders;
        for (const auto& elem : elements_) {
            const auto& name = elem.getName();
            if (name.empty() || name[0] == L'.') continue;

            String type = L"Unknown";
            if (name.find(L"CommonModule.") == 0) type = L"CommonModule";
            else if (name.find(L"Catalog.") == 0) type = L"Catalog";
            else if (name.find(L"Document.") == 0) type = L"Document";
            else if (name.find(L"Form.") == 0) type = L"Form";

            if (folders.find(type) == folders.end()) {
                auto folder = std::make_shared<MetadataItem>();
                folder->id = L"folder_" + type; folder->name = type;
                folder->type = L"Folder"; folder->is_folder = true;
                folders[type] = folder; root->children.push_back(folder);
            }

            auto item = std::make_shared<MetadataItem>();
            item->id = name; item->name = name; item->type = type; item->is_folder = elem.isCatalog();
            folders[type]->children.push_back(item);
        }
        return root;
    }

    // 🔑 Явная инстанциация шаблонов (обязательно для компиляции)
    template int V8Container::loadImpl<Format15>();
    template int V8Container::loadImpl<Format16>();

} // namespace v8::core