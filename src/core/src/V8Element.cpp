#include "v8reader/core/V8Element.h"
#include <cstring>

namespace v8::core {

    V8Element::V8Element(const String& name) {
        setName(name);
    }

    void V8Element::setName(const String& name) {
        name_ = name;

        // ���������� ���������: ����.����� + ��� � UTF-16LE + 4 ����
        // ��� �������� ��� wchar_t[], ������ ������ 2 �����, ��� ������������� ����
        size_t name_bytes = name.size() * sizeof(wchar_t);
        size_t header_size = ElemHeaderFixed::Size() + name_bytes + 4;

        header_.assign(header_size, 0);

        // �������� ������������� ����� (����������� ����� ����� buildHeader)
        // �������� ���
        if (!name.empty()) {
            std::memcpy(header_.data() + ElemHeaderFixed::Size(),
                name.data(), name_bytes);
        }
        // ��������� 4 ����� ��� ���� �� ���������
    }

    uint64_t V8Element::getCreateTime() const {
        if (!cached_create_time_ && !header_.empty()) {
            if (header_.size() >= sizeof(uint64_t)) {
                uint64_t val;
                std::memcpy(&val, header_.data(), sizeof(uint64_t));
                cached_create_time_ = val;
            }
        }
        return cached_create_time_.value_or(0);
    }

    uint64_t V8Element::getModifyTime() const {
        if (!cached_modify_time_ && header_.size() >= 16) {
            uint64_t val;
            std::memcpy(&val, header_.data() + 8, sizeof(uint64_t));
            cached_modify_time_ = val;
        }
        return cached_modify_time_.value_or(0);
    }

    bool V8Element::parseHeader() {
        if (header_.size() < ElemHeaderFixed::Size()) {
            return false;
        }

        // ��������� ��������� �����
        std::memcpy(&cached_create_time_, header_.data(), sizeof(uint64_t));
        std::memcpy(&cached_modify_time_, header_.data() + 8, sizeof(uint64_t));

        // ��������� ���: ����� ����.����� ��� UTF-16LE ������ �� ������� 0x0000
        size_t name_start = ElemHeaderFixed::Size();
        size_t name_len = 0;

        for (size_t i = name_start; i + 1 < header_.size(); i += 2) {
            if (header_[i] == 0 && header_[i + 1] == 0) {
                break;
            }
            name_len += 2;
        }

        if (name_len > 0) {
            name_.assign(
                reinterpret_cast<const wchar_t*>(header_.data() + name_start),
                name_len / sizeof(wchar_t)
            );
        }

        return true;
    }

    void V8Element::buildHeader(uint64_t create_time, uint64_t modify_time) {
        size_t name_bytes = name_.size() * sizeof(wchar_t);
        size_t header_size = ElemHeaderFixed::Size() + name_bytes + 4;

        header_.assign(header_size, 0);

        // ���������� ��������� �����
        std::memcpy(header_.data(), &create_time, sizeof(uint64_t));
        std::memcpy(header_.data() + 8, &modify_time, sizeof(uint64_t));
        // reserved (4 �����) ��� 0

        // ���������� ���
        if (!name_.empty()) {
            std::memcpy(header_.data() + ElemHeaderFixed::Size(),
                name_.data(), name_bytes);
        }
    }

} // namespace v8::core