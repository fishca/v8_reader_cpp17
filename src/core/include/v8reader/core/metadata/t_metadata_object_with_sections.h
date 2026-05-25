#pragma once

#include "TMDO.h"
#include <memory>
#include <map>
#include <QString>

namespace v8reader::core {

/**
 * @brief Базовый класс для объектов метаданных, имеющих секции.
 * 
 * Многие объекты 1С (справочники, документы и т.д.) имеют внутреннюю структуру,
 * разделенную на секции (реквизиты, формы, команды, модули и т.п.).
 * Этот класс предоставляет базовую логику для чтения и хранения таких секций.
 */
class MetadataObjectWithSections : public TMDO {
public:
    explicit MetadataObjectWithSections() = default;
    ~MetadataObjectWithSections() override = default;

    /**
     * @brief Загрузка данных объекта из потока.
     * @param stream Входной поток с данными.
     * @param version Версия формата файла (.1CD).
     */
    bool Load(QDataStream& stream, int version) override;

    /**
     * @brief Получить секцию по имени.
     * @param name Имя секции.
     * @return Указатель на секцию или nullptr, если не найдена.
     */
    [[nodiscard]] TMDO* getSection(const QString& name) const;

    /**
     * @brief Проверить наличие секции.
     * @param name Имя секции.
     * @return true, если секция существует.
     */
    [[nodiscard]] bool hasSection(const QString& name) const;

    /**
     * @brief Получить список имен всех секций.
     * @return Вектор имен секций.
     */
    [[nodiscard]] std::vector<QString> getSectionNames() const;

    /**
     * @brief Получить количество секций.
     */
    [[nodiscard]] size_t getSectionCount() const;

protected:
    /**
     * @brief Фабричный метод для создания экземпляра секции по её имени.
     * Должен быть переопределен в наследниках для поддержки конкретных типов секций.
     * @param name Имя секции.
     * @return Умный указатель на созданный объект секции или nullptr.
     */
    virtual std::unique_ptr<TMDO> createSection(const QString& name);

private:
    // Карта секций: имя -> объект секции
    std::map<QString, std::unique_ptr<TMDO>> m_sections;
};

} // namespace v8reader::core
