#include "t_tabular.h"
#include "t_requisite.h"
#include <QDataStream>
#include <QDebug>

namespace v8reader::core {

bool TTabular::Load(QDataStream& stream, int version) {
    // 1. Загружаем базовые свойства TMDO (имя, синомин и т.д.)
    TMDO::Load(stream, version);

    // 2. Читаем количество строк по умолчанию (DefaultRowCount)
    stream >> m_default_row_count;

    // 3. Читаем количество реквизитов
    quint32 reqCount;
    stream >> reqCount;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading tabular section:" << getName();
        return false;
    }

    // 4. Читаем каждый реквизит
    for (quint32 i = 0; i < reqCount; ++i) {
        auto requisite = std::make_unique<TRequisite>();
        requisite->Load(stream, version);
        m_requisites.push_back(std::move(requisite));
    }

    return true;
}

void TTabular::AddRequisite(std::unique_ptr<TRequisite> req) {
    m_requisites.push_back(std::move(req));
}

} // namespace v8reader::core
