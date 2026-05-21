#pragma once

#include "v8_types.h"
#include <vector>
#include <memory>
#include <string>

namespace v8reader::core {

// Forward declarations
class TRequisite;

/**
 * @brief Базовый класс для всех объектов метаданных 1С (TMDO - TMetaDataObject).
 * 
 * Содержит общие свойства всех объектов конфигурации:
 * - Имя (Name)
 * - Синоним (Synonym)
 * - Комментарий (Comment)
 * - Внутренняя ссылка (Ref)
 * - Список реквизитов
 */
class TMDO {
public:
    TMDO();
    virtual ~TMDO();

    // Геттеры и сеттеры основных свойств
    const QString& getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    const QString& getSynonym() const { return m_synonym; }
    void setSynonym(const QString& synonym) { m_synonym = synonym; }

    const QString& getComment() const { return m_comment; }
    void setComment(const QString& comment) { m_comment = comment; }

    const QByteArray& getRef() const { return m_ref; }
    void setRef(const QByteArray& ref) { m_ref = ref; }

    // Работа с реквизитами
    const std::vector<std::shared_ptr<TRequisite>>& getRequisites() const { return m_requisites; }
    void addRequisite(std::shared_ptr<TRequisite> req);
    void clearRequisites();
    size_t requisitesCount() const { return m_requisites.size(); }

    /**
     * @brief Чтение данных объекта из потока.
     * 
     * @param stream Входной поток с данными .1CD
     * @param version Версия формата файла (15 или 16)
     * @return true если чтение прошло успешно
     */
    virtual bool readFromStream(QIODevice& stream, int version);

protected:
    /**
     * @brief Чтение строки из потока с учетом версии формата.
     * 
     * В версии 15 строки хранятся как ANSI, в версии 16 - как UTF-16LE.
     */
    QString readString(QIODevice& stream, int version);

    /**
     * @brief Чтение байтового массива (Ref, UUID и т.д.).
     */
    QByteArray readByteArray(QIODevice& stream, int size);

private:
    QString m_name;
    QString m_synonym;
    QString m_comment;
    QByteArray m_ref;
    std::vector<std::shared_ptr<TRequisite>> m_requisites;
};

} // namespace v8reader::core
