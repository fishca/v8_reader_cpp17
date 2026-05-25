#include "v8reader/core/V8Format.h"
#include <charconv>
#include <system_error>

namespace v8reader::core {

    // ��������������� ������� ��� hex <-> uint �����������

    namespace detail {
        // ����������� uint32 -> 8-���������� hex (little-endian ������� �������� �� �����, ������ ������)
        void uint32_to_hex(uint32_t value, char* out_hex_8) {
            // ������: 8 hex-��������, �������� "00001000"
            for (int i = 7; i >= 0; --i) {
                uint8_t nibble = value & 0xF;
                out_hex_8[i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
                value >>= 4;
            }
        }

        uint32_t hex_to_uint32(const char* hex_8) {
            uint32_t result = 0;
            for (int i = 0; i < 8; ++i) {
                result <<= 4;
                char c = hex_8[i];
                if (c >= '0' && c <= '9') result |= (c - '0');
                else if (c >= 'a' && c <= 'f') result |= (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') result |= (c - 'A' + 10);
            }
            return result;
        }

        void uint64_to_hex(uint64_t value, char* out_hex_16) {
            for (int i = 15; i >= 0; --i) {
                uint8_t nibble = value & 0xF;
                out_hex_16[i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
                value >>= 4;
            }
        }

        uint64_t hex_to_uint64(const char* hex_16) {
            uint64_t result = 0;
            for (int i = 0; i < 16; ++i) {
                result <<= 4;
                char c = hex_16[i];
                if (c >= '0' && c <= '9') result |= (c - '0');
                else if (c >= 'a' && c <= 'f') result |= (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') result |= (c - 'A' + 10);
            }
            return result;
        }
    }

    // ===== BlockHeader15 =====

    uint32_t BlockHeader15::getDataSize() const {
        return detail::hex_to_uint32(data_size_hex.data());
    }

    uint32_t BlockHeader15::getPageSize() const {
        return detail::hex_to_uint32(page_size_hex.data());
    }

    uint32_t BlockHeader15::getNextAddr() const {
        return detail::hex_to_uint32(next_addr_hex.data());
    }

    void BlockHeader15::setDataSize(uint32_t size) {
        detail::uint32_to_hex(size, data_size_hex.data());
    }

    void BlockHeader15::setPageSize(uint32_t size) {
        detail::uint32_to_hex(size, page_size_hex.data());
    }

    void BlockHeader15::setNextAddr(uint32_t addr) {
        detail::uint32_to_hex(addr, next_addr_hex.data());
    }

    BlockHeader15 BlockHeader15::create(uint32_t data_size, uint32_t page_size, uint32_t next_addr) {
        BlockHeader15 hdr;
        hdr.setDataSize(data_size);
        hdr.setPageSize(page_size);
        hdr.setNextAddr(next_addr);
        return hdr;
    }

    // ===== BlockHeader16 =====

    uint64_t BlockHeader16::getDataSize() const {
        return detail::hex_to_uint64(data_size_hex.data());
    }

    uint64_t BlockHeader16::getPageSize() const {
        return detail::hex_to_uint64(page_size_hex.data());
    }

    uint64_t BlockHeader16::getNextAddr() const {
        return detail::hex_to_uint64(next_addr_hex.data());
    }

    void BlockHeader16::setDataSize(uint64_t size) {
        detail::uint64_to_hex(size, data_size_hex.data());
    }

    void BlockHeader16::setPageSize(uint64_t size) {
        detail::uint64_to_hex(size, page_size_hex.data());
    }

    void BlockHeader16::setNextAddr(uint64_t addr) {
        detail::uint64_to_hex(addr, next_addr_hex.data());
    }

    BlockHeader16 BlockHeader16::create(uint64_t data_size, uint64_t page_size, uint64_t next_addr) {
        BlockHeader16 hdr;
        hdr.setDataSize(data_size);
        hdr.setPageSize(page_size);
        hdr.setNextAddr(next_addr);
        return hdr;
    }

} // namespace v8reader::core