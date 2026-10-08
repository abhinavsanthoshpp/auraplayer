#include "OrionEqualizerDialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

OrionEqualizerDialog::OrionEqualizerDialog(OrionEngine *engine, QWidget *parent)
    : QDialog(parent), m_engine(engine) {
    setWindowTitle("OrionPlayer — Audio & Video Adjustments");
    resize(520, 360);
    setupUi();
}

void OrionEqualizerDialog::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);

    // ================= TAB 1: AUDIO EQUALIZER =================
    auto *audioTab = new QWidget();
    auto *audioLayout = new QVBoxLayout(audioTab);

    auto *topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Preset:", audioTab));

    m_presetCombo = new QComboBox(audioTab);
    m_presetCombo->addItem("Flat");
    m_presetCombo->addItem("Rock");
    m_presetCombo->addItem("Pop");
    m_presetCombo->addItem("Jazz");
    m_presetCombo->addItem("Bass Boost");
    m_presetCombo->addItem("Vocal");
    m_presetCombo->addItem("Electronic");
    topRow->addWidget(m_presetCombo);

    topRow->addStretch(1);

    m_resetAudioBtn = new QPushButton("Reset EQ", audioTab);
    topRow->addWidget(m_resetAudioBtn);
    audioLayout->addLayout(topRow);

    // 10 Bands
    auto *bandsBox = new QGroupBox("10-Band Graphic Equalizer", audioTab);
    auto *bandsLayout = new QHBoxLayout(bandsBox);

    static const QString bandNames[] = {
        "31Hz", "62Hz", "125Hz", "250Hz", "500Hz",
        "1kHz", "2kHz", "4kHz", "8kHz", "16kHz"
    };

    m_bandSliders.resize(10);
    m_bandLabels.resize(10);

    for (int i = 0; i < 10; ++i) {
        auto *col = new QVBoxLayout();
        auto *valLabel = new QLabel("0dB", bandsBox);
        valLabel->setAlignment(Qt::AlignCenter);
        valLabel->setStyleSheet("font-size: 10px; color: #58a6ff;");
        m_bandLabels[i] = valLabel;

        auto *slider = new QSlider(Qt::Vertical, bandsBox);
        slider->setRange(-12, 12);
        slider->setValue(0);
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setTickInterval(6);
        m_bandSliders[i] = slider;

        auto *hzLabel = new QLabel(bandNames[i], bandsBox);
        hzLabel->setAlignment(Qt::AlignCenter);
        hzLabel->setStyleSheet("font-size: 10px; color: #8b949e;");

        col->addWidget(valLabel);
        col->addWidget(slider, 1, Qt::AlignHCenter);
        col->addWidget(hzLabel);

        bandsLayout->addLayout(col);

        connect(slider, &QSlider::valueChanged, this, [this, i](int val) {
            onBandChanged(i, val);
        });
    }

    audioLayout->addWidget(bandsBox, 1);
    tabs->addTab(audioTab, "🎵 Audio Equalizer");

    // ================= TAB 2: VIDEO ADJUSTMENTS =================
    auto *videoTab = new QWidget();
    auto *videoLayout = new QVBoxLayout(videoTab);

    auto *grid = new QGridLayout();
    grid->setSpacing(12);

    // Brightness
    grid->addWidget(new QLabel("Brightness:", videoTab), 0, 0);
    m_brightSlider = new QSlider(Qt::Horizontal, videoTab);
    m_brightSlider->setRange(-100, 100);
    m_brightSlider->setValue(0);
    m_brightVal = new QLabel("0", videoTab);
    m_brightVal->setFixedWidth(35);
    grid->addWidget(m_brightSlider, 0, 1);
    grid->addWidget(m_brightVal, 0, 2);

    // Contrast
    grid->addWidget(new QLabel("Contrast:", videoTab), 1, 0);
    m_contrastSlider = new QSlider(Qt::Horizontal, videoTab);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSlider->setValue(0);
    m_contrastVal = new QLabel("0", videoTab);
    m_contrastVal->setFixedWidth(35);
    grid->addWidget(m_contrastSlider, 1, 1);
    grid->addWidget(m_contrastVal, 1, 2);

    // Saturation
    grid->addWidget(new QLabel("Saturation:", videoTab), 2, 0);
    m_satSlider = new QSlider(Qt::Horizontal, videoTab);
    m_satSlider->setRange(-100, 100);
    m_satSlider->setValue(0);
    m_satVal = new QLabel("0", videoTab);
    m_satVal->setFixedWidth(35);
    grid->addWidget(m_satSlider, 2, 1);
    grid->addWidget(m_satVal, 2, 2);

    // Gamma
    grid->addWidget(new QLabel("Gamma:", videoTab), 3, 0);
    m_gammaSlider = new QSlider(Qt::Horizontal, videoTab);
    m_gammaSlider->setRange(-100, 100);
    m_gammaSlider->setValue(0);
    m_gammaVal = new QLabel("0", videoTab);
    m_gammaVal->setFixedWidth(35);
    grid->addWidget(m_gammaSlider, 3, 1);
    grid->addWidget(m_gammaVal, 3, 2);

    videoLayout->addLayout(grid);
    videoLayout->addStretch(1);

    m_resetVideoBtn = new QPushButton("Reset Video Adjustments", videoTab);
    videoLayout->addWidget(m_resetVideoBtn, 0, Qt::AlignRight);

    tabs->addTab(videoTab, "🎬 Video Adjustments");
    rootLayout->addWidget(tabs);

    // Dialog Buttons
    auto *closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    rootLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    // Connections
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OrionEqualizerDialog::onPresetSelected);
    connect(m_resetAudioBtn, &QPushButton::clicked, this, &OrionEqualizerDialog::onResetAudioEq);

    connect(m_brightSlider, &QSlider::valueChanged, this, &OrionEqualizerDialog::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &OrionEqualizerDialog::onContrastChanged);
    connect(m_satSlider, &QSlider::valueChanged, this, &OrionEqualizerDialog::onSaturationChanged);
    connect(m_gammaSlider, &QSlider::valueChanged, this, &OrionEqualizerDialog::onGammaChanged);
    connect(m_resetVideoBtn, &QPushButton::clicked, this, &OrionEqualizerDialog::onResetVideoAdjust);
}

void OrionEqualizerDialog::onBandChanged(int index, int value) {
    if (index >= 0 && index < m_bandLabels.size()) {
        m_bandLabels[index]->setText(QString("%1%2dB").arg(value > 0 ? "+" : "").arg(value));
    }
    if (m_engine) {
        QVector<double> gains(10);
        for (int i = 0; i < 10; ++i) {
            gains[i] = m_bandSliders[i]->value();
        }
        m_engine->setEqualizerBands(gains);
    }
}

void OrionEqualizerDialog::applyPreset(const QVector<double> &values) {
    for (int i = 0; i < std::min(10, static_cast<int>(values.size())); ++i) {
        m_bandSliders[i]->blockSignals(true);
        m_bandSliders[i]->setValue(static_cast<int>(values[i]));
        m_bandLabels[i]->setText(QString("%1%2dB").arg(values[i] > 0 ? "+" : "").arg(values[i]));
        m_bandSliders[i]->blockSignals(false);
    }
    if (m_engine) {
        m_engine->setEqualizerBands(values);
    }
}

void OrionEqualizerDialog::onPresetSelected(int index) {
    switch (index) {
    case 0: // Flat
        applyPreset({0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        break;
    case 1: // Rock
        applyPreset({4.5, 3.0, 2.0, 0, -1.0, 1.0, 3.0, 4.0, 4.5, 5.0});
        break;
    case 2: // Pop
        applyPreset({-1.0, 1.0, 3.0, 4.0, 3.5, 1.0, -1.0, -1.5, 1.0, 2.0});
        break;
    case 3: // Jazz
        applyPreset({3.5, 2.5, 1.0, 2.0, -1.5, -1.5, 0, 1.5, 3.0, 3.5});
        break;
    case 4: // Bass Boost
        applyPreset({7.0, 6.0, 5.0, 3.0, 1.0, 0, 0, 0, 0, 0});
        break;
    case 5: // Vocal
        applyPreset({-2.0, -3.0, -2.0, 1.0, 4.0, 4.5, 3.5, 1.0, 0, -2.0});
        break;
    case 6: // Electronic
        applyPreset({5.0, 4.0, 2.0, 0, -2.0, 2.0, 1.0, 3.0, 5.0, 5.5});
        break;
    }
}

void OrionEqualizerDialog::onResetAudioEq() {
    m_presetCombo->setCurrentIndex(0);
    applyPreset({0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}

void OrionEqualizerDialog::onBrightnessChanged(int value) {
    m_brightVal->setText(QString::number(value));
    if (m_engine) m_engine->setBrightness(value);
}

void OrionEqualizerDialog::onContrastChanged(int value) {
    m_contrastVal->setText(QString::number(value));
    if (m_engine) m_engine->setContrast(value);
}

void OrionEqualizerDialog::onSaturationChanged(int value) {
    m_satVal->setText(QString::number(value));
    if (m_engine) m_engine->setSaturation(value);
}

void OrionEqualizerDialog::onGammaChanged(int value) {
    m_gammaVal->setText(QString::number(value));
    if (m_engine) m_engine->setGamma(value);
}

void OrionEqualizerDialog::onResetVideoAdjust() {
    m_brightSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_satSlider->setValue(0);
    m_gammaSlider->setValue(0);
}
