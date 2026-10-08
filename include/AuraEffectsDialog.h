#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVector>
#include "AuraEngine.h"

class AuraEffectsDialog : public QDialog {
    Q_OBJECT

public:
    explicit AuraEffectsDialog(AuraEngine *engine, QWidget *parent = nullptr);

    void refreshFromEngine();

private slots:
    // Audio Equalizer
    void onEqToggled(bool checked);
    void onPreampChanged(int val);
    void onBandChanged(int index, int val);
    void onPresetSelected(int index);

    // Video Adjustments
    void onVideoAdjustToggled(bool checked);
    void onBrightnessChanged(int val);
    void onContrastChanged(int val);
    void onSaturationChanged(int val);
    void onGammaChanged(int val);
    void onHueChanged(int val);
    void onResetVideoClicked();

    // Synchronization
    void onAudioSyncChanged(double val);
    void onSubSyncChanged(double val);

private:
    void setupUi();
    QWidget *createAudioTab();
    QWidget *createVideoTab();
    QWidget *createSyncTab();
    void applyEqualizer();

    AuraEngine *m_engine = nullptr;

    // Audio Equalizer controls
    QCheckBox *m_eqEnable = nullptr;
    QCheckBox *m_twoPass = nullptr;
    QComboBox *m_presetCombo = nullptr;
    QSlider *m_preampSlider = nullptr;
    QLabel *m_preampLabel = nullptr;
    QVector<QSlider *> m_bandSliders;
    QVector<QLabel *> m_bandLabels;

    // Video Effects controls
    QCheckBox *m_videoEnable = nullptr;
    QSlider *m_brightSlider = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QSlider *m_satSlider = nullptr;
    QSlider *m_gammaSlider = nullptr;
    QSlider *m_hueSlider = nullptr;
    QLabel *m_brightVal = nullptr;
    QLabel *m_contrastVal = nullptr;
    QLabel *m_satVal = nullptr;
    QLabel *m_gammaVal = nullptr;
    QLabel *m_hueVal = nullptr;

    // Synchronization controls
    QDoubleSpinBox *m_audioSyncSpin = nullptr;
    QDoubleSpinBox *m_subSyncSpin = nullptr;
};
