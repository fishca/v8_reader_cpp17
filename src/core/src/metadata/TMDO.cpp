#include "TMDO.h"
#include "TRequisite.h"
#include <QDataStream>
#include <QTextCodec>

namespace v8reader::core {

TMDO::TMDO() = default;

TMDO::~TMDO() = default;

void TMDO::addRequisite(std::shared_ptr<TRequisite> req) {
    if (req) {
        m_requisites.push_back(req);
    }
}

void TMDO::clearRequisites() {
    m_requisites.clear();
}

bool TMDO::readFromStream(QIODevice& stream, int version) {
    if (!stream.isOpen()) {
        return false;
    }

    // Чтение имени
    m_name = readString(stream, version);
    
    // Чтение синонима
    m_synonym = readString(stream, version);
    
    // Чтение комментария
    m_comment = readString(stream, version);
    
    // Чтение Ref (размер зависит от версии: 12 байт для v15, 16 байт для v16)
    int refSize = (version == 16) ? 16 : 12;
    m_ref = readByteArray(stream, refSize);
    
    // Чтение количества реквизитов
    qint32 reqCount = 0;
    if (version == 16) {
        qint64 reqCount64 = 0;
        stream.read(reinterpret_cast<char*>(&reqCount64), sizeof(reqCount64));
        reqCount = static_cast<qint32>(reqCount64);
    } else {
        stream.read(reinterpret_cast<char*>(&reqCount), sizeof(reqCount));
    }
    
    // Чтение реквизитов
    for (qint32 i = 0; i < reqCount; ++i) {
        auto req = std::make_shared<TRequisite>();
        if (req->readFromStream(stream, version)) {
            addRequisite(req);
        }
    }
    
    return true;
}

QString TMDO::readString(QIODevice& stream, int version) {
    if (version == 16) {
        // UTF-16LE строки
        qint32 length = 0;
        stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        
        if (length <= 0) {
            return QString();
        }
        
        QByteArray data = stream.read(length * 2); // 2 байта на символ
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), length);
    } else {
        // ANSI строки (версия 15)
        qint32 length = 0;
        stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        
        if (length <= 0) {
            return QString();
        }
        
        QByteArray data = stream.read(length);
        // Предполагаем кодировку CP1251 для русских версий 1С
        QTextCodec* codec = QTextCodec::codecForName("CP1251");
        return codec ? codec->toUnicode(data) : QString::fromLocal8Bit(data);
    }
}

QByteArray TMDO::readByteArray(QIODevice& stream, int size) {
    return stream.read(size);
}

} // namespace v8reader::core
