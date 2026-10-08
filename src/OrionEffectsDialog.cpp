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

#include "OrionEffectsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

OrionEffectsDialog::OrionEffectsDialog(OrionEngine *engine, QWidget *parent)
    : QDialog(parent), m_engine(engine) {
    setWindowTitle("Adjustments and Effects — OrionPlayer");
    resize(560, 440);
    setupUi();
}

void OrionEffectsDialog::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    auto *tabs = new QTabWidget(this);
    tabs->addTab(createAudioTab(), "Audio Effects");
    tabs->addTab(createVideoTab(), "Video Effects");
    tabs->addTab(createSyncTab(), "Synchronization");
    rootLayout->addWidget(tabs, 1);

    auto *bottomRow = new QHBoxLayout();
    bottomRow->addStretch(1);
    auto *closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomRow->addWidget(closeBtn);
    rootLayout->addLayout(bottomRow);
}

QWidget *OrionEffectsDialog::createAudioTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Graphic Equalizer Sub-box
    auto *eqBox = new QGroupBox("Graphic Equalizer", tab);
    auto *eqLayout = new QVBoxLayout(eqBox);

    auto *topRow = new QHBoxLayout();
    m_eqEnable = new QCheckBox("Enable", eqBox);
    m_twoPass = new QCheckBox("2 Pass", eqBox);

    auto *presetLabel = new QLabel("Preset:", eqBox);
    m_presetCombo = new QComboBox(eqBox);
    m_presetCombo->addItem("Flat");
    m_presetCombo->addItem("Classical");
    m_presetCombo->addItem("Club");
    m_presetCombo->addItem("Dance");
    m_presetCombo->addItem("Full Bass");
    m_presetCombo->addItem("Full Bass and Treble");
    m_presetCombo->addItem("Full Treble");
    m_presetCombo->addItem("Headphones");
    m_presetCombo->addItem("Large Hall");
    m_presetCombo->addItem("Live");
    m_presetCombo->addItem("Party");
    m_presetCombo->addItem("Pop");
    m_presetCombo->addItem("Reggae");
    m_presetCombo->addItem("Rock");
    m_presetCombo->addItem("Ska");
    m_presetCombo->addItem("Soft");
    m_presetCombo->addItem("Soft Rock");
    m_presetCombo->addItem("Techno");

    topRow->addWidget(m_eqEnable);
    topRow->addWidget(m_twoPass);
    topRow->addSpacing(16);
    topRow->addWidget(presetLabel);
    topRow->addWidget(m_presetCombo, 1);
    eqLayout->addLayout(topRow);

    // Preamp + 10 Bands
    auto *slidersRow = new QHBoxLayout();

    // Preamp
    auto *preampCol = new QVBoxLayout();
    m_preampLabel = new QLabel("0.0 dB", eqBox);
    m_preampLabel->setAlignment(Qt::AlignCenter);
    m_preampLabel->setStyleSheet("font-size: 10px; font-weight: bold; color: #58a6ff;");
    m_preampSlider = new QSlider(Qt::Vertical, eqBox);
    m_preampSlider->setRange(-20, 20);
    m_preampSlider->setValue(0);
    auto *preampTitle = new QLabel("Preamp", eqBox);
    preampTitle->setAlignment(Qt::AlignCenter);
    preampTitle->setStyleSheet("font-size: 10px; color: #8b949e;");

    preampCol->addWidget(m_preampLabel);
    preampCol->addWidget(m_preampSlider, 1, Qt::AlignHCenter);
    preampCol->addWidget(preampTitle);
    slidersRow->addLayout(preampCol);
    slidersRow->addSpacing(10);

    // 10 Bands (VLC frequencies: 60, 170, 310, 600, 1K, 3K, 6K, 12K, 14K, 16K)
    static const QString bandNames[] = {"60Hz", "170Hz", "310Hz", "600Hz", "1kHz", "3kHz", "6kHz", "12kHz", "14kHz", "16kHz"};
    m_bandSliders.resize(10);
    m_bandLabels.resize(10);

    for (int i = 0; i < 10; ++i) {
        auto *col = new QVBoxLayout();
        auto *valLbl = new QLabel("0 dB", eqBox);
        valLbl->setAlignment(Qt::AlignCenter);
        valLbl->setStyleSheet("font-size: 10px; color: #58a6ff;");
        m_bandLabels[i] = valLbl;

        auto *slider = new QSlider(Qt::Vertical, eqBox);
        slider->setRange(-20, 20);
        slider->setValue(0);
        m_bandSliders[i] = slider;

        auto *nameLbl = new QLabel(bandNames[i], eqBox);
        nameLbl->setAlignment(Qt::AlignCenter);
        nameLbl->setStyleSheet("font-size: 10px; color: #8b949e;");

        col->addWidget(valLbl);
        col->addWidget(slider, 1, Qt::AlignHCenter);
        col->addWidget(nameLbl);
        slidersRow->addLayout(col);

        connect(slider, &QSlider::valueChanged, this, [this, i](int val) {
            onBandChanged(i, val);
        });
    }

    eqLayout->addLayout(slidersRow);
    layout->addWidget(eqBox, 1);

    connect(m_eqEnable, &QCheckBox::toggled, this, &OrionEffectsDialog::onEqToggled);
    connect(m_preampSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onPreampChanged);
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrionEffectsDialog::onPresetSelected);

    return tab;
}

QWidget *OrionEffectsDialog::createVideoTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *adjustBox = new QGroupBox("Image Adjust", tab);
    auto *boxLayout = new QVBoxLayout(adjustBox);

    m_videoEnable = new QCheckBox("Enable Image Adjust", adjustBox);
    boxLayout->addWidget(m_videoEnable);

    auto *grid = new QGridLayout();
    grid->setSpacing(10);

    auto addRow = [&](int row, const QString &label, QSlider *&slider, QLabel *&valLbl, int min, int max) {
        grid->addWidget(new QLabel(label, adjustBox), row, 0);
        slider = new QSlider(Qt::Horizontal, adjustBox);
        slider->setRange(min, max);
        slider->setValue(0);
        valLbl = new QLabel("0", adjustBox);
        valLbl->setFixedWidth(35);
        grid->addWidget(slider, row, 1);
        grid->addWidget(valLbl, row, 2);
    };

    addRow(0, "Brightness:", m_brightSlider, m_brightVal, -100, 100);
    addRow(1, "Contrast:", m_contrastSlider, m_contrastVal, -100, 100);
    addRow(2, "Saturation:", m_satSlider, m_satVal, -100, 100);
    addRow(3, "Gamma:", m_gammaSlider, m_gammaVal, -100, 100);
    addRow(4, "Hue:", m_hueSlider, m_hueVal, -100, 100);

    boxLayout->addLayout(grid);

    auto *resetBtn = new QPushButton("Reset Image Adjust", adjustBox);
    boxLayout->addWidget(resetBtn, 0, Qt::AlignRight);
    layout->addWidget(adjustBox);
    layout->addStretch(1);

    connect(m_videoEnable, &QCheckBox::toggled, this, &OrionEffectsDialog::onVideoAdjustToggled);
    connect(m_brightSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onContrastChanged);
    connect(m_satSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onSaturationChanged);
    connect(m_gammaSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onGammaChanged);
    connect(m_hueSlider, &QSlider::valueChanged, this, &OrionEffectsDialog::onHueChanged);
    connect(resetBtn, &QPushButton::clicked, this, &OrionEffectsDialog::onResetVideoClicked);

    return tab;
}

QWidget *OrionEffectsDialog::createSyncTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(14);

    // Audio / Video synchronization box
    auto *audioSyncBox = new QGroupBox("Audio / Video Synchronization", tab);
    auto *audioSyncLayout = new QHBoxLayout(audioSyncBox);
    audioSyncLayout->addWidget(new QLabel("Audio track synchronization:", audioSyncBox));

    m_audioSyncSpin = new QDoubleSpinBox(audioSyncBox);
    m_audioSyncSpin->setRange(-60.0, 60.0);
    m_audioSyncSpin->setSingleStep(0.05);
    m_audioSyncSpin->setDecimals(3);
    m_audioSyncSpin->setSuffix(" s");
    audioSyncLayout->addWidget(m_audioSyncSpin);
    layout->addWidget(audioSyncBox);

    // Subtitles / Video synchronization box
    auto *subSyncBox = new QGroupBox("Subtitles / Video Synchronization", tab);
    auto *subSyncLayout = new QHBoxLayout(subSyncBox);
    subSyncLayout->addWidget(new QLabel("Subtitle track synchronization:", subSyncBox));

    m_subSyncSpin = new QDoubleSpinBox(subSyncBox);
    m_subSyncSpin->setRange(-60.0, 60.0);
    m_subSyncSpin->setSingleStep(0.05);
    m_subSyncSpin->setDecimals(3);
    m_subSyncSpin->setSuffix(" s");
    subSyncLayout->addWidget(m_subSyncSpin);
    layout->addWidget(subSyncBox);

    layout->addStretch(1);

    connect(m_audioSyncSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrionEffectsDialog::onAudioSyncChanged);
    connect(m_subSyncSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrionEffectsDialog::onSubSyncChanged);

    return tab;
}

void OrionEffectsDialog::onEqToggled(bool checked) {
    if (!checked) {
        if (m_engine) {
            m_engine->setEqualizerBands(QVector<double>(10, 0.0), 0.0);
        }
    } else {
        applyEqualizer();
    }
}

void OrionEffectsDialog::onPreampChanged(int val) {
    m_preampLabel->setText(QString("%1%2 dB").arg(val > 0 ? "+" : "").arg(val));
    applyEqualizer();
}

void OrionEffectsDialog::onBandChanged(int index, int val) {
    if (index >= 0 && index < m_bandLabels.size()) {
        m_bandLabels[index]->setText(QString("%1%2 dB").arg(val > 0 ? "+" : "").arg(val));
    }
    applyEqualizer();
}

void OrionEffectsDialog::applyEqualizer() {
    if (!m_eqEnable->isChecked() || !m_engine) return;

    QVector<double> gains(10);
    for (int i = 0; i < 10; ++i) {
        gains[i] = m_bandSliders[i]->value();
    }
    double preamp = m_preampSlider->value();
    m_engine->setEqualizerBands(gains, preamp);
}

void OrionEffectsDialog::onPresetSelected(int index) {
    m_eqEnable->setChecked(true);

    static const QVector<QVector<double>> presets = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                         // Flat
        {4.8, 4.8, 3.8, 0, -2.4, -2.4, 0, 2.4, 3.8, 4.8},       // Classical
        {0, 0, 2.0, 4.0, 4.0, 4.0, 2.0, 0, 0, 0},               // Club
        {8.0, 6.0, 2.0, 0, 0, -4.0, -6.0, -6.0, 0, 0},          // Dance
        {8.0, 8.0, 8.0, 4.0, 1.0, -3.0, -6.0, -7.0, -7.0, -7.0},// Full Bass
        {6.0, 4.5, 0, -5.0, -4.0, 1.0, 6.5, 9.0, 10.0, 10.5},   // Full Bass & Treble
        {-8.0, -8.0, -8.0, -3.0, 2.0, 7.0, 12.0, 14.0, 14.0, 14.5},// Full Treble
        {4.0, 9.0, 4.5, -2.5, -1.5, 1.0, 4.0, 8.0, 10.0, 11.5}, // Headphones
        {9.0, 9.0, 5.0, 5.0, 0, -4.0, -4.0, -4.0, 0, 0},        // Large Hall
        {-4.0, 0, 3.0, 4.0, 4.5, 4.5, 3.0, 2.0, 2.0, 2.0},      // Live
        {6.0, 6.0, 0, 0, 0, 0, 0, 0, 6.0, 6.0},                  // Party
        {-1.5, 1.0, 5.0, 4.5, 3.5, -1.0, -2.0, -2.0, -1.5, -1.5}, // Pop
        {0, 0, -0.5, -4.0, 0, 5.0, 5.0, 0, 0, 0},               // Reggae
        {6.5, 4.0, -4.5, -6.5, -2.5, 3.0, 6.5, 8.0, 8.5, 9.0},  // Rock
        {-2.0, -3.5, -3.0, -0.5, 3.0, 4.5, 7.0, 7.5, 8.0, 7.5}, // Ska
        {4.0, 1.5, 0, -1.5, 0, 3.5, 7.0, 8.5, 9.5, 10.5},       // Soft
        {3.5, 3.5, 2.0, 0, -2.0, -4.0, -2.5, 0, 2.0, 7.5},      // Soft Rock
        {7.0, 5.5, 0, -4.5, -4.0, 0, 7.0, 8.0, 8.0, 7.5}        // Techno
    };

    if (index >= 0 && index < presets.size()) {
        const auto &p = presets[index];
        for (int i = 0; i < 10; ++i) {
            m_bandSliders[i]->blockSignals(true);
            m_bandSliders[i]->setValue(static_cast<int>(p[i]));
            m_bandLabels[i]->setText(QString("%1%2 dB").arg(p[i] > 0 ? "+" : "").arg(static_cast<int>(p[i])));
            m_bandSliders[i]->blockSignals(false);
        }
        applyEqualizer();
    }
}

void OrionEffectsDialog::onVideoAdjustToggled(bool checked) {
    if (!checked) {
        if (m_engine) {
            m_engine->setBrightness(0);
            m_engine->setContrast(0);
            m_engine->setSaturation(0);
            m_engine->setGamma(0);
            m_engine->setHue(0);
        }
    } else {
        if (m_engine) {
            m_engine->setBrightness(m_brightSlider->value());
            m_engine->setContrast(m_contrastSlider->value());
            m_engine->setSaturation(m_satSlider->value());
            m_engine->setGamma(m_gammaSlider->value());
            m_engine->setHue(m_hueSlider->value());
        }
    }
}

void OrionEffectsDialog::onBrightnessChanged(int val) {
    m_brightVal->setText(QString::number(val));
    if (m_videoEnable->isChecked() && m_engine) m_engine->setBrightness(val);
}

void OrionEffectsDialog::onContrastChanged(int val) {
    m_contrastVal->setText(QString::number(val));
    if (m_videoEnable->isChecked() && m_engine) m_engine->setContrast(val);
}

void OrionEffectsDialog::onSaturationChanged(int val) {
    m_satVal->setText(QString::number(val));
    if (m_videoEnable->isChecked() && m_engine) m_engine->setSaturation(val);
}

void OrionEffectsDialog::onGammaChanged(int val) {
    m_gammaVal->setText(QString::number(val));
    if (m_videoEnable->isChecked() && m_engine) m_engine->setGamma(val);
}

void OrionEffectsDialog::onHueChanged(int val) {
    m_hueVal->setText(QString::number(val));
    if (m_videoEnable->isChecked() && m_engine) m_engine->setHue(val);
}

void OrionEffectsDialog::onResetVideoClicked() {
    m_brightSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_satSlider->setValue(0);
    m_gammaSlider->setValue(0);
    m_hueSlider->setValue(0);
}

void OrionEffectsDialog::onAudioSyncChanged(double val) {
    if (m_engine) m_engine->setAudioDelay(val);
}

void OrionEffectsDialog::onSubSyncChanged(double val) {
    if (m_engine) m_engine->setSubtitleDelay(val);
}

void OrionEffectsDialog::refreshFromEngine() {
    if (!m_engine) return;
    m_audioSyncSpin->blockSignals(true);
    m_audioSyncSpin->setValue(m_engine->audioDelay());
    m_audioSyncSpin->blockSignals(false);

    m_subSyncSpin->blockSignals(true);
    m_subSyncSpin->setValue(m_engine->subtitleDelay());
    m_subSyncSpin->blockSignals(false);
}
