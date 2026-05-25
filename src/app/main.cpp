#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QTimer>

#include "v8reader/core/IV8Repository.h"
#include "v8reader/ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication::setOrganizationName("v8_reader_project");
    QApplication::setApplicationName("v8_reader");
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Просмотрщик конфигураций 1С");
    parser.addHelpOption();
    parser.addVersionOption();

    // ✅ Исправлено: 3 аргумента, без переносов строк внутри кавычек
    parser.addPositionalArgument(
        "file",
        "Путь к файлу конфигурации (.cf/.cfu/.1CD)",
        "[file]"
    );
    parser.process(app);

    // Создание ядра и главного окна
    auto repository = v8reader::core::createV8Repository();
    v8reader::ui::MainWindow window(std::move(repository));
    window.show();

    // Если файл передан через командную строку — открываем его после показа окна
    const auto positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        QTimer::singleShot(100, [&window, path = positionalArgs.first()]() {
            // TODO: Здесь будет вызов window.loadFile(path);
            Q_UNUSED(path);
            });
    }

    return QApplication::exec();
}