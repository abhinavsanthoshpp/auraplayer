#pragma once

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVector>
#include "OrionEngine.h"

class OrionEqualizerDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionEqualizerDialog(OrionEngine *engine, QWidget *parent = nullptr);

private slots:
    void onBandChanged(int index, int value);
    void onPresetSelected(int index);
    void onResetAudioEq();

    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onSaturationChanged(int value);
    void onGammaChanged(int value);
    void onResetVideoAdjust();

private:
    void setupUi();
    void applyPreset(const QVector<double> &values);

    OrionEngine *m_engine = nullptr;

    // Audio Equalizer
    QVector<QSlider *> m_bandSliders;
    QVector<QLabel *> m_bandLabels;
    QComboBox *m_presetCombo = nullptr;
    QPushButton *m_resetAudioBtn = nullptr;

    // Video Adjustments
    QSlider *m_brightSlider = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QSlider *m_satSlider = nullptr;
    QSlider *m_gammaSlider = nullptr;
    QLabel *m_brightVal = nullptr;
    QLabel *m_contrastVal = nullptr;
    QLabel *m_satVal = nullptr;
    QLabel *m_gammaVal = nullptr;
    QPushButton *m_resetVideoBtn = nullptr;
};
