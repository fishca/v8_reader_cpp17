#include "v8reader/core/metadata/sections/section_commands.h"
#include <QDataStream>
#include <QDebug>

namespace v8reader::core {

bool SectionCommands::Load(QDataStream& stream, int version) {
    // Загружаем базовые данные секции
    bool result = TMDO::Load(stream, version);

    // Читаем количество команд
    quint32 cmdCount;
    stream >> cmdCount;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading commands count in section:" << getName();
        return result;
    }

    // Читаем каждую команду
    for (quint32 i = 0; i < cmdCount; ++i) {
        auto command = std::make_unique<TComand>();
        command->Load(stream, version);
        m_commands.push_back(std::move(command));
    }
    
    return result;
}

void SectionCommands::addCommand(std::unique_ptr<TComand> cmd) {
    m_commands.push_back(std::move(cmd));
}

const std::vector<std::unique_ptr<TComand>>& SectionCommands::getCommands() const {
    return m_commands;
}

TComand* SectionCommands::findCommand(const QString& name) const {
    for (const auto& cmd : m_commands) {
        if (cmd->getName() == name) {
            return cmd.get();
        }
    }
    return nullptr;
}

size_t SectionCommands::count() const {
    return m_commands.size();
}

} // namespace v8reader::core
