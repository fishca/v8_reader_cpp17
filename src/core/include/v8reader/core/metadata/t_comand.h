#pragma once

#include "TMDO.h"
#include <memory>
#include <vector>
#include <QString>
#include <QDataStream>

namespace v8reader::core::metadata {

/**
 * @brief Класс, представляющий команду объекта метаданных (Command).
 *
 * Команды используются в формах, панелях навигации и других интерфейсах.
 * Наследуется от TMDO, так как имеет имя, синоним и другие стандартные свойства.
 */
class TComand : public TMDO {
public:
    explicit TComand() = default;
    ~TComand() override = default;

    // Запрет копирования, разрешение перемещения
    TComand(const TComand&) = delete;
    TComand& operator=(const TComand&) = delete;
    TComand(TComand&&) noexcept = default;
    TComand& operator=(TComand&&) noexcept = default;

    /**
     * @brief Парсинг свойств команды из потока.
     * @param stream Входной поток с данными.
     * @param version Версия формата файла.
     * @return true если парсинг успешен
     */
    bool Load(QDataStream& stream, int version) override;

    /**
     * @brief Получение имени действия (Action).
     */
    [[nodiscard]] const QString& GetAction() const { return m_action; }

    /**
     * @brief Получение режима использования (Use).
     */
    [[nodiscard]] const QString& GetUse() const { return m_use; }

    /**
     * @brief Получение представления команды (Presentation).
     */
    [[nodiscard]] const QString& GetPresentation() const { return m_presentation; }

private:
    QString m_action;       // Действие (ссылка на метод или команду)
    QString m_use;          // Режим использования (например, "Навигация", "Команда")
    QString m_presentation; // Представление (текст кнопки/пункта меню)
};

} // namespace v8reader::core::metadata
