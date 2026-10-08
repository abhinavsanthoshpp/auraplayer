#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include "AuraEngine.h"
#include "AuraVideoWidget.h"
#include "AuraControls.h"
#include "AuraStudioDrawer.h"
#include "AuraStreamDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openMedia(const QString &path);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Playback & File slots
    void onOpenFile();
    void onOpenNetworkStream();
    void onTogglePlayPause();
    void onStop();
    void onNext();
    void onPrevious();
    void onStepForward();
    void onStepBack();
    void onSeekRelative(double deltaSecs);
    void onVolumeDelta(double delta);
    void onSpeedDelta(double delta);

    // Modes & UI
    void onToggleFullscreen();
    void onToggleAlwaysOnTop();
    void onToggleStudioDrawer();
    void onTakeScreenshot();
    void onUserActivity();
    void onAutoHideTimeout();
    void showOsdMessage(const QString &text, int timeoutMs = 1500);

private:
    void setupFramelessCanvas();
    void setupTopAuraCapsule();
    void applyNebulaTheme();
    void repositionFloatingOverlays();

    AuraEngine *m_engine = nullptr;
    AuraVideoWidget *m_videoWidget = nullptr;
    AuraControls *m_cyberDeck = nullptr;
    AuraStudioDrawer *m_studioDrawer = nullptr;

    // Top Floating Aura Capsule
    QWidget *m_topCapsule = nullptr;
    QLabel *m_capsuleTitle = nullptr;
    QPushButton *m_capsuleOpenBtn = nullptr;
    QPushButton *m_capsuleStreamBtn = nullptr;
    QPushButton *m_capsuleStudioBtn = nullptr;
    QPushButton *m_capsulePipBtn = nullptr;
    QPushButton *m_capsuleMaxBtn = nullptr;
    QPushButton *m_capsuleCloseBtn = nullptr;

    // Overlay OSD
    QLabel *m_osdLabel = nullptr;
    QTimer m_osdTimer;

    // Auto-hide timer
    QTimer m_autoHideTimer;
    bool m_isFullscreen = false;
    bool m_isAlwaysOnTop = false;

    AuraStreamDialog *m_streamDialog = nullptr;
};
