#pragma once
#include <QTabWidget>
#include <QLabel>
#include <memory>

namespace v8::core {
    class V8Container;
}

namespace v8::ui {

    class ContentPane : public QTabWidget {
        Q_OBJECT

    public:
        explicit ContentPane(QWidget* parent = nullptr);
        void setContainer(v8::core::V8Container* container);
        void showContent(const std::wstring& itemId, const std::wstring& itemType);

    private:
        v8::core::V8Container* container_;
    };

} // namespace v8::ui