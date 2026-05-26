#pragma once

#include "v8reader/core/metadata/TMDO.h"
#include <QString>
#include <QByteArray>
#include <QDataStream>

namespace v8reader::core {

/**
 * @brief Класс реквизита объекта метаданных 1С.
 *
 * Реквизит - это поле данных объекта (например, поле справочника или документа).
 */
class TRequisite : public TMDO {
public:
    TRequisite();
    virtual ~TRequisite() override;

    // Геттеры и сеттеры
    const QString& getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    const QString& getSynonym() const { return m_synonym; }
    void setSynonym(const QString& synonym) { m_synonym = synonym; }

    const QString& getTypeDescription() const { return m_typeDescription; }
    void setTypeDescription(const QString& desc) { m_typeDescription = desc; }

    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }

    int getLength() const { return m_length; }
    void setLength(int length) { m_length = length; }

    int getPrecision() const { return m_precision; }
    void setPrecision(int precision) { m_precision = precision; }

    bool isAllowNull() const { return m_allowNull; }
    void setAllowNull(bool allow) { m_allowNull = allow; }

    // Для тестов
    int typeId() const { return m_index; }
    bool allowNull() const { return m_allowNull; }

    /**
     * @brief Чтение данных реквизита из потока.
     */
    bool Load(QDataStream& stream, int version) override;

protected:
    QString readString(QDataStream& stream, int version);

private:
    QString m_name;
    QString m_synonym;
    QString m_typeDescription;
    int m_index = 0;
    int m_length = 0;
    int m_precision = 0;
    bool m_allowNull = false;
};

} // namespace v8reader::core
