#include "v8reader/ui/ContentPane.h"
#include "v8reader/core/V8Container.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QFont>

namespace v8::ui {

    ContentPane::ContentPane(QWidget* parent)
        : QTabWidget(parent), container_(nullptr)
    {
        setTabPosition(QTabWidget::North);
        setDocumentMode(true);
    }

    void ContentPane::setContainer(v8::core::V8Container* container)
    {
        container_ = container;
    }

    void ContentPane::showContent(const std::wstring& itemId, const std::wstring& itemType)
    {
        if (!container_) return;

        // 🔑 Очищаем существующие вкладки (исправлено: переименована переменная)
        while (count() > 0) {
            QWidget* page = widget(0);  // ✅ 'page' вместо 'widget'
            removeTab(0);
            delete page;
        }

        // Получаем текст модуля
        auto moduleText = container_->getModuleText(itemId);
        if (moduleText) {
            auto* editor = new QPlainTextEdit();
            editor->setPlainText(QString::fromStdWString(*moduleText));
            editor->setFont(QFont("Consolas", 10));
            editor->setLineWrapMode(QPlainTextEdit::NoWrap);

            addTab(editor, tr("Модуль"));
            setCurrentWidget(editor);
        }
        else {
            auto* info = new QLabel(tr("Элемент не содержит модуля"));
            info->setAlignment(Qt::AlignCenter);
            addTab(info, tr("Информация"));
        }
    }

} // namespace v8::ui
