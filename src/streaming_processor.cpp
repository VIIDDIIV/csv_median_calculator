#include "streaming_processor.hpp"
#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <cmath>
#include <future>

namespace csv_median_calculator {

    StreamingProcessor::StreamingProcessor(const config& cfg, std::vector<std::filesystem::path> files)
        : cfg_(cfg), files_(std::move(files)) {}

    /**
 * @brief Запускает многопоточную потоковую обработку.
 *
 * Алгоритм:
 * 1. Определяется число потоков-читателей (равно числу аппаратных ядер, но не больше числа файлов).
 * 2. Файлы равномерно распределяются между потоками.
 * 3. Каждый читатель открывает свою группу файлов, читает их с помощью BufferedReader,
 *    парсит строки через parse_record и помещает записи в общую потокобезопасную
 *    приоритетную очередь (ThreadSafePriorityQueue).
 * 4. Основной поток ожидает небольшое время для накопления данных, затем начинает
 *    извлекать записи из очереди (они автоматически упорядочены по receive_ts).
 * 5. Для каждой записи вызывается median_calculator::add_value, и если медиана изменилась,
 *    запись добавляется в буферизированный писатель (BufferedWriter).
 * 6. По завершении всех читателей и опустошения очереди основной поток завершается,
 *    дожидается окончания читателей и сбрасывает буфер записи.
 *
 * В процессе измеряется время, затраченное на операции с очередью и с кучами.
 *
 * @return 0 при успешном завершении.
 */
    int StreamingProcessor::run() {
        auto total_start = std::chrono::steady_clock::now();

        // Тип элемента очереди: запись + индекс файла 
        using QueueItem = std::pair<record, size_t>;
        struct Compare {
            bool operator()(const QueueItem& a, const QueueItem& b) const {
                return a.first.receive_ts > b.first.receive_ts; 
            }
        };
        ThreadSafePriorityQueue<QueueItem, Compare> queue;

        std::atomic<size_t> files_left(files_.size());

        // Определяем число потоков-читателей 
        unsigned int num_readers = std::thread::hardware_concurrency();
        if (num_readers == 0) num_readers = 2;
        num_readers = std::min(num_readers, (unsigned int)files_.size());

        // Разбиваем файлы на части для каждого потока
        std::vector<std::future<void>> readers;
        size_t chunk_size = files_.size() / num_readers;
        size_t remainder = files_.size() % num_readers;
        size_t start = 0;

        for (unsigned int r = 0; r < num_readers; ++r) {
            size_t end = start + chunk_size + (r < remainder ? 1 : 0);
            readers.push_back(std::async(std::launch::async, [this, &queue, &files_left, start, end]() {
                for (size_t i = start; i < end; ++i) {
                    const auto& path = files_[i];
                    BufferedReader reader(path);
                    std::string header;
                    if (!reader.getline(header)) {
                        spdlog::error("Cannot read header from {}", path.string());
                        files_left--;
                        continue;
                    }
                    // заголовок не проверяем – файл уже валидирован
                    std::string line;
                    while (reader.getline(line)) {
                        auto rec_opt = parse_record(line);
                        if (rec_opt) {
                            queue.push({ *rec_opt, i });
                        }
                        else {
                            spdlog::warn("Skipping invalid line in file {}", path.string());
                        }
                    }
                    files_left--;
                }
                }));
            start = end;
        }

        // Даём читателям время собрать данные из каждого файла
        using namespace std::chrono_literals;       
        std::this_thread::sleep_for(15ms);
           

        // Подготовка выходного файла с буферизированным писателем
        std::filesystem::create_directories(cfg_.output_dir);
        std::filesystem::path out_file = std::filesystem::path(cfg_.output_dir) / "median_result.csv";
        BufferedWriter writer(out_file, 2 * 1024 * 1024); 

        median_calculator calc;
        double last_median = 0.0;
        bool first = true;
        size_t changes = 0;

        auto to_fixed_int = [](double x) -> int64_t {
            return static_cast<int64_t>(std::round(x * 1e8));
            };

        spdlog::info("Processing records...");

   

        QueueItem item;
        while (files_left.load() > 0 || !queue.empty()) {
            bool got = queue.pop(item);
     
            if (!got) break; // очередь остановлена и пуста

            
            double new_median = calc.add_value(item.first.price);

            int64_t new_fixed = to_fixed_int(new_median);
            int64_t last_fixed = to_fixed_int(last_median);
            if (first || new_fixed != last_fixed) {
                writer.write_line(item.first.receive_ts, new_median);
                last_median = new_median;
                ++changes;
                first = false;
            }
        }

        // Дожидаемся завершения всех читателей (на случай, если они ещё не закончили)
        for (auto& f : readers) {
            if (f.valid()) f.wait();
        }

        auto total_end = std::chrono::steady_clock::now();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();
        spdlog::info("Total processing time: {} ms", total_ms);
        spdlog::info("Finished. Median changes written: {}", changes);
        spdlog::info("Result saved to: {}", out_file.string());
        return 0;
    }

} // namespace csv_median_calculator