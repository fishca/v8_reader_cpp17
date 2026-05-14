#pragma once
#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
class QSplitter; class QAction; class QLabel;
QT_END_NAMESPACE

namespace v8::core { class IV8Repository; }
namespace v8::ui {
    class MetadataTree; class ContentPane;

    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(std::unique_ptr<v8::core::IV8Repository> repo, QWidget* parent = nullptr);
        ~MainWindow() override;

    private slots:
        void onOpenFile();
        void onItemSelected(const std::wstring& itemId, const std::wstring& itemType);
        void onLoadComplete(bool success, const QString& message);

    private:
        void setupMenu();
        void setupCentralWidget();
        void setupStatusBar();
        
        MetadataTree* treeView_{};
        ContentPane* contentPane_{};
        QSplitter* splitter_{};
        QLabel* statusLabel_{};
        std::unique_ptr<v8::core::IV8Repository> repository_;
    };
}