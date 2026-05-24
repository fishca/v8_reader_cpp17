#include "t_comand.h"
#include <QDataStream>

namespace v8reader::core::metadata {

bool TComand::Load(QDataStream& stream, int version) {
    // 1. Загружаем базовые свойства TMDO (Имя, Синоним, Комментарий и т.д.)
    if (!TMDO::Load(stream, version)) {
        return false;
    }

    // 2. Читаем специфичные свойства команды
    // Структура может отличаться в зависимости от версии формата,
    // здесь приведен примерный порядок чтения на основе типовой структуры 1С

    // Чтение поля Action (Действие)
    qint32 actionLen = 0;
    stream >> actionLen;
    if (actionLen > 0) {
        QByteArray data = stream.readRawData(actionLen * 2);
        m_action = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), actionLen);
    }

    // Чтение поля Use (Использование)
    qint32 useLen = 0;
    stream >> useLen;
    if (useLen > 0) {
        QByteArray data = stream.readRawData(useLen * 2);
        m_use = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), useLen);
    }

    // Чтение поля Presentation (Представление)
    qint32 presLen = 0;
    stream >> presLen;
    if (presLen > 0) {
        QByteArray data = stream.readRawData(presLen * 2);
        m_presentation = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), presLen);
    }

    // Примечание: Реальный порядок и наличие полей нужно сверять с исходным C++ Builder кодом
    // и тестировать на реальных файлах .cf/.1CD

    return true;
}

} // namespace v8reader::core::metadata
