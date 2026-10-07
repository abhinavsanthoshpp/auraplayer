#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "AuraEngine.h"
#include "AuraVideoWidget.h"
#include "AuraControls.h"

class AuraPlaylist;
class AuraEqualizerDialog;
class AuraMediaInfoDialog;
class AuraStreamDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openMedia(const QString &path);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File / Media actions
    void onOpenFile();
    void onOpenMultipleFiles();
    void onOpenFolder();
    void onOpenNetworkStream();

    // Playback actions
    void onTogglePlayPause();
    void onStop();
    void onNext();
    void onPrevious();
    void onStepForward();
    void onStepBack();
    void onSeekRelative(double deltaSecs);
    void onVolumeDelta(double delta);
    void onSpeedDelta(double delta);

    // View / Mode actions
    void onToggleFullscreen();
    void onToggleAlwaysOnTop();
    void onTogglePlaylist();
    void onShowEqualizer();
    void onShowMediaInfo();
    void onTakeScreenshot();
    void onAbout();

    // UI reactivity & auto-hide
    void onUserActivity();
    void onAutoHideTimeout();
    void showOsdMessage(const QString &text, int timeoutMs = 1500);

private:
    void createMenuBar();
    void createCentralLayout();
    void setupShortcuts();
    void applyTheme();

    AuraEngine *m_engine = nullptr;
    AuraVideoWidget *m_videoWidget = nullptr;
    AuraControls *m_controls = nullptr;

    // Overlay OSD label
    QLabel *m_osdLabel = nullptr;
    QTimer m_osdTimer;

    // Auto-hide controls timer
    QTimer m_autoHideTimer;
    bool m_isFullscreen = false;
    bool m_isAlwaysOnTop = false;

    // Dialogs
    AuraEqualizerDialog *m_equalizerDialog = nullptr;
    AuraMediaInfoDialog *m_mediaInfoDialog = nullptr;
    AuraStreamDialog *m_streamDialog = nullptr;
    AuraPlaylist *m_playlistWidget = nullptr;
};
