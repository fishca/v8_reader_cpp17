#pragma once
#include <cstdint>
#include <cstddef>

namespace v8::core {

	// ���������� ������ 1�
	constexpr const wchar_t* EXT_CF = L".cf";
	constexpr const wchar_t* EXT_CFU = L".cfu";
	constexpr const wchar_t* EXT_CFE = L".cfe";
	constexpr const wchar_t* EXT_EPF = L".epf";
	constexpr const wchar_t* EXT_ERF = L".erf";

	// ��������� � ��������� �������
	constexpr uint32_t FAT_UNDEFINED_32 = 0x7fffffff;
	constexpr uint64_t FAT_UNDEFINED_64 = 0xffffffffffffffffull;

	constexpr uint32_t DEFAULT_PAGE_SIZE = 512;
	constexpr uint64_t FORMAT16_BASE_OFFSET = 0x1359;  // �������� ��� ������� 8.3.16+

	// ���� ��������
	constexpr int V8_OK = 0;
	constexpr int V8_ERROR = -50;
	constexpr int V8_NOT_V8_FILE = V8_ERROR - 1;
	constexpr int V8_HEADER_CORRUPT = V8_ERROR - 2;
	constexpr int V8_FILE_NOT_FOUND = V8_ERROR - 3;
	constexpr int V8_ZLIB_ERROR = V8_ERROR - 20;

	// ������ ��������� ��������
	constexpr size_t ELEM_HEADER_FIXED_SIZE = 8 + 8 + 4; // date_create + date_modify + reserved

} // namespace v8::core