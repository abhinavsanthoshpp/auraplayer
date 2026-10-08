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
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("AuraPlayer — VLC-Grade Professional Media Player");
    resize(1000, 680);
    setAcceptDrops(true);

    m_engine = new AuraEngine(this);
    m_engine->initialize();

    createCentralLayout();
    createMenuBar();
    applyVlcTheme();

    connect(m_engine, &AuraEngine::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(m_engine, &AuraEngine::playbackStopped, this, &MainWindow::onPlaybackStopped);
    connect(m_engine, &AuraEngine::playbackPaused, this, &MainWindow::onPlaybackPaused);
    connect(m_engine, &AuraEngine::speedChanged, this, &MainWindow::onSpeedChanged);
    connect(m_engine, &AuraEngine::videoReconfigured, this, [this](int w, int h) {
        Q_UNUSED(w); Q_UNUSED(h);
        updateStatusBar();
    });
}

MainWindow::~MainWindow() {
}

void MainWindow::createCentralLayout() {
    auto *centralContainer = new QWidget(this);
    auto *layout = new QVBoxLayout(centralContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Stacked widget: Index 0 = Video canvas, Index 1 = Playlist view
    m_stackedWidget = new QStackedWidget(centralContainer);

    m_videoWidget = new AuraVideoWidget(m_engine, m_stackedWidget);
    m_playlistView = new AuraPlaylistView(m_stackedWidget);

    m_stackedWidget->addWidget(m_videoWidget);   // 0
    m_stackedWidget->addWidget(m_playlistView);  // 1
    m_stackedWidget->setCurrentIndex(0);

    layout->addWidget(m_stackedWidget, 1);

    // VLC Toolbar at the bottom
    m_toolbar = new AuraVlcToolbar(m_engine, centralContainer);
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
    connect(m_toolbar, &AuraVlcToolbar::playPauseClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_toolbar, &AuraVlcToolbar::stopClicked, this, &MainWindow::onStop);
    connect(m_toolbar, &AuraVlcToolbar::prevClicked, this, &MainWindow::onPrevious);
    connect(m_toolbar, &AuraVlcToolbar::nextClicked, this, &MainWindow::onNext);
    connect(m_toolbar, &AuraVlcToolbar::fullscreenClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_toolbar, &AuraVlcToolbar::extendedSettingsClicked, this, &MainWindow::onShowEffects);
    connect(m_toolbar, &AuraVlcToolbar::playlistClicked, this, &MainWindow::onTogglePlaylist);
    connect(m_toolbar, &AuraVlcToolbar::snapshotClicked, this, &MainWindow::onTakeSnapshot);
    connect(m_toolbar, &AuraVlcToolbar::frameStepClicked, this, &MainWindow::onFrameStep);
    connect(m_toolbar, &AuraVlcToolbar::loopModeChanged, this, [this](AuraVlcToolbar::LoopMode mode) {
        if (mode == AuraVlcToolbar::LoopMode::RepeatAll) {
            m_playlistView->setLoopMode(AuraPlaylistView::LoopMode::RepeatAll);
        } else if (mode == AuraVlcToolbar::LoopMode::RepeatOne) {
            m_playlistView->setLoopMode(AuraPlaylistView::LoopMode::RepeatOne);
        } else {
            m_playlistView->setLoopMode(AuraPlaylistView::LoopMode::None);
        }
    });
    connect(m_toolbar, &AuraVlcToolbar::shuffleClicked, this, [this]() {
        m_playlistView->shuffle();
    });

    // Video canvas event connections
    connect(m_videoWidget, &AuraVideoWidget::doubleClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_videoWidget, &AuraVideoWidget::singleClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_videoWidget, &AuraVideoWidget::fileDropped, this, &MainWindow::openMedia);
    connect(m_videoWidget, &AuraVideoWidget::wheelScrolled, this, [this](int delta) {
        onVolumeDelta(delta > 0 ? 5.0 : -5.0);
    });

    // Playlist connections
    connect(m_playlistView, &AuraPlaylistView::trackSelected, this, &MainWindow::openMedia);
}

void MainWindow::createMenuBar() {
    auto *mb = menuBar();

    // ================= 1. MEDIA MENU =================
    auto *mediaMenu = mb->addMenu("&Media");
    mediaMenu->addAction("&Open File...", this, &MainWindow::onOpenFile, QKeySequence::Open);
    mediaMenu->addAction("Open &Multiple Files...", this, &MainWindow::onOpenMultipleFiles, QKeySequence("Ctrl+Shift+O"));
    mediaMenu->addAction("Open &Folder...", this, &MainWindow::onOpenFolder, QKeySequence("Ctrl+F"));
    mediaMenu->addAction("Open &Network Stream...", this, &MainWindow::onOpenNetworkStream, QKeySequence("Ctrl+N"));
    mediaMenu->addAction("Open &Location from Clipboard...", this, &MainWindow::onOpenClipboardLocation, QKeySequence("Ctrl+V"));
    mediaMenu->addSeparator();
    mediaMenu->addAction("&Quit", this, &QWidget::close, QKeySequence::Quit);

    // ================= 2. PLAYBACK MENU =================
    auto *playMenu = mb->addMenu("&Playback");
    m_actPlayPause = playMenu->addAction("Play", this, &MainWindow::onTogglePlayPause, Qt::Key_Space);
    playMenu->addAction("&Stop", this, &MainWindow::onStop, Qt::Key_S);
    playMenu->addAction("&Previous", this, &MainWindow::onPrevious, Qt::Key_P);
    playMenu->addAction("&Next", this, &MainWindow::onNext, Qt::Key_N);
    playMenu->addSeparator();

    auto *speedMenu = playMenu->addMenu("&Speed");
    speedMenu->addAction("&Faster", this, &MainWindow::onSpeedFaster, Qt::Key_BracketRight);
    speedMenu->addAction("&Slower", this, &MainWindow::onSpeedSlower, Qt::Key_BracketLeft);
    speedMenu->addAction("&Normal Speed", this, &MainWindow::onSpeedNormal, Qt::Key_Equal);

    auto *jumpMenu = playMenu->addMenu("&Jump");
    jumpMenu->addAction("Very Short Forward (+3 sec)", this, [this]() { onJumpRelative(3.0); }, QKeySequence("Shift+Right"));
    jumpMenu->addAction("Very Short Backward (-3 sec)", this, [this]() { onJumpRelative(-3.0); }, QKeySequence("Shift+Left"));
    jumpMenu->addAction("Short Forward (+10 sec)", this, [this]() { onJumpRelative(10.0); }, QKeySequence("Alt+Right"));
    jumpMenu->addAction("Short Backward (-10 sec)", this, [this]() { onJumpRelative(-10.0); }, QKeySequence("Alt+Left"));
    jumpMenu->addAction("Medium Forward (+1 min)", this, [this]() { onJumpRelative(60.0); }, QKeySequence("Ctrl+Right"));
    jumpMenu->addAction("Medium Backward (-1 min)", this, [this]() { onJumpRelative(-60.0); }, QKeySequence("Ctrl+Left"));
    jumpMenu->addAction("Long Forward (+5 min)", this, [this]() { onJumpRelative(300.0); }, QKeySequence("Ctrl+Alt+Right"));
    jumpMenu->addAction("Long Backward (-5 min)", this, [this]() { onJumpRelative(-300.0); }, QKeySequence("Ctrl+Alt+Left"));

    playMenu->addSeparator();
    playMenu->addAction("Step Forward Frame-by-Frame", this, &MainWindow::onFrameStep, Qt::Key_E);

    // ================= 3. AUDIO MENU =================
    auto *audioMenu = mb->addMenu("&Audio");
    audioMenu->addAction("Volume &Up (+5%)", this, [this]() { onVolumeDelta(5.0); }, QKeySequence("Ctrl+Up"));
    audioMenu->addAction("Volume &Down (-5%)", this, [this]() { onVolumeDelta(-5.0); }, QKeySequence("Ctrl+Down"));
    audioMenu->addAction("&Mute", this, [this]() { if (m_engine) m_engine->toggleMute(); }, Qt::Key_M);
    audioMenu->addSeparator();
    audioMenu->addAction("Audio &Track Delay (+50ms)", this, [this]() {
        if (m_engine) m_engine->setAudioDelay(m_engine->audioDelay() + 0.05);
    }, Qt::Key_K);
    audioMenu->addAction("Audio Track Delay (-50ms)", this, [this]() {
        if (m_engine) m_engine->setAudioDelay(m_engine->audioDelay() - 0.05);
    }, Qt::Key_J);

    // ================= 4. VIDEO MENU =================
    auto *videoMenu = mb->addMenu("&Video");
    m_actFullscreen = videoMenu->addAction("&Fullscreen", this, &MainWindow::onToggleFullscreen, Qt::Key_F11);
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
    videoMenu->addAction("Take &Snapshot", this, &MainWindow::onTakeSnapshot, QKeySequence("Shift+S"));

    // ================= 5. SUBTITLE MENU =================
    auto *subMenu = mb->addMenu("&Subtitle");
    subMenu->addAction("&Add Subtitle File...", this, &MainWindow::onAddSubtitleFile);
    subMenu->addSeparator();
    subMenu->addAction("Subtitle Delay (+50ms)", this, [this]() {
        if (m_engine) m_engine->setSubtitleDelay(m_engine->subtitleDelay() + 0.05);
    }, Qt::Key_X);
    subMenu->addAction("Subtitle Delay (-50ms)", this, [this]() {
        if (m_engine) m_engine->setSubtitleDelay(m_engine->subtitleDelay() - 0.05);
    }, Qt::Key_Z);

    // ================= 6. TOOLS MENU =================
    auto *toolsMenu = mb->addMenu("&Tools");
    toolsMenu->addAction("&Effects and Filters", this, &MainWindow::onShowEffects, QKeySequence("Ctrl+E"));
    toolsMenu->addAction("&Media Information", this, &MainWindow::onShowMediaInfo, QKeySequence("Ctrl+I"));
    toolsMenu->addSeparator();
    toolsMenu->addAction("&Preferences", this, &MainWindow::onShowPreferences, QKeySequence("Ctrl+P"));

    // ================= 7. VIEW MENU =================
    auto *viewMenu = mb->addMenu("&View");
    viewMenu->addAction("&Playlist", this, &MainWindow::onTogglePlaylist, QKeySequence("Ctrl+L"));
    m_actAdvanced = viewMenu->addAction("&Advanced Controls", this, &MainWindow::onToggleAdvancedControls);
    m_actAdvanced->setCheckable(true);
    m_actAdvanced->setChecked(false);
    viewMenu->addSeparator();
    viewMenu->addAction("&Status Bar", this, [this](bool checked) {
        statusBar()->setVisible(checked);
    })->setCheckable(true);

    // ================= 8. HELP MENU =================
    auto *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About AuraPlayer", this, &MainWindow::onAbout, QKeySequence("Shift+F1"));
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

    m_playlistView->addFile(path);
    m_engine->loadFile(path);
    m_engine->play();

    m_stackedWidget->setCurrentIndex(0); // Switch to video display
    setWindowTitle(QString("AuraPlayer — %1").arg(QFileInfo(path).fileName()));
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
    if (!m_streamDialog) m_streamDialog = new AuraStreamDialog(this);
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
    if (!m_effectsDialog) m_effectsDialog = new AuraEffectsDialog(m_engine, this);
    m_effectsDialog->refreshFromEngine();
    m_effectsDialog->show();
    m_effectsDialog->raise();
    m_effectsDialog->activateWindow();
}

void MainWindow::onShowMediaInfo() {
    if (!m_mediaInfoDialog) m_mediaInfoDialog = new AuraMediaInfoDialog(m_engine, this);
    m_mediaInfoDialog->refresh();
    m_mediaInfoDialog->show();
    m_mediaInfoDialog->raise();
    m_mediaInfoDialog->activateWindow();
}

void MainWindow::onShowPreferences() {
    if (!m_prefsDialog) m_prefsDialog = new AuraPreferencesDialog(m_engine, this);
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
    QMessageBox::about(this, "About AuraPlayer",
        "<h3>AuraPlayer 1.0</h3>"
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
