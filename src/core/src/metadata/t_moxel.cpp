#include "v8reader/core/metadata/t_moxel.h"
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
        QByteArray buffer(dataSize, Qt::Uninitialized);
        int bytesRead = stream.readRawData(buffer.data(), dataSize);
        if (bytesRead != dataSize) {
            return false; // Ошибка чтения
        }
        m_data = buffer;
    }

    return true;
}

} // namespace v8reader::core::metadata
