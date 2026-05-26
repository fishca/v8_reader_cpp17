#include "v8reader/core/V8Container.h"
#include "v8reader/core/TreeReader.h"
#include <zlib.h>
#include <algorithm>
#include <unordered_set>
#include <regex>
#include <cwctype>
#include <string>
#include <functional>
#include <iostream>
#include <codecvt>
#include <cstring>
#include <QDebug>

namespace v8reader::core {
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

        std::string toAsciiFiltered(const std::vector<uint8_t>& data, bool stripZeroBytes) {
            std::string out;
            out.reserve(data.size());
            for (uint8_t b : data) {
                if (stripZeroBytes && b == 0) continue;
                const char c = static_cast<char>(b);
                if ((c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') ||
                    c == '{' || c == '}' || c == '-' || c == ',' || c == ' ' || c == '"' ||
                    c == '\n' || c == '\r' || c == '\t') {
                    out.push_back(c);
                } else {
                    out.push_back(' ');
                }
            }
            return out;
        }

        String pickDisplayNameFromText(const String& text) {
            if (text.empty()) return {};
            auto isLikelyHumanName = [](const String& v) -> bool {
                if (v.size() < 3 || v.size() > 120) return false;
                if (v.find(L'{') != String::npos || v.find(L'}') != String::npos ||
                    v.find(L',') != String::npos || v.find(L';') != String::npos) return false;
                size_t letters = 0;
                size_t allowed = 0;
                for (wchar_t c : v) {
                    const bool isCyr = (c >= 0x0400 && c <= 0x04FF);
                    const bool isLat = (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z');
                    const bool isNum = (c >= L'0' && c <= L'9');
                    const bool isSep = (c == L' ' || c == L'_' || c == L'-' || c == L'.' || c == L'(' || c == L')');
                    if (isCyr || isLat) letters++;
                    if (isCyr || isLat || isNum || isSep) allowed++;
                }
                if (letters == 0) return false;
                const double ratio = static_cast<double>(allowed) / static_cast<double>(v.size());
                return ratio >= 0.95;
            };

            std::wregex quoted(LR"re("([^"]{3,120})")re");
            auto it = std::wsregex_iterator(text.begin(), text.end(), quoted);
            auto end = std::wsregex_iterator();
            for (; it != end; ++it) {
                String v = (*it)[1].str();
                if (isGuidLike(v)) continue;
                if (isLikelyHumanName(v)) return v;
            }
            return {};
        }

        bool isLikelyHumanNameStrict(const String& v) {
            if (v.size() < 3 || v.size() > 120) return false;
            if (v.find(L'{') != String::npos || v.find(L'}') != String::npos ||
                v.find(L',') != String::npos || v.find(L';') != String::npos ||
                v.find(L'\n') != String::npos || v.find(L'\r') != String::npos ||
                v.find(L'\t') != String::npos) return false;
            size_t letters = 0;
            for (wchar_t c : v) {
                const bool isCyr = (c >= 0x0400 && c <= 0x04FF);
                const bool isLat = (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z');
                const bool isNum = (c >= L'0' && c <= L'9');
                const bool isSep = (c == L' ' || c == L'_' || c == L'-' || c == L'.' || c == L'(' || c == L')');
                if (!(isCyr || isLat || isNum || isSep)) return false;
                if (isCyr || isLat) letters++;
            }
            return letters > 0;
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
        std::string path_utf8;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        try {
            path_utf8 = converter.to_bytes(filepath_);
        } catch (...) {
            last_error_ = L"Failed to convert filepath to UTF-8";
            return V8_FILE_NOT_FOUND;
        }
        
        std::ifstream file(path_utf8, std::ios::binary);
        if (!file) {
            last_error_ = L"Failed to open file: " + filepath_;
            return V8_FILE_NOT_FOUND;
        }

        if (!detectFormat(file)) return V8_NOT_V8_FILE;
        return is_format16_ ? loadImpl<Format16>() : loadImpl<Format15>();
    }

    template<typename Format>
    int V8Container::loadImpl() {
        std::string path_utf8;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        try {
            path_utf8 = converter.to_bytes(filepath_);
        } catch (...) {
            return V8_FILE_NOT_FOUND;
        }
        
        std::ifstream file(path_utf8, std::ios::binary);
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

    // ... (РЅР°С‡Р°Р»Рѕ С„Р°Р№Р»Р° Р±РµР· РёР·РјРµРЅРµРЅРёР№) ...

    std::vector<uint8_t> V8Container::decompressZlib(const std::vector<uint8_t>& src) const {
        z_stream stream{};
        stream.next_in = const_cast<Bytef*>(src.data());
        stream.avail_in = static_cast<uInt>(src.size());

        // вњ… РџРѕРґР°РІР»СЏРµРј [[nodiscard]], С‚Р°Рє РєР°Рє РѕС€РёР±РєСѓ РѕР±СЂР°Р±Р°С‚С‹РІР°РµРј РЅРёР¶Рµ
        if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) { inflateEnd(&stream); return {}; }

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

    // ... (РѕСЃС‚Р°Р»СЊРЅРѕР№ РєРѕРґ Р±РµР· РёР·РјРµРЅРµРЅРёР№) ...

    const V8Element* V8Container::findElement(const String& name) const {
        auto it = name_index_.find(name);
        return (it == name_index_.end()) ? nullptr : &elements_[it->second];

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

    std::optional<std::vector<uint8_t>> V8Container::extractData(const String& name) const {
        const auto* elem = findElement(name);
        if (!elem) return std::nullopt;
        return elem->isCompressed() ? decompressZlib(elem->getData()) : elem->getData();

    String V8Container::getMetadataSummaryText() const {
        auto toLower = [](String v) {
            std::transform(v.begin(), v.end(), v.begin(),
                [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
            return v;
        };
        auto decodeUtf16 = [](const std::vector<uint8_t>& data) -> std::optional<String> {
            if (data.empty() || (data.size() % 2) != 0) return std::nullopt;
            String result(data.size() / 2, L'\0');
            std::memcpy(result.data(), data.data(), data.size());
            return result;
        };
        auto inflateWith = [](const std::vector<uint8_t>& src, int windowBits) -> std::vector<uint8_t> {
            if (src.empty()) return {};
            z_stream stream{};
            stream.next_in = const_cast<Bytef*>(src.data());
            stream.avail_in = static_cast<uInt>(src.size());
            if (inflateInit2(&stream, windowBits) != Z_OK) { inflateEnd(&stream); return {}; }
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
        std::unordered_map<String, const V8Element*> byNameLower;
        for (const auto& e : elements_) byNameLower[toLower(e.getName())] = &e;

        auto readText = [&](const String& name, bool requireBraces) -> std::optional<String> {
            const auto* elem = findElement(name);
            if (!elem) {
                auto it = byNameLower.find(toLower(name));
                if (it != byNameLower.end()) elem = it->second;
            }
            if (!elem) {
                const String low = toLower(name);
                for (const auto& [n, p] : byNameLower) {
                    if (n.rfind(low + L".", 0) == 0) {
                        elem = p;
                        break;
                    }
                }
            }
            if (!elem) return std::nullopt;
            std::vector<std::vector<uint8_t>> blobs;
            const auto& raw = elem->getData();
            if (raw.empty()) return std::nullopt;
            blobs.push_back(raw);
            auto z1 = inflateWith(raw, -MAX_WBITS);
            if (!z1.empty()) blobs.push_back(std::move(z1));
            auto z2 = inflateWith(raw, MAX_WBITS);
            if (!z2.empty()) blobs.push_back(std::move(z2));
            if (elem->isCompressed()) {
                auto z3 = decompressZlib(raw);
                if (!z3.empty()) blobs.push_back(std::move(z3));
            }
            for (const auto& b : blobs) {
                auto utf = decodeUtf16(b);
                if (utf && (!requireBraces || utf->find(L'{') != String::npos)) return utf;
                String ansi;
                ansi.reserve(b.size());
                for (uint8_t c : b) ansi.push_back(static_cast<wchar_t>(c));
                if (!ansi.empty() && (!requireBraces || ansi.find(L'{') != String::npos)) return ansi;
            }
            return std::nullopt;
        };

        auto extractRootMetadataGuidFromRaw = [&]() -> String {
            const V8Element* rootElem = findElement(L"root");
            if (!rootElem) {
                auto it = byNameLower.find(L"root");
                if (it != byNameLower.end()) rootElem = it->second;
            }
            if (!rootElem) return {};
            auto raw = rootElem->getData();
            if (raw.empty()) return {};
            if (rootElem->isCompressed()) {
                auto dec = decompressZlib(raw);
                if (!dec.empty()) raw = std::move(dec);
            }
            std::string ascii;
            ascii.reserve(raw.size());
            for (uint8_t b : raw) {
                if (b == 0) continue;
                const char c = static_cast<char>(b);
                if ((c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F') ||
                    c == '{' || c == '}' || c == '-' || c == ',' || c == ' ' ||
                    c == '\n' || c == '\r' || c == '\t') {
                    ascii.push_back(c);
                }
            }
            static const std::regex rootRgx(
                R"(\{\s*-?\d+\s*,\s*([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\s*,)");
            static const std::regex guidRgx(
                R"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))");
            std::smatch m;
            if (std::regex_search(ascii, m, rootRgx) && m.size() >= 2) {
                String g(m[1].str().begin(), m[1].str().end());
                return toLower(g);
            }
            if (std::regex_search(ascii, m, guidRgx) && m.size() >= 2) {
                String g(m[1].str().begin(), m[1].str().end());
                return toLower(g);
            }
            return {};
        };

        String out;
        auto rootText = readText(L"root", false);
        String metadataGuid;
        if (rootText) {
            std::wregex rootGuidRgx(LR"(\{\s*-?\d+\s*,\s*([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\s*,)");
            std::wregex guidRgxAny(LR"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))");
            std::wsmatch rm;
            if (std::regex_search(*rootText, rm, rootGuidRgx) && rm.size() >= 2) {
                metadataGuid = toLower(rm[1].str());
                out += L"root: guid метаданных: " + metadataGuid + L"\n";
            } else if (std::regex_search(*rootText, rm, guidRgxAny) && rm.size() >= 2) {
                metadataGuid = toLower(rm[1].str());
                out += L"root: guid метаданных: " + metadataGuid + L"\n";
            } else {
                metadataGuid = extractRootMetadataGuidFromRaw();
                if (!metadataGuid.empty()) {
                    out += L"root: metadata guid (raw): " + metadataGuid + L"\n";
                } else {
                    out += L"root: metadata guid not found\n";
                }
            }
        } else {
            metadataGuid = extractRootMetadataGuidFromRaw();
            if (!metadataGuid.empty()) out += L"root: metadata guid (raw): " + metadataGuid + L"\n";
            else out += L"root: not found\n";
        }

        auto metaText = metadataGuid.empty() ? std::optional<String>{} : readText(metadataGuid, true);

        std::unordered_map<String, String> sectionTitle;
        std::unordered_set<String> sectionSet;
        for (const auto& s : kSections) {
            const String g = toLower(s.guid);
            sectionTitle[g] = s.title;
            sectionSet.insert(g);
        }
        auto collectSectionCounts = [&](const String& text) {
            std::unordered_map<String, size_t> sectionCounts;
            auto tree = parse1CText(text);
            if (tree) {
                for (const auto& s : kSections) {
                    const String g = toLower(s.guid);
                    const int declared = getSectionDeclaredCount(tree.get(), g);
                    if (declared > 0) {
                        sectionCounts[g] = static_cast<size_t>(declared);
                        continue;
                    }
                    auto guids = collectSectionObjectGuids(tree.get(), g);
                    sectionCounts[g] = guids.size();
                }
                return sectionCounts;
            }

            // Fallback for non-standard / unparsable metadata text.
            std::wregex guidRgx(LR"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))");
            String activeSection;
            std::unordered_map<String, std::unordered_set<String>> sectionObjects;
            auto it = std::wsregex_iterator(text.begin(), text.end(), guidRgx);
            auto end = std::wsregex_iterator();
            for (; it != end; ++it) {
                String g = toLower((*it)[1].str());
                if (sectionSet.find(g) != sectionSet.end()) {
                    activeSection = g;
                    continue;
                }
                if (!activeSection.empty()) sectionObjects[activeSection].insert(g);
            }
            for (const auto& [sg, set] : sectionObjects) sectionCounts[sg] = set.size();
            return sectionCounts;
        };

        std::unordered_map<String, size_t> sectionCounts;
        if (metaText) {
            sectionCounts = collectSectionCounts(*metaText);
        }

        size_t total = 0;
        for (const auto& s : kSections) total += sectionCounts[toLower(s.guid)];

        if (total == 0) {
            // Fallback: pick best metadata candidate by section/object coverage.
            String bestGuid;
            std::optional<String> bestText;
            size_t bestSections = 0;
            size_t bestObjects = 0;
            for (const auto& e : elements_) {
                String name = toLower(e.getName());
                if (!isGuidLike(name)) continue;
                auto t = readText(name, true);
                if (!t) continue;
                auto so = collectSectionCounts(*t);
                size_t sCount = 0;
                size_t oCount = 0;
                for (const auto& s : kSections) {
                    const auto cnt = so[toLower(s.guid)];
                    if (cnt > 0) ++sCount;
                    oCount += cnt;
                }
                if (sCount > bestSections || (sCount == bestSections && oCount > bestObjects)) {
                    bestSections = sCount;
                    bestObjects = oCount;
                    bestGuid = name;
                    bestText = t;
                }
            }
            if (bestText && bestSections > 0) {
                out += L"fallback metadata guid: " + bestGuid + L"\n";
                sectionCounts = collectSectionCounts(*bestText);
                total = 0;
                for (const auto& s : kSections) total += sectionCounts[toLower(s.guid)];
            }
        }

        for (const auto& s : kSections) {
            const String g = toLower(s.guid);
            const size_t cnt = sectionCounts[g];
            String titleRu = sectionTitle[g];
            if (titleRu == L"Catalogs") titleRu = L"\u0421\u043f\u0440\u0430\u0432\u043e\u0447\u043d\u0438\u043a\u0438";
            else if (titleRu == L"Documents") titleRu = L"\u0414\u043e\u043a\u0443\u043c\u0435\u043d\u0442\u044b";
            else if (titleRu == L"Reports") titleRu = L"\u041e\u0442\u0447\u0435\u0442\u044b";
            else if (titleRu == L"Processings") titleRu = L"\u041e\u0431\u0440\u0430\u0431\u043e\u0442\u043a\u0438";
            else if (titleRu == L"CommonModules") titleRu = L"\u041e\u0431\u0449\u0438\u0435\u041c\u043e\u0434\u0443\u043b\u0438";
            else if (titleRu == L"Enums") titleRu = L"\u041f\u0435\u0440\u0435\u0447\u0438\u0441\u043b\u0435\u043d\u0438\u044f";
            else if (titleRu == L"Constants") titleRu = L"\u041a\u043e\u043d\u0441\u0442\u0430\u043d\u0442\u044b";
            else if (titleRu == L"InfoRegisters") titleRu = L"\u0420\u0435\u0433\u0438\u0441\u0442\u0440\u044b\u0421\u0432\u0435\u0434\u0435\u043d\u0438\u0439";
            else if (titleRu == L"AccumRegisters") titleRu = L"\u0420\u0435\u0433\u0438\u0441\u0442\u0440\u044b\u041d\u0430\u043a\u043e\u043f\u043b\u0435\u043d\u0438\u044f";
            else if (titleRu == L"AccountingRegisters") titleRu = L"\u0420\u0435\u0433\u0438\u0441\u0442\u0440\u044b\u0411\u0443\u0445\u0433\u0430\u043b\u0442\u0435\u0440\u0438\u0438";
            else if (titleRu == L"CalculationRegisters") titleRu = L"\u0420\u0435\u0433\u0438\u0441\u0442\u0440\u044b\u0420\u0430\u0441\u0447\u0435\u0442\u0430";
            else if (titleRu == L"ChartOfAccounts") titleRu = L"\u041f\u043b\u0430\u043d\u044b\u0421\u0447\u0435\u0442\u043e\u0432";
            else if (titleRu == L"ChartOfCharacteristicTypes") titleRu = L"\u041f\u043b\u0430\u043d\u044b\u0412\u0438\u0434\u043e\u0432\u0425\u0430\u0440\u0430\u043a\u0442\u0435\u0440\u0438\u0441\u0442\u0438\u043a";
            else if (titleRu == L"ChartOfCalculationTypes") titleRu = L"\u041f\u043b\u0430\u043d\u044b\u0412\u0438\u0434\u043e\u0432\u0420\u0430\u0441\u0447\u0435\u0442\u0430";
            else if (titleRu == L"ExchangePlans") titleRu = L"\u041f\u043b\u0430\u043d\u044b\u041e\u0431\u043c\u0435\u043d\u0430";
            else if (titleRu == L"DocumentJournals") titleRu = L"\u0416\u0443\u0440\u043d\u0430\u043b\u044b\u0414\u043e\u043a\u0443\u043c\u0435\u043d\u0442\u043e\u0432";
            else if (titleRu == L"Numerators") titleRu = L"\u041d\u0443\u043c\u0435\u0440\u0430\u0442\u043e\u0440\u044b";
            else if (titleRu == L"Tasks") titleRu = L"\u0417\u0430\u0434\u0430\u0447\u0438";
            else if (titleRu == L"BusinessProcesses") titleRu = L"\u0411\u0438\u0437\u043d\u0435\u0441\u041f\u0440\u043e\u0446\u0435\u0441\u0441\u044b";
            else if (titleRu == L"Subsystems") titleRu = L"\u041f\u043e\u0434\u0441\u0438\u0441\u0442\u0435\u043c\u044b";
            out += titleRu + L" [" + g + L"]: " + std::to_wstring(cnt) + L"\n";
        }
        out += L"Total objects: " + std::to_wstring(total);
        return out;

    std::shared_ptr<MetadataItem> V8Container::buildMetadataTree() const {
        auto root = std::make_shared<MetadataItem>();
        root->name = L"Configuration";
        root->type = L"Root";
        root->is_folder = true;

        auto new_toLower = [](String value) {
            std::transform(value.begin(), value.end(), value.begin(),
                [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
            return value;
        };
        auto new_isGuidLike = [](const String& value) {
            static const std::wregex re(
                LR"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
            return std::regex_match(value, re);
        };
        auto new_decodeText = [&](const V8Element& elem, bool requireBraces) -> std::optional<String> {
            auto decodeUtf16 = [](const std::vector<uint8_t>& data) -> std::optional<String> {
                if (data.empty() || (data.size() % 2) != 0) return std::nullopt;
                String result(data.size() / 2, L'\0');
                std::memcpy(result.data(), data.data(), data.size());
                return result;
            };
            auto inflateWith = [](const std::vector<uint8_t>& src, int windowBits) -> std::vector<uint8_t> {
                if (src.empty()) return {};
                z_stream stream{};
                stream.next_in = const_cast<Bytef*>(src.data());
                stream.avail_in = static_cast<uInt>(src.size());
                if (inflateInit2(&stream, windowBits) != Z_OK) { inflateEnd(&stream); return {}; }
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

            std::vector<std::vector<uint8_t>> candidates;
            const auto& raw = elem.getData();
            if (raw.empty()) return std::nullopt;
            candidates.push_back(raw);
            auto z1 = inflateWith(raw, -MAX_WBITS);
            if (!z1.empty()) candidates.push_back(std::move(z1));
            auto z2 = inflateWith(raw, MAX_WBITS);
            if (!z2.empty()) candidates.push_back(std::move(z2));
            if (elem.isCompressed()) {
                auto z3 = decompressZlib(raw);
                if (!z3.empty()) candidates.push_back(std::move(z3));
            }

            for (const auto& blob : candidates) {
                auto utf = decodeUtf16(blob);
                if (utf) {
                    if (!requireBraces || utf->find(L'{') != String::npos) return utf;
                }
                String ansi;
                ansi.reserve(blob.size());
                for (uint8_t b : blob) ansi.push_back(static_cast<wchar_t>(b));
                if (!ansi.empty()) {
                    if (!requireBraces || ansi.find(L'{') != String::npos) return ansi;
                }
            }
            return std::nullopt;
        };
        std::unordered_map<String, const V8Element*> new_byNameLower;
        new_byNameLower.reserve(elements_.size());
        for (const auto& e : elements_) {
            new_byNameLower[new_toLower(e.getName())] = &e;
        }
        auto new_getTextByName = [&](const String& fileName) -> std::optional<String> {
            const V8Element* elem = findElement(fileName);
            if (!elem) {
                auto it = new_byNameLower.find(new_toLower(fileName));
                if (it != new_byNameLower.end()) elem = it->second;
            }
            if (!elem) {
                // CF frequently stores object payload as GUID.0/GUID.1 even when metadata references bare GUID.
                const String low = new_toLower(fileName);
                for (const auto& [n, p] : new_byNameLower) {
                    if (n.rfind(low + L".", 0) == 0) {
                        elem = p;
                        break;
                    }
                }
            }
            if (!elem) return std::nullopt;
            const String low = new_toLower(fileName);
            const bool requireBraces = !(low == L"version" || low == L"root");
            return new_decodeText(*elem, requireBraces);
        };
        auto new_rawFallback = [&]() -> std::shared_ptr<MetadataItem> {
            auto raw = std::make_shared<MetadataItem>();
            raw->id = L"folder_raw";
            raw->name = L"ContainerRaw";
            raw->type = L"Folder";
            raw->is_folder = true;
            std::unordered_map<String, std::vector<String>> groups;
            for (const auto& elem : elements_) {
                String n = new_toLower(elem.getName());
                if (n.empty() || n == L"root" || n == L"version" || n == L"versions") continue;
                const auto dot = n.find(L'.');
                if (dot != String::npos && new_isGuidLike(n.substr(0, dot))) groups[n.substr(0, dot)].push_back(n);
                else if (new_isGuidLike(n)) groups[n].push_back(n);
            }
            for (const auto& [g, entries] : groups) {
                auto folder = std::make_shared<MetadataItem>();
                folder->id = L"group_" + g;
                folder->name = g;
                folder->type = L"RawGroup";
                folder->is_folder = true;
                for (const auto& e : entries) {
                    auto leaf = std::make_shared<MetadataItem>();
                    leaf->id = e;
                    leaf->name = e;
                    leaf->type = L"Raw";
                    leaf->uuid = e;
                    leaf->is_folder = false;
                    folder->children.push_back(leaf);
                }
                raw->children.push_back(folder);
            }
            root->children.push_back(raw);
            return root;
        };

        const auto new_boot = bootstrapMetadataTree(new_getTextByName);

        static const std::vector<std::pair<String, String>> new_sections = {
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
            {L"37f2fa9a-b276-11d4-9435-004095e12fc7", L"Subsystems"}
        };
        const std::unordered_map<String, std::vector<int>> new_namePaths = {
            {L"cf4abea6-37b2-11d4-940f-008048da11f9", {0,1,9,1,2}},
            {L"061d872a-5787-460e-95ac-ed74ea3a3e84", {0,1,9,1,2}},
            {L"0195e80c-b157-11d4-9435-004095e12fc7", {0,1,1,1,1,2}},
            {L"13134201-f60b-11d5-a3c7-0050bae0a776", {0,1,15,1,2}},
            {L"f2de87a8-64e5-45eb-a22d-b3aedab050e7", {0,1,15,1,2}},
            {L"857c4a91-e5f4-4fac-86ec-787626f1c108", {0,1,12,2}},
            {L"37f2fa9a-b276-11d4-9435-004095e12fc7", {0,1,1,2}},
            {L"fcd3404e-1523-48ce-9bc0-ecdb822684a1", {0,1,1,2}},
            {L"631b75a0-29e2-11d6-a3c7-0050bae0a776", {0,1,3,1,2}},
            {L"bf845118-327b-4682-b5c6-285d2a0eb296", {0,1,3,1,2}},
            {L"2deed9b8-0056-4ffe-a473-c20a6c32a0bc", {0,1,16,1,2}},
            {L"b64d9a40-1642-11d6-a3c7-0050bae0a776", {0,1,13,1,2}},
            {L"238e7e88-3c5f-48b2-8a3b-81ebbecb20ed", {0,1,15,1,2}},
            {L"82a1b659-b220-4d94-a9bd-14d757b95a48", {0,1,13,1,2}},
            {L"30b100d6-b29f-47ac-aec7-cb8ca8a54767", {0,1,1,1,2}},
            {L"0fe48980-252d-11d6-a3c7-0050bae0a776", {0,1,1,2}}
        };
        auto new_guessMetadataGuid = [&]() -> String {
            struct CandidateScore {
                String guid;
                int sectionHits{0};
                int objectHits{0};
            };
            CandidateScore best;
            for (const auto& e : elements_) {
                String name = new_toLower(e.getName());
                if (!new_isGuidLike(name)) continue;
                auto txt = new_decodeText(e, true);
                if (!txt) continue;
                auto t = parse1CText(*txt);
                if (!t) continue;

                int sectionHits = 0;
                int objectHits = 0;
                for (const auto& [sg, _title] : new_sections) {
                    auto guids = collectSectionObjectGuids(t.get(), sg);
                    if (!guids.empty()) {
                        ++sectionHits;
                        objectHits += static_cast<int>(guids.size());
                    }
                }

                if (sectionHits == 0) {
                    // Fallback text score for non-standard trees.
                    String low = new_toLower(*txt);
                    for (const auto& [sg, _title] : new_sections) {
                        if (low.find(new_toLower(sg)) != String::npos) ++sectionHits;
                    }
                }

                if (sectionHits > best.sectionHits ||
                    (sectionHits == best.sectionHits && objectHits > best.objectHits)) {
                    best = {name, sectionHits, objectHits};
                }
            }
            return best.sectionHits > 0 ? best.guid : String{};
        };
        auto new_hasObjectStorage = [&](const String& guid) -> bool {
            if (new_byNameLower.find(guid) != new_byNameLower.end()) return true;
            for (const auto& [n, _p] : new_byNameLower) {
                if (n.rfind(guid + L".", 0) == 0) return true;
            }
            return false;
        };

        String activeMetadataGuid;
        std::unique_ptr<TreeNode> activeMetadataTree;
        String activeMetadataText;
        if (new_boot.ok && new_boot.metadata_tree) {
            activeMetadataGuid = new_boot.metadata_guid;
            auto txt = new_getTextByName(activeMetadataGuid);
            if (txt) {
                activeMetadataText = *txt;
                activeMetadataTree = parse1CText(*txt);
            }
            std::wcerr << L"[md-bootstrap] ok, version=" << new_boot.version
                       << L", metadata_guid=" << activeMetadataGuid << L"\n";
        } else {
            std::wcerr << L"[md-bootstrap] failed: " << new_boot.error << L"\n";
            activeMetadataGuid = new_guessMetadataGuid();
            if (!activeMetadataGuid.empty()) {
                auto txt = new_getTextByName(activeMetadataGuid);
                if (txt) {
                    activeMetadataText = *txt;
                    activeMetadataTree = parse1CText(*txt);
                }
                std::wcerr << L"[md-bootstrap] fallback metadata_guid=" << activeMetadataGuid << L"\n";
            }
        }
        if (!activeMetadataTree) return ensureRawFallback();

        auto new_collectSectionGuidsRegex = [&](const String& sectionGuid) -> std::vector<String> {
            std::vector<String> out;
            if (activeMetadataText.empty()) return out;
            static const std::wregex rgxGuid(LR"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))");
            std::unordered_set<String> sectionSet;
            for (const auto& [sg, _] : new_sections) sectionSet.insert(new_toLower(sg));
            std::unordered_set<String> uniq;
            String activeSection;
            auto begin = std::wsregex_iterator(activeMetadataText.begin(), activeMetadataText.end(), rgxGuid);
            auto end = std::wsregex_iterator();
            for (auto it = begin; it != end; ++it) {
                String g = new_toLower((*it)[1].str());
                if (!new_isGuidLike(g)) continue;
                if (sectionSet.find(g) != sectionSet.end()) {
                    activeSection = g;
                    continue;
                }
                if (activeSection == new_toLower(sectionGuid) && new_hasObjectStorage(g) && uniq.insert(g).second) {
                    out.push_back(g);
                }
            }
            return out;
        };

        int totalSectionsBuilt = 0;
        for (const auto& [guid, title] : new_sections) {
            auto objectGuids = collectSectionObjectGuids(activeMetadataTree.get(), guid);
            if (objectGuids.empty()) {
                objectGuids = new_collectSectionGuidsRegex(guid);
            }
            if (objectGuids.empty()) continue;
            ++totalSectionsBuilt;

            auto folder = std::make_shared<MetadataItem>();
            folder->id = L"folder_" + new_toLower(guid);
            folder->name = title;
            folder->type = L"Folder";
            folder->is_folder = true;

            for (const auto& objectGuid : objectGuids) {
                String displayName = objectGuid;
                auto text = new_getTextByName(objectGuid);
                if (text) {
                    auto tree = parse1CText(*text);
                    auto itPath = new_namePaths.find(guid);
                    if (tree && itPath != new_namePaths.end()) {
                        TreeNode* nameNode = getNodeByPath(tree.get(), itPath->second);
                        if (nameNode && !nameNode->value.empty()) displayName = nameNode->value;
                    }
                }

                auto item = std::make_shared<MetadataItem>();
                item->id = objectGuid;
                item->uuid = objectGuid;
                item->name = displayName.empty() ? objectGuid : displayName;
                item->type = title;
                item->is_folder = false;
                folder->children.push_back(item);
            }
            if (!folder->children.empty()) root->children.push_back(folder);
        }

        if (totalSectionsBuilt == 0) {
            const String altGuid = new_guessMetadataGuid();
            if (!altGuid.empty() && altGuid != activeMetadataGuid) {
                std::wcerr << L"[md-bootstrap] retry with better metadata_guid=" << altGuid << L"\n";
                auto txt = new_getTextByName(altGuid);
                if (txt) {
                    activeMetadataText = *txt;
                    activeMetadataTree = parse1CText(*txt);
                    root->children.clear();
                    for (const auto& [guid, title] : new_sections) {
                        auto objectGuids = collectSectionObjectGuids(activeMetadataTree.get(), guid);
                        if (objectGuids.empty()) objectGuids = new_collectSectionGuidsRegex(guid);
                        if (objectGuids.empty()) continue;

                        auto folder = std::make_shared<MetadataItem>();
                        folder->id = L"folder_" + new_toLower(guid);
                        folder->name = title;
                        folder->type = L"Folder";
                        folder->is_folder = true;

                        for (const auto& objectGuid : objectGuids) {
                            String displayName = objectGuid;
                            auto text = new_getTextByName(objectGuid);
                            if (text) {
                                auto tree = parse1CText(*text);
                                auto itPath = new_namePaths.find(guid);
                                if (tree && itPath != new_namePaths.end()) {
                                    TreeNode* nameNode = getNodeByPath(tree.get(), itPath->second);
                                    if (nameNode && !nameNode->value.empty()) displayName = nameNode->value;
                                }
                            }
                            auto item = std::make_shared<MetadataItem>();
                            item->id = objectGuid;
                            item->uuid = objectGuid;
                            item->name = displayName.empty() ? objectGuid : displayName;
                            item->type = title;
                            item->is_folder = false;
                            folder->children.push_back(item);
                        }
                        if (!folder->children.empty()) root->children.push_back(folder);
                    }
                }
            }
        }

        if (root->children.empty()) {
            return ensureRawFallback();
        }
        return root;

    // рџ”‘ РЇРІРЅР°СЏ РёРЅСЃС‚Р°РЅС†РёР°С†РёСЏ С€Р°Р±Р»РѕРЅРѕРІ (РѕР±СЏР·Р°С‚РµР»СЊРЅРѕ РґР»СЏ РєРѕРјРїРёР»СЏС†РёРё)
    template int V8Container::loadImpl<Format15>();
    template int V8Container::loadImpl<Format16>();

} // namespace v8reader::core
