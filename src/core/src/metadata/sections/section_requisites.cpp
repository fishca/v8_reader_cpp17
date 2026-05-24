#include "section_requisites.h"
#include <QDataStream>
#include <QDebug>

namespace v8reader::core {

void SectionRequisites::Load(QDataStream& stream, int version) {
    // Загружаем базовые данные секции (имя и т.д.)
    TMDO::Load(stream, version);

    // Читаем количество реквизитов
    quint32 reqCount;
    stream >> reqCount;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading requisites count in section:" << getName();
        return;
    }

    // Читаем каждый реквизит
    for (quint32 i = 0; i < reqCount; ++i) {
        auto requisite = std::make_unique<TRequisite>();
        requisite->Load(stream, version);
        m_requisites.push_back(std::move(requisite));
    }
}

void SectionRequisites::addRequisite(std::unique_ptr<TRequisite> req) {
    m_requisites.push_back(std::move(req));
}

const std::vector<std::unique_ptr<TRequisite>>& SectionRequisites::getRequisites() const {
    return m_requisites;
}

TRequisite* SectionRequisites::findRequisite(const QString& name) const {
    for (const auto& req : m_requisites) {
        if (req->getName() == name) {
            return req.get();
        }
    }
    return nullptr;
}

size_t SectionRequisites::count() const {
    return m_requisites.size();
}

} // namespace v8reader::core
