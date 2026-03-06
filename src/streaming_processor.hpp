#pragma once

#include "config_parser.hpp"
#include "types.hpp"
#include "thread_safe_priority_queue.hpp"
#include "median_calculator.hpp"
#include "buffer_reader.hpp"
#include "csv_parser.hpp"
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>      
#include <sstream>

namespace csv_median_calculator {

    /**
 * @brief Буферизированный писатель для выходного CSV-файла.
 *
 * Накапливает строки во внутреннем строковом буфере и сбрасывает на диск,
 * когда размер буфера превышает заданный порог. Это уменьшает число системных вызовов
 * и ускоряет запись при большом количестве изменений медианы.
 */
    class BufferedWriter {
    public:
        explicit BufferedWriter(const std::filesystem::path& path, size_t buffer_size = 1024 * 1024)
            : path_(path), buffer_size_(buffer_size) {
            file_.exceptions(std::ios::badbit | std::ios::failbit);
            file_.open(path_, std::ios::out | std::ios::trunc);
            file_ << "receive_ts;price_median\n";
            file_.precision(8);
            file_ << std::fixed;
        }

        void write_line(uint64_t ts, double median) {
            buffer_ << ts << ";" << median << "\n";
            if (buffer_.tellp() >= static_cast<std::streamoff>(buffer_size_)) {
                flush();
            }
        }

        void flush() {
            file_ << buffer_.str();
            buffer_.str("");
            buffer_.clear();
        }

        ~BufferedWriter() {
            flush(); // записываем остатки
        }

    private:
        std::filesystem::path path_;
        size_t buffer_size_;
        std::ofstream file_;
        std::ostringstream buffer_;
    };

    /**
 * @brief Класс для многопоточной потоковой обработки больших файлов.
 *
 * Создаёт пул читателей, которые параллельно читают файлы и помещают записи
 * в общую приоритетную очередь. Основной поток извлекает записи в порядке
 * возрастания receive_ts, вычисляет медиану и записывает результат.
 */
    class StreamingProcessor {
    public:
        StreamingProcessor(const config& cfg, std::vector<std::filesystem::path> files);

        // Запускает многопоточную обработку и возвращает код результата.
        int run();

    private:
        const config& cfg_;
        std::vector<std::filesystem::path> files_;
    };

} // namespace csv_median_calculator