#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <clocale>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    std::setlocale(LC_NUMERIC, "C"); // Strictly required by libmpv

    app.setApplicationName("AuraPlayer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AuraPlayer");
    app.setDesktopFileName("auraplayer");

    QCommandLineParser parser;
    parser.setApplicationDescription("AuraPlayer — Ultra-Fast Open-Source Hardware Accelerated Media Player");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("media", "Media file or stream URL to play directly on startup");

    parser.process(app);

    MainWindow window;
    window.show();

    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        QString target = positionalArgs.first();
        window.openMedia(target);
    }

    return app.exec();
}
