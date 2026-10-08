#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVector>
#include "OrionEngine.h"

class OrionStudioDrawer : public QWidget {
    Q_OBJECT

public:
    explicit OrionStudioDrawer(OrionEngine *engine, QWidget *parent = nullptr);

    void addFile(const QString &filePath);
    void addFiles(const QStringList &filePaths);
    void clearQueue();
    QString playNext();
    QString playPrevious();
    void refreshTelemetry();
    void selectTab(int index);

signals:
    void trackSelected(const QString &filePath);
    void closeRequested();

private slots:
    // Queue slots
    void onQueueItemDoubleClicked(QListWidgetItem *item);
    void onAddMediaClicked();
    void onClearQueueClicked();
    void onShuffleClicked();
    void onFilterTextChanged(const QString &text);

    // Audio Studio slots
    void onEqPresetClicked(const QString &presetName);
    void onEqBandChanged(int index, int value);
    void onResetEqClicked();

    // Video Shader slots
    void onBrightnessChanged(int val);
    void onContrastChanged(int val);
    void onSaturationChanged(int val);
    void onGammaChanged(int val);
    void onResetVideoClicked();

private:
    void setupUi();
    QWidget *createQueueTab();
    QWidget *createAudioTab();
    QWidget *createVideoTab();
    QWidget *createTelemetryTab();

    OrionEngine *m_engine = nullptr;
    QTabWidget *m_tabs = nullptr;

    // Queue tab elements
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_queueList = nullptr;
    QPushButton *m_shuffleBtn = nullptr;
    QPushButton *m_loopBtn = nullptr;
    int m_currentIndex = -1;
    bool m_loopAll = true;

    // Audio tab elements
    QVector<QSlider *> m_eqSliders;
    QVector<QLabel *> m_eqValueLabels;

    // Video tab elements
    QSlider *m_brightSlider = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QSlider *m_satSlider = nullptr;
    QSlider *m_gammaSlider = nullptr;
    QLabel *m_brightVal = nullptr;
    QLabel *m_contrastVal = nullptr;
    QLabel *m_satVal = nullptr;
    QLabel *m_gammaVal = nullptr;

    // Telemetry tab elements
    QLabel *m_telemetryCodec = nullptr;
    QLabel *m_telemetryRes = nullptr;
    QLabel *m_telemetryFps = nullptr;
    QLabel *m_telemetryHwdec = nullptr;
    QLabel *m_telemetryAudio = nullptr;
    QLabel *m_telemetryBitrate = nullptr;
    QLabel *m_telemetryBuffer = nullptr;
};
