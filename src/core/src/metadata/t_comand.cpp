#include "t_comand.h"
#include "../parser/v8_stream_reader.h" // Предполагаемый путь к утилите чтения

namespace v8reader::core::metadata {

bool TComand::Load(QIODevice& stream, int version) {
    // 1. Загружаем базовые свойства TMDO (Имя, Синоним, Комментарий и т.д.)
    if (!TMDO::Load(stream, version)) {
        return false;
    }

    // 2. Читаем специфичные свойства команды
    // Структура может отличаться в зависимости от версии формата, 
    // здесь приведен примерный порядок чтения на основе типовой структуры 1С
    
    parser::V8StreamReader reader(stream, version);

    // Чтение поля Action (Действие)
    if (reader.HasNext()) {
        m_action = reader.ReadString();
    }

    // Чтение поля Use (Использование)
    if (reader.HasNext()) {
        m_use = reader.ReadString();
    }

    // Чтение поля Presentation (Представление)
    if (reader.HasNext()) {
        m_presentation = reader.ReadString();
    }
    
    // Примечание: Реальный порядок и наличие полей нужно сверять с исходным C++ Builder кодом
    // и тестировать на реальных файлах .cf/.1CD
    
    return true;
}

} // namespace v8reader::core::metadata
