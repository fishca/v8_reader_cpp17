#ifndef V8READER_TYPES_V8_TYPES_H
#define V8READER_TYPES_V8_TYPES_H

#include <QString>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include <QUuid>
#include <QDateTime>
#include <QVariant>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace v8reader {

/**
 * @brief Базовые типы данных для замены VCL типов C++ Builder
 * 
 * Этот файл содержит алиасы и утилиты для замены специфичных типов
 * C++ Builder (VCL) на стандартные типы C++17 и Qt6
 */

// === Строковые типы ===

/**
 * @brief Аналог UnicodeString из VCL
 * Используется для всех строковых данных в 1С
 */
using V8String = QString;

/**
 * @brief Аналог AnsiString из VCL
 * Используется для работы с ANSI данными
 */
using V8AnsiString = QByteArray;

/**
 * @brief Аналог WideString из VCL
 */
using V8WideString = QString;

// === Байтовые массивы ===

/**
 * @brief Аналог DynamicArray<Byte> из VCL
 * Используется для хранения бинарных данных
 */
using V8ByteArray = QByteArray;

/**
 * @brief Аналог TBytes из VCL
 */
using V8Bytes = QVector<quint8>;

// === Коллекции ===

/**
 * @brief Аналог TList из VCL
 * @tparam T Тип элементов списка
 */
template<typename T>
using V8List = QVector<T>;

/**
 * @brief Аналог TObjectList из VCL
 * @tparam T Тип объектов (должен наследовать QObject или быть указателем)
 */
template<typename T>
using V8ObjectList = QVector<std::shared_ptr<T>>;

/**
 * @brief Аналог TStringList из VCL
 */
using V8StringList = QStringList;

/**
 * @brief Аналог TStrings из VCL
 */
using V8Strings = QStringList;

/**
 * @brief Аналог TMap из VCL
 * @tparam K Тип ключа
 * @tparam V Тип значения
 */
template<typename K, typename V>
using V8Map = QMap<K, V>;

/**
 * @brief Аналог TDictionary из VCL
 * @tparam K Тип ключа
 * @tparam V Тип значения
 */
template<typename K, typename V>
using V8Dictionary = QMap<K, V>;

// === GUID/UUID ===

/**
 * @brief Аналог TGUID из VCL
 */
using V8Guid = QUuid;

/**
 * @brief Создать GUID из строки
 * @param str Строковое представление GUID
 * @return V8Guid
 */
inline V8Guid createGuid(const V8String& str) {
    return QUuid::fromString(str);
}

/**
 * @brief Создать новый случайный GUID
 * @return V8Guid
 */
inline V8Guid createNewGuid() {
    return QUuid::createUuid();
}

/**
 * @brief Преобразовать GUID в строку
 * @param guid GUID
 * @return Строковое представление
 */
inline V8String guidToString(const V8Guid& guid) {
    return guid.toString(QUuid::WithoutBraces);
}

// === Дата и время ===

/**
 * @brief Аналог TDateTime из VCL
 */
using V8DateTime = QDateTime;

/**
 * @brief Аналог TDate из VCL
 */
using V8Date = QDate;

/**
 * @brief Аналог TTime из VCL
 */
using V8Time = QTime;

// === Вариантный тип ===

/**
 * @brief Аналог Variant из VCL
 * Может хранить различные типы данных
 */
using V8Variant = QVariant;

// === Умные указатели ===

/**
 * @brief Умный указатель с уникальным владением
 * @tparam T Тип объекта
 */
template<typename T>
using V8UniquePtr = std::unique_ptr<T>;

/**
 * @brief Умный указатель с разделяемым владением
 * @tparam T Тип объекта
 */
template<typename T>
using V8SharedPtr = std::shared_ptr<T>;

/**
 * @brief Слабый указатель
 * @tparam T Тип объекта
 */
template<typename T>
using V8WeakPtr = std::weak_ptr<T>;

/**
 * @brief Создать умный указатель с разделяемым владением
 * @tparam T Тип объекта
 * @tparam Args Типы аргументов конструктора
 * @param args Аргументы конструктора
 * @return V8SharedPtr<T>
 */
template<typename T, typename... Args>
inline V8SharedPtr<T> makeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

/**
 * @brief Создать уникальный умный указатель
 * @tparam T Тип объекта
 * @tparam Args Типы аргументов конструктора
 * @param args Аргументы конструктора
 * @return V8UniquePtr<T>
 */
template<typename T, typename... Args>
inline V8UniquePtr<T> makeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// === Числовые типы ===

/**
 * @brief Аналог Integer из VCL (32-битное целое)
 */
using V8Integer = qint32;

/**
 * @brief Аналог Int64 из VCL (64-битное целое)
 */
using V8Int64 = qint64;

/**
 * @brief Аналог Cardinal из VCL (32-битное беззнаковое)
 */
using V8Cardinal = quint32;

/**
 * @brief Аналог Word из VCL (16-битное беззнаковое)
 */
using V8Word = quint16;

/**
 * @brief Аналог Byte из VCL (8-битное беззнаковое)
 */
using V8Byte = quint8;

/**
 * @brief Аналог Double из VCL
 */
using V8Double = double;

/**
 * @brief Аналог Float из VCL
 */
using V8Float = float;

// === Булевый тип ===

/**
 * @brief Аналог Boolean из VCL
 */
using V8Boolean = bool;

// === Потоки ===

/**
 * @brief Базовый класс для потоков
 * Аналог TStream из VCL
 */
class V8Stream {
public:
    virtual ~V8Stream() = default;
    
    /**
     * @brief Читать данные из потока
     * @param buffer Буфер для чтения
     * @param count Количество байт для чтения
     * @return Количество прочитанных байт
     */
    virtual V8Integer read(void* buffer, V8Integer count) = 0;
    
    /**
     * @brief Записать данные в поток
     * @param buffer Буфер с данными
     * @param count Количество байт для записи
     * @return Количество записанных байт
     */
    virtual V8Integer write(const void* buffer, V8Integer count) = 0;
    
    /**
     * @brief Изменить позицию в потоке
     * @param offset Смещение
     * @param origin Начальная точка (0 - начало, 1 - текущая, 2 - конец)
     * @return Новая позиция
     */
    virtual V8Int64 seek(V8Int64 offset, int origin) = 0;
    
    /**
     * @brief Получить текущую позицию в потоке
     * @return Позиция
     */
    virtual V8Int64 position() const = 0;
    
    /**
     * @brief Получить размер потока
     * @return Размер
     */
    virtual V8Int64 size() const = 0;
    
    /**
     * @brief Установить размер потока
     * @param newSize Новый размер
     */
    virtual void setSize(V8Int64 newSize) = 0;
};

/**
 * @brief Поток для работы с QByteArray
 */
class ByteArrayStream : public V8Stream {
public:
    explicit ByteArrayStream(QByteArray& data) : m_data(data), m_pos(0) {}
    
    V8Integer read(void* buffer, V8Integer count) override {
        V8Integer available = qMin(count, static_cast<V8Integer>(m_data.size() - m_pos));
        if (available <= 0) return 0;
        
        memcpy(buffer, m_data.constData() + m_pos, available);
        m_pos += available;
        return available;
    }
    
    V8Integer write(const void* buffer, V8Integer count) override {
        if (m_pos + count > m_data.size()) {
            m_data.resize(m_pos + count);
        }
        memcpy(m_data.data() + m_pos, buffer, count);
        m_pos += count;
        return count;
    }
    
    V8Int64 seek(V8Int64 offset, int origin) override {
        switch (origin) {
            case 0: // soFromBeginning
                m_pos = offset;
                break;
            case 1: // soFromCurrent
                m_pos += offset;
                break;
            case 2: // soFromEnd
                m_pos = m_data.size() + offset;
                break;
        }
        m_pos = qBound(V8Int64(0), m_pos, m_data.size());
        return m_pos;
    }
    
    V8Int64 position() const override {
        return m_pos;
    }
    
    V8Int64 size() const override {
        return m_data.size();
    }
    
    void setSize(V8Int64 newSize) override {
        m_data.resize(newSize);
        m_pos = qMin(m_pos, newSize);
    }
    
private:
    QByteArray& m_data;
    V8Int64 m_pos;
};

// === Утилиты конвертации ===

/**
 * @brief Конвертировать QString в std::string (UTF-8)
 */
inline std::string toStdString(const V8String& str) {
    return str.toUtf8().constData();
}

/**
 * @brief Конвертировать std::string в QString (UTF-8)
 */
inline V8String toV8String(const std::string& str) {
    return QString::fromUtf8(str.c_str());
}

/**
 * @brief Конвертировать QByteArray в V8Bytes
 */
inline V8Bytes toV8Bytes(const QByteArray& ba) {
    V8Bytes result;
    result.reserve(ba.size());
    for (int i = 0; i < ba.size(); ++i) {
        result.append(static_cast<quint8>(ba[i]));
    }
    return result;
}

/**
 * @brief Конвертировать V8Bytes в QByteArray
 */
inline QByteArray toQByteArray(const V8Bytes& bytes) {
    QByteArray result;
    result.reserve(bytes.size());
    for (quint8 b : bytes) {
        result.append(static_cast<char>(b));
    }
    return result;
}

} // namespace v8reader

#endif // V8READER_TYPES_V8_TYPES_H
