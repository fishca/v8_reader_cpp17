#include "v8reader/core/V8File.h"
#include "v8reader/core/V8Container.h"
#include <QFile>
#include <QBuffer>
#include <QDataStream>

namespace v8reader::core {

// Внутренняя реализация V8File на основе V8Container
class V8FileImpl : public V8File {
public:
    explicit V8FileImpl(std::shared_ptr<v8::core::V8Container> container)
        : m_container(std::move(container)) {}

    std::shared_ptr<V8Element> getRoot() const override {
        // TODO: Преобразовать MetadataItem из старого API в V8Element
        return nullptr;
    }

    int getVersion() const override {
        return m_container->isFormat16() ? 16 : 15;
    }

    bool isCatalog() const override {
        // Проверяем наличие корневого элемента "root"
        return m_container->findElement(L"root") != nullptr;
    }

    QVector<std::shared_ptr<V8Element>> findElementsByName(const QString& name) const override {
        QVector<std::shared_ptr<V8Element>> result;
        // TODO: Реализовать поиск
        return result;
    }

    std::optional<QByteArray> extractData(const QString& name) const override {
        auto data = m_container->extractData(name.toStdWString());
        if (data.has_value()) {
            return QByteArray(reinterpret_cast<const char*>(data->data()), 
                             static_cast<int>(data->size()));
        }
        return std::nullopt;
    }

    std::optional<QString> extractModuleText(const QString& name) const override {
        auto text = m_container->getModuleText(name.toStdWString());
        if (text.has_value()) {
            return QString::fromStdWString(*text);
        }
        return std::nullopt;
    }

    QString getLastError() const override {
        return QString::fromStdWString(m_container->getLastError());
    }

private:
    std::shared_ptr<v8::core::V8Container> m_container;
};

std::shared_ptr<V8File> V8File::loadFromFile(const QString& filePath) {
    try {
        auto container = std::make_shared<v8::core::V8Container>(filePath.toStdWString());
        if (container->load() == 0) {
            return std::make_shared<V8FileImpl>(container);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error loading file:" << filePath << ":" << e.what();
    }
    return nullptr;
}

std::shared_ptr<V8File> V8File::loadFromDevice(std::unique_ptr<QIODevice> device) {
    // TODO: Реализовать загрузку из устройства
    Q_UNUSED(device);
    return nullptr;
}

} // namespace v8reader::core
