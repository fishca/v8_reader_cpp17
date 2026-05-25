#pragma once

#include "t_metadata_object_with_sections.h"
#include "sections/section_requisites.h"
#include "sections/section_commands.h"
#include <memory>
#include <QString>
#include <map>

namespace v8reader::core {

/**
 * @brief Класс метаданных "Справочник" (Catalog).
 * 
 * Пример объекта с секциями: реквизиты, табличные части, формы, команды.
 * Демонстрирует переопределение фабричного метода createSection().
 */
class TCatalog : public MetadataObjectWithSections {
public:
    TCatalog() = default;
    ~TCatalog() override = default;

    /**
     * @brief Фабричный метод для создания секций справочника.
     * Переопределяет базовый метод для поддержки конкретных типов секций.
     */
    std::unique_ptr<TMDO> createSection(const QString& name) override;

    /**
     * @brief Получить секцию реквизитов.
     */
    [[nodiscard]] SectionRequisites* getRequisitesSection() const;

    /**
     * @brief Получить секцию команд.
     */
    [[nodiscard]] SectionCommands* getCommandsSection() const;
};

} // namespace v8reader::core
