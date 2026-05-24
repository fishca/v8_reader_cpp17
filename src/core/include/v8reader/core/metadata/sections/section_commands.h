#pragma once

#include "t_mdo.h"
#include "t_comand.h"
#include <memory>
#include <vector>
#include <QString>

namespace v8reader::core {

/**
 * @brief Секция "Команды" (Commands).
 * 
 * Содержит список команд объекта метаданных.
 */
class SectionCommands : public TMDO {
public:
    SectionCommands() = default;
    ~SectionCommands() override = default;

    void Load(QDataStream& stream, int version) override;

    /**
     * @brief Добавить команду в список.
     */
    void addCommand(std::unique_ptr<TComand> cmd);

    /**
     * @brief Получить список всех команд.
     */
    [[nodiscard]] const std::vector<std::unique_ptr<TComand>>& getCommands() const;

    /**
     * @brief Получить команду по имени.
     */
    [[nodiscard]] TComand* findCommand(const QString& name) const;

    /**
     * @brief Количество команд.
     */
    [[nodiscard]] size_t count() const;

private:
    std::vector<std::unique_ptr<TComand>> m_commands;
};

} // namespace v8reader::core
