#include "v8reader/core/Types.h"
#include <sstream>
#include <iomanip>

namespace v8::core {

    String Guid::toString() const {
        std::wstringstream ss;
        ss << std::uppercase << std::hex << std::setfill(L'0') << L"{";
        ss << std::setw(8) << data1 << L"-";
        ss << std::setw(4) << data2 << L"-";
        ss << std::setw(4) << data3 << L"-";
        ss << std::setw(2) << static_cast<int>(data4[0]) << std::setw(2) << static_cast<int>(data4[1]) << L"-";
        ss << std::setw(2) << static_cast<int>(data4[2]) << std::setw(2) << static_cast<int>(data4[3])
            << std::setw(2) << static_cast<int>(data4[4]) << std::setw(2) << static_cast<int>(data4[5])
            << std::setw(2) << static_cast<int>(data4[6]) << std::setw(2) << static_cast<int>(data4[7]);
        ss << L"}";
        return ss.str();
    }

    Guid Guid::parse(const String& str) {
        Guid result{};
        if (str.length() < 36 || str[0] != L'{' || str[36] != L'}') {
            return result;
        }

        std::wstringstream ss(str.substr(1, 36));
        wchar_t dash;
        ss >> std::hex >> result.data1 >> dash >> result.data2 >> dash >> result.data3 >> dash;

        for (int i = 0; i < 8; ++i) {
            int byte;
            ss >> std::hex >> byte;
            result.data4[i] = static_cast<uint8_t>(byte);
        }
        return result;
    }

} // namespace v8::core