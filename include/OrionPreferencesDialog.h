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
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include "OrionEngine.h"

class OrionPreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionPreferencesDialog(OrionEngine *engine, QWidget *parent = nullptr);

private slots:
    void onCategoryChanged(int index);
    void onSaveClicked();
    void onResetDefaultsClicked();

private:
    void setupUi();
    QWidget *createInterfacePage();
    QWidget *createAudioPage();
    QWidget *createVideoPage();
    QWidget *createSubtitlesPage();
    QWidget *createCodecsPage();

    OrionEngine *m_engine = nullptr;

    QListWidget *m_categoryList = nullptr;
    QStackedWidget *m_pages = nullptr;

    // Settings elements
    QComboBox *m_themeCombo = nullptr;
    QCheckBox *m_integrateMpris = nullptr;
    QCheckBox *m_saveRecent = nullptr;

    QSpinBox *m_defaultVolume = nullptr;
    QComboBox *m_audioOutputCombo = nullptr;

    QComboBox *m_hwdecCombo = nullptr;
    QComboBox *m_deinterlaceCombo = nullptr;
    QComboBox *m_aspectDefaultCombo = nullptr;

    QComboBox *m_subSizeCombo = nullptr;
    QComboBox *m_subEncodingCombo = nullptr;
};
