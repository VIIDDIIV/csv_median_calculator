#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace csv_median_calculator {

    /**
 * @brief Потокобезопасная приоритетная очередь с блокировками.
 *
 * Реализует min-heap (если компаратор std::greater) или max-heap (std::less).
 * Метод pop блокируется до появления элемента или остановки очереди.
 * Используется для обмена данными между потоками-читателями и основным потоком.
 */
    template<typename T, typename Compare = std::less<T>>
    class ThreadSafePriorityQueue {
    public:
        void push(T value) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push(std::move(value));
            }
            cond_.notify_one();
        }

        // Ожидает появления элемента. Возвращает false, если очередь остановлена и пуста.
        bool pop(T& value) {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { return !queue_.empty() || stopped_; });
            if (queue_.empty() && stopped_) return false;
            value = std::move(const_cast<T&>(queue_.top()));
            queue_.pop();
            return true;
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped_ = true;
            }
            cond_.notify_all();
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

    private:
        std::priority_queue<T, std::vector<T>, Compare> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cond_;
        bool stopped_ = false;
    };

} 