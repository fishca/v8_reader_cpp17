#pragma once
#include <QTreeWidget>
#include <unordered_map>

namespace v8::core { struct MetadataItem; }
namespace v8::ui {

class MetadataTree : public QTreeWidget {
    Q_OBJECT
public:
    explicit MetadataTree(QWidget* parent = nullptr);
    void populate(const std::shared_ptr<const v8::core::MetadataItem>& root);

signals:
    void itemSelected(const std::wstring& itemId, const std::wstring& itemType);

private:
    QTreeWidgetItem* addItem(QTreeWidgetItem* parent, const std::shared_ptr<const v8::core::MetadataItem>& item);
    std::unordered_map<QTreeWidgetItem*, std::wstring> itemMap_;

private slots:
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
};
}