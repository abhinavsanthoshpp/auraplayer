#include "AuraEngine.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("AuraPlayer");
    app.setApplicationVersion("1.0.0");

    qInfo() << "=== Initializing AuraPlayer Core Engine ===";
    AuraEngine engine;
    if (!engine.initialize()) {
        qCritical() << "Fatal: Failed to initialize AuraEngine.";
        return 1;
    }

    qInfo() << "=== AuraPlayer Core Engine initialized successfully with hardware acceleration ===";
    return 0;
}
