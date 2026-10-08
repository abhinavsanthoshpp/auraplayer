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

#include "OrionStreamDialog.h"

OrionStreamDialog::OrionStreamDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Open Network Stream — OrionPlayer");
    resize(480, 160);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *descLabel = new QLabel(
        "Enter a network URL or live stream address (HTTP, HTTPS, RTSP, RTMP, HLS, or YouTube):", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #8b949e;");
    layout->addWidget(descLabel);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://... or rtsp://...");
    m_urlEdit->setStyleSheet(
        "QLineEdit { background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 8px; color: #ffffff; }"
        "QLineEdit:focus { border: 1px solid #58a6ff; }"
    );
    layout->addWidget(m_urlEdit);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_playBtn = new QPushButton("▶ Play Stream", this);
    m_playBtn->setStyleSheet("background: #1f6feb; color: white; font-weight: bold; padding: 6px 14px;");

    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_playBtn);
    layout->addLayout(btnRow);

    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_playBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &QDialog::accept);
}

QString OrionStreamDialog::streamUrl() const {
    return m_urlEdit->text().trimmed();
}
