#pragma once
#include "Types.h"
#include "V8Format.h"
#include <vector>
#include <memory>
#include <optional>

namespace v8::core {

    // ������������� �������� (����� ��� ��������) ������ .cf
    class V8Element {
    public:
        V8Element() = default;
        explicit V8Element(const String& name);

        // �������
        [[nodiscard]] const String& getName() const { return name_; }
        [[nodiscard]] const std::vector<uint8_t>& getHeader() const { return header_; }
        [[nodiscard]] const std::vector<uint8_t>& getData() const { return data_; }

        // �������
        void setName(const String& name);
        void setHeader(std::vector<uint8_t> header) { header_ = std::move(header); }
        void setData(std::vector<uint8_t> data) { data_ = std::move(data); }

        // �����
        [[nodiscard]] bool isCatalog() const { return is_catalog_; }
        void setIsCatalog(bool v) { is_catalog_ = v; }

        [[nodiscard]] bool isCompressed() const { return is_compressed_; }
        void setIsCompressed(bool v) { is_compressed_ = v; }

        // ��������� ����� �� ���������
        [[nodiscard]] uint64_t getCreateTime() const;
        [[nodiscard]] uint64_t getModifyTime() const;

        // ������� ��������� ��������
        [[nodiscard]] bool parseHeader();

        // ������ ��������� �� �����
        void buildHeader(uint64_t create_time, uint64_t modify_time);

    private:
        String name_;
        std::vector<uint8_t> header_;  // ����� ��������� (����.����� + UTF-16 ��� + 4 ����)
        std::vector<uint8_t> data_;    // ������ �������� (�������� ������)

        bool is_catalog_ = false;
        bool is_compressed_ = false;

        // ������������ �������� �� ���������
        mutable std::optional<uint64_t> cached_create_time_;
        mutable std::optional<uint64_t> cached_modify_time_;
    };

} // namespace v8::core