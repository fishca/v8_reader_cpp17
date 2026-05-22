#include "t_tabular.h"
#include "../parser/v8_stream_reader.h"

namespace v8reader::core::metadata {

void TTabular::Load(QIODevice& stream, int version) {
    // 1. Загружаем базовые свойства TMDO
    TMDO::Load(stream, version);

    parser::V8StreamReader reader(stream, version);

    // 2. Читаем количество строк по умолчанию (DefaultRowCount)
    if (reader.HasNext()) {
        m_default_row_count = reader.ReadInt();
    }

    // 3. Читаем список реквизитов
    // Обычно в потоке сначала идет количество элементов, затем сами элементы
    if (reader.HasNext()) {
        int requisiteCount = reader.ReadInt();
        for (int i = 0; i < requisiteCount; ++i) {
            auto req = std::make_unique<TRequisite>();
            // Рекурсивный вызов Load для реквизита, если он тоже читает из потока
            // Или отдельный метод чтения, если структура плоская
            // В данном случае предполагаем, что TRequisite имеет свой метод чтения из того же потока
            req->Load(stream, version); 
            m_requisites.push_back(std::move(req));
        }
    }

    // Примечание: Точная структура хранения списка реквизитов в бинарном файле
    // может отличаться (например, использование индексов или вложенных потоков).
    // Требуется верификация на реальных данных.
}

void TTabular::AddRequisite(std::unique_ptr<TRequisite> req) {
    m_requisites.push_back(std::move(req));
}

} // namespace v8reader::core::metadata
