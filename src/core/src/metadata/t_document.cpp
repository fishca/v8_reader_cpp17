#include "v8reader/core/metadata/t_document.h"
#include "v8reader/core/metadata/t_comand.h"
#include "v8reader/core/metadata/TRequisite.h"
#include <QDebug>

namespace v8reader::core {

TDocument::TDocument(const QString& name)
    : MetadataObjectWithSections() {
    setName(name);
}

std::unique_ptr<TMDO> TDocument::createSection(const QString& sectionName) {
    // Нормализация имени (убираем пробелы, приводим к нижнему регистру для сравнения)
    QString normalizedName = sectionName.trimmed();
    
    // Поддержка русских и английских имен секций
    
    if (normalizedName == "Реквизиты" || normalizedName == "Requisites") {
        return std::make_unique<TRequisite>();
    }
    
    if (normalizedName == "Команды" || normalizedName == "Commands") {
        return std::make_unique<TComand>();
    }
    
    if (normalizedName == "Табличные части" || normalizedName == "TabularSections") {
        return std::make_unique<TTabular>(); 
    }
    
    if (normalizedName == "Движения" || normalizedName == "Movements") {
        // Секция движений (для регистровых документов)
        qWarning() << "Section 'Movements' not fully implemented yet for TDocument";
        return std::make_unique<TMDO>();
    }

    if (normalizedName == "Формы" || normalizedName == "Forms") {
        return std::make_unique<TMDO>();
    }

    if (normalizedName == "Макеты" || normalizedName == "Templates") {
        return std::make_unique<TMDO>();
    }

    // Если секция неизвестна, создаем базовый объект
    qWarning() << "Unknown section for TDocument:" << sectionName;
    return std::make_unique<TMDO>();
}

bool TDocument::Load(QDataStream& stream, int version) {
    // Вызываем базовую реализацию, которая прочитает имя, GUID и секции
    if (!MetadataObjectWithSections::Load(stream, version)) {
        return false;
    }
    
    // Здесь можно добавить специфичную для документа логику после загрузки секций
    // Например, валидацию наличия обязательных секций
    
    return true;
}

} // namespace v8reader::core
