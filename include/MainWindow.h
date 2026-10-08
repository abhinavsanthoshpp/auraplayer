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

#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QStatusBar>
#include <QAction>
#include "OrionEngine.h"
#include "OrionVideoWidget.h"
#include "OrionVlcToolbar.h"
#include "OrionPlaylistView.h"
#include "OrionEffectsDialog.h"
#include "OrionMediaInfoDialog.h"
#include "OrionPreferencesDialog.h"
#include "OrionStreamDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openMedia(const QString &path);

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
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
    void onCycleAspectRatio();
    void onCycleAudioTrack();
    void onCycleSubtitleTrack();
    void onToggleDeinterlace();
    void onAudioDelayDelta(double delta);
    void onSubtitleDelayDelta(double delta);
    void onTakeSnapshot();
    void onToggleFullscreen();
    void onToggleAlwaysOnTop();
    void onToggleMute();
    void onVolumeDelta(double delta);
    void showVideoContextMenu(const QPoint &globalPos);

    // Tools & View
    void onShowEffects();
    void onShowMediaInfo();
    void onShowPreferences();
    void onTogglePlaylist();
    void onToggleAdvancedControls();
    void onAbout();
    void onShowDonate();

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

    OrionEngine *m_engine = nullptr;
    OrionVideoWidget *m_videoWidget = nullptr;
    OrionPlaylistView *m_playlistView = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

    OrionVlcToolbar *m_toolbar = nullptr;

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
    OrionEffectsDialog *m_effectsDialog = nullptr;
    OrionMediaInfoDialog *m_mediaInfoDialog = nullptr;
    OrionPreferencesDialog *m_prefsDialog = nullptr;
    OrionStreamDialog *m_streamDialog = nullptr;

    bool m_isFullscreen = false;
    bool m_isAlwaysOnTop = false;
    QString m_pendingMedia;
};
