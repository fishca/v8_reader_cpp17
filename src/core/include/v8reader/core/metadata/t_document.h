#pragma once

#include "metadata_object_with_sections.h"
#include "t_tabular.h"
#include <memory>
#include <QString>

namespace v8reader::core {

/**
 * @brief Класс, представляющий объект метаданных "Документ".
 * 
 * Наследуется от MetadataObjectWithSections и реализует фабричный метод
 * для создания специфичных для документа секций (Реквизиты, Движения, Табличные части).
 */
class TDocument : public MetadataObjectWithSections {
public:
    explicit TDocument(const QString& name = QString(), const Guid& guid = Guid());
    ~TDocument() override = default;

    // Переопределяем фабричный метод для создания секций
    std::unique_ptr<TMDO> createSection(const QString& sectionName) override;

    // Методы для загрузки данных (если нужна специфичная логика)
    bool Load(QDataStream& stream) override;

    // Удобные методы доступа к часто используемым секциям
    /**
     * @brief Получить секцию табличных частей.
     * Создает её, если она ещё не существует.
     */
    std::shared_ptr<TTabular> getTabularSections();

    /**
     * @brief Получить секцию реквизитов.
     */
    std::shared_ptr<TRequisite> getRequisites();

private:
    // Кэш для часто используемых секций (опционально, для производительности)
    // В данной реализации будем искать в общем списке секций родителя
};

} // namespace v8reader::core
