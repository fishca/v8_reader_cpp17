#include "MetadataFactory.h"
#include <QMutexLocker>
#include <QMutex>

namespace v8reader::core {

// Статическая функция для получения хэша с ленивой инициализацией
QHash<QString, MetadataFactory::CreatorFunc>& MetadataFactory::creators() {
    static QHash<QString, CreatorFunc> instance;
    return instance;
}

void MetadataFactory::registerType(const QString& typeName, CreatorFunc creator) {
    creators().insert(typeName, creator);
}

std::unique_ptr<TMDO> MetadataFactory::createObject(const QString& typeName) {
    auto it = creators().constFind(typeName);
    if (it != creators().constEnd()) {
        return it.value()();
    }
    
    // Если тип не найден, создаем базовый TMDO
    // В будущем можно добавить логирование или обработку ошибок
    return std::make_unique<TMDO>();
}

bool MetadataFactory::isRegistered(const QString& typeName) {
    return creators().contains(typeName);
}

QStringList MetadataFactory::getRegisteredTypes() {
    return creators().keys();
}

} // namespace v8reader::core
