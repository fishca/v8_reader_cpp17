#include "v8reader/core/metadata/TRequisite.h"
#include <QDataStream>
#include <QString>
#include <QIODevice>

namespace v8reader::core {

TRequisite::TRequisite() = default;

TRequisite::~TRequisite() = default;

bool TRequisite::Load(QDataStream& stream, int version) {
    if (!stream.device() || !stream.device()->isOpen()) {
        return false;
    }

    // Чтение имени
    m_name = readString(stream, version);

    // Чтение синонима
    m_synonym = readString(stream, version);

    // Чтение описания типа
    m_typeDescription = readString(stream, version);

    // Чтение индекса
    if (version == 16) {
        qint64 index64 = 0;
        stream >> index64;
        m_index = static_cast<int>(index64);
    } else {
        stream >> m_index;
    }

    // Чтение длины
    if (version == 16) {
        qint64 length64 = 0;
        stream >> length64;
        m_length = static_cast<int>(length64);
    } else {
        stream >> m_length;
    }

    // Чтение точности (для числовых типов)
    if (version == 16) {
        qint64 precision64 = 0;
        stream >> precision64;
        m_precision = static_cast<int>(precision64);
    } else {
        stream >> m_precision;
    }

    // Чтение флага "Разрешить NULL"
    char allowNullChar = 0;
    stream.readRawData(&allowNullChar, sizeof(allowNullChar));
    m_allowNull = (allowNullChar != 0);

    return true;
}

QString TRequisite::readString(QDataStream& stream, int version) {
    if (version == 16) {
        // UTF-16LE строки
        qint32 length = 0;
        stream >> length;

        if (length <= 0) {
            return QString();
        }

        QByteArray data(length * 2, Qt::Uninitialized);
        stream.readRawData(data.data(), length * 2);
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), length);
    } else {
        // ANSI строки (версия 15)
        qint32 length = 0;
        stream >> length;

        if (length <= 0) {
            return QString();
        }

        QByteArray data(length, Qt::Uninitialized);
        stream.readRawData(data.data(), length);
        // Используем QString::fromLocal8Bit вместо QTextCodec (устарел в Qt6)
        return QString::fromLocal8Bit(data);
    }
}

} // namespace v8reader::core
