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

#pragma once

#include <QDialog>

class OrionDonateDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionDonateDialog(QWidget *parent = nullptr);

private:
    void setupUi();
};
