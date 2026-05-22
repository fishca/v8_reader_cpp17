# План переноса классов метаданных из v8_reader в v8_reader_cpp17

## Цель
Перенести основные классы для работы с метаданными 1С из проекта на C++ Builder в кроссплатформенный проект на C++17/Qt6, сохраняя архитектуру и функциональность.

## Этапы переноса

### Этап 1: Базовые классы и инфраструктура (Высокий приоритет)

Этот этап критически важен, так как создает фундамент для всех последующих классов. Без этих компонентов работа с метаданными невозможна.

#### 1.1. Адаптация типов данных и утилит
**Файлы-источники:** `Common.h`, `types/`, базовые определения  
**Целевые файлы:** `src/core/include/v8reader/core/types.hpp`, `src/core/src/types.cpp`

**Задачи:**
- [ ] **Замена строковых типов:**
  - `UnicodeString` (VCL) → `QString` (Qt) или `std::u16string`
  - Реализовать функции конвертации между кодировками (UTF-16 ↔ UTF-8)
  - Создать алиасы: `using V8String = QString;`
- [ ] **Замена потоков:**
  - `TStream` / `TMemoryStream` → `QIODevice` / `QBuffer` / `QFile`
  - Обертки для чтения бинарных данных (`readUInt32`, `readString`)
- [ ] **Замена GUID:**
  - `TGUID` → `QUuid`
  - Утилиты для парсинга GUID из байтовых массивов
- [ ] **Удаление спецификаторов:**
  - Заменить `__fastcall` на стандартный вызов (или оставить пустым для кроссплатформенности)
  - Убрать зависимости от `System.Classes.hpp`, `System.SysUtils.hpp`

#### 1.2. Базовый класс объекта метаданных (`TMDO`)
**Файл-источник:** `MDO.h` / `MDO.cpp`  
**Целевой файл:** `src/core/include/v8reader/core/metadata/metadata_object.hpp`

**Задачи:**
- [ ] Создать абстрактный базовый класс `MetadataObject`
- [ ] Перенести свойства: `Name`, `Synonym`, `Comment`, `GUID`, `Parent`
- [ ] Реализовать метод `loadFromStream(QIODevice*)`
- [ ] Реализовать метод `parseProperties(const QString& treeData)`
- [ ] Добавить виртуальные методы для полиморфной загрузки секций

#### 1.3. Парсинг дерева свойств (`tree`)
**Файл-источник:** `tree.h` / `tree.cpp`  
**Целевой файл:** `src/core/include/v8reader/core/parser/property_tree_parser.hpp`

**Задачи:**
- [ ] Переписать парсер текста `.1CD` (формат дерева свойств)
- [ ] Реализовать токенизатор строк вида `"Имя"="Значение"`
- [ ] Обработка вложенных структур `{...}`
- [ ] Поддержка экранирования кавычек
- [ ] Покрыть модульными тестами (Catch2 / GoogleTest)

#### 1.4. Классы реквизитов и измерений
**Файлы-источники:** `Requisite.h`, `AccountingFlag.h`, `DimensionAccountingFlag.h`  
**Целевые файлы:** `src/core/include/v8reader/core/metadata/requisite.hpp`, `.../accounting_flags.hpp`

**Задачи:**
- [ ] Класс `Requisite`: имя, тип, синоним, комментарий
- [ ] Класс `TAccountingFlag`: флаг бухгалтерского учета
- [ ] Класс `TDimensionAccountingFlag`: флаг аналитического учета
- [ ] Реализовать парсинг списков реквизитов из потока

#### 1.5. Команды и формы
**Файлы-источники:** `Comand.h`, `Form1C.h`  
**Целевые файлы:** `src/core/include/v8reader/core/metadata/command.hpp`, `.../form.hpp`

**Задачи:**
- [ ] Класс `Command`: имя, представление, модификаторы
- [ ] Класс `Form1C`: имя формы, тип, данные формы (binary blob)
- [ ] Методы извлечения данных формы для последующей десериализации

#### 1.6. Табличные части и Мохель (MoXel)
**Файлы-источники:** `Tabular.h`, `Moxel.h`  
**Целевые файлы:** `src/core/include/v8reader/core/metadata/tabular_section.hpp`, `.../moxel.hpp`

**Задачи:**
- [ ] Класс `TabularSection`: список полей, имя
- [ ] Класс `Moxel`: структура хранения табличных данных (если используется в метаданных)
- [ ] Интеграция с основными объектами (Каталоги, Документы)

#### 1.7. Базовый класс для объектов с секциями
**Файл-источник:** `MetadataObjectWithSections.h`  
**Целевой файл:** `src/core/include/v8reader/core/metadata/object_with_sections.hpp`

**Задачи:**
- [ ] Наследование от `MetadataObject`
- [ ] Реализация хранилища секций (`std::map<QString, QByteArray>`)
- [ ] Метод `loadSections(QIODevice*)` для чтения заголовков секций
- [ ] Метод `getSection(const QString& name)` для доступа к данным

---

### Критерии готовности Этапа 1
1. ✅ Собирается базовый проект без ошибок линковки.
2. ✅ Проходят юнит-тесты на парсинг дерева свойств.
3. ✅ Можно загрузить "пустой" объект метаданных из файла `.1CD`.
4. ✅ Отсутствуют зависимости от заголовков C++ Builder.
5. ✅ Код компилируется на Linux и Windows.

### Этап 2: Основные объекты метаданных (Средний приоритет)
- MetadataObjectWithSections
- TCatalogs
- TDocuments
- TReports
- TDataProcessors
- TInformationRegisters
- TAccumulationRegisters
- TAccountingRegisters
- TChartOfAccounts
- TConstants
- TSubsystem
- TCommonModules
- TEnums

### Этап 3: Дополнительные объекты метаданных (Низкий приоритет)
- TBusinessProceses
- TTasks
- TJournal
- TChartOfCalculationTypes
- TChartOfCharacteristicTypes
- TCalculationRegisters
- TExternalDataSources
- TFilterCriteria
- TSequences
- TSettingsStorages
- TEventSubscriptions
- TScheduledJobs
- TFunctionalOptions
- TFunctionalOptionsParameters
- TDefinedTypes
- TInterfaces
- TRoles
- TBots
- TCommonAttributes
- TCommonCommands
- TCommonTemplates
- TCommandGroups
- TExchangePlans
- TSessionParameters
- IntegrationServices, HTTPServices, WebServices, WebSocketClients, WSReferences, XDTOPackages
- CommonPictures, CommonForms, Styles, StyleItems

### Этап 4: Менеджеры и утилиты (Средний приоритет)
- MetaDataManager
- MetaObject
- ModuleTextStorage
- ConfigStorage
- Common

## Технические задачи адаптации
- Замена VCL на Qt/C++17 (UnicodeString → QString, DynamicArray<Byte> → QByteArray, TGUID → QUuid)
- Удаление спецификаторов __fastcall
- Модернизация управления памятью (std::unique_ptr, std::shared_ptr)

## Структура целевого проекта
```
src/core/
├── include/v8reader/core/
│   ├── metadata/
│   ├── types/
│   ├── parser/
│   └── storage/
└── src/
    ├── metadata/
    ├── types/
    ├── parser/
    └── storage/
```

## Рекомендации
1. Начать с Этапа 1 - создать работающий фундамент
2. Покрывать тестами каждый класс после переноса
3. Использовать feature-ветки для каждого типа метаданных
4. Вести документацию
5. Создать CI/CD пайплайны

## Оценка сроков
- Всего классов: 53
- Общая оценка: 94-139 часов
