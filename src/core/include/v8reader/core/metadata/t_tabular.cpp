#include "t_tabular.h"
#include "t_requisite.h"
#include <QDataStream>

namespace v8reader::core::metadata {

void TTabular::Load(QIODevice& stream, int version) {
    // Загружаем базовые свойства TMDO (имя, синоним и т.д.)
    TMDO::Load(stream, version);

    QDataStream in(&stream);
    in.setVersion(QDataStream::Qt_6_7);

    // Читаем количество строк по умолчанию
    in >> m_default_row_count;

    // Читаем количество реквизитов
    qint32 reqCount = 0;
    in >> reqCount;

    // Читаем каждый реквизит
    for (qint32 i = 0; i < reqCount; ++i) {
        auto requisite = std::make_unique<TRequisite>();
        // Предполагаем, что TRequisite имеет метод Load, принимающий QDataStream&
        // или мы читаем его данные здесь вручную
        // Для простоты пока заглушка - в реальной реализации нужно читать данные реквизита
        // requisite->Load(stream, version); 
        
        // Временная заглушка: создаем пустой реквизит
        // TODO: Реализовать полноценное чтение реквизита из потока
        AddRequisite(std::move(requisite));
    }
}

void TTabular::AddRequisite(std::unique_ptr<TRequisite> req) {
    if (req) {
        m_requisites.push_back(std::move(req));
    }
}

} // namespace v8reader::core::metadata
