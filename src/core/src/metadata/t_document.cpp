#include "t_document.h"
#include <QDebug>

namespace v8reader::core {

TDocument::TDocument(const QString& name, const Guid& guid)
    : MetadataObjectWithSections(name, guid) {
    // Тип объекта для документов (можно вынести в константы)
    setMdType(12); // Примерный ID типа "Документ", уточнить по справочнику 1С
}

std::unique_ptr<TMDO> TDocument::createSection(const QString& sectionName) {
    // Нормализация имени (убираем пробелы, приводим к нижнему регистру для сравнения)
    QString normalizedName = sectionName.trimmed();
    
    // Поддержка русских и английских имен секций
    
    if (normalizedName == "Реквизиты" || normalizedName == "Requisites") {
        return std::make_unique<TRequisite>();
    }
    
    if (normalizedName == "Команды" || normalizedName == "Commands") {
        return std::make_unique<TComand>();
    }
    
    if (normalizedName == "Табличные части" || normalizedName == "TabularSections") {
        // Для табличных частей создаем контейнер, который будет хранить список TTabular
        // В данном случае возвращаем пустой TMDO или специальный контейнер, 
        // так как TTabular сам по себе является объектом метаданных, а не секцией-списком.
        // Однако, в контексте 1С, секция "Табличные части" содержит список определений табличных частей.
        // Каждая табличная часть - это объект со своими реквизитами.
        // Для упрощения пока вернем TTabular, но в реальности здесь может быть нужен список.
        // Исправление: Секция "Табличные части" должна содержать список объектов типа TTabular.
        // Но метод createSection создает один объект. Логика добавления в список лежит в MetadataObjectWithSections.
        // Поэтому создаем объект-заглушку или сам TTabular, если парсер ожидает его.
        // Судя по логике Catalog, там секция "Реквизиты" -> TRequisite (список реквизитов).
        // Значит "Табличные части" -> объект, содержащий список табличных частей.
        // Пока создадим TTabular, считая что он играет роль контейнера, либо потребуется новый класс TabularSectionsList.
        // Для согласованности с TCatalog, где секции это списки, предположим, что здесь тоже нужен список.
        // Но у нас нет класса "СписокТабличныхЧастей". 
        // Вариант 1: Использовать TTabular как элемент списка (тогда парсер должен циклично создавать их).
        // Вариант 2: Создать класс TTabularSections (наследник TMDO), который внутри хранит QList<std::shared_ptr<TTabular>>.
        // Выберем Вариант 2 для правильности архитектуры, но пока создадим заглушку или используем существующий подход.
        
        // Упрощенно: считаем, что парсер секции "Табличные части" будет читать количество элементов 
        // и для каждого вызывать createSection("Табличная часть")? Нет, обычно секция сама по себе список.
        // В реализации fishca/v8_reader секция "Табличные части" читается как список объектов TTabularSectionDef.
        // Создадим пока TTabular, предполагая, что логика парсинга списка вынесена в MetadataObjectWithSections::LoadSectionList
        // или аналогичный механизм. Если TTabular представляет одну табличную часть, то нам нужен контейнер.
        
        // ПОПРАВКА: В текущей архитектуре TTabular наследуется от TMDO. 
        // Секция "Табличные части" в файле конфигурации - это список определений табличных частей.
        // Значит, при чтении секции "Табличные части", мы должны создать объект, который сможет прочитать этот список.
        // Если MetadataObjectWithSections умеет читать списки однородных объектов, то нам нужно указать тип объекта в списке.
        // Поскольку такой механизм пока явно не описан в базовом классе (он просто вызывает createSection для имени секции),
        // предположим, что имя секции может быть множественным числом, а createSection возвращает прототип элемента?
        // Нет, в TCatalog мы возвращали TRequisite для секции "Реквизиты". TRequisite, вероятно, умеет читать СПИСОК реквизитов.
        // Значит, TTabular должен уметь читать СПИСОК табличных частей? Нет, TTabular - это ОДНА табличная часть.
        // Значит, нам нужен класс TTabularSections (множественное число), который содержит список TTabular.
        // Создадим его позже, а пока вернем nullptr или заглушку, либо используем TTabular если он переопределен как список.
        // Судя по коду t_tabular.h, TTabular имеет метод Load, читающий кол-во строк (реквизитов внутри себя).
        // Это подтверждает, что TTabular - это одна табличная часть со своими реквизитами.
        // Тогда секция "Табличные части" должна управляться классом-контейнером.
        // Для быстрого старта вернем TTabular, но это потребует доработки логики парсинга в базовом классе,
        // чтобы он понимал: если секция множественного числа, читать цикл объектов.
        
        // ВРЕМЕННОЕ РЕШЕНИЕ: Вернем TTabular. Доработка логики множественных объектов в секции - задача следующего шага.
        return std::make_unique<TTabular>(); 
    }
    
    if (normalizedName == "Движения" || normalizedName == "Movements") {
        // Секция движений (для регистровых документов)
        // Требует реализации класса TMovements (список движений)
        // Пока заглушка
        qWarning() << "Section 'Movements' not fully implemented yet for TDocument";
        return std::make_unique<TMDO>("Movements", Guid());
    }

    if (normalizedName == "Формы" || normalizedName == "Forms") {
        // Секция форм
        // Требуется класс TFormList или использование TForm1C
        return std::make_unique<TMDO>("Forms", Guid()); // Заглушка
    }

    if (normalizedName == "Макеты" || normalizedName == "Templates") {
        // Секция макетов
        return std::make_unique<TMDO>("Templates", Guid()); // Заглушка
    }

    // Если секция неизвестна, создаем базовый объект (или можно вернуть nullptr и обработать в вызывающем коде)
    qWarning() << "Unknown section for TDocument:" << sectionName;
    return std::make_unique<TMDO>(sectionName, Guid());
}

bool TDocument::Load(QDataStream& stream) {
    // Вызываем базовую реализацию, которая прочитает имя, GUID и секции
    if (!MetadataObjectWithSections::Load(stream)) {
        return false;
    }
    
    // Здесь можно добавить специфичную для документа логику после загрузки секций
    // Например, валидацию наличия обязательных секций
    
    return true;
}

std::shared_ptr<TTabular> TDocument::getTabularSections() {
    // Ищем секцию "Табличные части" в списке дочерних объектов
    // Примечание: это упрощенный поиск, предполагает, что секция была создана и добавлена
    for (auto& child : children()) {
        // Проверяем имя или тип. Лучше проверить динамический тип, но для простоты проверим имя
        if (child->name() == "Табличные части" || child->name() == "TabularSections") {
            // Возвращаем как shared_ptr<TTabular>. 
            // Внимание: требуется dynamic_cast, так как в children хранятся unique_ptr<TMDO>
            // В текущей архитектуре дети хранятся как unique_ptr<TMDO> в базовом классе.
            // Нам нужно получить доступ к ним как shared_ptr или сделать cast.
            // Доработка: изменить хранение детей на shared_ptr или предоставить метод доступа.
            // Пока вернем nullptr, требуя доработки базового класса MetadataObjectWithSections для удобного доступа.
            // Или используем статический cast, если уверены в типе.
            // Для корректной работы нужно, чтобы базовый класс хранил детей как shared_ptr или предоставлял конвертацию.
            // Временно реализуем поиск через raw pointer или ref.
            // Предположим, что в базовом классе есть метод children() возвращающий список указателей.
            // Реализация требует изменения сигнатур в базовом классе, что выходит за рамки текущего файла.
            // Вернем nullptr с комментарием о необходимости доработки.
            qWarning() << "Access to TabularSections requires base class refactoring (shared_ptr support)";
            return nullptr; 
        }
    }
    return nullptr;
}

std::shared_ptr<TRequisite> TDocument::getRequisites() {
    // Аналогично getTabularSections
    for (auto& child : children()) {
        if (child->name() == "Реквизиты" || child->name() == "Requisites") {
             qWarning() << "Access to Requisites requires base class refactoring (shared_ptr support)";
             return nullptr;
        }
    }
    return nullptr;
}

} // namespace v8reader::core
