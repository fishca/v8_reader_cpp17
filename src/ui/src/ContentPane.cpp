#include "v8reader/ui/ContentPane.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace v8::ui {

ContentPane::ContentPane(QWidget* parent) : QTabWidget(parent) {
    setTabPosition(QTabWidget::North);
    setDocumentMode(true);
}

void ContentPane::showContent(const std::wstring& itemId, const std::wstring&) {
    if (count() == 0) {
        addTab(createModuleTab(), tr("Модуль"));
        addTab(createPropertiesTab(), tr("Свойства"));
    }
    
    if (itemId == L"item_1") {
        moduleEdit_->setPlainText("&НаКлиенте\nПроцедура ОбработкаПроведения()\n  // TODO\nКонецПроцедуры");
        propsTable_->setRowCount(3);
        propsTable_->setItem(0, 0, new QTableWidgetItem("Имя")); propsTable_->setItem(0, 1, new QTableWidgetItem("Номенклатура"));
        propsTable_->setItem(1, 0, new QTableWidgetItem("Тип")); propsTable_->setItem(1, 1, new QTableWidgetItem("Справочник"));
        propsTable_->setItem(2, 0, new QTableWidgetItem("Иерархия")); propsTable_->setItem(2, 1, new QTableWidgetItem("Да"));
    }
}

QWidget* ContentPane::createModuleTab() {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    moduleEdit_ = new QPlainTextEdit();
    moduleEdit_->setFont(QFont("Consolas", 10));
    moduleEdit_->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(moduleEdit_);
    return widget;
}

QWidget* ContentPane::createPropertiesTab() {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    propsTable_ = new QTableWidget(0, 2);
    propsTable_->setHorizontalHeaderLabels({"Свойство", "Значение"});
    propsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    propsTable_->verticalHeader()->setVisible(false);
    layout->addWidget(propsTable_);
    return widget;
}

}