/*
 * Orion Player — High-Performance Media Player for Linux
 * Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
 * All Rights Reserved.
 *
 * This software and its associated documentation, website, and design assets
 * are the intellectual property of Abhinav Santhosh. Unauthorized copying,
 * rebranding, redistribution, or commercial use is strictly prohibited.
 * See LICENSE file for full terms and conditions.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QIcon>
#include <clocale>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    std::setlocale(LC_NUMERIC, "C"); // Strictly required by libmpv

    app.setApplicationName("OrionPlayer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OrionPlayer");
    app.setDesktopFileName("orionplayer");
    app.setWindowIcon(QIcon(":/icon.svg"));

    QCommandLineParser parser;
    parser.setApplicationDescription("OrionPlayer — Ultra-Fast Open-Source Hardware Accelerated Media Player");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("media", "Media file or stream URL to play directly on startup");

    parser.process(app);

    MainWindow window;
    window.show();
    app.processEvents();

    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        QString target = positionalArgs.first();
        window.openMedia(target);
    }

    return app.exec();
}
