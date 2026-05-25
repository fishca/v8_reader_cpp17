#include "v8reader/core/metadata/t_catalog.h"
#include <QDebug>

namespace v8reader::core {

std::unique_ptr<TMDO> TCatalog::createSection(const QString& name) {
    // Фабричный метод для создания секций справочника по имени
    if (name == "Реквизиты" || name == "Requisites") {
        return std::make_unique<SectionRequisites>();
    }
    
    if (name == "Команды" || name == "Commands") {
        return std::make_unique<SectionCommands>();
    }
    
    // TODO: Добавить поддержку других секций справочника:
    // - Табличные части (ТабличныеСекции / TabularSections)
    // - Формы (Формы / Forms)
    // - Макеты (Макеты / Templates)
    // - Модули (МодульОбъекта / ObjectModule)
    
    qWarning() << "Unknown section type for Catalog:" << name;
    return nullptr;
}

SectionRequisites* TCatalog::getRequisitesSection() const {
    auto* section = getSection("Реквизиты");
    if (!section) {
        section = getSection("Requisites");
    }
    return static_cast<SectionRequisites*>(section);
}

SectionCommands* TCatalog::getCommandsSection() const {
    auto* section = getSection("Команды");
    if (!section) {
        section = getSection("Commands");
    }
    return static_cast<SectionCommands*>(section);
}

} // namespace v8reader::core
