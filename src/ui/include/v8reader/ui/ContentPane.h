#pragma once
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QTableWidget>

namespace v8::ui {
    class ContentPane : public QTabWidget {
        Q_OBJECT
    public:
        explicit ContentPane(QWidget* parent = nullptr);
        void showContent(const std::wstring& itemId, const std::wstring& itemType);
    private:
        QWidget* createModuleTab();
        QWidget* createPropertiesTab();
        QPlainTextEdit* moduleEdit_{};
        QTableWidget* propsTable_{};
    };
}