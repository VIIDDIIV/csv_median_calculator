/**
 * @file median_calculator.cpp
 * @brief Реализация расчёта медианы.
 */

#include "median_calculator.hpp"

#include <algorithm>

namespace csv_median_calculator {

    namespace {
        struct max_heap_compare {
            bool operator()(double a, double b) const noexcept {
                return a < b; // для max-heap (наибольший вверху)
            }
        };
        struct min_heap_compare {
            bool operator()(double a, double b) const noexcept {
                return a > b; // для min-heap (наименьший вверху)
            }
        };
    } 

    double median_calculator::add_value(double price_) {
        if (_left.empty() || price_ <= _left.front()) {
            _left.push_back(price_);
            std::push_heap(_left.begin(), _left.end(), max_heap_compare{});
        }
        else {
            _right.push_back(price_);
            std::push_heap(_right.begin(), _right.end(), min_heap_compare{});
        }
        balance();
        return current_median();
    }

    double median_calculator::current_median() const noexcept {
        if (_left.empty() && _right.empty()) {
            return 0.0; 
        }
        if (_left.size() > _right.size()) {
            return _left.front();
        }
        else if (_right.size() > _left.size()) {
            return _right.front();
        }
        else {
            return (_left.front() + _right.front()) / 2.0;
        }
    }

    void median_calculator::balance() {
        if (_left.size() > _right.size() + 1) {
            // Перемещаем вершину левой кучи в правую
            std::pop_heap(_left.begin(), _left.end(), max_heap_compare{});
            double val = _left.back();
            _left.pop_back();

            _right.push_back(val);
            std::push_heap(_right.begin(), _right.end(), min_heap_compare{});
        }
        else if (_right.size() > _left.size()) {
            // Перемещаем вершину правой кучи в левую
            std::pop_heap(_right.begin(), _right.end(), min_heap_compare{});
            double val = _right.back();
            _right.pop_back();

            _left.push_back(val);
            std::push_heap(_left.begin(), _left.end(), max_heap_compare{});
        }
    }

    void median_calculator::clear() noexcept {
        _left.clear();
        _right.clear();
    }

} // namespace csv_median_calculator