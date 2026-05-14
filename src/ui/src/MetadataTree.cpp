#include "v8reader/core/IV8Repository.h"  // Содержит полное определение MetadataItem

#include "v8reader/ui/MetadataTree.h"
#include <QIcon>

namespace v8::ui {

MetadataTree::MetadataTree(QWidget* parent) : QTreeWidget(parent) {
    setHeaderLabel(tr("Метаданные"));
    setExpandsOnDoubleClick(true);
    setAlternatingRowColors(true);
    connect(this, &QTreeWidget::currentItemChanged, this, &MetadataTree::onCurrentItemChanged);
}

void MetadataTree::populate(const std::shared_ptr<const v8::core::MetadataItem>& root) {
    clear(); itemMap_.clear();
    if (!root) return;
    for (const auto& child : root->children) addItem(nullptr, child);
    expandAll();
}

QTreeWidgetItem* MetadataTree::addItem(QTreeWidgetItem* parent, const std::shared_ptr<const v8::core::MetadataItem>& item) {
    auto* treeItem = new QTreeWidgetItem(parent);
    treeItem->setText(0, QString::fromStdWString(item->name));
    treeItem->setData(0, Qt::UserRole, QString::fromStdWString(item->id));
    
    if (item->is_folder) treeItem->setIcon(0, QIcon::fromTheme("folder"));
    else if (item->type == L"Catalog") treeItem->setIcon(0, QIcon::fromTheme("database"));
    
    itemMap_[treeItem] = item->id;
    for (const auto& child : item->children) addItem(treeItem, child);
    return treeItem;
}

void MetadataTree::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem*) {
    if (!current) return;
    auto it = itemMap_.find(current);
    if (it != itemMap_.end()) emit itemSelected(it->second, L"Unknown");
}

}