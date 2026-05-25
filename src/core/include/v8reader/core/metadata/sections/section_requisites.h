#pragma once

#include "TMDO.h"
#include "TRequisite.h"
#include <memory>
#include <vector>
#include <QString>

namespace v8reader::core {

/**
 * @brief Секция "Реквизиты" (Properties/Requisites).
 * 
 * Содержит список реквизитов объекта метаданных.
 */
class SectionRequisites : public TMDO {
public:
    SectionRequisites() = default;
    ~SectionRequisites() override = default;

    void Load(QDataStream& stream, int version) override;

    /**
     * @brief Добавить реквизит в список.
     */
    void addRequisite(std::unique_ptr<TRequisite> req);

    /**
     * @brief Получить список всех реквизитов.
     */
    [[nodiscard]] const std::vector<std::unique_ptr<TRequisite>>& getRequisites() const;

    /**
     * @brief Получить реквизит по имени.
     */
    [[nodiscard]] TRequisite* findRequisite(const QString& name) const;

    /**
     * @brief Количество реквизитов.
     */
    [[nodiscard]] size_t count() const;

private:
    std::vector<std::unique_ptr<TRequisite>> m_requisites;
};

} // namespace v8reader::core
