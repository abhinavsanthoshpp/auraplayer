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
