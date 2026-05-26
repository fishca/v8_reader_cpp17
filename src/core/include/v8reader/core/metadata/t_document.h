#pragma once

#include "t_metadata_object_with_sections.h"
#include "t_tabular.h"
#include <memory>
#include <QString>

namespace v8reader::core {

// Forward declaration
class TRequisite;
class TComand;

/**
 * @brief Класс, представляющий объект метаданных "Документ".
 * 
 * Наследуется от MetadataObjectWithSections и реализует фабричный метод
 * для создания специфичных для документа секций (Реквизиты, Движения, Табличные части).
 */
class TDocument : public MetadataObjectWithSections {
public:
    explicit TDocument(const QString& name = QString());
    ~TDocument() override = default;

    // Переопределяем фабричный метод для создания секций
    std::unique_ptr<TMDO> createSection(const QString& sectionName) override;

    // Методы для загрузки данных (если нужна специфичная логика)
    bool Load(QDataStream& stream, int version) override;

private:
    // Кэш для часто используемых секций (опционально, для производительности)
    // В данной реализации будем искать в общем списке секций родителя
};

} // namespace v8reader::core
