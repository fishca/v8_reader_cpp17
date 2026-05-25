#pragma once

#include "v8reader/core/metadata/TMDO.h"
#include <QString>
#include <QHash>
#include <QScopedPointer>
#include <functional>

namespace v8reader::core {

/**
 * @brief Фабрика для создания объектов метаданных по типу
 * 
 * Позволяет динамически создавать объекты на основе строкового идентификатора типа,
 * извлеченного из файла конфигурации.
 */
class MetadataFactory {
public:
    using CreatorFunc = std::function<std::unique_ptr<TMDO>()>;

    /**
     * @brief Зарегистрировать тип метаданных
     * @param typeName Имя типа (например, "Catalog", "Document")
     * @param creator Функция-конструктор
     */
    static void registerType(const QString& typeName, CreatorFunc creator);

    /**
     * @brief Создать объект метаданных по имени типа
     * @param typeName Имя типа
     * @return Уникальный указатель на созданный объект или nullptr, если тип не найден
     */
    static std::unique_ptr<TMDO> createObject(const QString& typeName);

    /**
     * @brief Проверить наличие зарегистрированного типа
     */
    static bool isRegistered(const QString& typeName);

    /**
     * @brief Получить список всех зарегистрированных типов
     */
    static QStringList getRegisteredTypes();

private:
    // Запрещаем создание экземпляров
    MetadataFactory() = delete;
    ~MetadataFactory() = delete;

    static QHash<QString, CreatorFunc>& creators();
};

} // namespace v8reader::core
