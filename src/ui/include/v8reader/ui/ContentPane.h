#pragma once
#include <QTabWidget>
#include <QLabel>
#include <memory>

namespace v8reader::core {
    class V8Container;
}

namespace v8reader::ui {

    class ContentPane : public QTabWidget {
        Q_OBJECT

    public:
        explicit ContentPane(QWidget* parent = nullptr);
        void setContainer(v8reader::core::V8Container* container);
        void showContent(const std::wstring& itemId, const std::wstring& itemType);

    private:
        v8reader::core::V8Container* container_;
    };

} // namespace v8reader::ui