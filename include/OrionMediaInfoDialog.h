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
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "OrionEngine.h"

class OrionMediaInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionMediaInfoDialog(OrionEngine *engine, QWidget *parent = nullptr);

    void refresh();

private:
    void addRow(int &row, const QString &property, const QString &value);
    static QString formatBytes(int64_t bytes);

    OrionEngine *m_engine = nullptr;
    QTableWidget *m_table = nullptr;
};
