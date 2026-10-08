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

#include "OrionMediaInfoDialog.h"
#include <QHeaderView>
#include <QLabel>
#include <QFileInfo>

OrionMediaInfoDialog::OrionMediaInfoDialog(OrionEngine *engine, QWidget *parent)
    : QDialog(parent), m_engine(engine) {
    setWindowTitle("OrionPlayer — Media Information");
    resize(480, 420);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *headerLabel = new QLabel("📋 Stream & Codec Metadata", this);
    headerLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #58a6ff;");
    layout->addWidget(headerLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Property", "Value"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setStyleSheet(
        "QTableWidget { background: #12161f; border: 1px solid #30363d; border-radius: 6px; gridline-color: #21262d; }"
        "QTableWidget::item { padding: 6px; color: #c9d1d9; }"
        "QHeaderView::section { background: #161b22; color: #8b949e; font-weight: bold; border: 1px solid #21262d; padding: 4px; }"
    );
    layout->addWidget(m_table, 1);

    auto *closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    refresh();
}

void OrionMediaInfoDialog::addRow(int &row, const QString &property, const QString &value) {
    m_table->insertRow(row);

    auto *propItem = new QTableWidgetItem(property);
    propItem->setForeground(QColor(139, 148, 158));
    propItem->setFont(QFont("Inter", 10, QFont::Bold));

    auto *valItem = new QTableWidgetItem(value.isEmpty() ? "N/A" : value);
    valItem->setForeground(QColor(240, 246, 252));

    m_table->setItem(row, 0, propItem);
    m_table->setItem(row, 1, valItem);
    row++;
}

QString OrionMediaInfoDialog::formatBytes(int64_t bytes) {
    if (bytes <= 0) return "N/A";
    constexpr int64_t oneGB = 1024LL * 1024LL * 1024LL;
    constexpr int64_t oneMB = 1024LL * 1024LL;
    if (bytes >= oneGB) {
        return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
    if (bytes >= oneMB) {
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }
    return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
}

void OrionMediaInfoDialog::refresh() {
    m_table->setRowCount(0);
    if (!m_engine) return;

    MediaMetadata meta = m_engine->getMetadata();
    int row = 0;

    addRow(row, "Media Title", meta.title.isEmpty() ? QFileInfo(m_engine->currentFilePath()).fileName() : meta.title);
    addRow(row, "Source Path", m_engine->currentFilePath());
    addRow(row, "Container Format", meta.format.toUpper());
    addRow(row, "File Size", formatBytes(meta.fileSize));
    addRow(row, "Duration", QString("%1s (%2 mins)").arg(meta.duration, 0, 'f', 1).arg(meta.duration / 60.0, 0, 'f', 1));
    addRow(row, "Video Resolution", meta.videoWidth > 0 ? QString("%1 x %2").arg(meta.videoWidth).arg(meta.videoHeight) : "Audio Only");
    addRow(row, "Video Framerate", meta.videoFps > 0 ? QString("%1 FPS").arg(meta.videoFps, 0, 'f', 2) : "N/A");
    addRow(row, "Video Codec", meta.videoCodec.toUpper());
    addRow(row, "Video Bitrate", meta.videoBitrate > 0 ? QString("%1 kbps").arg(meta.videoBitrate / 1000) : "Variable / Auto");
    addRow(row, "Audio Codec", meta.audioCodec.toUpper());
    addRow(row, "Audio Channels", meta.audioChannels > 0 ? (meta.audioChannels == 2 ? "Stereo (2.0)" : (meta.audioChannels == 6 ? "5.1 Surround" : QString("%1 Channels").arg(meta.audioChannels))) : "N/A");
    addRow(row, "Audio Sample Rate", meta.audioSampleRate > 0 ? QString("%1 Hz (%2 kHz)").arg(meta.audioSampleRate).arg(meta.audioSampleRate / 1000.0, 0, 'f', 1) : "N/A");
    addRow(row, "Audio Bitrate", meta.audioBitrate > 0 ? QString("%1 kbps").arg(meta.audioBitrate / 1000) : "Auto");
    addRow(row, "Hardware Decoder", "Intel VA-API / NVDEC (Zero-Copy)");
}
