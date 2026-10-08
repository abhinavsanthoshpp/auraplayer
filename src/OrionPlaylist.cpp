#include "OrionPlaylist.h"
#include <QFileDialog>
#include <QLabel>
#include <QHeaderView>

OrionPlaylist::OrionPlaylist(QWidget *parent)
    : QWidget(parent) {
    setObjectName("OrionPlaylistWidget");
    setFixedWidth(280);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *headerLabel = new QLabel("📑 Playlist Queue", this);
    headerLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #58a6ff; padding: 4px;");
    layout->addWidget(headerLabel);

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setStyleSheet(
        "QListWidget { background: #12161f; border: 1px solid #30363d; border-radius: 6px; padding: 4px; }"
        "QListWidget::item { padding: 6px; border-radius: 4px; color: #c9d1d9; }"
        "QListWidget::item:selected { background: #1f6feb; color: #ffffff; }"
        "QListWidget::item:hover { background: #21262d; }"
    );
    layout->addWidget(m_listWidget, 1);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(4);

    m_addBtn = new QPushButton("➕ Add", this);
    m_removeBtn = new QPushButton("➖ Del", this);
    m_clearBtn = new QPushButton("🗑️ Clear", this);
    m_shuffleBtn = new QPushButton("🔀", this);
    m_loopBtn = new QPushButton("🔁 All", this);

    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_removeBtn);
    btnRow->addWidget(m_clearBtn);
    btnRow->addWidget(m_shuffleBtn);
    btnRow->addWidget(m_loopBtn);

    layout->addLayout(btnRow);

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &OrionPlaylist::onItemDoubleClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &OrionPlaylist::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &OrionPlaylist::onRemoveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &OrionPlaylist::onClearClicked);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &OrionPlaylist::onShuffleClicked);
    connect(m_loopBtn, &QPushButton::clicked, this, &OrionPlaylist::onLoopModeClicked);
}

void OrionPlaylist::addFile(const QString &filePath) {
    if (filePath.isEmpty()) return;
    QFileInfo fi(filePath);
    auto *item = new QListWidgetItem(fi.fileName(), m_listWidget);
    item->setToolTip(filePath);
    item->setData(Qt::UserRole, filePath);
    emit playlistChanged();
}

void OrionPlaylist::addFiles(const QStringList &filePaths) {
    for (const auto &p : filePaths) {
        addFile(p);
    }
}

void OrionPlaylist::clear() {
    m_listWidget->clear();
    m_currentIndex = -1;
    emit playlistChanged();
}

QString OrionPlaylist::currentFilePath() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_listWidget->count()) {
        return m_listWidget->item(m_currentIndex)->data(Qt::UserRole).toString();
    }
    return QString();
}

QString OrionPlaylist::playNext() {
    if (m_listWidget->count() == 0) return QString();

    if (m_loopMode == LoopMode::RepeatOne && m_currentIndex >= 0) {
        return currentFilePath();
    }

    m_currentIndex++;
    if (m_currentIndex >= m_listWidget->count()) {
        if (m_loopMode == LoopMode::RepeatAll) {
            m_currentIndex = 0;
        } else {
            m_currentIndex = m_listWidget->count() - 1;
            return QString();
        }
    }
    m_listWidget->setCurrentRow(m_currentIndex);
    return currentFilePath();
}

QString OrionPlaylist::playPrevious() {
    if (m_listWidget->count() == 0) return QString();

    m_currentIndex--;
    if (m_currentIndex < 0) {
        if (m_loopMode == LoopMode::RepeatAll) {
            m_currentIndex = m_listWidget->count() - 1;
        } else {
            m_currentIndex = 0;
            return QString();
        }
    }
    m_listWidget->setCurrentRow(m_currentIndex);
    return currentFilePath();
}

void OrionPlaylist::onItemDoubleClicked(QListWidgetItem *item) {
    if (!item) return;
    m_currentIndex = m_listWidget->row(item);
    QString path = item->data(Qt::UserRole).toString();
    emit trackSelected(path);
}

void OrionPlaylist::onAddClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Add Files to Playlist", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    addFiles(files);
}

void OrionPlaylist::onRemoveClicked() {
    int row = m_listWidget->currentRow();
    if (row >= 0) {
        delete m_listWidget->takeItem(row);
        if (m_currentIndex == row) {
            m_currentIndex = -1;
        } else if (m_currentIndex > row) {
            m_currentIndex--;
        }
        emit playlistChanged();
    }
}

void OrionPlaylist::onClearClicked() {
    clear();
}

void OrionPlaylist::onShuffleClicked() {
    int n = m_listWidget->count();
    if (n <= 1) return;

    for (int i = n - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        if (i != j) {
            auto *itemI = m_listWidget->takeItem(i);
            auto *itemJ = m_listWidget->takeItem(j);
            m_listWidget->insertItem(j, itemI);
            m_listWidget->insertItem(i, itemJ);
        }
    }
    emit playlistChanged();
}

void OrionPlaylist::onLoopModeClicked() {
    if (m_loopMode == LoopMode::RepeatAll) {
        m_loopMode = LoopMode::RepeatOne;
        m_loopBtn->setText("🔂 1");
    } else if (m_loopMode == LoopMode::RepeatOne) {
        m_loopMode = LoopMode::None;
        m_loopBtn->setText("➡️ Off");
    } else {
        m_loopMode = LoopMode::RepeatAll;
        m_loopBtn->setText("🔁 All");
    }
}
