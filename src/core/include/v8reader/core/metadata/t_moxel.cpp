#include "t_moxel.h"
#include <QDataStream>

namespace v8reader::core::metadata {

TMoxel::TMoxel() = default;

bool TMoxel::Load(QIODevice& stream, int version) {
    if (!stream.isOpen()) {
        return false;
    }

    // Загружаем базовые свойства TMDO (имя, синоним и т.д.)
    if (!TMDO::Load(stream, version)) {
        return false;
    }

    QDataStream in(&stream);
    in.setVersion(QDataStream::Qt_6_7);
    
    // Читаем тип мохеля
    QString typeStr;
    in >> typeStr;
    m_type = typeStr;

    // Читаем размер данных
    qint32 dataSize = 0;
    in >> dataSize;

    // Читаем сами данные
    if (dataSize > 0) {
        m_data = stream.read(dataSize);
        if (m_data.size() != dataSize) {
            return false; // Ошибка чтения
        }
    }

    return !in.atEnd() || stream.atEnd(); // Успех, если не было ошибок чтения
}

} // namespace v8reader::core::metadata
