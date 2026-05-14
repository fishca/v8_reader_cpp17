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
    parser.addPositionalArgument("file", "Путь к файлу конфигурации", "[file]");
    parser.process(app);
    
    auto repository = v8::core::createV8Repository();
    v8::ui::MainWindow window(std::move(repository));
    window.show();
    
    return QApplication::exec();
}