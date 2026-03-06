#include "csv_parser.hpp"
#include <charconv>

namespace csv_median_calculator {

    /**
 * @brief ѕарсит строку CSV и извлекает запись (receive_ts, price).
 *
 * ‘ункци€ предназначена дл€ высокоскоростного разбора строк формата CSV с разделителем ';'.
 * »з всех полей нас интересуют
 * только receive_ts (индекс 0) и price (индекс 2); остальные пол€ (exchange_ts, quantity, side,
 * и опционально rebuild) игнорируютс€.
 *
 * јлгоритм работы:
 * 1. ”станавливаем указатели на начало и конец строки.
 * 2. ѕроходим по строке, выдел€€ пол€ до точки с зап€той или конца строки.
 * 3. ƒл€ пол€ с индексом 0 пытаемс€ преобразовать его в uint64_t (receive_ts).
 * 4. ƒл€ пол€ с индексом 2 пытаемс€ преобразовать его в double (price).
 * 5. ≈сли преобразование любого из об€зательных полей не удалось или строка содержит
 *    менее трЄх полей, возвращаем std::nullopt.
 * 6. ¬ противном случае возвращаем структуру record с полученными значени€ми.
 *
 * @param line —трока CSV (без завершающего символа новой строки).
 * @return std::optional<record>, содержащий запись при успехе, иначе std::nullopt.
 */
    std::optional<record> parse_record(const std::string& line) {
        const char* begin = line.data();
        const char* end = begin + line.size();
        const char* ptr = begin;
        int field_idx = 0;
        uint64_t ts = 0;
        double price = 0.0;
        bool have_ts = false, have_price = false;

        while (ptr < end && field_idx <= 2) {
            const char* field_start = ptr;
            while (ptr < end && *ptr != ';') ++ptr;
            if (field_idx == 0) {
                auto [p, ec] = std::from_chars(field_start, ptr, ts);
                if (ec != std::errc()) return std::nullopt;
                have_ts = true;
            }
            else if (field_idx == 2) {
                auto [p, ec] = std::from_chars(field_start, ptr, price);
                if (ec != std::errc()) return std::nullopt;
                have_price = true;
            }
            ++field_idx;
            if (ptr < end && *ptr == ';') ++ptr;
        }
        if (!have_ts || !have_price) return std::nullopt;
        return record{ ts, price };
    }

} 