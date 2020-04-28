#include <unicode/unistr.h>
#include <vector>
#pragma comment(lib, "icuuc.lib")

inline std::string convert(const std::string& val, const char* From, const char* To) {
    icu::UnicodeString src(val.c_str(), From);
    const size_t length = static_cast<size_t>(src.extract(0, src.length(), nullptr, To));
    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], To);
    return std::move(std::string(result.begin(), result.end() - 1));

}

std::string UTF8_To_ShiftJIS(const std::string& val) {
    constexpr const char* ShiftJIS = "shift-jis";
    constexpr const char* UTF8 = "utf8";
    return convert(val, UTF8, ShiftJIS);
}
