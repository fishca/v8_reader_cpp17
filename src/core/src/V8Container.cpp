#include "v8reader/core/V8Container.h"
#include <zlib.h>
#include <algorithm>
#include <unordered_set>
#include <regex>
#include <cwctype>

namespace v8::core {
    namespace {
        struct SectionInfo {
            const wchar_t* guid;
            const wchar_t* title;
        };

        constexpr SectionInfo kSections[] = {
            {L"cf4abea6-37b2-11d4-940f-008048da11f9", L"Catalogs"},
            {L"061d872a-5787-460e-95ac-ed74ea3a3e84", L"Documents"},
            {L"631b75a0-29e2-11d6-a3c7-0050bae0a776", L"Reports"},
            {L"bf845118-327b-4682-b5c6-285d2a0eb296", L"Processings"},
            {L"0fe48980-252d-11d6-a3c7-0050bae0a776", L"CommonModules"},
            {L"f6a80749-5ad7-400b-8519-39dc5dff2542", L"Enums"},
            {L"0195e80c-b157-11d4-9435-004095e12fc7", L"Constants"},
            {L"13134201-f60b-11d5-a3c7-0050bae0a776", L"InfoRegisters"},
            {L"b64d9a40-1642-11d6-a3c7-0050bae0a776", L"AccumRegisters"},
            {L"2deed9b8-0056-4ffe-a473-c20a6c32a0bc", L"AccountingRegisters"},
            {L"f2de87a8-64e5-45eb-a22d-b3aedab050e7", L"CalculationRegisters"},
            {L"238e7e88-3c5f-48b2-8a3b-81ebbecb20ed", L"ChartOfAccounts"},
            {L"82a1b659-b220-4d94-a9bd-14d757b95a48", L"ChartOfCharacteristicTypes"},
            {L"30b100d6-b29f-47ac-aec7-cb8ca8a54767", L"ChartOfCalculationTypes"},
            {L"857c4a91-e5f4-4fac-86ec-787626f1c108", L"ExchangePlans"},
            {L"4612bd75-71b7-4a5c-8cc5-2b0b65f9fa0d", L"DocumentJournals"},
            {L"36a8e346-9aaa-4af9-bdbd-83be3c177977", L"Numerators"},
            {L"3e63355c-1378-4953-be9b-1deb5fb6bec5", L"Tasks"},
            {L"fcd3404e-1523-48ce-9bc0-ecdb822684a1", L"BusinessProcesses"},
            {L"37f2fa9a-b276-11d4-9435-004095e12fc7", L"Subsystems"},
        };

        bool isGuidLike(const String& value) {
            static const std::wregex re(
                LR"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
            return std::regex_match(value, re);
        }

        String toLowerCopy(String value) {
            std::transform(value.begin(), value.end(), value.begin(),
                [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
            return value;
        }

        std::optional<String> decodeUtf16LE(const std::vector<uint8_t>& data) {
            if (data.empty() || (data.size() % 2) != 0) return std::nullopt;
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

        std::optional<String> decodeAnsi(const std::vector<uint8_t>& data) {
            if (data.empty()) return std::nullopt;
            String out;
            out.reserve(data.size());
            for (uint8_t c : data) out.push_back(static_cast<wchar_t>(c));
            return out;
        }
    } // namespace

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

        last_error_ = L"Unrecognized 1C file format";
        return false;
    }

    int V8Container::load() {
        std::ifstream file(filepath_, std::ios::binary);
        if (!file) {
            last_error_ = L"Failed to open file: " + filepath_;
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
            last_error_ = L"Failed to read file header";
            return V8_HEADER_CORRUPT;
        }

        BlockHdr fat_block_hdr;
        file.read(reinterpret_cast<char*>(&fat_block_hdr), fat_block_hdr.Size());
        if (!file || !fat_block_hdr.isValid()) {
            last_error_ = L"Failed to read FAT block header";
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
                    auto data = readBlockData(file, fat.data_addr + Format::BASE_OFFSET, false);
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
        std::vector<uint8_t> data;

        if (is_format16_) {
            uint64_t current_addr = start_addr;
            while (current_addr != FAT_UNDEFINED_64) {
                file.seekg(current_addr);
                BlockHeader16 hdr;
                file.read(reinterpret_cast<char*>(&hdr), hdr.Size());
                if (!file || !hdr.isValid()) return {};

                const auto chunk_size = static_cast<size_t>(hdr.getDataSize());
                const auto old_size = data.size();
                data.resize(old_size + chunk_size);
                file.read(reinterpret_cast<char*>(data.data() + old_size), static_cast<std::streamsize>(chunk_size));
                if (!file) return {};

                current_addr = hdr.getNextAddr();
                if (current_addr != FAT_UNDEFINED_64) {
                    current_addr += FORMAT16_BASE_OFFSET;
                }
            }
        }
        else {
            uint64_t current_addr = start_addr;
            while (current_addr != FAT_UNDEFINED_32) {
                file.seekg(current_addr);
                BlockHeader15 hdr;
                file.read(reinterpret_cast<char*>(&hdr), hdr.Size());
                if (!file || !hdr.isValid()) return {};

                const auto chunk_size = static_cast<size_t>(hdr.getDataSize());
                const auto old_size = data.size();
                data.resize(old_size + chunk_size);
                file.read(reinterpret_cast<char*>(data.data() + old_size), static_cast<std::streamsize>(chunk_size));
                if (!file) return {};

                current_addr = hdr.getNextAddr();
            }
        }

        return compressed ? decompressZlib(data) : data;
    }

    // ... (РЅР°С‡Р°Р»Рѕ С„Р°Р№Р»Р° Р±РµР· РёР·РјРµРЅРµРЅРёР№) ...

    std::vector<uint8_t> V8Container::decompressZlib(const std::vector<uint8_t>& src) const {
        z_stream stream{};
        stream.next_in = const_cast<Bytef*>(src.data());
        stream.avail_in = static_cast<uInt>(src.size());

        // вњ… РџРѕРґР°РІР»СЏРµРј [[nodiscard]], С‚Р°Рє РєР°Рє РѕС€РёР±РєСѓ РѕР±СЂР°Р±Р°С‚С‹РІР°РµРј РЅРёР¶Рµ
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
                (void)inflateEnd(&stream); // вњ… РЇРІРЅРѕ РёРіРЅРѕСЂРёСЂСѓРµРј, РµСЃР»Рё СѓР¶Рµ СѓРїР°Р»Рё
                return {};
            }
            if (stream.avail_out == 0) output.resize(output.size() * 2);
        }

        (void)inflateEnd(&stream); // вњ… РџРѕРґР°РІР»СЏРµРј РїСЂРµРґСѓРїСЂРµР¶РґРµРЅРёРµ
        return output;
    }

    // ... (РѕСЃС‚Р°Р»СЊРЅРѕР№ РєРѕРґ Р±РµР· РёР·РјРµРЅРµРЅРёР№) ...

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
        root->name = L"Configuration";
        root->type = L"Root";
        root->is_folder = true;
        auto ensureRawFallback = [&]() -> std::shared_ptr<MetadataItem> {
            if (!root->children.empty()) return root;
            auto rawFolder = std::make_shared<MetadataItem>();
            rawFolder->id = L"folder_raw";
            rawFolder->name = L"ContainerRaw";
            rawFolder->type = L"Folder";
            rawFolder->is_folder = true;
            std::unordered_map<String, std::vector<String>> groups;
            std::vector<String> special;
            for (const auto& elem : elements_) {
                const String& name = elem.getName();
                if (name.empty() || name[0] == L'.') continue;
                if (name == L"root" || name == L"version" || name == L"versions") {
                    special.push_back(name);
                    continue;
                }
                const auto dotPos = name.find(L'.');
                if (dotPos != String::npos && isGuidLike(name.substr(0, dotPos))) {
                    groups[name.substr(0, dotPos)].push_back(name);
                } else if (isGuidLike(name)) {
                    groups[name].push_back(name);
                }
            }

            for (const auto& [baseGuid, entries] : groups) {
                if (entries.empty()) continue;
                if (entries.size() == 1 && entries.front() == baseGuid) {
                    auto item = std::make_shared<MetadataItem>();
                    item->id = baseGuid;
                    item->name = baseGuid;
                    item->type = L"Raw";
                    item->is_folder = false;
                    rawFolder->children.push_back(item);
                    continue;
                }

                auto groupNode = std::make_shared<MetadataItem>();
                groupNode->id = L"group_" + baseGuid;
                groupNode->name = baseGuid;
                groupNode->type = L"RawGroup";
                groupNode->is_folder = true;
                for (const auto& fullName : entries) {
                    auto child = std::make_shared<MetadataItem>();
                    child->id = fullName;
                    child->name = fullName;
                    child->type = L"Raw";
                    child->is_folder = false;
                    groupNode->children.push_back(child);
                }
                rawFolder->children.push_back(groupNode);
            }

            for (const auto& name : special) {
                auto item = std::make_shared<MetadataItem>();
                item->id = name;
                item->name = name;
                item->type = L"Raw";
                item->is_folder = false;
                rawFolder->children.push_back(item);
            }
            if (!rawFolder->children.empty()) root->children.push_back(rawFolder);
            return root;
            };
        auto inflateWith = [](const std::vector<uint8_t>& src, int windowBits) -> std::vector<uint8_t> {
            z_stream stream{};
            stream.next_in = const_cast<Bytef*>(src.data());
            stream.avail_in = static_cast<uInt>(src.size());
            if (inflateInit2(&stream, windowBits) != Z_OK) return {};
            std::vector<uint8_t> out(1024 * 1024);
            while (true) {
                stream.next_out = out.data() + stream.total_out;
                stream.avail_out = static_cast<uInt>(out.size() - stream.total_out);
                const int ret = inflate(&stream, Z_NO_FLUSH);
                if (ret == Z_STREAM_END) {
                    out.resize(stream.total_out);
                    break;
                }
                if (ret != Z_OK) {
                    (void)inflateEnd(&stream);
                    return {};
                }
                if (stream.avail_out == 0) out.resize(out.size() * 2);
            }
            (void)inflateEnd(&stream);
            return out;
            };

        std::unordered_map<String, const V8Element*> byName;
        byName.reserve(elements_.size());
        for (const auto& elem : elements_) {
            byName[toLowerCopy(elem.getName())] = &elem;
        }

        const auto rootIt = byName.find(L"root");
        if (rootIt == byName.end()) return ensureRawFallback();

        const auto& rootRawData = rootIt->second->getData();
        std::vector<std::vector<uint8_t>> rootCandidates;
        rootCandidates.push_back(rootRawData);
        auto rootInflatedRaw = inflateWith(rootRawData, -MAX_WBITS);
        if (!rootInflatedRaw.empty() && rootInflatedRaw != rootRawData) rootCandidates.push_back(std::move(rootInflatedRaw));
        auto rootInflatedZlib = inflateWith(rootRawData, MAX_WBITS);
        if (!rootInflatedZlib.empty() && rootInflatedZlib != rootRawData) rootCandidates.push_back(std::move(rootInflatedZlib));

        String configGuid;
        std::wregex rgxRootGuid(LR"(\{([0-9a-fA-F-]{36})\})");
        for (const auto& blob : rootCandidates) {
            auto maybeText = decodeUtf16LE(blob);
            if (!maybeText) maybeText = decodeAnsi(blob);
            if (!maybeText) continue;
            std::wsmatch match;
            if (std::regex_search(*maybeText, match, rgxRootGuid) && match.size() >= 2) {
                configGuid = toLowerCopy(match[1].str());
                break;
            }
        }
        if (configGuid.empty()) return ensureRawFallback();

        const auto cfgIt = byName.find(configGuid);
        if (cfgIt == byName.end()) return ensureRawFallback();

        const auto& cfgRawData = cfgIt->second->getData();
        std::vector<std::vector<uint8_t>> cfgCandidates;
        cfgCandidates.push_back(cfgRawData);
        auto cfgInflatedRaw = inflateWith(cfgRawData, -MAX_WBITS);
        if (!cfgInflatedRaw.empty() && cfgInflatedRaw != cfgRawData) cfgCandidates.push_back(std::move(cfgInflatedRaw));
        auto cfgInflatedZlib = inflateWith(cfgRawData, MAX_WBITS);
        if (!cfgInflatedZlib.empty() && cfgInflatedZlib != cfgRawData) cfgCandidates.push_back(std::move(cfgInflatedZlib));

        std::optional<String> cfgText;
        for (const auto& blob : cfgCandidates) {
            auto maybeText = decodeUtf16LE(blob);
            if (!maybeText) maybeText = decodeAnsi(blob);
            if (maybeText) {
                cfgText = std::move(maybeText);
                break;
            }
        }
        if (!cfgText) return ensureRawFallback();

        std::unordered_map<String, std::vector<String>> groups;
        for (const auto& s : kSections) groups[toLowerCopy(s.guid)] = {};

        std::wregex rgxPair(LR"(\{([0-9a-fA-F-]{36}),\s*0\})");
        auto begin = std::wsregex_iterator(cfgText->begin(), cfgText->end(), rgxPair);
        auto end = std::wsregex_iterator();
        String activeSection;
        for (auto it = begin; it != end; ++it) {
            const String guid = toLowerCopy((*it)[1].str());
            if (groups.find(guid) != groups.end()) {
                activeSection = guid;
                continue;
            }
            if (!activeSection.empty() && byName.find(guid) != byName.end()) {
                groups[activeSection].push_back(guid);
            }
        }

        std::wregex rgxQuoted(LR"re("([^"]+)")re");
        for (const auto& section : kSections) {
            const String sectionGuid = toLowerCopy(section.guid);
            auto grpIt = groups.find(sectionGuid);
            if (grpIt == groups.end() || grpIt->second.empty()) continue;

            auto folder = std::make_shared<MetadataItem>();
            folder->id = L"folder_" + sectionGuid;
            folder->name = section.title;
            folder->type = L"Folder";
            folder->is_folder = true;

            std::unordered_set<String> uniq;
            for (const auto& objGuid : grpIt->second) {
                if (!uniq.insert(objGuid).second) continue;
                const auto fileIt = byName.find(objGuid);
                if (fileIt == byName.end()) continue;

                auto objData = fileIt->second->isCompressed() ? decompressZlib(fileIt->second->getData()) : fileIt->second->getData();
                auto objText = decodeUtf16LE(objData);

                String displayName = objGuid;
                if (objText) {
                    std::wsmatch qMatch;
                    if (std::regex_search(*objText, qMatch, rgxQuoted) && qMatch.size() >= 2) {
                        const String candidate = qMatch[1].str();
                        if (!candidate.empty() && !isGuidLike(candidate)) displayName = candidate;
                    }
                }

                auto item = std::make_shared<MetadataItem>();
                item->id = objGuid;
                item->name = displayName;
                item->type = section.title;
                item->uuid = objGuid;
                item->is_folder = false;
                folder->children.push_back(item);
            }

            if (!folder->children.empty()) root->children.push_back(folder);
        }

        return ensureRawFallback();
    }
    // рџ”‘ РЇРІРЅР°СЏ РёРЅСЃС‚Р°РЅС†РёР°С†РёСЏ С€Р°Р±Р»РѕРЅРѕРІ (РѕР±СЏР·Р°С‚РµР»СЊРЅРѕ РґР»СЏ РєРѕРјРїРёР»СЏС†РёРё)
    template int V8Container::loadImpl<Format15>();
    template int V8Container::loadImpl<Format16>();

} // namespace v8::core
