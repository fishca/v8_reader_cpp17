#pragma once

#include "t_mdo.h"
#include <memory>
#include <vector>
#include <QString>

namespace v8reader::core::metadata {

/**
 * @brief Класс, представляющий табличную часть объекта метаданных (TabularSection).
 * 
 * Табличные части используются в документах, планах счетов и других объектах
 * для хранения списков данных (список товаров, список движений и т.д.).
 * Наследуется от TMDO.
 */
class TTabular : public TMDO {
public:
    explicit TTabular() = default;
    ~TTabular() override = default;

    // Запрет копирования, разрешение перемещения
    TTabular(const TTabular&) = delete;
    TTabular& operator=(const TTabular&) = delete;
    TTabular(TTabular&&) noexcept = default;
    TTabular& operator=(TTabular&&) noexcept = default;

    /**
     * @brief Парсинг свойств табличной части из потока.
     * @param stream Входной поток с данными.
     * @param version Версия формата файла.
     */
    void Load(QIODevice& stream, int version) override;

    /**
     * @brief Добавление реквизита в табличную часть.
     */
    void AddRequisite(std::unique_ptr<TRequisite> req);

    /**
     * @brief Получение списка реквизитов.
     */
    [[nodiscard]] const std::vector<std::unique_ptr<TRequisite>>& GetRequisites() const { return m_requisites; }

    /**
     * @brief Получение количества строк по умолчанию.
     */
    [[nodiscard]] int GetDefaultRowCount() const { return m_default_row_count; }

private:
    std::vector<std::unique_ptr<TRequisite>> m_requisites; // Список реквизитов табличной части
    int m_default_row_count = 0; // Количество строк по умолчанию
};

} // namespace v8reader::core::metadata
