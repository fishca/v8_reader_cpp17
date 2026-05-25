#include "v8reader/ui/MetadataTree.h"
#include "v8reader/core/V8Container.h"
#include <QHeaderView>

namespace v8reader::ui {

    MetadataTree::MetadataTree(QWidget* parent)
        : QTreeWidget(parent), container_(nullptr)
    {
        setHeaderLabel(tr("Метаданные"));
        setExpandsOnDoubleClick(true);
        setAlternatingRowColors(true);
        setIndentation(20);

        connect(this, &QTreeWidget::currentItemChanged,
            this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
                onCurrentItemChanged(current, previous);
            });
    }

    void MetadataTree::populate(const std::shared_ptr<v8reader::core::MetadataItem>& root,
        v8reader::core::V8Container* container) // 🔑 Принимаем указатель
    {
        clear();
        itemMap_.clear();
        container_ = container; // Сохраняем указатель

        if (!root) return;

        for (const auto& child : root->children) {
            addItem(nullptr, child);
        }
        expandAll();
    }

    QTreeWidgetItem* MetadataTree::addItem(QTreeWidgetItem* parent,
        const std::shared_ptr<v8reader::core::MetadataItem>& item)
    {
        auto* treeItem = parent
            ? new QTreeWidgetItem(parent)
            : new QTreeWidgetItem(this);
        treeItem->setText(0, QString::fromStdWString(item->name));
        treeItem->setIcon(0, getIconForType(QString::fromStdWString(item->type), item->is_folder));

        // Сохраняем ID и тип для последующего доступа
        itemMap_[treeItem] = item->id;
        treeItem->setData(0, Qt::UserRole + 1, QString::fromStdWString(item->type));

        // Рекурсивно добавляем дочерние элементы
        for (const auto& child : item->children) {
            addItem(treeItem, child);
        }

        return treeItem;
    }

    QIcon MetadataTree::getIconForType(const QString& type, bool isFolder) const
    {
        if (isFolder) {
            return QIcon::fromTheme("folder", QIcon(":/icons/folder.png"));
        }

        // Иконки для различных типов метаданных
        if (type == "Catalog" || type == "Справочники")
            return QIcon::fromTheme("database", QIcon(":/icons/catalog.png"));
        if (type == "Document" || type == "Документы")
            return QIcon::fromTheme("document", QIcon(":/icons/document.png"));
        if (type == "CommonModule" || type == "Общие модули")
            return QIcon::fromTheme("code", QIcon(":/icons/module.png"));
        if (type == "Report" || type == "Отчеты")
            return QIcon::fromTheme("report", QIcon(":/icons/report.png"));
        if (type == "Processing" || type == "Обработки")
            return QIcon::fromTheme("processing", QIcon(":/icons/processing.png"));

        return QIcon::fromTheme("file", QIcon(":/icons/file.png"));
    }

    void MetadataTree::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
    {
        if (!current) return;

        auto it = itemMap_.find(current);
        if (it != itemMap_.end()) {
            QString type = current->data(0, Qt::UserRole + 1).toString();
            emit itemSelected(it->second, type.toStdWString());
        }
    }

} // namespace v8reader::ui
