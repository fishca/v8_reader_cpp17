#pragma once

#include "v8reader/core/metadata/TMDO.h"
#include <QString>
#include <QByteArray>
#include <memory>
#include <vector>

namespace v8reader::core::metadata {

/**
 * @brief Класс для представления объекта "Мохель" (вложенный объект метаданных)
 * 
 * В 1С "Мохель" (от англ. "Moxel" или "Nested") — это вложенный объект,
 * используемый для описания сложной структуры данных внутри другого объекта.
 */
class TMoxel : public TMDO {
public:
    explicit TMoxel();
    ~TMoxel() override = default;

    /**
     * @brief Загрузка данных из потока
     * @param stream Входной поток с данными
     * @param version Версия формата файла
     * @return true если загрузка успешна
     */
    bool Load(QDataStream& stream, int version) override;

    /**
     * @brief Получить тип мохеля
     */
    [[nodiscard]] const QString& GetType() const { return m_type; }

    /**
     * @brief Установить тип мохеля
     */
    void SetType(const QString& type) { m_type = type; }

    /**
     * @brief Получить данные мохеля (сырые байты или сериализованное представление)
     */
    [[nodiscard]] const QByteArray& GetData() const { return m_data; }

    /**
     * @brief Установить данные мохеля
     */
    void SetData(const QByteArray& data) { m_data = data; }

protected:
    QString m_type;      ///< Тип вложенного объекта
    QByteArray m_data;   ///< Данные вложенного объекта
};

} // namespace v8reader::core::metadata
