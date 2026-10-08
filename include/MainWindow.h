#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QStatusBar>
#include <QAction>
#include "AuraEngine.h"
#include "AuraVideoWidget.h"
#include "AuraVlcToolbar.h"
#include "AuraPlaylistView.h"
#include "AuraEffectsDialog.h"
#include "AuraMediaInfoDialog.h"
#include "AuraPreferencesDialog.h"
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
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // Media menu
    void onOpenFile();
    void onOpenMultipleFiles();
    void onOpenFolder();
    void onOpenNetworkStream();
    void onOpenClipboardLocation();

    // Playback slots
    void onTogglePlayPause();
    void onStop();
    void onNext();
    void onPrevious();
    void onSpeedFaster();
    void onSpeedSlower();
    void onSpeedNormal();
    void onJumpRelative(double seconds);
    void onFrameStep();

    // Video & Audio
    void onAddSubtitleFile();
    void onSetAspectRatio(const QString &ratio);
    void onTakeSnapshot();
    void onToggleFullscreen();
    void onToggleAlwaysOnTop();
    void onVolumeDelta(double delta);

    // Tools & View
    void onShowEffects();
    void onShowMediaInfo();
    void onShowPreferences();
    void onTogglePlaylist();
    void onToggleAdvancedControls();
    void onAbout();

    // Engine feedback
    void onPlaybackStarted();
    void onPlaybackStopped();
    void onPlaybackPaused(bool paused);
    void onSpeedChanged(double speed);
    void updateStatusBar();

private:
    void createMenuBar();
    void createCentralLayout();
    void applyVlcTheme();

    AuraEngine *m_engine = nullptr;
    AuraVideoWidget *m_videoWidget = nullptr;
    AuraPlaylistView *m_playlistView = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

    AuraVlcToolbar *m_toolbar = nullptr;

    // Status bar labels
    QLabel *m_statusText = nullptr;
    QLabel *m_statusSpeed = nullptr;
    QLabel *m_statusMediaInfo = nullptr;

    // Actions that need state sync
    QAction *m_actPlayPause = nullptr;
    QAction *m_actAdvanced = nullptr;
    QAction *m_actAlwaysOnTop = nullptr;
    QAction *m_actFullscreen = nullptr;

    // Dialogs
    AuraEffectsDialog *m_effectsDialog = nullptr;
    AuraMediaInfoDialog *m_mediaInfoDialog = nullptr;
    AuraPreferencesDialog *m_prefsDialog = nullptr;
    AuraStreamDialog *m_streamDialog = nullptr;

    bool m_isFullscreen = false;
    bool m_isAlwaysOnTop = false;
};
