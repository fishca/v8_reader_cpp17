#include "t_moxel.h"
#include <QDataStream>

namespace v8reader::core::metadata {

TMoxel::TMoxel() = default;

bool TMoxel::Load(QDataStream& stream, int version) {
    if (stream.atEnd()) {
        return false;
    }

    // Загружаем базовые свойства TMDO (имя, синоним и т.д.)
    if (!TMDO::Load(stream, version)) {
        return false;
    }

    // Читаем тип мохеля
    QString typeStr;
    stream >> typeStr;
    m_type = typeStr;

    // Читаем размер данных
    qint32 dataSize = 0;
    stream >> dataSize;

    // Читаем сами данные
    if (dataSize > 0) {
        m_data = stream.readRawData(dataSize);
        if (m_data.size() != dataSize) {
            return false; // Ошибка чтения
        }
    }

    return true;
}

} // namespace v8reader::core::metadata
