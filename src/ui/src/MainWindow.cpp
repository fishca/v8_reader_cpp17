#include "v8reader/ui/MainWindow.h"
#include "v8reader/ui/MetadataTree.h"
#include "v8reader/ui/ContentPane.h"
#include "v8reader/core/V8Container.h"
#include "v8reader/core/IV8Repository.h"

#include <QDebug> // 🔑 Добавляем для вывода в консоль Visual Studio
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
        const auto path = QFileDialog::getOpenFileName(this, tr("Открыть конфигурацию"), QString(), filters);

        if (path.isEmpty()) return;

        qDebug() << "📂 Opening file:" << path;

        statusLabel_->setText(tr("Загрузка: %1").arg(QFileInfo(path).fileName()));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // 1. Создаем контейнер
        auto container = std::make_unique<v8::core::V8Container>(path.toStdWString());

        // 2. Загружаем
        int result = container->load();

        // 🔍 ДИАГНОСТИКА: Выводим результаты в окно "Вывод" (Output) в VS
        qDebug() << "🔍 Load Result Code:" << result;
        qDebug() << "🔍 Last Error:" << QString::fromStdWString(container->getLastError());
        qDebug() << "🔍 Elements Found:" << container->getElements().size();

        QApplication::restoreOverrideCursor();

        if (result == v8::core::V8_OK) {
            // 3. Строим дерево
            auto tree = container->buildMetadataTree();
            qDebug() << "🌳 Tree Root Children:" << tree->children.size();

            if (tree->children.empty()) {
                QMessageBox::warning(this, tr("Внимание"),
                    tr("Файл загружен, но метаданные не найдены.\nПроверьте формат файла."));
            }
            else {
                // 🔑 ИСПРАВЛЕНИЕ АРХИТЕКТУРЫ:
                // Сохраняем контейнер в MainWindow, чтобы ContentPane мог потом читать модули
                container_ = std::move(container);

                // Передаем УКАЗАТЕЛЬ (не unique_ptr), чтобы дерево и контент использовали один экземпляр
                treeView_->populate(tree, container_.get());
                contentPane_->setContainer(container_.get());

                statusLabel_->setText(tr("Загружено: %1").arg(QFileInfo(path).fileName()));
                setWindowTitle(tr("V8 Reader - %1").arg(QFileInfo(path).fileName()));
            }
        }
        else {
            QMessageBox::critical(this, tr("Ошибка"),
                tr("Не удалось загрузить файл:\nКод: %1\n%2")
                .arg(result)
                .arg(QString::fromStdWString(container->getLastError())));
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
