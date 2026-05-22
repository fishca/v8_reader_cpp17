#include "t_metadata_object_with_sections.h"
#include <QDataStream>
#include <QDebug>

namespace v8reader::core {

void MetadataObjectWithSections::Load(QDataStream& stream, int version) {
    // Сначала загружаем базовые свойства объекта (имя, синоним и т.д.)
    TMDO::Load(stream, version);

    // Читаем количество секций
    quint32 sectionCount;
    stream >> sectionCount;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading section count for object:" << getName();
        return;
    }

    // Читаем каждую секцию
    for (quint32 i = 0; i < sectionCount; ++i) {
        QString sectionName;
        // Читаем имя секции (UTF-16LE)
        stream >> sectionName; 
        
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Error reading section name at index" << i << "for object:" << getName();
            break;
        }

        // Создаем экземпляр секции через фабричный метод
        auto section = createSection(sectionName);
        
        if (section) {
            // Загружаем данные секции
            section->Load(stream, version);
            m_sections[sectionName] = std::move(section);
        } else {
            qWarning() << "Unknown section type:" << sectionName << "in object:" << getName();
            // TODO: Возможно, нужно пропустить данные неизвестной секции в потоке
            // Для этого нужно знать размер секции или иметь маркер конца
        }
    }
}

TMDO* MetadataObjectWithSections::getSection(const QString& name) const {
    auto it = m_sections.find(name);
    return (it != m_sections.end()) ? it->second.get() : nullptr;
}

bool MetadataObjectWithSections::hasSection(const QString& name) const {
    return m_sections.contains(name);
}

std::vector<QString> MetadataObjectWithSections::getSectionNames() const {
    std::vector<QString> names;
    names.reserve(m_sections.size());
    for (const auto& [name, ptr] : m_sections) {
        names.push_back(name);
    }
    return names;
}

size_t MetadataObjectWithSections::getSectionCount() const {
    return m_sections.size();
}

std::unique_ptr<TMDO> MetadataObjectWithSections::createSection(const QString& name) {
    // Базовая реализация возвращает nullptr.
    // Наследники должны переопределить этот метод для создания конкретных типов секций.
    Q_UNUSED(name);
    return nullptr;
}

} // namespace v8reader::core
