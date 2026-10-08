/*
 * Orion Player — High-Performance Media Player for Linux & Windows
 * Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
 * All Rights Reserved.
 *
 * This software and its associated documentation, website, and design assets
 * are the intellectual property of Abhinav Santhosh. Unauthorized copying,
 * rebranding, redistribution, or commercial use is strictly prohibited.
 * See LICENSE file for full terms and conditions.
 */

#include "OrionDonateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>
#include <QTimer>

OrionDonateDialog::OrionDonateDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Support & Donate — Orion Player");
    setFixedSize(520, 680);
    setStyleSheet("background-color: #0d1117; color: #f0f6fc;");
    setupUi();
}

void OrionDonateDialog::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(16);

    // Header
    auto *title = new QLabel("Support Orion Player Development", this);
    title->setStyleSheet("font-size: 19px; font-weight: bold; color: #ffffff;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto *desc = new QLabel(
        "Orion Player is 100% free, source-available, and ad-free software created by "
        "<b>Abhinav Santhosh</b>. Your direct support keeps development active, "
        "powers hardware acceleration research, and keeps it independent!",
        this
    );
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #8b949e; font-size: 12px; line-height: 1.4;");
    desc->setAlignment(Qt::AlignCenter);
    layout->addWidget(desc);

    // 1. UPI Payment Section (0% Fee Direct Bank Transfer)
    auto *upiFrame = new QFrame(this);
    upiFrame->setStyleSheet(
        "QFrame { background-color: #161b22; border: 1px solid #30363d; border-radius: 10px; padding: 12px; }"
    );
    auto *upiLayout = new QVBoxLayout(upiFrame);
    upiLayout->setSpacing(10);

    auto *upiHeader = new QHBoxLayout();
    auto *upiTitle = new QLabel("UPI (Direct Bank Transfer • 0% Fee)", upiFrame);
    upiTitle->setStyleSheet("font-weight: bold; color: #46d369; font-size: 13px;");
    auto *upiBadge = new QLabel("INSTANT", upiFrame);
    upiBadge->setStyleSheet(
        "background: rgba(70, 211, 105, 0.15); color: #46d369; "
        "border: 1px solid rgba(70, 211, 105, 0.4); border-radius: 4px; padding: 2px 6px; font-size: 10px; font-weight: bold;"
    );
    upiHeader->addWidget(upiTitle);
    upiHeader->addStretch(1);
    upiHeader->addWidget(upiBadge);
    upiLayout->addLayout(upiHeader);

    // UPI ID row with copy button
    auto *upiRow = new QHBoxLayout();
    auto *upiIdLabel = new QLabel("abhinava6525@naviaxis", upiFrame);
    upiIdLabel->setStyleSheet(
        "font-family: monospace; font-size: 14px; font-weight: bold; color: #58a6ff; "
        "background: #0d1117; padding: 6px 12px; border-radius: 6px; border: 1px solid #21262d;"
    );

    auto *copyBtn = new QPushButton("Copy", upiFrame);
    copyBtn->setStyleSheet(
        "background-color: #21262d; color: #c9d1d9; border: 1px solid #30363d; "
        "border-radius: 6px; padding: 6px 14px; font-weight: bold; font-size: 12px;"
    );
    copyBtn->setCursor(Qt::PointingHandCursor);

    connect(copyBtn, &QPushButton::clicked, this, [copyBtn]() {
        QApplication::clipboard()->setText("abhinava6525@naviaxis");
        copyBtn->setText("Copied!");
        copyBtn->setStyleSheet(
            "background-color: #238636; color: white; border: 1px solid #2ea043; "
            "border-radius: 6px; padding: 6px 14px; font-weight: bold; font-size: 12px;"
        );
        QTimer::singleShot(2500, [copyBtn]() {
            copyBtn->setText("Copy");
            copyBtn->setStyleSheet(
                "background-color: #21262d; color: #c9d1d9; border: 1px solid #30363d; "
                "border-radius: 6px; padding: 6px 14px; font-weight: bold; font-size: 12px;"
            );
        });
    });

    upiRow->addWidget(upiIdLabel, 1);
    upiRow->addWidget(copyBtn);
    upiLayout->addLayout(upiRow);

    // Live QR Code Image
    auto *qrLabel = new QLabel(upiFrame);
    QPixmap qrPix(":/icons/upi_qr.png");
    if (!qrPix.isNull()) {
        qrLabel->setPixmap(qrPix.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    qrLabel->setAlignment(Qt::AlignCenter);
    qrLabel->setStyleSheet("padding: 4px; background: white; border-radius: 8px; margin: 0 100px;");
    upiLayout->addWidget(qrLabel);

    auto *upiApps = new QLabel(
        "Scan with Google Pay, PhonePe, Paytm, Navi, Cred, BHIM or any banking app", upiFrame
    );
    upiApps->setStyleSheet("color: #8b949e; font-size: 11px;");
    upiApps->setAlignment(Qt::AlignCenter);
    upiLayout->addWidget(upiApps);

    layout->addWidget(upiFrame);

    // 2. Buy Me a Coffee Button (International Cards / PayPal)
    auto *bmacBtn = new QPushButton("Support via Buy Me a Coffee (Cards & International)", this);
    bmacBtn->setCursor(Qt::PointingHandCursor);
    bmacBtn->setStyleSheet(
        "QPushButton { background-color: #ffdd00; color: #000000; font-weight: bold; font-size: 14px; "
        "border-radius: 8px; padding: 12px; border: none; } "
        "QPushButton:hover { background-color: #ffe333; }"
    );
    connect(bmacBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://buymeacoffee.com/abhinavsanthoshpp"));
    });
    layout->addWidget(bmacBtn);

    // 3. GitHub Sponsors Button
    auto *ghBtn = new QPushButton("Sponsor on GitHub", this);
    ghBtn->setCursor(Qt::PointingHandCursor);
    ghBtn->setStyleSheet(
        "QPushButton { background-color: #21262d; color: #ea4aaa; font-weight: bold; font-size: 13px; "
        "border-radius: 8px; padding: 10px; border: 1px solid rgba(234, 74, 170, 0.4); } "
        "QPushButton:hover { background-color: rgba(234, 74, 170, 0.15); border-color: #ea4aaa; }"
    );
    connect(ghBtn, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/sponsors/abhinavsanthoshpp"));
    });
    layout->addWidget(ghBtn);

    // Footer button
    auto *closeBtn = new QPushButton("Close", this);
    closeBtn->setStyleSheet(
        "background-color: transparent; color: #8b949e; border: 1px solid #30363d; "
        "border-radius: 6px; padding: 8px; font-size: 12px;"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn);
}
