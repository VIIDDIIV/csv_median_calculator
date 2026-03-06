

#pragma once

#include <vector>

namespace csv_median_calculator {

    /**
 * @brief Балансирует размеры двух куч, поддерживая инвариант:
 *        левая куча (max-heap) содержит не более чем на 1 элемент больше правой (min-heap).
 *
 * Если левая куча становится больше на 2, её вершина перемещается в правую.
 * Если правая куча становится больше левой, её вершина перемещается в левую.
 * Это гарантирует, что медиана всегда доступна как вершина одной из куч или их среднее.
 */
    class median_calculator {
    public:
   
        [[nodiscard]] double add_value(double price_);

        [[nodiscard]] double current_median() const noexcept;

    
        void clear() noexcept;

    private:
      
        void balance();

        // Левая куча (max-heap): хранит значения ≤ медианы.
        std::vector<double> _left;
        // Правая куча (min-heap): хранит значения ≥ медианы.
        std::vector<double> _right;
    };

} 