#include "v8reader/core/metadata/TMDO.h"
#include "v8reader/core/metadata/TRequisite.h"
#include <QDataStream>
#include <QIODevice>

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

bool TMDO::Load(QDataStream& stream, int version) {
    if (!stream.device() || !stream.device()->isOpen()) {
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
        stream >> reqCount64;
        reqCount = static_cast<qint32>(reqCount64);
    } else {
        stream >> reqCount;
    }

    // Чтение реквизитов
    for (qint32 i = 0; i < reqCount; ++i) {
        auto req = std::make_shared<TRequisite>();
        if (req->Load(stream, version)) {
            addRequisite(req);
        }
    }

    return true;
}

QString TMDO::readString(QDataStream& stream, int /* version */) {
    // Строки в файлах .1CD хранятся в кодировке UTF-16LE независимо от версии формата.
    // Версия формата влияет только на размер адресов, но не на кодировку строк.

    qint32 length = 0;
    stream >> length;

    if (length <= 0) {
        return QString();
    }

    QByteArray data(length * 2, Qt::Uninitialized);
    stream.readRawData(data.data(), length * 2); // 2 байта на символ (UTF-16LE)
    return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), length);
}

QByteArray TMDO::readByteArray(QDataStream& stream, int size) {
    QByteArray data(size, Qt::Uninitialized);
    stream.readRawData(data.data(), size);
    return data;
}

} // namespace v8reader::core
