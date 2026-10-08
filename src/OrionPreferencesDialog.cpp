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

#include "OrionPreferencesDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

OrionPreferencesDialog::OrionPreferencesDialog(OrionEngine *engine, QWidget *parent)
    : QDialog(parent), m_engine(engine) {
    setWindowTitle("Simple Preferences — OrionPlayer");
    resize(580, 420);
    setupUi();
}

void OrionPreferencesDialog::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    auto *mainSplit = new QHBoxLayout();

    // Left category icons list
    m_categoryList = new QListWidget(this);
    m_categoryList->setFixedWidth(140);
    m_categoryList->addItem("🖥️ Interface");
    m_categoryList->addItem("🎵 Audio");
    m_categoryList->addItem("🎬 Video");
    m_categoryList->addItem("💬 Subtitles / OSD");
    m_categoryList->addItem("⚙️ Input / Codecs");
    m_categoryList->setCurrentRow(0);
    mainSplit->addWidget(m_categoryList);

    // Right stacked pages
    m_pages = new QStackedWidget(this);
    m_pages->addWidget(createInterfacePage());
    m_pages->addWidget(createAudioPage());
    m_pages->addWidget(createVideoPage());
    m_pages->addWidget(createSubtitlesPage());
    m_pages->addWidget(createCodecsPage());
    mainSplit->addWidget(m_pages, 1);

    rootLayout->addLayout(mainSplit, 1);

    // Bottom action row
    auto *bottomRow = new QHBoxLayout();
    auto *resetBtn = new QPushButton("Reset Preferences", this);
    auto *cancelBtn = new QPushButton("Cancel", this);
    auto *saveBtn = new QPushButton("Save", this);
    saveBtn->setDefault(true);

    bottomRow->addWidget(resetBtn);
    bottomRow->addStretch(1);
    bottomRow->addWidget(cancelBtn);
    bottomRow->addWidget(saveBtn);
    rootLayout->addLayout(bottomRow);

    connect(m_categoryList, &QListWidget::currentRowChanged, this, &OrionPreferencesDialog::onCategoryChanged);
    connect(resetBtn, &QPushButton::clicked, this, &OrionPreferencesDialog::onResetDefaultsClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &OrionPreferencesDialog::onSaveClicked);
}

QWidget *OrionPreferencesDialog::createInterfacePage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);

    auto *box = new QGroupBox("Interface Settings", page);
    auto *form = new QFormLayout(box);

    m_themeCombo = new QComboBox(box);
    m_themeCombo->addItem("Professional Dark (Default)");
    m_themeCombo->addItem("Classic VLC Slate");
    m_themeCombo->addItem("High Contrast");
    form->addRow("Color Theme:", m_themeCombo);

    m_integrateMpris = new QCheckBox("Integrate with system media keys & lockscreen (MPRIS2)", box);
    m_integrateMpris->setChecked(true);
    form->addRow(m_integrateMpris);

    m_saveRecent = new QCheckBox("Save recently played media history", box);
    m_saveRecent->setChecked(true);
    form->addRow(m_saveRecent);

    layout->addWidget(box);
    layout->addStretch(1);
    return page;
}

QWidget *OrionPreferencesDialog::createAudioPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);

    auto *box = new QGroupBox("Audio Settings", page);
    auto *form = new QFormLayout(box);

    m_defaultVolume = new QSpinBox(box);
    m_defaultVolume->setRange(0, 200);
    m_defaultVolume->setValue(100);
    m_defaultVolume->setSuffix("%");
    form->addRow("Default Startup Volume:", m_defaultVolume);

    m_audioOutputCombo = new QComboBox(box);
    m_audioOutputCombo->addItem("Automatic (PipeWire / PulseAudio)");
    m_audioOutputCombo->addItem("ALSA direct");
    m_audioOutputCombo->addItem("JACK Audio");
    form->addRow("Output Module:", m_audioOutputCombo);

    layout->addWidget(box);
    layout->addStretch(1);
    return page;
}

QWidget *OrionPreferencesDialog::createVideoPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);

    auto *box = new QGroupBox("Video Display & Output", page);
    auto *form = new QFormLayout(box);

    m_aspectDefaultCombo = new QComboBox(box);
    m_aspectDefaultCombo->addItem("Default (Source Aspect)");
    m_aspectDefaultCombo->addItem("16:9");
    m_aspectDefaultCombo->addItem("4:3");
    m_aspectDefaultCombo->addItem("2.35:1");
    form->addRow("Default Aspect Ratio:", m_aspectDefaultCombo);

    m_deinterlaceCombo = new QComboBox(box);
    m_deinterlaceCombo->addItem("Auto");
    m_deinterlaceCombo->addItem("On");
    m_deinterlaceCombo->addItem("Off");
    form->addRow("Deinterlacing Mode:", m_deinterlaceCombo);

    layout->addWidget(box);
    layout->addStretch(1);
    return page;
}

QWidget *OrionPreferencesDialog::createSubtitlesPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);

    auto *box = new QGroupBox("Subtitles / On Screen Display", page);
    auto *form = new QFormLayout(box);

    m_subSizeCombo = new QComboBox(box);
    m_subSizeCombo->addItem("Normal (55pt)");
    m_subSizeCombo->addItem("Large (65pt)");
    m_subSizeCombo->addItem("Small (45pt)");
    form->addRow("Subtitle Font Size:", m_subSizeCombo);

    m_subEncodingCombo = new QComboBox(box);
    m_subEncodingCombo->addItem("Default (UTF-8)");
    m_subEncodingCombo->addItem("Universal (Windows-1252)");
    m_subEncodingCombo->addItem("ISO-8859-1");
    form->addRow("Default Subtitle Encoding:", m_subEncodingCombo);

    layout->addWidget(box);
    layout->addStretch(1);
    return page;
}

QWidget *OrionPreferencesDialog::createCodecsPage() {
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);

    auto *box = new QGroupBox("Hardware-accelerated Decoding", page);
    auto *form = new QFormLayout(box);

    m_hwdecCombo = new QComboBox(box);
    m_hwdecCombo->addItem("Automatic Safe (VA-API / NVDEC)");
    m_hwdecCombo->addItem("Intel VA-API (Zero-Copy)");
    m_hwdecCombo->addItem("NVIDIA NVDEC");
    m_hwdecCombo->addItem("Disable (Software Decoding)");
    form->addRow("Hardware Acceleration:", m_hwdecCombo);

    layout->addWidget(box);
    layout->addStretch(1);
    return page;
}

void OrionPreferencesDialog::onCategoryChanged(int index) {
    if (m_pages && index >= 0 && index < m_pages->count()) {
        m_pages->setCurrentIndex(index);
    }
}

void OrionPreferencesDialog::onSaveClicked() {
    accept();
}

void OrionPreferencesDialog::onResetDefaultsClicked() {
    m_defaultVolume->setValue(100);
    m_hwdecCombo->setCurrentIndex(0);
    m_deinterlaceCombo->setCurrentIndex(0);
}
