#pragma once
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class BufferedReader {
public:
    explicit BufferedReader(const fs::path& path, size_t buf_size = 1024 * 1024 * 8)
        : stream(path, std::ios::binary), buffer(buf_size), buf_start(0), buf_end(0) {}

    bool getline(std::string& out) {
        out.clear();
        while (true) {
            if (buf_start >= buf_end) {
                if (!refill()) return false; 
            }
            // Ищем конец строки в буфере
            char* line_start = buffer.data() + buf_start;
            char* line_end = std::find(line_start, buffer.data() + buf_end, '\n');
            if (line_end != buffer.data() + buf_end) {
                // Нашли перевод строки
                out.append(line_start, line_end - line_start);
                buf_start = line_end - buffer.data() + 1; // пропускаем \n
                return true;
            }
            else {
                // Строка не поместилась в буфер – добавляем остаток и продолжаем
                out.append(line_start, buf_end - buf_start);
                buf_start = 0;
                buf_end = 0;
                if (!refill()) return false; // достигнут конец файла
            }
        }
    }

private:
    std::ifstream stream;
    std::vector<char> buffer;
    size_t buf_start, buf_end;

    bool refill() {
        stream.read(buffer.data(), buffer.size());
        buf_end = stream.gcount();
        buf_start = 0;
        return buf_end > 0;
    }
};