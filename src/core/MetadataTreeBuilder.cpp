#include "MetadataTreeBuilder.h"
#include "v8reader/core/TreeReader.h"
#include <zlib.h>
#include <algorithm>
#include <regex>
#include <unordered_set>
#include <cwctype>
#include <codecvt>

namespace v8reader::core {

// ============================================================================
// Статические константы
// ============================================================================

const std::vector<MetadataTreeBuilder::SectionInfo> MetadataTreeBuilder::kSections = {
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

const std::unordered_map<MetadataTreeBuilder::String, std::vector<int>> MetadataTreeBuilder::kNamePaths = {
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

// ============================================================================
// Конструктор / Деструктор
// ============================================================================

MetadataTreeBuilder::MetadataTreeBuilder(const V8Container* container)
    : m_container(container) {
}

// ============================================================================
// Публичный интерфейс
// ============================================================================

MetadataTreeBuilder::MetadataItemPtr MetadataTreeBuilder::build() {
    auto root = std::make_shared<MetadataItem>();
    root->id = L"root";
    root->name = L"Configuration";
    root->type = L"Root";
    root->is_folder = true;

    // 1. Инициализация
    initialize();

    // 2. Построение секций
    buildSections(root);

    // 3. Fallback, если ничего не построилось
    if (root->children.empty()) {
        return createRawFallback();
    }

    return root;
}

// ============================================================================
// Основные этапы построения
// ============================================================================

void MetadataTreeBuilder::initialize() {
    // Индексация элементов по имени (lower-case)
    for (const auto& elem : m_container->getElements()) {
        m_elementsByNameLower[toLower(elem.getName())] = &elem;
    }

    // Bootstrap метаданных
    auto bootstrapResult = bootstrapMetadata();

    if (bootstrapResult.ok && bootstrapResult.metadata_tree) {
        m_metadataGuid = bootstrapResult.metadata_guid;
        m_metadataTree = std::move(bootstrapResult.metadata_tree);
        
        // Загрузка текста метаданных
        auto textOpt = getTextByName(m_metadataGuid);
        if (textOpt) {
            m_metadataText = *textOpt;
        }
    } else {
        // Попытка угадать GUID метаданных
        m_metadataGuid = guessMetadataGuid();
        if (!m_metadataGuid.empty()) {
            auto textOpt = getTextByName(m_metadataGuid);
            if (textOpt) {
                m_metadataText = *textOpt;
                m_metadataTree = parse1CText(*textOpt);
            }
        }
    }
}

void MetadataTreeBuilder::buildSections(MetadataItemPtr root) {
    int sectionsBuilt = 0;

    for (const auto& section : kSections) {
        // Сбор GUID объектов секции
        std::vector<String> objectGuids;

        if (m_metadataTree) {
            objectGuids = collectSectionObjectGuids(m_metadataTree.get(), section.guid);
        }

        // Fallback на regex, если дерево не распарсилось или пусто
        if (objectGuids.empty()) {
            objectGuids = collectSectionGuidsRegex(section.guid);
        }

        if (objectGuids.empty()) {
            continue;
        }

        ++sectionsBuilt;

        // Создание папки секции
        auto folder = std::make_shared<MetadataItem>();
        folder->id = L"folder_" + toLower(section.guid);
        folder->name = section.title;
        folder->type = L"Folder";
        folder->is_folder = true;

        // Обработка каждого объекта секции
        for (const auto& objectGuid : objectGuids) {
            processObject(folder, objectGuid, section.guid);
        }

        if (!folder->children.empty()) {
            root->children.push_back(folder);
        }
    }
}

void MetadataTreeBuilder::processObject(
        MetadataItemPtr parent,
        const String& objectGuid,
        const String& sectionGuid) {

    String displayName = objectGuid;

    // Попытка получить текст объекта и извлечь имя
    auto textOpt = getTextByName(objectGuid);
    if (textOpt) {
        auto tree = parse1CText(*textOpt);
        auto itPath = kNamePaths.find(sectionGuid);
        
        if (tree && itPath != kNamePaths.end()) {
            TreeNode* nameNode = getNodeByPath(tree.get(), itPath->second);
            if (nameNode && !nameNode->value.empty()) {
                displayName = nameNode->value;
            }
        }
    }

    // Создание элемента метаданных
    auto item = std::make_shared<MetadataItem>();
    item->id = objectGuid;
    item->uuid = objectGuid;
    item->name = displayName.empty() ? objectGuid : displayName;
    item->type = parent->name;  // Тип = название секции
    item->is_folder = false;

    parent->children.push_back(item);

    // Сохранение в карту для возможной связи с детьми
    m_guidMap[objectGuid] = item.get();
}

// ============================================================================
// Вспомогательные методы
// ============================================================================

MetadataTreeBuilder::BootstrapResult MetadataTreeBuilder::bootstrapMetadata() {
    BootstrapResult result;

    // Поиск объекта 'root'
    const V8Element* rootElem = nullptr;
    auto it = m_elementsByNameLower.find(L"root");
    if (it != m_elementsByNameLower.end()) {
        rootElem = it->second;
    }

    if (!rootElem) {
        result.error = L"root element not found";
        return result;
    }

    // Декодирование текста root
    auto rootText = decodeText(*rootElem, false);
    if (!rootText) {
        result.error = L"failed to decode root text";
        return result;
    }

    // Поиск GUID метаданных в тексте root
    static const std::wregex rootGuidRgx(
        LR"(\{\s*-?\d+\s*,\s*([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\s*,)");
    
    std::wsmatch match;
    if (std::regex_search(*rootText, match, rootGuidRgx) && match.size() >= 2) {
        result.metadata_guid = toLower(match[1].str());
    }

    if (result.metadata_guid.empty()) {
        result.error = L"metadata guid not found in root";
        return result;
    }

    // Загрузка и парсинг текста метаданных
    auto metadataText = getTextByName(result.metadata_guid);
    if (!metadataText) {
        result.error = L"failed to load metadata text";
        return result;
    }

    result.metadata_tree = parse1CText(*metadataText);
    if (!result.metadata_tree) {
        result.error = L"failed to parse metadata tree";
        return result;
    }

    result.ok = true;
    result.version = L"1.0";  // TODO: извлечь из version
    return result;
}

std::optional<MetadataTreeBuilder::String> MetadataTreeBuilder::getTextByName(
        const String& fileName) const {
    
    const V8Element* elem = nullptr;
    
    // Прямой поиск
    auto it = m_elementsByNameLower.find(toLower(fileName));
    if (it != m_elementsByNameLower.end()) {
        elem = it->second;
    }

    // Поиск по префиксу (GUID.0, GUID.1 и т.д.)
    if (!elem) {
        const String low = toLower(fileName);
        for (const auto& [name, ptr] : m_elementsByNameLower) {
            if (name.rfind(low + L".", 0) == 0) {
                elem = ptr;
                break;
            }
        }
    }

    if (!elem) {
        return std::nullopt;
    }

    const String low = toLower(fileName);
    const bool requireBraces = !(low == L"version" || low == L"root");
    
    return decodeText(*elem, requireBraces);
}

std::optional<MetadataTreeBuilder::String> MetadataTreeBuilder::decodeText(
        const V8Element& elem, bool requireBraces) const {
    
    auto decodeUtf16 = [](const ByteArray& data) -> std::optional<String> {
        if (data.empty() || (data.size() % 2) != 0) return std::nullopt;
#ifdef _WIN32
        String result(data.size() / 2, L'\0');
        std::memcpy(result.data(), data.data(), data.size());
        return result;
#else
        try {
            std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>, wchar_t> conv;
            return conv.from_bytes(
                reinterpret_cast<const char*>(data.data()),
                reinterpret_cast<const char*>(data.data() + data.size()));
        } catch (...) {
            return std::nullopt;
        }
#endif
    };

    auto inflateWith = [](const ByteArray& src, int windowBits) -> ByteArray {
        if (src.empty()) return {};
        
        z_stream stream{};
        stream.next_in = const_cast<Bytef*>(src.data());
        stream.avail_in = static_cast<uInt>(src.size());
        
        if (inflateInit2(&stream, windowBits) != Z_OK) {
            inflateEnd(&stream);
            return {};
        }

        ByteArray out(1024 * 1024);
        while (true) {
            stream.next_out = out.data() + stream.total_out;
            stream.avail_out = static_cast<uInt>(out.size() - stream.total_out);
            
            const int ret = inflate(&stream, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) {
                out.resize(stream.total_out);
                break;
            }
            if (ret != Z_OK) {
                inflateEnd(&stream);
                return {};
            }
            if (stream.avail_out == 0) {
                out.resize(out.size() * 2);
            }
        }
        
        inflateEnd(&stream);
        return out;
    };

    // Кандидаты на декодирование
    std::vector<ByteArray> candidates;
    const auto& raw = elem.getData();
    
    if (raw.empty()) return std::nullopt;
    
    candidates.push_back(raw);
    
    auto z1 = inflateWith(raw, -MAX_WBITS);
    if (!z1.empty()) candidates.push_back(std::move(z1));
    
    auto z2 = inflateWith(raw, MAX_WBITS);
    if (!z2.empty()) candidates.push_back(std::move(z2));
    
    if (elem.isCompressed()) {
        auto z3 = m_container->decompressZlib(raw);
        if (!z3.empty()) candidates.push_back(std::move(z3));
    }

    // Перебор кандидатов
    for (const auto& blob : candidates) {
        auto utf = decodeUtf16(blob);
        if (utf) {
            if (!requireBraces || utf->find(L'{') != String::npos) {
                return utf;
            }
        }
        
        // ANSI fallback
        String ansi;
        ansi.reserve(blob.size());
        for (uint8_t b : blob) {
            ansi.push_back(static_cast<wchar_t>(b));
        }
        if (!ansi.empty() && (!requireBraces || ansi.find(L'{') != String::npos)) {
            return ansi;
        }
    }

    return std::nullopt;
}

std::vector<MetadataTreeBuilder::String> MetadataTreeBuilder::collectSectionGuidsRegex(
        const String& sectionGuid) const {
    
    std::vector<String> result;
    
    if (m_metadataText.empty()) {
        return result;
    }

    static const std::wregex rgxGuid(
        LR"(([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}))");

    std::unordered_set<String> sectionSet;
    for (const auto& s : kSections) {
        sectionSet.insert(toLower(s.guid));
    }

    std::unordered_set<String> uniqueGuids;
    String activeSection;

    auto begin = std::wsregex_iterator(m_metadataText.begin(), m_metadataText.end(), rgxGuid);
    auto end = std::wsregex_iterator();

    for (auto it = begin; it != end; ++it) {
        String g = toLower((*it)[1].str());
        
        if (!isGuidLike(g)) continue;

        // Проверка на секцию
        if (sectionSet.find(g) != sectionSet.end()) {
            activeSection = g;
            continue;
        }

        // Если текущая секция совпадает и объект есть в хранилище
        if (activeSection == toLower(sectionGuid) && 
            hasObjectStorage(g) && 
            uniqueGuids.insert(g).second) {
            result.push_back(g);
        }
    }

    return result;
}

bool MetadataTreeBuilder::hasObjectStorage(const String& guid) const {
    // Прямое наличие
    if (m_elementsByNameLower.find(guid) != m_elementsByNameLower.end()) {
        return true;
    }

    // Наличие по префиксу (GUID.0, GUID.1 и т.д.)
    for (const auto& [name, _ptr] : m_elementsByNameLower) {
        if (name.rfind(guid + L".", 0) == 0) {
            return true;
        }
    }

    return false;
}

MetadataTreeBuilder::String MetadataTreeBuilder::guessMetadataGuid() const {
    struct CandidateScore {
        String guid;
        int sectionHits{0};
        int objectHits{0};
    };

    CandidateScore best;

    for (const auto& elem : m_container->getElements()) {
        String name = toLower(elem.getName());
        
        if (!isGuidLike(name)) continue;

        auto txt = decodeText(elem, true);
        if (!txt) continue;

        auto tree = parse1CText(*txt);
        if (!tree) continue;

        int sectionHits = 0;
        int objectHits = 0;

        for (const auto& section : kSections) {
            auto guids = collectSectionObjectGuids(tree.get(), section.guid);
            if (!guids.empty()) {
                ++sectionHits;
                objectHits += static_cast<int>(guids.size());
            }
        }

        // Fallback: поиск по тексту
        if (sectionHits == 0) {
            String low = toLower(*txt);
            for (const auto& section : kSections) {
                if (low.find(toLower(section.guid)) != String::npos) {
                    ++sectionHits;
                }
            }
        }

        if (sectionHits > best.sectionHits ||
            (sectionHits == best.sectionHits && objectHits > best.objectHits)) {
            best = {name, sectionHits, objectHits};
        }
    }

    return best.sectionHits > 0 ? best.guid : String{};
}

MetadataTreeBuilder::MetadataItemPtr MetadataTreeBuilder::createRawFallback() const {
    auto root = std::make_shared<MetadataItem>();
    root->id = L"root";
    root->name = L"Configuration";
    root->type = L"Root";
    root->is_folder = true;

    auto raw = std::make_shared<MetadataItem>();
    raw->id = L"folder_raw";
    raw->name = L"ContainerRaw";
    raw->type = L"Folder";
    raw->is_folder = true;

    std::unordered_map<String, std::vector<String>> groups;

    for (const auto& elem : m_container->getElements()) {
        String n = toLower(elem.getName());
        
        if (n.empty() || n == L"root" || n == L"version" || n == L"versions") {
            continue;
        }

        const auto dot = n.find(L'.');
        if (dot != String::npos && isGuidLike(n.substr(0, dot))) {
            groups[n.substr(0, dot)].push_back(n);
        } else if (isGuidLike(n)) {
            groups[n].push_back(n);
        }
    }

    for (const auto& [guid, entries] : groups) {
        auto folder = std::make_shared<MetadataItem>();
        folder->id = L"group_" + guid;
        folder->name = guid;
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
}

MetadataTreeBuilder::String MetadataTreeBuilder::toLower(String value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return value;
}

bool MetadataTreeBuilder::isGuidLike(const String& value) {
    static const std::wregex re(
        LR"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
    return std::regex_match(value, re);
}

} // namespace v8reader::core
