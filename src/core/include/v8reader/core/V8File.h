#pragma once

#include "v8reader/types/v8_types.h"
#include "v8reader/core/V8Element.h"
#include <QString>
#include <QVector>
#include <QMap>
#include <QIODevice>
#include <memory>
#include <optional>

namespace v8reader::core {

/**
 * @brief Абстракция для работы с файлами конфигурации 1С (.cf, .1CD, .epf, .erf)
 * 
 * Этот класс предоставляет высокоуровневый интерфейс для чтения файлов конфигурации,
 * инкапсулируя детали формата хранения данных (версии 8.0-8.3.x, сжатие, FAT-таблицы).
 * 
 * Пример использования:
 * @code
 * auto file = V8File::loadFromFile("path/to/config.cf");
 * if (file) {
 *     auto root = file->getRoot();
 *     for (const auto& element : root->getChildren()) {
 *         qDebug() << element->getName();
 *     }
 * }
 * @endcode
 */
class V8File {
public:
    virtual ~V8File() = default;

    /**
     * @brief Загрузить файл конфигурации из указанного пути
     * @param filePath Путь к файлу (.cf, .1CD, .epf, .erf)
     * @return Умный указатель на загруженный файл или nullptr при ошибке
     */
    static std::shared_ptr<V8File> loadFromFile(const QString& filePath);

    /**
     * @brief Загрузить файл конфигурации из потока
     * @param device Устройство ввода/вывода (QFile, QBuffer и т.д.)
     * @return Умный указатель на загруженный файл или nullptr при ошибке
     */
    static std::shared_ptr<V8File> loadFromDevice(std::unique_ptr<QIODevice> device);

    /**
     * @brief Получить корневой элемент дерева метаданных
     * @return Умный указатель на корневой элемент
     */
    [[nodiscard]] virtual std::shared_ptr<V8Element> getRoot() const = 0;

    /**
     * @brief Получить версию формата файла (15 или 16)
     * @return Версия формата
     */
    [[nodiscard]] virtual int getVersion() const = 0;

    /**
     * @brief Проверить, является ли файл каталогом (имеет иерархическую структуру)
     * @return true если это каталог
     */
    [[nodiscard]] virtual bool isCatalog() const = 0;

    /**
     * @brief Получить список всех элементов по имени
     * @param name Имя элемента (может включать путь)
     * @return Вектор найденных элементов
     */
    [[nodiscard]] virtual QVector<std::shared_ptr<V8Element>> findElementsByName(const QString& name) const = 0;

    /**
     * @brief Извлечь данные элемента по имени
     * @param name Имя элемента
     * @return Байтовый массив с данными или nullptr если не найдено
     */
    [[nodiscard]] virtual std::optional<QByteArray> extractData(const QString& name) const = 0;

    /**
     * @brief Извлечь текст модуля по имени
     * @param name Имя модуля
     * @return Текст модуля или nullptr если не найдено
     */
    [[nodiscard]] virtual std::optional<QString> extractModuleText(const QString& name) const = 0;

    /**
     * @brief Получить последнюю ошибку
     * @return Описание последней ошибки
     */
    [[nodiscard]] virtual QString getLastError() const = 0;

protected:
    V8File() = default;
};

} // namespace v8reader::core
