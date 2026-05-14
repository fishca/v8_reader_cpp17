#include "v8reader/ui/MainWindow.h"
#include "v8reader/ui/MetadataTree.h"
#include "v8reader/ui/ContentPane.h"
#include "v8reader/core/IV8Repository.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QSplitter>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QtConcurrent>
#include <QFileInfo>
#include <QLabel>          // 🔑 КРИТИЧЕСКИ ВАЖНО: полное определение QLabel
#include <QFuture>         // Для Qt6

namespace v8::ui {

    MainWindow::MainWindow(std::unique_ptr<v8::core::IV8Repository> repo, QWidget* parent)
        : QMainWindow(parent), repository_(std::move(repo))
    {
        setWindowTitle(tr("v8_reader — Просмотр конфигурации 1С"));
        resize(1200, 700);
        setupMenu();
        setupCentralWidget();
        setupStatusBar();

        repository_->setLoadCallback([this](bool ok, const std::wstring& err) {
            onLoadComplete(ok, QString::fromStdWString(err));
            });
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

        auto* helpMenu = menuBar()->addMenu(tr("&Справка"));
        auto* aboutAction = new QAction(tr("О программе..."), this);
        connect(aboutAction, &QAction::triggered, [this]() {
            QMessageBox::about(this, tr("v8_reader"),
                tr("Просмотрщик конфигураций 1С\nC++17 + Qt6"));
            });
        helpMenu->addAction(aboutAction);
    }

    void MainWindow::setupCentralWidget() {
        splitter_ = new QSplitter(Qt::Horizontal, this);
        treeView_ = new MetadataTree(this);
        treeView_->setMinimumWidth(250);
        contentPane_ = new ContentPane(this);

        splitter_->addWidget(treeView_);
        splitter_->addWidget(contentPane_);
        splitter_->setStretchFactor(1, 1);
        setCentralWidget(splitter_);

        connect(treeView_, &MetadataTree::itemSelected, this, &MainWindow::onItemSelected);
    }

    void MainWindow::setupStatusBar() {
        statusLabel_ = new QLabel(tr("Готов к работе")); // ✅ Теперь работает
        statusBar()->addWidget(statusLabel_, 1);
    }

    void MainWindow::onOpenFile() {
        const auto path = QFileDialog::getOpenFileName(this, tr("Открыть конфигурацию"),
            QString(), tr("Файлы 1С (*.cf *.cfu *.1CD);;Все файлы (*.*)"));
        if (path.isEmpty()) return;

        statusLabel_->setText(tr("Загрузка: %1").arg(QFileInfo(path).fileName()));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // 🔑 Исправление предупреждения C4858 в Qt6
        auto future = QtConcurrent::run([this, p = path.toStdWString()]() {
            return repository_->loadFromFile(p);
            });
        // Явно игнорируем QFuture, чтобы анализатор не ругался
        Q_UNUSED(future);
    }

    void MainWindow::onItemSelected(const std::wstring& itemId, const std::wstring& itemType) {
        contentPane_->showContent(itemId, itemType);
    }

    void MainWindow::onLoadComplete(bool success, const QString& message) {
        QApplication::restoreOverrideCursor();
        if (success) {
            treeView_->populate(repository_->getRoot());
            statusLabel_->setText(tr("Загружено: %1 элементов")
                .arg(repository_->getRoot()->children.size()));
        }
        else {
            QMessageBox::critical(this, tr("Ошибка"), message);
        }
    }

} // namespace v8::ui