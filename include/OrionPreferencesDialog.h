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
