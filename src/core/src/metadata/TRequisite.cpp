#include "TRequisite.h"
#include <QTextCodec>

namespace v8reader::core {

TRequisite::TRequisite() = default;

TRequisite::~TRequisite() = default;

bool TRequisite::Load(QIODevice& stream, int version) {
    if (!stream.isOpen()) {
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
        stream.read(reinterpret_cast<char*>(&index64), sizeof(index64));
        m_index = static_cast<int>(index64);
    } else {
        stream.read(reinterpret_cast<char*>(&m_index), sizeof(m_index));
    }
    
    // Чтение длины
    if (version == 16) {
        qint64 length64 = 0;
        stream.read(reinterpret_cast<char*>(&length64), sizeof(length64));
        m_length = static_cast<int>(length64);
    } else {
        stream.read(reinterpret_cast<char*>(&m_length), sizeof(m_length));
    }
    
    // Чтение точности (для числовых типов)
    if (version == 16) {
        qint64 precision64 = 0;
        stream.read(reinterpret_cast<char*>(&precision64), sizeof(precision64));
        m_precision = static_cast<int>(precision64);
    } else {
        stream.read(reinterpret_cast<char*>(&m_precision), sizeof(m_precision));
    }
    
    // Чтение флага "Разрешить NULL"
    char allowNullChar = 0;
    stream.read(&allowNullChar, sizeof(allowNullChar));
    m_allowNull = (allowNullChar != 0);
    
    return true;
}

QString TRequisite::readString(QIODevice& stream, int version) {
    if (version == 16) {
        // UTF-16LE строки
        qint32 length = 0;
        stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        
        if (length <= 0) {
            return QString();
        }
        
        QByteArray data = stream.read(length * 2);
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), length);
    } else {
        // ANSI строки (версия 15)
        qint32 length = 0;
        stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        
        if (length <= 0) {
            return QString();
        }
        
        QByteArray data = stream.read(length);
        QTextCodec* codec = QTextCodec::codecForName("CP1251");
        return codec ? codec->toUnicode(data) : QString::fromLocal8Bit(data);
    }
}

} // namespace v8reader::core
