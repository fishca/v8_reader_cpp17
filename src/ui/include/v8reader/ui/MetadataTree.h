#pragma once
#include <QTreeWidget>
#include <QIcon>
#include <memory>
#include <unordered_map>

namespace v8::core {
    struct MetadataItem;
    class V8Container;
}

namespace v8::ui {

    class MetadataTree : public QTreeWidget {
        Q_OBJECT

    public:
        explicit MetadataTree(QWidget* parent = nullptr);

        void populate(const std::shared_ptr<v8::core::MetadataItem>& root,
            std::unique_ptr<v8::core::V8Container> container);

    signals:
        void itemSelected(const std::wstring& itemId, const std::wstring& itemType);

    private:
        QTreeWidgetItem* addItem(QTreeWidgetItem* parent,
            const std::shared_ptr<v8::core::MetadataItem>& item);
        QIcon getIconForType(const QString& type, bool isFolder) const;

        std::unordered_map<QTreeWidgetItem*, std::wstring> itemMap_;
        std::unique_ptr<v8::core::V8Container> container_;

    private slots:
        void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    };

} // namespace v8::ui