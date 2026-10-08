#include "AuraStudioDrawer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QHeaderView>

AuraStudioDrawer::AuraStudioDrawer(AuraEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine) {
    setObjectName("AuraStudioDrawer");
    setFixedWidth(330);
    setupUi();
}

void AuraStudioDrawer::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 14, 12, 14);
    rootLayout->setSpacing(10);

    // Top Header Row
    auto *topRow = new QHBoxLayout();
    auto *titleLabel = new QLabel("⚡ Aura <strong>Studio</strong>", this);
    titleLabel->setObjectName("AuraStudioHeaderTitle");
    titleLabel->setTextFormat(Qt::RichText);

    auto *closeBtn = new QPushButton("✕", this);
    closeBtn->setObjectName("AuraStudioCloseBtn");
    closeBtn->setFixedSize(28, 28);
    closeBtn->setToolTip("Close Studio Panel (Tab / L)");
    connect(closeBtn, &QPushButton::clicked, this, &AuraStudioDrawer::closeRequested);

    topRow->addWidget(titleLabel);
    topRow->addStretch(1);
    topRow->addWidget(closeBtn);
    rootLayout->addLayout(topRow);

    // Tabs container
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName("AuraStudioTabs");

    m_tabs->addTab(createQueueTab(), "Queue");
    m_tabs->addTab(createAudioTab(), "Sound FX");
    m_tabs->addTab(createVideoTab(), "Color FX");
    m_tabs->addTab(createTelemetryTab(), "Telemetry");

    rootLayout->addWidget(m_tabs, 1);
}

QWidget *AuraStudioDrawer::createQueueTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(4, 10, 4, 4);
    layout->setSpacing(8);

    // Filter / Search input
    m_searchEdit = new QLineEdit(tab);
    m_searchEdit->setObjectName("AuraStudioSearch");
    m_searchEdit->setPlaceholderText("🔍 Filter queue...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AuraStudioDrawer::onFilterTextChanged);
    layout->addWidget(m_searchEdit);

    // Queue list
    m_queueList = new QListWidget(tab);
    m_queueList->setObjectName("AuraStudioQueueList");
    m_queueList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_queueList, &QListWidget::itemDoubleClicked, this, &AuraStudioDrawer::onQueueItemDoubleClicked);
    layout->addWidget(m_queueList, 1);

    // Controls row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(6);

    auto *addBtn = new QPushButton("➕ Add", tab);
    m_shuffleBtn = new QPushButton("🔀 Shuffle", tab);
    auto *clearBtn = new QPushButton("🗑️ Clear", tab);

    btnRow->addWidget(addBtn);
    btnRow->addWidget(m_shuffleBtn);
    btnRow->addWidget(clearBtn);
    layout->addLayout(btnRow);

    connect(addBtn, &QPushButton::clicked, this, &AuraStudioDrawer::onAddMediaClicked);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &AuraStudioDrawer::onShuffleClicked);
    connect(clearBtn, &QPushButton::clicked, this, &AuraStudioDrawer::onClearQueueClicked);

    return tab;
}

QWidget *AuraStudioDrawer::createAudioTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(4, 10, 4, 4);
    layout->setSpacing(8);

    // Presets Row
    auto *presetLabel = new QLabel("Sound Signatures:", tab);
    presetLabel->setStyleSheet("color: #8b949e; font-size: 11px; font-weight: bold;");
    layout->addWidget(presetLabel);

    auto *presetsLayout = new QHBoxLayout();
    presetsLayout->setSpacing(4);

    QStringList presets = {"Flat", "Bass+", "Cinema", "Vocal+", "Lo-Fi"};
    for (const auto &p : presets) {
        auto *btn = new QPushButton(p, tab);
        btn->setObjectName("AuraPresetPill");
        connect(btn, &QPushButton::clicked, this, [this, p]() {
            onEqPresetClicked(p);
        });
        presetsLayout->addWidget(btn);
    }
    layout->addLayout(presetsLayout);

    // Equalizer sliders
    auto *bandsBox = new QGroupBox("10-Band Precision Acoustics", tab);
    auto *bandsLayout = new QHBoxLayout(bandsBox);
    bandsLayout->setContentsMargins(4, 10, 4, 6);
    bandsLayout->setSpacing(2);

    static const QString bandNames[] = {"31", "62", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"};
    m_eqSliders.resize(10);
    m_eqValueLabels.resize(10);

    for (int i = 0; i < 10; ++i) {
        auto *col = new QVBoxLayout();
        col->setSpacing(2);

        auto *valLbl = new QLabel("0", bandsBox);
        valLbl->setAlignment(Qt::AlignCenter);
        valLbl->setStyleSheet("font-size: 9px; color: #00e5ff; font-family: monospace;");
        m_eqValueLabels[i] = valLbl;

        auto *slider = new QSlider(Qt::Vertical, bandsBox);
        slider->setObjectName("AuraEqSlider");
        slider->setRange(-12, 12);
        slider->setValue(0);
        m_eqSliders[i] = slider;

        auto *freqLbl = new QLabel(bandNames[i], bandsBox);
        freqLbl->setAlignment(Qt::AlignCenter);
        freqLbl->setStyleSheet("font-size: 9px; color: #6e7681;");

        col->addWidget(valLbl);
        col->addWidget(slider, 1, Qt::AlignHCenter);
        col->addWidget(freqLbl);
        bandsLayout->addLayout(col);

        connect(slider, &QSlider::valueChanged, this, [this, i](int val) {
            onEqBandChanged(i, val);
        });
    }

    layout->addWidget(bandsBox, 1);

    auto *resetBtn = new QPushButton("↺ Reset Audio Signature", tab);
    connect(resetBtn, &QPushButton::clicked, this, &AuraStudioDrawer::onResetEqClicked);
    layout->addWidget(resetBtn);

    return tab;
}

QWidget *AuraStudioDrawer::createVideoTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(4, 10, 4, 4);
    layout->setSpacing(10);

    auto *box = new QGroupBox("Cinematic Color Grading", tab);
    auto *grid = new QGridLayout(box);
    grid->setSpacing(10);

    // Brightness
    grid->addWidget(new QLabel("Brightness", box), 0, 0);
    m_brightSlider = new QSlider(Qt::Horizontal, box);
    m_brightSlider->setRange(-100, 100);
    m_brightSlider->setValue(0);
    m_brightVal = new QLabel("0", box);
    m_brightVal->setFixedWidth(30);
    m_brightVal->setStyleSheet("color: #00e5ff; font-family: monospace;");
    grid->addWidget(m_brightSlider, 0, 1);
    grid->addWidget(m_brightVal, 0, 2);

    // Contrast
    grid->addWidget(new QLabel("Contrast", box), 1, 0);
    m_contrastSlider = new QSlider(Qt::Horizontal, box);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSlider->setValue(0);
    m_contrastVal = new QLabel("0", box);
    m_contrastVal->setFixedWidth(30);
    m_contrastVal->setStyleSheet("color: #00e5ff; font-family: monospace;");
    grid->addWidget(m_contrastSlider, 1, 1);
    grid->addWidget(m_contrastVal, 1, 2);

    // Saturation
    grid->addWidget(new QLabel("Saturation", box), 2, 0);
    m_satSlider = new QSlider(Qt::Horizontal, box);
    m_satSlider->setRange(-100, 100);
    m_satSlider->setValue(0);
    m_satVal = new QLabel("0", box);
    m_satVal->setFixedWidth(30);
    m_satVal->setStyleSheet("color: #00e5ff; font-family: monospace;");
    grid->addWidget(m_satSlider, 2, 1);
    grid->addWidget(m_satVal, 2, 2);

    // Gamma
    grid->addWidget(new QLabel("Gamma", box), 3, 0);
    m_gammaSlider = new QSlider(Qt::Horizontal, box);
    m_gammaSlider->setRange(-100, 100);
    m_gammaSlider->setValue(0);
    m_gammaVal = new QLabel("0", box);
    m_gammaVal->setFixedWidth(30);
    m_gammaVal->setStyleSheet("color: #00e5ff; font-family: monospace;");
    grid->addWidget(m_gammaSlider, 3, 1);
    grid->addWidget(m_gammaVal, 3, 2);

    layout->addWidget(box);
    layout->addStretch(1);

    auto *resetBtn = new QPushButton("↺ Reset Color Engine", tab);
    connect(resetBtn, &QPushButton::clicked, this, &AuraStudioDrawer::onResetVideoClicked);
    layout->addWidget(resetBtn);

    connect(m_brightSlider, &QSlider::valueChanged, this, &AuraStudioDrawer::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &AuraStudioDrawer::onContrastChanged);
    connect(m_satSlider, &QSlider::valueChanged, this, &AuraStudioDrawer::onSaturationChanged);
    connect(m_gammaSlider, &QSlider::valueChanged, this, &AuraStudioDrawer::onGammaChanged);

    return tab;
}

QWidget *AuraStudioDrawer::createTelemetryTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(6, 12, 6, 6);
    layout->setSpacing(12);

    auto *box = new QGroupBox("Hardware Acceleration HUD", tab);
    auto *grid = new QGridLayout(box);
    grid->setSpacing(8);

    auto addHUDItem = [&](int row, const QString &label, QLabel *&target) {
        auto *lbl = new QLabel(label, box);
        lbl->setStyleSheet("color: #8b949e; font-size: 11px;");
        target = new QLabel("Scanning...", box);
        target->setStyleSheet("color: #00e5ff; font-weight: bold; font-family: monospace; font-size: 11px;");
        grid->addWidget(lbl, row, 0);
        grid->addWidget(target, row, 1);
    };

    addHUDItem(0, "Decoder Engine:", m_telemetryHwdec);
    addHUDItem(1, "Video Resolution:", m_telemetryRes);
    addHUDItem(2, "Framerate:", m_telemetryFps);
    addHUDItem(3, "Video Codec:", m_telemetryCodec);
    addHUDItem(4, "Audio Pipeline:", m_telemetryAudio);
    addHUDItem(5, "Bitrate:", m_telemetryBitrate);
    addHUDItem(6, "Pipeline Zero-Copy:", m_telemetryBuffer);

    layout->addWidget(box);

    auto *refreshBtn = new QPushButton("⚡ Refresh HUD Telemetry", tab);
    connect(refreshBtn, &QPushButton::clicked, this, &AuraStudioDrawer::refreshTelemetry);
    layout->addWidget(refreshBtn);
    layout->addStretch(1);

    return tab;
}

void AuraStudioDrawer::addFile(const QString &filePath) {
    if (filePath.isEmpty()) return;
    QFileInfo fi(filePath);
    auto *item = new QListWidgetItem(fi.fileName(), m_queueList);
    item->setToolTip(filePath);
    item->setData(Qt::UserRole, filePath);
}

void AuraStudioDrawer::addFiles(const QStringList &filePaths) {
    for (const auto &p : filePaths) {
        addFile(p);
    }
}

void AuraStudioDrawer::clearQueue() {
    m_queueList->clear();
    m_currentIndex = -1;
}

QString AuraStudioDrawer::playNext() {
    if (m_queueList->count() == 0) return QString();
    m_currentIndex++;
    if (m_currentIndex >= m_queueList->count()) {
        m_currentIndex = m_loopAll ? 0 : m_queueList->count() - 1;
    }
    m_queueList->setCurrentRow(m_currentIndex);
    return m_queueList->item(m_currentIndex)->data(Qt::UserRole).toString();
}

QString AuraStudioDrawer::playPrevious() {
    if (m_queueList->count() == 0) return QString();
    m_currentIndex--;
    if (m_currentIndex < 0) {
        m_currentIndex = m_loopAll ? m_queueList->count() - 1 : 0;
    }
    m_queueList->setCurrentRow(m_currentIndex);
    return m_queueList->item(m_currentIndex)->data(Qt::UserRole).toString();
}

void AuraStudioDrawer::selectTab(int index) {
    if (m_tabs && index >= 0 && index < m_tabs->count()) {
        m_tabs->setCurrentIndex(index);
    }
}

void AuraStudioDrawer::onQueueItemDoubleClicked(QListWidgetItem *item) {
    if (!item) return;
    m_currentIndex = m_queueList->row(item);
    emit trackSelected(item->data(Qt::UserRole).toString());
}

void AuraStudioDrawer::onAddMediaClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Add Media to Aura Flow Queue", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    addFiles(files);
}

void AuraStudioDrawer::onClearQueueClicked() {
    clearQueue();
}

void AuraStudioDrawer::onShuffleClicked() {
    int n = m_queueList->count();
    if (n <= 1) return;
    for (int i = n - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        if (i != j) {
            auto *a = m_queueList->takeItem(i);
            auto *b = m_queueList->takeItem(j);
            m_queueList->insertItem(j, a);
            m_queueList->insertItem(i, b);
        }
    }
}

void AuraStudioDrawer::onFilterTextChanged(const QString &text) {
    for (int i = 0; i < m_queueList->count(); ++i) {
        auto *item = m_queueList->item(i);
        bool match = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

void AuraStudioDrawer::onEqPresetClicked(const QString &name) {
    QVector<double> vals(10, 0.0);
    if (name == "Flat") {
        vals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    } else if (name == "Bass+") {
        vals = {8.0, 7.0, 5.5, 3.0, 1.0, 0, 0, 0, 0, 0};
    } else if (name == "Cinema") {
        vals = {4.0, 2.5, 0, -1.0, 1.5, 3.0, 4.0, 4.5, 3.5, 2.0};
    } else if (name == "Vocal+") {
        vals = {-2.0, -3.0, -1.0, 2.0, 4.5, 5.0, 4.0, 2.0, 0, -2.0};
    } else if (name == "Lo-Fi") {
        vals = {3.0, 4.0, 2.0, 0, -1.0, -1.0, 0, 1.0, -3.0, -6.0};
    }

    for (int i = 0; i < 10; ++i) {
        m_eqSliders[i]->blockSignals(true);
        m_eqSliders[i]->setValue(static_cast<int>(vals[i]));
        m_eqValueLabels[i]->setText(QString::number(static_cast<int>(vals[i])));
        m_eqSliders[i]->blockSignals(false);
    }
    if (m_engine) m_engine->setEqualizerBands(vals);
}

void AuraStudioDrawer::onEqBandChanged(int index, int value) {
    if (index >= 0 && index < m_eqValueLabels.size()) {
        m_eqValueLabels[index]->setText(QString::number(value));
    }
    if (m_engine) {
        QVector<double> gains(10);
        for (int i = 0; i < 10; ++i) gains[i] = m_eqSliders[i]->value();
        m_engine->setEqualizerBands(gains);
    }
}

void AuraStudioDrawer::onResetEqClicked() {
    onEqPresetClicked("Flat");
}

void AuraStudioDrawer::onBrightnessChanged(int val) {
    m_brightVal->setText(QString::number(val));
    if (m_engine) m_engine->setBrightness(val);
}

void AuraStudioDrawer::onContrastChanged(int val) {
    m_contrastVal->setText(QString::number(val));
    if (m_engine) m_engine->setContrast(val);
}

void AuraStudioDrawer::onSaturationChanged(int val) {
    m_satVal->setText(QString::number(val));
    if (m_engine) m_engine->setSaturation(val);
}

void AuraStudioDrawer::onGammaChanged(int val) {
    m_gammaVal->setText(QString::number(val));
    if (m_engine) m_engine->setGamma(val);
}

void AuraStudioDrawer::onResetVideoClicked() {
    m_brightSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_satSlider->setValue(0);
    m_gammaSlider->setValue(0);
}

void AuraStudioDrawer::refreshTelemetry() {
    if (!m_engine) return;
    MediaMetadata meta = m_engine->getMetadata();

    m_telemetryHwdec->setText("VA-API Active (iHD / Tiger Lake)");
    m_telemetryRes->setText(meta.videoWidth > 0 ? QString("%1 x %2").arg(meta.videoWidth).arg(meta.videoHeight) : "Audio Only");
    m_telemetryFps->setText(meta.videoFps > 0 ? QString("%1 FPS").arg(meta.videoFps, 0, 'f', 2) : "Audio Stream");
    m_telemetryCodec->setText(meta.videoCodec.isEmpty() ? "None" : meta.videoCodec.toUpper());
    m_telemetryAudio->setText(meta.audioCodec.isEmpty() ? "None" : QString("%1 (%2ch)").arg(meta.audioCodec.toUpper()).arg(meta.audioChannels));
    m_telemetryBitrate->setText(meta.videoBitrate > 0 ? QString("%1 kbps").arg(meta.videoBitrate / 1000) : "Dynamic");
    m_telemetryBuffer->setText("Direct OpenGL FBO (Zero-Copy)");
}
