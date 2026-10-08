/*
 * Orion Player — High-Performance Open-Source Media Player for Linux
 * Copyright (C) 2026 Abhinav Santhosh <abhinavsanthoshpp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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
