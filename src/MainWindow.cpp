#include "MainWindow.h"
#include "AuraPlaylist.h"
#include "AuraEqualizerDialog.h"
#include "AuraMediaInfoDialog.h"
#include "AuraStreamDialog.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QFile>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QScreen>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("AuraPlayer — Ultra-Fast Media Player");
    resize(1100, 720);

    // Initialize media engine
    m_engine = new AuraEngine(this);
    if (!m_engine->initialize()) {
        QMessageBox::critical(this, "Engine Error", "Failed to initialize AuraEngine hardware accelerated backend.");
    }

    createCentralLayout();
    createMenuBar();
    setupShortcuts();
    applyTheme();

    // Auto-hide controls timer
    m_autoHideTimer.setInterval(2500);
    connect(&m_autoHideTimer, &QTimer::timeout, this, &MainWindow::onAutoHideTimeout);

    // OSD timer
    m_osdTimer.setSingleShot(true);
    connect(&m_osdTimer, &QTimer::timeout, this, [this]() {
        m_osdLabel->hide();
    });

    // Start auto-hide timer once media plays
    connect(m_engine, &AuraEngine::playbackStarted, this, [this]() {
        m_autoHideTimer.start();
        setWindowTitle(QString("AuraPlayer — %1").arg(QFileInfo(m_engine->currentFilePath()).fileName()));
    });
    connect(m_engine, &AuraEngine::playbackStopped, this, [this]() {
        m_autoHideTimer.stop();
        m_controls->show();
        setWindowTitle("AuraPlayer — Ultra-Fast Media Player");
    });
}

MainWindow::~MainWindow() {
}

void MainWindow::createCentralLayout() {
    auto *centralContainer = new QWidget(this);
    auto *containerLayout = new QHBoxLayout(centralContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Video & Controls column
    auto *videoControlsCol = new QWidget(centralContainer);
    auto *colLayout = new QVBoxLayout(videoControlsCol);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->setSpacing(0);

    // Video widget
    m_videoWidget = new AuraVideoWidget(m_engine, videoControlsCol);
    colLayout->addWidget(m_videoWidget, 1);

    // Controls bar
    m_controls = new AuraControls(m_engine, videoControlsCol);
    colLayout->addWidget(m_controls, 0);

    // Playlist drawer
    m_playlistWidget = new AuraPlaylist(centralContainer);
    m_playlistWidget->hide(); // Initially closed

    containerLayout->addWidget(videoControlsCol, 1);
    containerLayout->addWidget(m_playlistWidget, 0);

    setCentralWidget(centralContainer);

    // Floating OSD overlay on top of video widget
    m_osdLabel = new QLabel(m_videoWidget);
    m_osdLabel->setObjectName("AuraOsdLabel");
    m_osdLabel->setStyleSheet(
        "background: rgba(13, 17, 23, 0.88); color: #00e5ff; font-size: 16px; font-weight: bold;"
        "border: 1px solid #30363d; border-radius: 8px; padding: 10px 20px;"
    );
    m_osdLabel->setAlignment(Qt::AlignCenter);
    m_osdLabel->hide();

    // Wiring video widget events
    connect(m_videoWidget, &AuraVideoWidget::doubleClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_videoWidget, &AuraVideoWidget::singleClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_videoWidget, &AuraVideoWidget::userActivity, this, &MainWindow::onUserActivity);
    connect(m_videoWidget, &AuraVideoWidget::fileDropped, this, &MainWindow::openMedia);
    connect(m_videoWidget, &AuraVideoWidget::wheelScrolled, this, [this](int delta) {
        onVolumeDelta(delta > 0 ? 5.0 : -5.0);
    });

    // Wiring controls bar
    connect(m_controls, &AuraControls::playPauseClicked, this, &MainWindow::onTogglePlayPause);
    connect(m_controls, &AuraControls::stopClicked, this, &MainWindow::onStop);
    connect(m_controls, &AuraControls::nextClicked, this, &MainWindow::onNext);
    connect(m_controls, &AuraControls::prevClicked, this, &MainWindow::onPrevious);
    connect(m_controls, &AuraControls::stepForwardClicked, this, &MainWindow::onStepForward);
    connect(m_controls, &AuraControls::stepBackClicked, this, &MainWindow::onStepBack);
    connect(m_controls, &AuraControls::fullscreenClicked, this, &MainWindow::onToggleFullscreen);
    connect(m_controls, &AuraControls::playlistToggleClicked, this, &MainWindow::onTogglePlaylist);
    connect(m_controls, &AuraControls::equalizerClicked, this, &MainWindow::onShowEqualizer);
    connect(m_controls, &AuraControls::pipToggleClicked, this, &MainWindow::onToggleAlwaysOnTop);
    connect(m_controls, &AuraControls::userInteracted, this, &MainWindow::onUserActivity);

    // Wiring playlist
    connect(m_playlistWidget, &AuraPlaylist::trackSelected, this, &MainWindow::openMedia);
}

void MainWindow::createMenuBar() {
    auto *mb = menuBar();

    // 1. File Menu
    auto *fileMenu = mb->addMenu("&Media");
    fileMenu->addAction("&Open File...", this, &MainWindow::onOpenFile, QKeySequence::Open);
    fileMenu->addAction("Open &Multiple Files...", this, &MainWindow::onOpenMultipleFiles);
    fileMenu->addAction("Open &Folder...", this, &MainWindow::onOpenFolder);
    fileMenu->addAction("Open Network &Stream...", this, &MainWindow::onOpenNetworkStream, QKeySequence("Ctrl+U"));
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", this, &QWidget::close, QKeySequence::Quit);

    // 2. Playback Menu
    auto *playMenu = mb->addMenu("&Playback");
    playMenu->addAction("Play / &Pause", this, &MainWindow::onTogglePlayPause, Qt::Key_Space);
    playMenu->addAction("&Stop", this, &MainWindow::onStop);
    playMenu->addAction("&Previous Track", this, &MainWindow::onPrevious, Qt::Key_P);
    playMenu->addAction("&Next Track", this, &MainWindow::onNext, Qt::Key_N);
    playMenu->addSeparator();
    playMenu->addAction("Seek Forward 5s", this, [this]() { onSeekRelative(5.0); }, Qt::Key_Right);
    playMenu->addAction("Seek Backward 5s", this, [this]() { onSeekRelative(-5.0); }, Qt::Key_Left);
    playMenu->addAction("Seek Forward 30s", this, [this]() { onSeekRelative(30.0); }, QKeySequence("Ctrl+Right"));
    playMenu->addAction("Seek Backward 30s", this, [this]() { onSeekRelative(-30.0); }, QKeySequence("Ctrl+Left"));
    playMenu->addSeparator();
    playMenu->addAction("Speed Up (+0.1x)", this, [this]() { onSpeedDelta(0.1); }, Qt::Key_BracketRight);
    playMenu->addAction("Speed Down (-0.1x)", this, [this]() { onSpeedDelta(-0.1); }, Qt::Key_BracketLeft);
    playMenu->addAction("Reset Speed (1.0x)", this, [this]() { if (m_engine) m_engine->setSpeed(1.0); }, Qt::Key_Backspace);

    // 3. Audio Menu
    auto *audioMenu = mb->addMenu("&Audio");
    audioMenu->addAction("Volume &Up (+5%)", this, [this]() { onVolumeDelta(5.0); }, Qt::Key_Up);
    audioMenu->addAction("Volume &Down (-5%)", this, [this]() { onVolumeDelta(-5.0); }, Qt::Key_Down);
    audioMenu->addAction("&Mute / Unmute", this, [this]() { if (m_engine) m_engine->toggleMute(); }, Qt::Key_M);
    audioMenu->addSeparator();
    audioMenu->addAction("Audio Delay +100ms", this, [this]() {
        if (m_engine) {
            double d = m_engine->audioDelay() + 0.1;
            m_engine->setAudioDelay(d);
            showOsdMessage(QString("Audio Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
    }, Qt::Key_K);
    audioMenu->addAction("Audio Delay -100ms", this, [this]() {
        if (m_engine) {
            double d = m_engine->audioDelay() - 0.1;
            m_engine->setAudioDelay(d);
            showOsdMessage(QString("Audio Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
    }, Qt::Key_J);
    audioMenu->addSeparator();
    audioMenu->addAction("Audio &Equalizer...", this, &MainWindow::onShowEqualizer, Qt::Key_E);

    // 4. Video Menu
    auto *videoMenu = mb->addMenu("&Video");
    videoMenu->addAction("Subtitle Delay +100ms", this, [this]() {
        if (m_engine) {
            double d = m_engine->subtitleDelay() + 0.1;
            m_engine->setSubtitleDelay(d);
            showOsdMessage(QString("Sub Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
    }, Qt::Key_X);
    videoMenu->addAction("Subtitle Delay -100ms", this, [this]() {
        if (m_engine) {
            double d = m_engine->subtitleDelay() - 0.1;
            m_engine->setSubtitleDelay(d);
            showOsdMessage(QString("Sub Delay: %1 ms").arg(static_cast<int>(d * 1000)));
        }
    }, Qt::Key_Z);
    videoMenu->addSeparator();
    videoMenu->addAction("Video Color Adjustments...", this, &MainWindow::onShowEqualizer, Qt::Key_C);
    videoMenu->addAction("Take &Screenshot", this, &MainWindow::onTakeScreenshot, Qt::Key_S);

    // 5. View Menu
    auto *viewMenu = mb->addMenu("&View");
    viewMenu->addAction("&Fullscreen", this, &MainWindow::onToggleFullscreen, Qt::Key_F11);
    viewMenu->addAction("Always on &Top", this, &MainWindow::onToggleAlwaysOnTop);
    viewMenu->addAction("Playlist &Drawer", this, &MainWindow::onTogglePlaylist, Qt::Key_L);
    viewMenu->addAction("Media &Information...", this, &MainWindow::onShowMediaInfo, Qt::Key_I);

    // 6. Help Menu
    auto *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About AuraPlayer", this, &MainWindow::onAbout);
}

void MainWindow::setupShortcuts() {
    // Basic global hotkeys handled in keyPressEvent
}

void MainWindow::applyTheme() {
    QFile qssFile(":/style.qss");
    if (!qssFile.exists()) {
        qssFile.setFileName("resources/style.qss");
    }
    if (qssFile.open(QFile::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }
}

void MainWindow::openMedia(const QString &path) {
    if (path.isEmpty() || !m_engine) return;

    m_playlistWidget->addFile(path);
    m_engine->loadFile(path);
    m_engine->play();
    showOsdMessage(QString("Loaded: %1").arg(QFileInfo(path).fileName()), 2000);
}

void MainWindow::onOpenFile() {
    QString file = QFileDialog::getOpenFileName(
        this, "Open Media File", QString(),
        "Media Files (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    if (!file.isEmpty()) {
        openMedia(file);
    }
}

void MainWindow::onOpenMultipleFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Open Multiple Files", QString(),
        "Media Files (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    if (!files.isEmpty()) {
        m_playlistWidget->addFiles(files);
        openMedia(files.first());
    }
}

void MainWindow::onOpenFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Folder");
    if (!dir.isEmpty()) {
        QDir directory(dir);
        QStringList nameFilters;
        nameFilters << "*.mp4" << "*.mkv" << "*.webm" << "*.avi" << "*.mov" << "*.flv" << "*.mp3" << "*.flac" << "*.wav";
        QStringList files = directory.entryList(nameFilters, QDir::Files, QDir::Name);
        QStringList fullPaths;
        for (const auto &f : files) {
            fullPaths.append(directory.absoluteFilePath(f));
        }
        if (!fullPaths.isEmpty()) {
            m_playlistWidget->addFiles(fullPaths);
            openMedia(fullPaths.first());
        }
    }
}

void MainWindow::onOpenNetworkStream() {
    if (!m_streamDialog) {
        m_streamDialog = new AuraStreamDialog(this);
    }
    if (m_streamDialog->exec() == QDialog::Accepted) {
        QString url = m_streamDialog->streamUrl();
        if (!url.isEmpty()) {
            openMedia(url);
        }
    }
}

void MainWindow::onTogglePlayPause() {
    if (m_engine) {
        m_engine->togglePause();
    }
}

void MainWindow::onStop() {
    if (m_engine) {
        m_engine->stop();
    }
}

void MainWindow::onNext() {
    QString nextTrack = m_playlistWidget->playNext();
    if (!nextTrack.isEmpty()) {
        m_engine->loadFile(nextTrack);
        m_engine->play();
    }
}

void MainWindow::onPrevious() {
    QString prevTrack = m_playlistWidget->playPrevious();
    if (!prevTrack.isEmpty()) {
        m_engine->loadFile(prevTrack);
        m_engine->play();
    }
}

void MainWindow::onStepForward() {
    if (m_engine) {
        m_engine->frameStep();
    }
}

void MainWindow::onStepBack() {
    if (m_engine) {
        m_engine->frameBackStep();
    }
}

void MainWindow::onSeekRelative(double deltaSecs) {
    if (m_engine) {
        m_engine->seekRelative(deltaSecs);
        showOsdMessage(QString("Seek: %1%2s").arg(deltaSecs > 0 ? "+" : "").arg(deltaSecs, 0, 'f', 0));
    }
}

void MainWindow::onVolumeDelta(double delta) {
    if (m_engine) {
        double newVol = std::clamp(m_engine->volume() + delta, 0.0, 200.0);
        m_engine->setVolume(newVol);
        showOsdMessage(QString("Volume: %1%").arg(static_cast<int>(newVol)));
    }
}

void MainWindow::onSpeedDelta(double delta) {
    if (m_engine) {
        double newSpd = std::clamp(m_engine->speed() + delta, 0.25, 4.0);
        m_engine->setSpeed(newSpd);
        showOsdMessage(QString("Speed: %1x").arg(newSpd, 0, 'f', 2));
    }
}

void MainWindow::onToggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    if (m_isFullscreen) {
        menuBar()->hide();
        showFullScreen();
        m_autoHideTimer.start();
    } else {
        menuBar()->show();
        m_controls->show();
        showNormal();
    }
}

void MainWindow::onToggleAlwaysOnTop() {
    m_isAlwaysOnTop = !m_isAlwaysOnTop;
    setWindowFlag(Qt::WindowStaysOnTopHint, m_isAlwaysOnTop);
    show();
    showOsdMessage(m_isAlwaysOnTop ? "Pin: Always on Top" : "Pin: Normal Window");
}

void MainWindow::onTogglePlaylist() {
    if (m_playlistWidget->isVisible()) {
        m_playlistWidget->hide();
    } else {
        m_playlistWidget->show();
    }
}

void MainWindow::onShowEqualizer() {
    if (!m_equalizerDialog) {
        m_equalizerDialog = new AuraEqualizerDialog(m_engine, this);
    }
    m_equalizerDialog->show();
    m_equalizerDialog->raise();
    m_equalizerDialog->activateWindow();
}

void MainWindow::onShowMediaInfo() {
    if (!m_mediaInfoDialog) {
        m_mediaInfoDialog = new AuraMediaInfoDialog(m_engine, this);
    }
    m_mediaInfoDialog->refresh();
    m_mediaInfoDialog->show();
    m_mediaInfoDialog->raise();
    m_mediaInfoDialog->activateWindow();
}

void MainWindow::onTakeScreenshot() {
    if (m_engine) {
        m_engine->takeScreenshot();
        showOsdMessage("📸 Screenshot saved to Pictures");
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About AuraPlayer",
        "<h3>⚡ AuraPlayer 1.0</h3>"
        "<p>The Next-Generation Ultra-Fast Open-Source Media Player for Linux.</p>"
        "<p><b>Engine:</b> libmpv Core with Hardware VA-API / NVDEC Decoders<br>"
        "<b>GUI:</b> Qt6 Obsidian Glass with Native Wayland Integration<br>"
        "<b>Author:</b> Abhinav Santhosh (abhinavsanthoshpp)<br>"
        "<b>License:</b> GPL-3.0 Open Source</p>"
        "<p>Engineered to deliver faster launch, lower RAM, and smoother playback than standard VLC.</p>"
    );
}

void MainWindow::onUserActivity() {
    if (!m_controls->isVisible()) {
        m_controls->show();
    }
    if (m_isFullscreen && menuBar()->isHidden()) {
        // keep menu bar hidden in fullscreen
    }
    m_autoHideTimer.start();
}

void MainWindow::onAutoHideTimeout() {
    if (m_engine && m_engine->currentFilePath().isEmpty()) {
        return; // Don't hide when idle
    }
    if (m_controls->underMouse() || m_playlistWidget->underMouse()) {
        return; // Don't hide while cursor is over controls
    }
    m_controls->hide();
}

void MainWindow::showOsdMessage(const QString &text, int timeoutMs) {
    m_osdLabel->setText(text);
    m_osdLabel->adjustSize();

    // Center inside video widget
    int x = (m_videoWidget->width() - m_osdLabel->width()) / 2;
    int y = 40; // near top
    m_osdLabel->move(std::max(10, x), y);
    m_osdLabel->show();
    m_osdTimer.start(timeoutMs);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    onUserActivity();

    switch (event->key()) {
    case Qt::Key_Space:
        onTogglePlayPause();
        break;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ControlModifier) {
            onSeekRelative(-30.0);
        } else if (event->modifiers() & Qt::ShiftModifier) {
            onSeekRelative(-1.0);
        } else {
            onSeekRelative(-5.0);
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) {
            onSeekRelative(30.0);
        } else if (event->modifiers() & Qt::ShiftModifier) {
            onSeekRelative(1.0);
        } else {
            onSeekRelative(5.0);
        }
        break;
    case Qt::Key_Up:
        onVolumeDelta(5.0);
        break;
    case Qt::Key_Down:
        onVolumeDelta(-5.0);
        break;
    case Qt::Key_M:
        if (m_engine) m_engine->toggleMute();
        break;
    case Qt::Key_F:
    case Qt::Key_F11:
        onToggleFullscreen();
        break;
    case Qt::Key_Escape:
        if (m_isFullscreen) onToggleFullscreen();
        break;
    case Qt::Key_S:
        onTakeScreenshot();
        break;
    case Qt::Key_L:
        onTogglePlaylist();
        break;
    case Qt::Key_E:
    case Qt::Key_C:
        onShowEqualizer();
        break;
    case Qt::Key_I:
        onShowMediaInfo();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_engine) {
        m_engine->stop();
    }
    QMainWindow::closeEvent(event);
}
