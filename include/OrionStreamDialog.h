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

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class OrionStreamDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionStreamDialog(QWidget *parent = nullptr);

    QString streamUrl() const;

private:
    QLineEdit *m_urlEdit = nullptr;
    QPushButton *m_playBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
};
