#include "v8reader/ui/MainWindow.h"
#include "v8reader/ui/MetadataTree.h"
#include "v8reader/ui/ContentPane.h"
#include "v8reader/core/V8Container.h"
#include "v8reader/core/IV8Repository.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QSplitter>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QFileInfo>

namespace v8::ui {

    MainWindow::MainWindow(std::unique_ptr<v8::core::IV8Repository> repo, QWidget* parent)
        : QMainWindow(parent), repository_(std::move(repo))
    {
        setWindowTitle(tr("V8 Reader"));
        resize(1200, 700);
        setupMenu();
        setupCentralWidget();
        setupStatusBar();
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::setupMenu() {
        auto* fileMenu = menuBar()->addMenu(tr("&Файл"));

        auto* openAction = new QAction(tr("&Открыть..."), this);
        openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
        fileMenu->addAction(openAction);

        fileMenu->addSeparator();
        auto* exitAction = new QAction(tr("Выход"), this);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
        fileMenu->addAction(exitAction);
    }

    void MainWindow::setupCentralWidget() {
        splitter_ = new QSplitter(Qt::Horizontal, this);

        treeView_ = new MetadataTree(this);
        treeView_->setMinimumWidth(300);

        contentPane_ = new ContentPane(this);

        splitter_->addWidget(treeView_);
        splitter_->addWidget(contentPane_);
        splitter_->setStretchFactor(1, 1);

        setCentralWidget(splitter_);

        // ✅ Правильное подключение (лямбда-функция)
        connect(treeView_, &MetadataTree::itemSelected,
            this, [this](const std::wstring& id, const std::wstring& type) {
                contentPane_->showContent(id, type);
            });
    }

    void MainWindow::setupStatusBar() {
        statusLabel_ = new QLabel(tr("Готов к работе"));
        statusBar()->addWidget(statusLabel_, 1);
    }

    void MainWindow::onOpenFile() {
        const auto filters = tr("Файлы конфигурации 1С (*.cf *.cfu);;Все файлы (*.*)");
        const auto path = QFileDialog::getOpenFileName(
            this, tr("Открыть конфигурацию"), QString(), filters);

        if (path.isEmpty()) return;

        // 🔑 Сохраняем путь в член класса
        currentFilePath_ = path;

        statusLabel_->setText(tr("Загрузка: %1").arg(QFileInfo(path).fileName()));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Используем V8Container напрямую для загрузки
        auto container = std::make_unique<v8::core::V8Container>(path.toStdWString());
        int result = container->load();

        QApplication::restoreOverrideCursor();

        if (result == v8::core::V8_OK) {
            // Построение дерева метаданных
            auto tree = container->buildMetadataTree();
            treeView_->populate(tree, std::move(container));
            statusLabel_->setText(tr("Загружено: %1").arg(QFileInfo(path).fileName()));
        }
        else {
            QMessageBox::critical(this, tr("Ошибка"),
                tr("Не удалось загрузить файл:\n%1").arg(container->getLastError()));
            statusLabel_->setText(tr("Ошибка загрузки"));
        }
    }

    void MainWindow::onItemSelected(const std::wstring& itemId, const std::wstring& itemType) {
        // Показываем содержимое элемента
        contentPane_->showContent(itemId, itemType);
    }
    // 🔑 НОВАЯ МЕТОД: Обработка завершения загрузки
    void MainWindow::onLoadComplete(bool success, const QString& message)
    {
        QApplication::restoreOverrideCursor();

        if (success) {
            // 🔑 Используем currentFilePath_, который мы сохранили ранее
            QString fileName = QFileInfo(currentFilePath_).fileName();
            statusLabel_->setText(tr("Загружено: %1").arg(fileName));
            setWindowTitle(tr("V8 Reader - %1").arg(fileName));
        }
        else {
            QMessageBox::critical(this, tr("Ошибка"),
                tr("Не удалось загрузить файл:\n%1").arg(message));
            statusLabel_->setText(tr("Ошибка загрузки"));
        }
    }

} // namespace v8::ui
