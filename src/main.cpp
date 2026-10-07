#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // Enable High DPI scaling & crisp font rendering
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName("AuraPlayer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AuraPlayer");
    app.setDesktopFileName("auraplayer.desktop");

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
