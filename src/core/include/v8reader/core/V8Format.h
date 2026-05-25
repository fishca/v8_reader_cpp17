#pragma once
#include "v8_constants.h"
#include <cstdint>
#include <cstddef>
#include <array>

namespace v8reader::core {

    // ========== Заголовки файла ==========
    struct FileHeader15 {
        uint32_t next_page_addr = FAT_UNDEFINED_32;
        uint32_t page_size = DEFAULT_PAGE_SIZE;
        uint32_t storage_ver = 0;
        uint32_t reserved = 0;

        static constexpr size_t Size() { return 16; }
        [[nodiscard]] bool isValid() const { return reserved == 0; }
    };

    struct FileHeader16 {
        uint64_t next_page_addr = FAT_UNDEFINED_64;
        uint32_t page_size = DEFAULT_PAGE_SIZE;
        uint32_t storage_ver = 0;
        uint32_t reserved = 0;

        static constexpr size_t Size() { return 20; }
        [[nodiscard]] bool isValid() const { return reserved == 0; }
    };

    // ========== FAT Entry ==========
    struct FatEntry15 {
        uint32_t header_addr;
        uint32_t data_addr;
        uint32_t terminator; // Читаем из файла напрямую

        // ✅ ИСПРАВЛЕНО: Статическая константа для шаблонов
        static constexpr uint32_t UNDEFINED_VALUE = FAT_UNDEFINED_32;
        static constexpr size_t Size() { return 12; }
        [[nodiscard]] bool isTerminator() const { return header_addr == UNDEFINED_VALUE; }
    };

    struct FatEntry16 {
        uint64_t header_addr;
        uint64_t data_addr;
        uint64_t terminator;

        static constexpr uint64_t UNDEFINED_VALUE = FAT_UNDEFINED_64;
        static constexpr size_t Size() { return 24; }
        [[nodiscard]] bool isTerminator() const { return header_addr == UNDEFINED_VALUE; }
    };

    // ========== Block Headers ==========
    struct BlockHeader15 {
        char cr1, lf1;
        std::array<char, 8> data_size_hex;
        char space1;
        std::array<char, 8> page_size_hex;
        char space2;
        std::array<char, 8> next_addr_hex;
        char space3;
        char cr2, lf2;

        static constexpr size_t Size() { return 31; }
        [[nodiscard]] bool isValid() const {
            return cr1 == 0x0d && lf1 == 0x0a && space1 == ' ' &&
                space2 == ' ' && space3 == ' ' && cr2 == 0x0d && lf2 == 0x0a;
        }

        [[nodiscard]] uint32_t getDataSize() const;
        [[nodiscard]] uint32_t getPageSize() const;
        [[nodiscard]] uint32_t getNextAddr() const;
        void setDataSize(uint32_t size);
        void setPageSize(uint32_t size);
        void setNextAddr(uint32_t addr);
        static BlockHeader15 create(uint32_t data_size, uint32_t page_size, uint32_t next_addr = FAT_UNDEFINED_32);
    };

    struct BlockHeader16 {
        char cr1, lf1;
        std::array<char, 16> data_size_hex;
        char space1;
        std::array<char, 16> page_size_hex;
        char space2;
        std::array<char, 16> next_addr_hex;
        char space3;
        char cr2, lf2;

        static constexpr size_t Size() { return 55; }
        [[nodiscard]] bool isValid() const {
            return cr1 == 0x0d && lf1 == 0x0a && space1 == ' ' &&
                space2 == ' ' && space3 == ' ' && cr2 == 0x0d && lf2 == 0x0a;
        }

        [[nodiscard]] uint64_t getDataSize() const;
        [[nodiscard]] uint64_t getPageSize() const;
        [[nodiscard]] uint64_t getNextAddr() const;
        void setDataSize(uint64_t size);
        void setPageSize(uint64_t size);
        void setNextAddr(uint64_t addr);
        static BlockHeader16 create(uint64_t data_size, uint64_t page_size, uint64_t next_addr = FAT_UNDEFINED_64);
    };

    // ========== Element Header ==========
    struct ElemHeaderFixed {
        uint64_t time_create;
        uint64_t time_modify;
        uint32_t reserved;
        static constexpr size_t Size() { return 20; }
    };

    // ========== Format Policy ==========
    template<typename FatT, typename BlockHdrT, typename FileHdrT>
    struct FormatPolicy {
        using fat_entry_t = FatT;
        using block_header_t = BlockHdrT;
        using file_header_t = FileHdrT;

        // ✅ ТЕПЕРЬ ВАЛИДНО: Берем static constexpr из структуры
        static constexpr auto UNDEFINED_VALUE = FatT::UNDEFINED_VALUE;
        static constexpr auto BASE_OFFSET = std::is_same_v<FileHdrT, FileHeader16>
            ? FORMAT16_BASE_OFFSET : 0;
        static constexpr uint64_t DEFAULT_PAGE = DEFAULT_PAGE_SIZE;
    };

    using Format15 = FormatPolicy<FatEntry15, BlockHeader15, FileHeader15>;
    using Format16 = FormatPolicy<FatEntry16, BlockHeader16, FileHeader16>;

} // namespace v8reader::core
