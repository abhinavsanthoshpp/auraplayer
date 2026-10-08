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
