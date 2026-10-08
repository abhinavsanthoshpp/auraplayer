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

#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QIcon>
#include <cmath>

static QString formatOsdTime(double seconds) {
    if (seconds < 0) seconds = 0;
    int total = static_cast<int>(seconds);
    int s = total % 60;
    int m = (total / 60) % 60;
    int h = total / 3600;
    if (h > 0) {
        return QString("%1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("OrionPlayer — VLC-Grade Professional Media Player");
    setWindowIcon(QIcon(":/icon.svg"));
    resize(1000, 680);
    setAcceptDrops(true);

    m_engine = new OrionEngine(this);
    m_engine->initialize();

    createCentralLayout();
    createMenuBar();
    applyVlcTheme();

    // Install application-level event filter for unified VLC hotkey navigation
    qApp->installEventFilter(this);

    if (m_videoWidget) {
        connect(m_videoWidget, &OrionVideoWidget::renderContextReady, this, [this]() {
            if (!m_pendingMedia.isEmpty()) {
                QString path = m_pendingMedia;
                m_pendingMedia.clear();
                openMedia(path);
            }
        });
        m_videoWidget->ensureRenderContextInitialized();
    }

    connect(m_engine, &OrionEngine::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(m_engine, &OrionEngine::playbackStopped, this, &MainWindow::onPlaybackStopped);
    connect(m_engine, &OrionEngine::playbackPaused, this, &MainWindow::onPlaybackPaused);
    connect(m_engine, &OrionEngine::speedChanged, this, &MainWindow::onSpeedChanged);
    connect(m_engine, &OrionEngine::videoReconfigured, this, [this](int w, int h) {
        Q_UNUSED(w); Q_UNUSED(h);
        updateStatusBar();
    });
}

MainWindow::~MainWindow() {
    qApp->removeEventFilter(this);
    delete m_videoWidget;
    m_videoWidget = nullptr;
    delete m_engine;
    m_engine = nullptr;
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (m_videoWidget) {
        m_videoWidget->ensureRenderContextInitialized();
    }
}

void MainWindow::createCentralLayout() {
    auto *centralContainer = new QWidget(this);
    auto *layout = new QVBoxLayout(centralContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Stacked widget: Index 0 = Video canvas, Index 1 = Playlist view
    m_stackedWidget = new QStackedWidget(centralContainer);

    m_videoWidget = new OrionVideoWidget(m_engine, m_stackedWidget);
    m_playlistView = new OrionPlaylistView(m_stackedWidget);

    m_stackedWidget->addWidget(m_videoWidget);   // 0
    m_stackedWidget->addWidget(m_playlistView);  // 1
    m_stackedWidget->setCurrentIndex(0);

    layout->addWidget(m_stackedWidget, 1);

    // VLC Toolbar at the bottom
    m_toolbar = new OrionVlcToolbar(m_engine, centralContainer);
    layout->addWidget(m_toolbar, 0);

    setCentralWidget(centralContainer);

    // Status bar at the very bottom
    auto *sb = statusBar();
    m_statusText = new QLabel("Ready", sb);
    m_statusSpeed = new QLabel("1.00x", sb);
    m_statusMediaInfo = new QLabel("", sb);

    m_statusText->setStyleSheet("padding: 2px 8px; color: #8b949e;");
    m_statusSpeed->setStyleSheet("padding: 2px 8px; color: #58a6ff; font-weight: bold;");
    m_statusMediaInfo->setStyleSheet("padding: 2px 8px; color: #8b949e;");

    sb->addWidget(m_statusText, 1);
    sb->addPermanentWidget(m_statusMediaInfo);
    sb->addPermanentWidget(m_statusSpeed);

    // Toolbar connections
    connect(m_toolbar, &OrionVlcToolbar::playPauseClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_toolbar, &OrionVlcToolbar::stopClicked, this, &MainWindow::onStop);
    connect(m_toolbar, &OrionVlcToolbar::prevClicked, this, &MainWindow::onPrevious);
    connect(m_toolbar, &OrionVlcToolbar::nextClicked, this, &MainWindow::onNext);
    connect(m_toolbar, &OrionVlcToolbar::fullscreenClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_toolbar, &OrionVlcToolbar::extendedSettingsClicked, this, &MainWindow::onShowEffects);
    connect(m_toolbar, &OrionVlcToolbar::playlistClicked, this, &MainWindow::onTogglePlaylist);
    connect(m_toolbar, &OrionVlcToolbar::snapshotClicked, this, &MainWindow::onTakeSnapshot);
    connect(m_toolbar, &OrionVlcToolbar::frameStepClicked, this, &MainWindow::onFrameStep);
    connect(m_toolbar, &OrionVlcToolbar::loopModeChanged, this, [this](OrionVlcToolbar::LoopMode mode) {
        if (mode == OrionVlcToolbar::LoopMode::RepeatAll) {
            m_playlistView->setLoopMode(OrionPlaylistView::LoopMode::RepeatAll);
        } else if (mode == OrionVlcToolbar::LoopMode::RepeatOne) {
            m_playlistView->setLoopMode(OrionPlaylistView::LoopMode::RepeatOne);
        } else {
            m_playlistView->setLoopMode(OrionPlaylistView::LoopMode::None);
        }
    });
    connect(m_toolbar, &OrionVlcToolbar::shuffleClicked, this, [this]() {
        m_playlistView->shuffle();
    });

    // Video canvas event connections
    connect(m_videoWidget, &OrionVideoWidget::doubleClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_videoWidget, &OrionVideoWidget::singleClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_videoWidget, &OrionVideoWidget::contextMenuRequested, this, &MainWindow::showVideoContextMenu);
    connect(m_videoWidget, &OrionVideoWidget::fileDropped, this, &MainWindow::openMedia);
    connect(m_videoWidget, &OrionVideoWidget::wheelScrolled, this, [this](int delta) {
        onVolumeDelta(delta > 0 ? 5.0 : -5.0);
    });

    // Playlist connections
    connect(m_playlistView, &OrionPlaylistView::trackSelected, this, &MainWindow::openMedia);
}

void MainWindow::createMenuBar() {
    auto *mb = menuBar();

    // Media menu
    auto *mediaMenu = mb->addMenu("&Media");
    mediaMenu->addAction("&Open File...", QKeySequence::Open, this, &MainWindow::onOpenFile);
    mediaMenu->addAction("Open &Multiple Files...", QKeySequence("Ctrl+Shift+O"), this, &MainWindow::onOpenMultipleFiles);
    mediaMenu->addAction("Open &Folder...", QKeySequence("Ctrl+F"), this, &MainWindow::onOpenFolder);
    mediaMenu->addAction("Open &Network Stream...", QKeySequence("Ctrl+N"), this, &MainWindow::onOpenNetworkStream);
    mediaMenu->addAction("Open &Location from Clipboard...", QKeySequence("Ctrl+V"), this, &MainWindow::onOpenClipboardLocation);
    mediaMenu->addSeparator();
    mediaMenu->addAction("&Quit", QKeySequence("Ctrl+Q"), this, &QWidget::close);

    // Playback menu
    auto *playMenu = mb->addMenu("&Playback");
    m_actPlayPause = playMenu->addAction("Play", Qt::Key_Space, this, &MainWindow::onTogglePlayPause);
    playMenu->addAction("&Stop", Qt::Key_S, this, &MainWindow::onStop);
    playMenu->addAction("&Previous", Qt::Key_P, this, &MainWindow::onPrevious);
    playMenu->addAction("&Next", Qt::Key_N, this, &MainWindow::onNext);
    playMenu->addSeparator();

    auto *speedMenu = playMenu->addMenu("&Speed");
    speedMenu->addAction("&Faster", Qt::Key_BracketRight, this, &MainWindow::onSpeedFaster);
    speedMenu->addAction("&Slower", Qt::Key_BracketLeft, this, &MainWindow::onSpeedSlower);
    speedMenu->addAction("&Normal Speed", Qt::Key_Equal, this, &MainWindow::onSpeedNormal);

    auto *jumpMenu = playMenu->addMenu("&Jump");
    jumpMenu->addAction("Very Short Forward (+3 sec)", QKeySequence("Shift+Right"), this, [this]() { onJumpRelative(3.0); });
    jumpMenu->addAction("Very Short Backward (-3 sec)", QKeySequence("Shift+Left"), this, [this]() { onJumpRelative(-3.0); });
    jumpMenu->addAction("Short Forward (+10 sec)", Qt::Key_Right, this, [this]() { onJumpRelative(10.0); });
    jumpMenu->addAction("Short Backward (-10 sec)", Qt::Key_Left, this, [this]() { onJumpRelative(-10.0); });
    jumpMenu->addAction("Medium Forward (+1 min)", QKeySequence("Ctrl+Right"), this, [this]() { onJumpRelative(60.0); });
    jumpMenu->addAction("Medium Backward (-1 min)", QKeySequence("Ctrl+Left"), this, [this]() { onJumpRelative(-60.0); });
    jumpMenu->addAction("Long Forward (+5 min)", QKeySequence("Ctrl+Alt+Right"), this, [this]() { onJumpRelative(300.0); });
    jumpMenu->addAction("Long Backward (-5 min)", QKeySequence("Ctrl+Alt+Left"), this, [this]() { onJumpRelative(-300.0); });

    playMenu->addSeparator();
    playMenu->addAction("Step Forward Frame-by-Frame", Qt::Key_E, this, &MainWindow::onFrameStep);

    // Audio menu
    auto *audioMenu = mb->addMenu("&Audio");
    audioMenu->addAction("Volume &Up (+5%)", Qt::Key_Up, this, [this]() { onVolumeDelta(5.0); });
    audioMenu->addAction("Volume &Down (-5%)", Qt::Key_Down, this, [this]() { onVolumeDelta(-5.0); });
    audioMenu->addAction("&Mute", Qt::Key_M, this, &MainWindow::onToggleMute);
    audioMenu->addAction("Cycle &Audio Track", Qt::Key_B, this, &MainWindow::onCycleAudioTrack);
    audioMenu->addSeparator();
    audioMenu->addAction("Audio Track Delay (+50ms)", Qt::Key_H, this, [this]() { onAudioDelayDelta(0.05); });
    audioMenu->addAction("Audio Track Delay (-50ms)", Qt::Key_G, this, [this]() { onAudioDelayDelta(-0.05); });

    // Video menu
    auto *videoMenu = mb->addMenu("&Video");
    m_actFullscreen = videoMenu->addAction("&Fullscreen", Qt::Key_F11, this, &MainWindow::onToggleFullscreen);
    m_actAlwaysOnTop = videoMenu->addAction("Always on &Top", this, &MainWindow::onToggleAlwaysOnTop);
    m_actAlwaysOnTop->setCheckable(true);
    videoMenu->addSeparator();

    auto *aspectMenu = videoMenu->addMenu("&Aspect Ratio");
    aspectMenu->addAction("Default", this, [this]() { onSetAspectRatio("default"); });
    aspectMenu->addAction("16:9", this, [this]() { onSetAspectRatio("16:9"); });
    aspectMenu->addAction("4:3", this, [this]() { onSetAspectRatio("4:3"); });
    aspectMenu->addAction("1:1", this, [this]() { onSetAspectRatio("1:1"); });
    aspectMenu->addAction("16:10", this, [this]() { onSetAspectRatio("16:10"); });
    aspectMenu->addAction("2.21:1", this, [this]() { onSetAspectRatio("2.21:1"); });
    aspectMenu->addAction("2.35:1", this, [this]() { onSetAspectRatio("2.35:1"); });
    videoMenu->addAction("Cycle Aspect &Ratio", Qt::Key_A, this, &MainWindow::onCycleAspectRatio);

    auto *deintMenu = videoMenu->addMenu("&Deinterlace");
    deintMenu->addAction("Off", this, [this]() { if (m_engine) m_engine->setDeinterlace(false); });
    deintMenu->addAction("On", this, [this]() { if (m_engine) m_engine->setDeinterlace(true); });
    videoMenu->addAction("Toggle &Deinterlace", Qt::Key_D, this, &MainWindow::onToggleDeinterlace);

    videoMenu->addSeparator();
    videoMenu->addAction("Take &Snapshot", QKeySequence("Shift+S"), this, &MainWindow::onTakeSnapshot);

    // Subtitle menu
    auto *subMenu = mb->addMenu("&Subtitle");
    subMenu->addAction("&Add Subtitle File...", this, &MainWindow::onAddSubtitleFile);
    subMenu->addAction("Cycle &Subtitle Track", Qt::Key_V, this, &MainWindow::onCycleSubtitleTrack);
    subMenu->addSeparator();
    subMenu->addAction("Subtitle Delay (+50ms)", Qt::Key_K, this, [this]() { onSubtitleDelayDelta(0.05); });
    subMenu->addAction("Subtitle Delay (-50ms)", Qt::Key_J, this, [this]() { onSubtitleDelayDelta(-0.05); });

    // Tools menu
    auto *toolsMenu = mb->addMenu("&Tools");
    toolsMenu->addAction("&Effects and Filters", QKeySequence("Ctrl+E"), this, &MainWindow::onShowEffects);
    toolsMenu->addAction("&Media Information", QKeySequence("Ctrl+I"), this, &MainWindow::onShowMediaInfo);
    toolsMenu->addSeparator();
    toolsMenu->addAction("&Preferences", QKeySequence("Ctrl+P"), this, &MainWindow::onShowPreferences);

    // View menu
    auto *viewMenu = mb->addMenu("&View");
    viewMenu->addAction("&Playlist", QKeySequence("Ctrl+L"), this, &MainWindow::onTogglePlaylist);
    m_actAdvanced = viewMenu->addAction("&Advanced Controls", this, &MainWindow::onToggleAdvancedControls);
    m_actAdvanced->setCheckable(true);
    m_actAdvanced->setChecked(false);
    viewMenu->addSeparator();
    viewMenu->addAction("&Status Bar", this, [this](bool checked) {
        statusBar()->setVisible(checked);
    })->setCheckable(true);

    // Help menu
    auto *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About OrionPlayer", QKeySequence("Shift+F1"), this, &MainWindow::onAbout);
}

void MainWindow::applyVlcTheme() {
    QFile file(":/style.qss");
    if (!file.exists()) file.setFileName("resources/style.qss");
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }
}

void MainWindow::openMedia(const QString &path) {
    if (path.isEmpty() || !m_engine) return;

    if (m_videoWidget) {
        m_videoWidget->ensureRenderContextInitialized();
        if (!m_videoWidget->isRenderContextReady()) {
            m_pendingMedia = path;
            m_playlistView->addFile(path);
            m_stackedWidget->setCurrentIndex(0);
            setWindowTitle(QString("OrionPlayer — %1").arg(QFileInfo(path).fileName()));
            return;
        }
    }

    m_playlistView->addFile(path);
    m_engine->loadFile(path);
    m_engine->play();

    m_stackedWidget->setCurrentIndex(0); // Switch to video display
    setWindowTitle(QString("OrionPlayer — %1").arg(QFileInfo(path).fileName()));
    updateStatusBar();
}

void MainWindow::onOpenFile() {
    QString file = QFileDialog::getOpenFileName(
        this, "Select Media File", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    if (!file.isEmpty()) openMedia(file);
}

void MainWindow::onOpenMultipleFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Select Multiple Media Files", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    if (!files.isEmpty()) {
        m_playlistView->addFiles(files);
        openMedia(files.first());
    }
}

void MainWindow::onOpenFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Media Folder");
    if (!dir.isEmpty()) {
        QDir directory(dir);
        QStringList filters;
        filters << "*.mp4" << "*.mkv" << "*.webm" << "*.avi" << "*.mov" << "*.flv" << "*.mp3" << "*.flac" << "*.wav";
        QStringList files = directory.entryList(filters, QDir::Files, QDir::Name);
        QStringList fullPaths;
        for (const auto &f : files) fullPaths.append(directory.absoluteFilePath(f));
        if (!fullPaths.isEmpty()) {
            m_playlistView->addFiles(fullPaths);
            openMedia(fullPaths.first());
        }
    }
}

void MainWindow::onOpenNetworkStream() {
    if (!m_streamDialog) m_streamDialog = new OrionStreamDialog(this);
    if (m_streamDialog->exec() == QDialog::Accepted) {
        QString url = m_streamDialog->streamUrl();
        if (!url.isEmpty()) openMedia(url);
    }
}

void MainWindow::onOpenClipboardLocation() {
    QString clip = QApplication::clipboard()->text().trimmed();
    if (clip.startsWith("http://") || clip.startsWith("https://") || clip.startsWith("rtsp://") || clip.startsWith("file://")) {
        openMedia(clip);
    } else {
        onOpenNetworkStream();
    }
}

void MainWindow::onTogglePlayPause() {
    if (!m_engine) return;
    bool willBePaused = !m_engine->isPaused();
    m_engine->togglePause();
    QString msg = willBePaused ? "⏸ Paused" : "▶ Playing";
    if (m_videoWidget) m_videoWidget->showOsd(msg, 900);
}

void MainWindow::onStop() {
    if (m_engine) {
        m_engine->stop();
        m_statusText->setText("Stopped");
        if (m_videoWidget) m_videoWidget->showOsd("⏹ Stopped", 900);
    }
}

void MainWindow::onNext() {
    QString nextTrack = m_playlistView->playNext();
    if (!nextTrack.isEmpty()) {
        openMedia(nextTrack);
        if (m_videoWidget) m_videoWidget->showOsd(QString("⏭ Next: %1").arg(QFileInfo(nextTrack).fileName()), 1200);
    }
}

void MainWindow::onPrevious() {
    QString prevTrack = m_playlistView->playPrevious();
    if (!prevTrack.isEmpty()) {
        openMedia(prevTrack);
        if (m_videoWidget) m_videoWidget->showOsd(QString("⏮ Prev: %1").arg(QFileInfo(prevTrack).fileName()), 1200);
    }
}

void MainWindow::onSpeedFaster() {
    if (!m_engine) return;
    double newSpeed = std::min(4.0, m_engine->speed() + 0.1);
    m_engine->setSpeed(newSpeed);
    QString msg = QString("⚡ Speed: %1x").arg(newSpeed, 0, 'f', 2);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1000);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onSpeedSlower() {
    if (!m_engine) return;
    double newSpeed = std::max(0.25, m_engine->speed() - 0.1);
    m_engine->setSpeed(newSpeed);
    QString msg = QString("⚡ Speed: %1x").arg(newSpeed, 0, 'f', 2);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1000);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onSpeedNormal() {
    if (!m_engine) return;
    m_engine->setSpeed(1.0);
    QString msg = "⚡ Speed: 1.00x";
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1000);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onJumpRelative(double seconds) {
    if (!m_engine) return;
    double oldPos = m_engine->position();
    double dur = m_engine->duration();
    m_engine->seekRelative(seconds);
    double targetPos = std::clamp(oldPos + seconds, 0.0, dur > 0 ? dur : 999999.0);

    QString sign = seconds >= 0 ? "+" : "-";
    int sAbs = static_cast<int>(std::abs(seconds));
    QString jumpStr;
    if (sAbs >= 60) {
        jumpStr = QString("%1%2m").arg(sign).arg(sAbs / 60);
    } else {
        jumpStr = QString("%1%2s").arg(sign).arg(sAbs);
    }

    QString icon = seconds >= 0 ? "⏩" : "⏪";
    QString msg;
    if (dur > 0) {
        msg = QString("%1 %2  (%3 / %4)")
            .arg(icon)
            .arg(jumpStr)
            .arg(formatOsdTime(targetPos))
            .arg(formatOsdTime(dur));
    } else {
        msg = QString("%1 %2  (%3)")
            .arg(icon)
            .arg(jumpStr)
            .arg(formatOsdTime(targetPos));
    }

    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onFrameStep() {
    if (m_engine) {
        m_engine->frameStep();
        if (m_videoWidget) m_videoWidget->showOsd("🎞️ Frame Step", 800);
    }
}

void MainWindow::onAddSubtitleFile() {
    QString sub = QFileDialog::getOpenFileName(
        this, "Select Subtitle File", QString(),
        "Subtitle Files (*.srt *.ass *.ssa *.vtt *.sub);;All Files (*)"
    );
    if (!sub.isEmpty() && m_engine) {
        m_engine->loadSubtitleFile(sub);
        QString msg = QString("💬 Loaded Subtitle: %1").arg(QFileInfo(sub).fileName());
        if (m_videoWidget) m_videoWidget->showOsd(msg, 2000);
        statusBar()->showMessage(msg, 3000);
    }
}

void MainWindow::onSetAspectRatio(const QString &ratio) {
    if (!m_engine) return;
    m_engine->setAspectRatio(ratio);
    QString msg = QString("📐 Aspect Ratio: %1").arg(ratio == "-1" || ratio == "default" ? "Default" : ratio);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 2000);
}

void MainWindow::onCycleAspectRatio() {
    if (!m_engine) return;
    QString ratio = m_engine->cycleAspectRatio();
    QString msg = QString("📐 Aspect Ratio: %1").arg(ratio == "-1" || ratio == "default" ? "Default" : ratio);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 2000);
}

void MainWindow::onCycleAudioTrack() {
    if (!m_engine) return;
    QString desc = m_engine->cycleAudioTrack();
    QString msg = QString("🎵 Audio Track: %1").arg(desc);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1500);
    statusBar()->showMessage(msg, 2500);
}

void MainWindow::onCycleSubtitleTrack() {
    if (!m_engine) return;
    QString desc = m_engine->cycleSubtitleTrack();
    QString msg = QString("💬 Subtitle Track: %1").arg(desc);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1500);
    statusBar()->showMessage(msg, 2500);
}

void MainWindow::onToggleDeinterlace() {
    if (!m_engine) return;
    bool on = m_engine->toggleDeinterlace();
    QString msg = QString("Deinterlace: %1").arg(on ? "On" : "Off");
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 2000);
}

void MainWindow::onAudioDelayDelta(double delta) {
    if (!m_engine) return;
    double newDelay = m_engine->audioDelay() + delta;
    m_engine->setAudioDelay(newDelay);
    int ms = static_cast<int>(std::round(newDelay * 1000.0));
    QString msg = QString("Audio Delay: %1 ms").arg(ms);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 2000);
}

void MainWindow::onSubtitleDelayDelta(double delta) {
    if (!m_engine) return;
    double newDelay = m_engine->subtitleDelay() + delta;
    m_engine->setSubtitleDelay(newDelay);
    int ms = static_cast<int>(std::round(newDelay * 1000.0));
    QString msg = QString("Subtitle Delay: %1 ms").arg(ms);
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1200);
    statusBar()->showMessage(msg, 2000);
}

void MainWindow::onTakeSnapshot() {
    if (m_engine) {
        m_engine->takeScreenshot();
        if (m_videoWidget) m_videoWidget->showOsd("📸 Snapshot Saved to ~/Pictures", 1500);
        statusBar()->showMessage("Snapshot saved to ~/Pictures", 3000);
    }
}

void MainWindow::onToggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    if (m_isFullscreen) {
        menuBar()->hide();
        m_toolbar->hide();
        statusBar()->hide();
        showFullScreen();
        if (m_videoWidget) m_videoWidget->showOsd("⛶ Fullscreen", 900);
    } else {
        menuBar()->show();
        m_toolbar->show();
        statusBar()->show();
        showNormal();
        if (m_videoWidget) m_videoWidget->showOsd("🗗 Windowed", 900);
    }
}

void MainWindow::onToggleAlwaysOnTop() {
    m_isAlwaysOnTop = !m_isAlwaysOnTop;
    setWindowFlag(Qt::WindowStaysOnTopHint, m_isAlwaysOnTop);
    show();
    m_actAlwaysOnTop->setChecked(m_isAlwaysOnTop);
}

void MainWindow::onToggleMute() {
    if (!m_engine) return;
    m_engine->toggleMute();
    bool muted = m_engine->isMuted();
    QString msg = muted ? "🔇 Muted" : QString("🔊 Volume: %1%").arg(static_cast<int>(std::round(m_engine->volume())));
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1000);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onVolumeDelta(double delta) {
    if (!m_engine) return;
    double newVol = std::clamp(m_engine->volume() + delta, 0.0, 200.0);
    m_engine->setVolume(newVol);
    QString msg = QString("🔊 Volume: %1%").arg(static_cast<int>(std::round(newVol)));
    if (m_videoWidget) m_videoWidget->showOsd(msg, 1000);
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onShowEffects() {
    if (!m_effectsDialog) m_effectsDialog = new OrionEffectsDialog(m_engine, this);
    m_effectsDialog->refreshFromEngine();
    m_effectsDialog->show();
    m_effectsDialog->raise();
    m_effectsDialog->activateWindow();
}

void MainWindow::onShowMediaInfo() {
    if (!m_mediaInfoDialog) m_mediaInfoDialog = new OrionMediaInfoDialog(m_engine, this);
    m_mediaInfoDialog->refresh();
    m_mediaInfoDialog->show();
    m_mediaInfoDialog->raise();
    m_mediaInfoDialog->activateWindow();
}

void MainWindow::onShowPreferences() {
    if (!m_prefsDialog) m_prefsDialog = new OrionPreferencesDialog(m_engine, this);
    m_prefsDialog->show();
    m_prefsDialog->raise();
    m_prefsDialog->activateWindow();
}

void MainWindow::onTogglePlaylist() {
    if (m_stackedWidget->currentIndex() == 0) {
        m_stackedWidget->setCurrentIndex(1); // Show Playlist
    } else {
        m_stackedWidget->setCurrentIndex(0); // Show Video
    }
}

void MainWindow::onToggleAdvancedControls() {
    bool visible = !m_toolbar->isAdvancedControlsVisible();
    m_toolbar->setAdvancedControlsVisible(visible);
    m_actAdvanced->setChecked(visible);
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Orion Player",
        "<h3>Orion Player 1.0</h3>"
        "<p>High-Performance Hardware-Accelerated Media Player for Linux.</p>"
        "<p>Designed with VLC-grade ergonomics and zero-copy Intel VA-API acceleration.</p>"
        "<hr>"
        "<p><b>Copyright © 2026 Abhinav Santhosh. All Rights Reserved.</b></p>"
        "<p>Author: <b>Abhinav Santhosh</b> (<a href='https://github.com/abhinavsanthoshpp'>@abhinavsanthoshpp</a>)<br>"
        "Website: <a href='https://abhinavsanthoshpp.github.io/orionplayer/'>abhinavsanthoshpp.github.io/orionplayer</a></p>"
        "<p style='font-size: 11px; color: #8b949e;'>"
        "The software, website, brand, and design assets are the exclusive intellectual property "
        "of Abhinav Santhosh. Unauthorized reproduction, rebranding, or commercial distribution "
        "is strictly prohibited under international copyright laws.</p>"
    );
}

void MainWindow::onPlaybackStarted() {
    m_actPlayPause->setText("Pause");
    m_statusText->setText("Playing");
    updateStatusBar();
}

void MainWindow::onPlaybackStopped() {
    m_actPlayPause->setText("Play");
    m_statusText->setText("Stopped");
    updateStatusBar();
}

void MainWindow::onPlaybackPaused(bool paused) {
    m_actPlayPause->setText(paused ? "Play" : "Pause");
    m_statusText->setText(paused ? "Paused" : "Playing");
}

void MainWindow::onSpeedChanged(double speed) {
    m_statusSpeed->setText(QString("%1x").arg(speed, 0, 'f', 2));
}

void MainWindow::updateStatusBar() {
    if (!m_engine) return;
    MediaMetadata meta = m_engine->getMetadata();
    if (meta.videoWidth > 0) {
        m_statusMediaInfo->setText(QString("%1x%2 • %3 • VA-API")
            .arg(meta.videoWidth)
            .arg(meta.videoHeight)
            .arg(meta.videoCodec.toUpper()));
    } else if (!meta.audioCodec.isEmpty()) {
        m_statusMediaInfo->setText(QString("Audio: %1").arg(meta.audioCodec.toUpper()));
    } else {
        m_statusMediaInfo->setText("");
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape && m_isFullscreen) {
        onToggleFullscreen();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        QWidget *focused = QApplication::focusWidget();

        // If the user is actively typing in a text field, pass the key to it
        if (focused && (
            qobject_cast<QLineEdit *>(focused) ||
            qobject_cast<QTextEdit *>(focused) ||
            qobject_cast<QPlainTextEdit *>(focused)
        )) {
            return QMainWindow::eventFilter(watched, event);
        }

        int key = keyEvent->key();
        Qt::KeyboardModifiers mods = keyEvent->modifiers();

        // 1. Seeking (Left / Right Arrow Keys)
        if (key == Qt::Key_Left) {
            if (mods & Qt::ShiftModifier) {
                onJumpRelative(-3.0);
            } else if ((mods & Qt::ControlModifier) && (mods & Qt::AltModifier)) {
                onJumpRelative(-300.0);
            } else if (mods & Qt::ControlModifier) {
                onJumpRelative(-60.0);
            } else {
                onJumpRelative(-10.0);
            }
            return true;
        }

        if (key == Qt::Key_Right) {
            if (mods & Qt::ShiftModifier) {
                onJumpRelative(3.0);
            } else if ((mods & Qt::ControlModifier) && (mods & Qt::AltModifier)) {
                onJumpRelative(300.0);
            } else if (mods & Qt::ControlModifier) {
                onJumpRelative(60.0);
            } else {
                onJumpRelative(10.0);
            }
            return true;
        }

        // 2. Volume (Up / Down Arrow Keys)
        if (key == Qt::Key_Up) {
            onVolumeDelta(5.0);
            return true;
        }

        if (key == Qt::Key_Down) {
            onVolumeDelta(-5.0);
            return true;
        }

        // 3. Play / Pause (Space)
        if (key == Qt::Key_Space) {
            onTogglePlayPause();
            return true;
        }

        // 4. Mute (M)
        if (key == Qt::Key_M && !(mods & Qt::ControlModifier)) {
            onToggleMute();
            return true;
        }

        // 5. Fullscreen (F or F11)
        if ((key == Qt::Key_F && !(mods & Qt::ControlModifier)) || key == Qt::Key_F11) {
            onToggleFullscreen();
            return true;
        }

        // 6. Escape: Exit fullscreen if active
        if (key == Qt::Key_Escape && m_isFullscreen) {
            onToggleFullscreen();
            return true;
        }

        // 7. Stop (S)
        if (key == Qt::Key_S && !(mods & (Qt::ControlModifier | Qt::ShiftModifier))) {
            onStop();
            return true;
        }

        // 8. Snapshot (Shift+S)
        if (key == Qt::Key_S && (mods & Qt::ShiftModifier)) {
            onTakeSnapshot();
            return true;
        }

        // 9. Playlist Navigation: N (Next) & P (Previous)
        if (key == Qt::Key_N && !(mods & Qt::ControlModifier)) {
            onNext();
            return true;
        }

        if (key == Qt::Key_P && !(mods & Qt::ControlModifier)) {
            onPrevious();
            return true;
        }

        // 10. Frame Step (E)
        if (key == Qt::Key_E && !(mods & Qt::ControlModifier)) {
            onFrameStep();
            return true;
        }

        // 11. Speed: [ (slower), ] (faster), = or Backspace (normal 1.0x)
        if (key == Qt::Key_BracketLeft) {
            onSpeedSlower();
            return true;
        }

        if (key == Qt::Key_BracketRight) {
            onSpeedFaster();
            return true;
        }

        if (key == Qt::Key_Equal || key == Qt::Key_Backspace) {
            onSpeedNormal();
            return true;
        }

        // 12. Aspect Ratio (A)
        if (key == Qt::Key_A && !(mods & Qt::ControlModifier)) {
            onCycleAspectRatio();
            return true;
        }

        // 13. Subtitles (V)
        if (key == Qt::Key_V && !(mods & Qt::ControlModifier)) {
            onCycleSubtitleTrack();
            return true;
        }

        // 14. Audio Track (B)
        if (key == Qt::Key_B && !(mods & Qt::ControlModifier)) {
            onCycleAudioTrack();
            return true;
        }

        // 15. Deinterlace (D)
        if (key == Qt::Key_D && !(mods & Qt::ControlModifier)) {
            onToggleDeinterlace();
            return true;
        }

        // 16. Audio Delay: G (-50ms) / H (+50ms)
        if (key == Qt::Key_G && !(mods & Qt::ControlModifier)) {
            onAudioDelayDelta(-0.05);
            return true;
        }

        if (key == Qt::Key_H && !(mods & Qt::ControlModifier)) {
            onAudioDelayDelta(0.05);
            return true;
        }

        // 17. Subtitle Delay: J / Z (-50ms), K / X (+50ms)
        if ((key == Qt::Key_J || key == Qt::Key_Z) && !(mods & Qt::ControlModifier)) {
            onSubtitleDelayDelta(-0.05);
            return true;
        }

        if ((key == Qt::Key_K || key == Qt::Key_X) && !(mods & Qt::ControlModifier)) {
            onSubtitleDelayDelta(0.05);
            return true;
        }

        // 18. Dialogs & Shortcuts:
        if (key == Qt::Key_E && (mods & Qt::ControlModifier)) {
            onShowEffects();
            return true;
        }

        if (key == Qt::Key_L && (mods & Qt::ControlModifier)) {
            onTogglePlaylist();
            return true;
        }

        if (key == Qt::Key_P && (mods & Qt::ControlModifier)) {
            onShowPreferences();
            return true;
        }

        if (key == Qt::Key_I && (mods & Qt::ControlModifier)) {
            onShowMediaInfo();
            return true;
        }

        if (key == Qt::Key_O && (mods & Qt::ControlModifier) && !(mods & Qt::ShiftModifier)) {
            onOpenFile();
            return true;
        }

        if (key == Qt::Key_O && (mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier)) {
            onOpenMultipleFiles();
            return true;
        }

        if (key == Qt::Key_N && (mods & Qt::ControlModifier)) {
            onOpenNetworkStream();
            return true;
        }

        if (key == Qt::Key_V && (mods & Qt::ControlModifier)) {
            onOpenClipboardLocation();
            return true;
        }

        if (key == Qt::Key_Q && (mods & Qt::ControlModifier)) {
            close();
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showVideoContextMenu(const QPoint &globalPos) {
    QMenu menu(this);
    menu.setObjectName("OrionVideoContextMenu");

    // Media
    auto *mediaMenu = menu.addMenu("Media");
    mediaMenu->addAction("Open File...", QKeySequence("Ctrl+O"), this, &MainWindow::onOpenFile);
    mediaMenu->addAction("Open Multiple Files...", QKeySequence("Ctrl+Shift+O"), this, &MainWindow::onOpenMultipleFiles);
    mediaMenu->addAction("Open Folder...", QKeySequence("Ctrl+F"), this, &MainWindow::onOpenFolder);
    mediaMenu->addAction("Open Network Stream...", QKeySequence("Ctrl+N"), this, &MainWindow::onOpenNetworkStream);
    mediaMenu->addAction("Open Location from Clipboard", QKeySequence("Ctrl+V"), this, &MainWindow::onOpenClipboardLocation);

    // Playback
    auto *playMenu = menu.addMenu("Playback");
    playMenu->addAction(m_engine && m_engine->isPaused() ? "Play" : "Pause", Qt::Key_Space, this, &MainWindow::onTogglePlayPause);
    playMenu->addAction("Stop", Qt::Key_S, this, &MainWindow::onStop);
    playMenu->addAction("Previous", Qt::Key_P, this, &MainWindow::onPrevious);
    playMenu->addAction("Next", Qt::Key_N, this, &MainWindow::onNext);
    playMenu->addSeparator();

    auto *jumpMenu = playMenu->addMenu("Jump");
    jumpMenu->addAction("Very Short Forward (+3s)", QKeySequence("Shift+Right"), this, [this]() { onJumpRelative(3.0); });
    jumpMenu->addAction("Very Short Backward (-3s)", QKeySequence("Shift+Left"), this, [this]() { onJumpRelative(-3.0); });
    jumpMenu->addAction("Short Forward (+10s)", Qt::Key_Right, this, [this]() { onJumpRelative(10.0); });
    jumpMenu->addAction("Short Backward (-10s)", Qt::Key_Left, this, [this]() { onJumpRelative(-10.0); });
    jumpMenu->addAction("Medium Forward (+1 min)", QKeySequence("Ctrl+Right"), this, [this]() { onJumpRelative(60.0); });
    jumpMenu->addAction("Medium Backward (-1 min)", QKeySequence("Ctrl+Left"), this, [this]() { onJumpRelative(-60.0); });
    jumpMenu->addAction("Long Forward (+5 min)", QKeySequence("Ctrl+Alt+Right"), this, [this]() { onJumpRelative(300.0); });
    jumpMenu->addAction("Long Backward (-5 min)", QKeySequence("Ctrl+Alt+Left"), this, [this]() { onJumpRelative(-300.0); });

    auto *speedMenu = playMenu->addMenu("Speed");
    speedMenu->addAction("Faster", Qt::Key_BracketRight, this, &MainWindow::onSpeedFaster);
    speedMenu->addAction("Slower", Qt::Key_BracketLeft, this, &MainWindow::onSpeedSlower);
    speedMenu->addAction("Normal Speed", Qt::Key_Equal, this, &MainWindow::onSpeedNormal);

    playMenu->addSeparator();
    playMenu->addAction("Step Forward Frame", Qt::Key_E, this, &MainWindow::onFrameStep);

    // Audio
    auto *audioMenu = menu.addMenu("Audio");
    auto *audioTracksMenu = audioMenu->addMenu("Audio Track");
    if (m_engine) {
        auto tracks = m_engine->getTracks();
        auto *ag = new QActionGroup(audioTracksMenu);
        auto *disableAct = audioTracksMenu->addAction("Disable", this, [this]() {
            if (m_engine) m_engine->setAudioTrack(-1);
        });
        disableAct->setCheckable(true);
        ag->addAction(disableAct);

        bool anySelected = false;
        for (const auto &t : tracks) {
            if (t.type == "audio") {
                QString label = t.title.isEmpty() ?
                    QString("Track %1 [%2 - %3]").arg(t.id).arg(t.language.isEmpty() ? "und" : t.language).arg(t.codec) :
                    t.title;
                auto *act = audioTracksMenu->addAction(label, this, [this, id = t.id]() {
                    if (m_engine) m_engine->setAudioTrack(id);
                });
                act->setCheckable(true);
                if (t.isSelected) { act->setChecked(true); anySelected = true; }
                ag->addAction(act);
            }
        }
        if (!anySelected) disableAct->setChecked(true);
    }
    audioMenu->addSeparator();
    audioMenu->addAction("Volume Up (+5%)", Qt::Key_Up, this, [this]() { onVolumeDelta(5.0); });
    audioMenu->addAction("Volume Down (-5%)", Qt::Key_Down, this, [this]() { onVolumeDelta(-5.0); });
    audioMenu->addAction("Mute", Qt::Key_M, this, &MainWindow::onToggleMute);
    audioMenu->addSeparator();
    audioMenu->addAction("Audio Delay (+50ms)", Qt::Key_H, this, [this]() { onAudioDelayDelta(0.05); });
    audioMenu->addAction("Audio Delay (-50ms)", Qt::Key_G, this, [this]() { onAudioDelayDelta(-0.05); });

    // Video
    auto *videoMenu = menu.addMenu("Video");
    videoMenu->addAction("Fullscreen", Qt::Key_F11, this, &MainWindow::onToggleFullscreen);
    auto *topAct = videoMenu->addAction("Always on Top", this, &MainWindow::onToggleAlwaysOnTop);
    topAct->setCheckable(true);
    topAct->setChecked(m_isAlwaysOnTop);
    videoMenu->addSeparator();

    auto *aspectMenu = videoMenu->addMenu("Aspect Ratio");
    const QStringList ratios = {"default", "16:9", "4:3", "1:1", "16:10", "2.21:1", "2.35:1"};
    for (const auto &r : ratios) {
        auto *act = aspectMenu->addAction(r == "default" ? "Default" : r, this, [this, r]() { onSetAspectRatio(r); });
        act->setCheckable(true);
        if (m_engine && m_engine->currentAspectRatio() == r) act->setChecked(true);
    }

    auto *deintMenu = videoMenu->addMenu("Deinterlace");
    auto *dOff = deintMenu->addAction("Off", this, [this]() { if (m_engine) m_engine->setDeinterlace(false); });
    auto *dOn = deintMenu->addAction("On", this, [this]() { if (m_engine) m_engine->setDeinterlace(true); });
    dOff->setCheckable(true); dOn->setCheckable(true);
    if (m_engine && m_engine->isDeinterlaceEnabled()) dOn->setChecked(true); else dOff->setChecked(true);

    videoMenu->addSeparator();
    videoMenu->addAction("Take Snapshot", QKeySequence("Shift+S"), this, &MainWindow::onTakeSnapshot);

    // Subtitle
    auto *subMenu = menu.addMenu("Subtitle");
    auto *subTracksMenu = subMenu->addMenu("Subtitle Track");
    if (m_engine) {
        auto tracks = m_engine->getTracks();
        auto *ag = new QActionGroup(subTracksMenu);
        auto *disableAct = subTracksMenu->addAction("Disable", this, [this]() {
            if (m_engine) m_engine->setSubtitleTrack(-1);
        });
        disableAct->setCheckable(true);
        ag->addAction(disableAct);

        bool anySelected = false;
        for (const auto &t : tracks) {
            if (t.type == "sub") {
                QString label = t.title.isEmpty() ?
                    QString("Track %1 [%2 - %3]").arg(t.id).arg(t.language.isEmpty() ? "und" : t.language).arg(t.codec) :
                    t.title;
                auto *act = subTracksMenu->addAction(label, this, [this, id = t.id]() {
                    if (m_engine) m_engine->setSubtitleTrack(id);
                });
                act->setCheckable(true);
                if (t.isSelected) { act->setChecked(true); anySelected = true; }
                ag->addAction(act);
            }
        }
        if (!anySelected) disableAct->setChecked(true);
    }
    subMenu->addAction("Add Subtitle File...", this, &MainWindow::onAddSubtitleFile);
    subMenu->addSeparator();
    subMenu->addAction("Subtitle Delay (+50ms)", Qt::Key_K, this, [this]() { onSubtitleDelayDelta(0.05); });
    subMenu->addAction("Subtitle Delay (-50ms)", Qt::Key_J, this, [this]() { onSubtitleDelayDelta(-0.05); });

    // Tools & View
    menu.addSeparator();
    auto *toolsMenu = menu.addMenu("Tools");
    toolsMenu->addAction("Effects and Filters", QKeySequence("Ctrl+E"), this, &MainWindow::onShowEffects);
    toolsMenu->addAction("Media Information", QKeySequence("Ctrl+I"), this, &MainWindow::onShowMediaInfo);
    toolsMenu->addAction("Preferences", QKeySequence("Ctrl+P"), this, &MainWindow::onShowPreferences);

    menu.addAction("Playlist", QKeySequence("Ctrl+L"), this, &MainWindow::onTogglePlaylist);
    menu.addSeparator();
    menu.addAction("Quit", QKeySequence("Ctrl+Q"), this, &QWidget::close);

    menu.exec(globalPos);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_engine) m_engine->stop();
    QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        openMedia(urls.first().toLocalFile());
        event->acceptProposedAction();
    }
}
