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

#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
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
#include <QIcon>
#include <cmath>

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
    mediaMenu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

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
    jumpMenu->addAction("Short Forward (+10 sec)", QKeySequence("Alt+Right"), this, [this]() { onJumpRelative(10.0); });
    jumpMenu->addAction("Short Backward (-10 sec)", QKeySequence("Alt+Left"), this, [this]() { onJumpRelative(-10.0); });
    jumpMenu->addAction("Medium Forward (+1 min)", QKeySequence("Ctrl+Right"), this, [this]() { onJumpRelative(60.0); });
    jumpMenu->addAction("Medium Backward (-1 min)", QKeySequence("Ctrl+Left"), this, [this]() { onJumpRelative(-60.0); });
    jumpMenu->addAction("Long Forward (+5 min)", QKeySequence("Ctrl+Alt+Right"), this, [this]() { onJumpRelative(300.0); });
    jumpMenu->addAction("Long Backward (-5 min)", QKeySequence("Ctrl+Alt+Left"), this, [this]() { onJumpRelative(-300.0); });

    playMenu->addSeparator();
    playMenu->addAction("Step Forward Frame-by-Frame", Qt::Key_E, this, &MainWindow::onFrameStep);

    // Audio menu
    auto *audioMenu = mb->addMenu("&Audio");
    audioMenu->addAction("Volume &Up (+5%)", QKeySequence("Ctrl+Up"), this, [this]() { onVolumeDelta(5.0); });
    audioMenu->addAction("Volume &Down (-5%)", QKeySequence("Ctrl+Down"), this, [this]() { onVolumeDelta(-5.0); });
    audioMenu->addAction("&Mute", Qt::Key_M, this, [this]() { if (m_engine) m_engine->toggleMute(); });
    audioMenu->addSeparator();
    audioMenu->addAction("Audio &Track Delay (+50ms)", Qt::Key_K, this, [this]() {
        if (m_engine) m_engine->setAudioDelay(m_engine->audioDelay() + 0.05);
    });
    audioMenu->addAction("Audio Track Delay (-50ms)", Qt::Key_J, this, [this]() {
        if (m_engine) m_engine->setAudioDelay(m_engine->audioDelay() - 0.05);
    });

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

    auto *deintMenu = videoMenu->addMenu("&Deinterlace");
    deintMenu->addAction("Off", this, [this]() { if (m_engine) m_engine->setDeinterlace(false); });
    deintMenu->addAction("On", this, [this]() { if (m_engine) m_engine->setDeinterlace(true); });

    videoMenu->addSeparator();
    videoMenu->addAction("Take &Snapshot", QKeySequence("Shift+S"), this, &MainWindow::onTakeSnapshot);

    // Subtitle menu
    auto *subMenu = mb->addMenu("&Subtitle");
    subMenu->addAction("&Add Subtitle File...", this, &MainWindow::onAddSubtitleFile);
    subMenu->addSeparator();
    subMenu->addAction("Subtitle Delay (+50ms)", Qt::Key_X, this, [this]() {
        if (m_engine) m_engine->setSubtitleDelay(m_engine->subtitleDelay() + 0.05);
    });
    subMenu->addAction("Subtitle Delay (-50ms)", Qt::Key_Z, this, [this]() {
        if (m_engine) m_engine->setSubtitleDelay(m_engine->subtitleDelay() - 0.05);
    });

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
    if (m_engine) m_engine->togglePause();
}

void MainWindow::onStop() {
    if (m_engine) {
        m_engine->stop();
        m_statusText->setText("Stopped");
    }
}

void MainWindow::onNext() {
    QString nextTrack = m_playlistView->playNext();
    if (!nextTrack.isEmpty()) openMedia(nextTrack);
}

void MainWindow::onPrevious() {
    QString prevTrack = m_playlistView->playPrevious();
    if (!prevTrack.isEmpty()) openMedia(prevTrack);
}

void MainWindow::onSpeedFaster() {
    if (m_engine) m_engine->setSpeed(m_engine->speed() + 0.1);
}

void MainWindow::onSpeedSlower() {
    if (m_engine) m_engine->setSpeed(std::max(0.25, m_engine->speed() - 0.1));
}

void MainWindow::onSpeedNormal() {
    if (m_engine) m_engine->setSpeed(1.0);
}

void MainWindow::onJumpRelative(double seconds) {
    if (m_engine) m_engine->seekRelative(seconds);
}

void MainWindow::onFrameStep() {
    if (m_engine) m_engine->frameStep();
}

void MainWindow::onAddSubtitleFile() {
    QString sub = QFileDialog::getOpenFileName(
        this, "Select Subtitle File", QString(),
        "Subtitle Files (*.srt *.ass *.ssa *.vtt *.sub);;All Files (*)"
    );
    if (!sub.isEmpty() && m_engine) {
        m_engine->loadSubtitleFile(sub);
        statusBar()->showMessage(QString("Loaded Subtitle: %1").arg(QFileInfo(sub).fileName()), 3000);
    }
}

void MainWindow::onSetAspectRatio(const QString &ratio) {
    if (m_engine) m_engine->setAspectRatio(ratio);
}

void MainWindow::onTakeSnapshot() {
    if (m_engine) {
        m_engine->takeScreenshot();
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
    } else {
        menuBar()->show();
        m_toolbar->show();
        statusBar()->show();
        showNormal();
    }
}

void MainWindow::onToggleAlwaysOnTop() {
    m_isAlwaysOnTop = !m_isAlwaysOnTop;
    setWindowFlag(Qt::WindowStaysOnTopHint, m_isAlwaysOnTop);
    show();
    m_actAlwaysOnTop->setChecked(m_isAlwaysOnTop);
}

void MainWindow::onVolumeDelta(double delta) {
    if (m_engine) {
        double newVol = std::clamp(m_engine->volume() + delta, 0.0, 200.0);
        m_engine->setVolume(newVol);
    }
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
    QMessageBox::about(this, "About OrionPlayer",
        "<h3>OrionPlayer 1.0</h3>"
        "<p>Professional Open-Source Media Player for Linux.</p>"
        "<p>Designed with the full versatility and professional controls of VLC Media Player, "
        "powered by Intel VA-API zero-copy hardware acceleration and libmpv engine.</p>"
        "<p><b>Author:</b> Abhinav Santhosh (<a href='https://github.com/abhinavsanthoshpp'>@abhinavsanthoshpp</a>)<br>"
        "<b>License:</b> GNU General Public License v3.0 (GPL-3.0)</p>"
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
