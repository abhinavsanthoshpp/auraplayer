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

#include "OrionPlaylistView.h"
#include <QFileDialog>
#include <QHeaderView>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QDir>

OrionPlaylistView::OrionPlaylistView(QWidget *parent)
    : QWidget(parent) {
    setObjectName("OrionPlaylistView");
    setupUi();
}

void OrionPlaylistView::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(6);

    // Top Filter Row
    auto *topRow = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search playlist...");
    m_searchEdit->setClearButtonEnabled(true);
    topRow->addWidget(m_searchEdit);
    rootLayout->addLayout(topRow);

    // Main Splitter: Left Category Tree + Right Media Table
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // Left Categories
    m_categoryTree = new QTreeWidget(splitter);
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setMaximumWidth(190);

    auto *itemPlaylist = new QTreeWidgetItem(m_categoryTree, {"Playlist"});
    itemPlaylist->setSelected(true);
    auto *itemMediaLib = new QTreeWidgetItem(m_categoryTree, {"Media Library"});
    new QTreeWidgetItem(itemMediaLib, {"My Videos"});
    new QTreeWidgetItem(itemMediaLib, {"My Music"});
    new QTreeWidgetItem(m_categoryTree, {"Network Streams"});
    m_categoryTree->expandAll();
    splitter->addWidget(m_categoryTree);

    // Right Table: Columns [Title, Duration, Artist, URI/Path]
    m_table = new QTableWidget(splitter);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Title", "Duration", "Artist", "URI / Location"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    splitter->addWidget(m_table);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    rootLayout->addWidget(splitter, 1);

    // Bottom Action Row
    auto *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(6);

    m_addFileBtn = new QPushButton("➕ Add File...", this);
    m_addFolderBtn = new QPushButton("📁 Add Folder...", this);
    m_removeBtn = new QPushButton("➖ Remove", this);
    m_clearBtn = new QPushButton("🗑️ Clear Playlist", this);
    m_sortBtn = new QPushButton("⇅ Sort by Title", this);

    bottomRow->addWidget(m_addFileBtn);
    bottomRow->addWidget(m_addFolderBtn);
    bottomRow->addWidget(m_removeBtn);
    bottomRow->addWidget(m_clearBtn);
    bottomRow->addStretch(1);
    bottomRow->addWidget(m_sortBtn);
    rootLayout->addLayout(bottomRow);

    // Connections
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &OrionPlaylistView::onTableItemDoubleClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &OrionPlaylistView::onSearchChanged);
    connect(m_addFileBtn, &QPushButton::clicked, this, &OrionPlaylistView::onAddFilesClicked);
    connect(m_addFolderBtn, &QPushButton::clicked, this, &OrionPlaylistView::onAddFolderClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &OrionPlaylistView::onRemoveSelectedClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &OrionPlaylistView::onClearClicked);
    connect(m_sortBtn, &QPushButton::clicked, this, &OrionPlaylistView::onSortByNameClicked);
}

void OrionPlaylistView::addFile(const QString &filePath, double duration) {
    if (filePath.isEmpty()) return;

    for (int r = 0; r < m_table->rowCount(); ++r) {
        auto *item = m_table->item(r, 0);
        if (item && item->data(Qt::UserRole).toString() == filePath) {
            m_currentIndex = r;
            m_table->selectRow(r);
            return;
        }
    }

    QFileInfo fi(filePath);
    int row = m_table->rowCount();
    m_table->insertRow(row);

    auto *titleItem = new QTableWidgetItem(fi.fileName());
    titleItem->setData(Qt::UserRole, filePath);

    auto *durItem = new QTableWidgetItem(duration > 0.0 ? formatDuration(duration) : "--:--");
    auto *artistItem = new QTableWidgetItem("Unknown");
    auto *pathItem = new QTableWidgetItem(filePath);

    m_table->setItem(row, 0, titleItem);
    m_table->setItem(row, 1, durItem);
    m_table->setItem(row, 2, artistItem);
    m_table->setItem(row, 3, pathItem);

    emit playlistChanged();
}

void OrionPlaylistView::addFiles(const QStringList &filePaths) {
    for (const auto &p : filePaths) {
        addFile(p);
    }
}

void OrionPlaylistView::clear() {
    m_table->setRowCount(0);
    m_currentIndex = -1;
    emit playlistChanged();
}

int OrionPlaylistView::count() const {
    return m_table->rowCount();
}

QString OrionPlaylistView::currentFilePath() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_table->rowCount()) {
        return m_table->item(m_currentIndex, 0)->data(Qt::UserRole).toString();
    }
    return QString();
}

QString OrionPlaylistView::playNext() {
    if (count() == 0) return QString();

    if (m_loopMode == LoopMode::RepeatOne && m_currentIndex >= 0) {
        return currentFilePath();
    }

    m_currentIndex++;
    if (m_currentIndex >= count()) {
        if (m_loopMode == LoopMode::RepeatAll) {
            m_currentIndex = 0;
        } else {
            m_currentIndex = count() - 1;
            return QString();
        }
    }
    m_table->selectRow(m_currentIndex);
    return currentFilePath();
}

QString OrionPlaylistView::playPrevious() {
    if (count() == 0) return QString();

    m_currentIndex--;
    if (m_currentIndex < 0) {
        if (m_loopMode == LoopMode::RepeatAll) {
            m_currentIndex = count() - 1;
        } else {
            m_currentIndex = 0;
            return QString();
        }
    }
    m_table->selectRow(m_currentIndex);
    return currentFilePath();
}

void OrionPlaylistView::shuffle() {
    int n = count();
    if (n <= 1) return;
    for (int i = n - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        if (i != j) {
            for (int col = 0; col < 4; ++col) {
                auto *itemI = m_table->takeItem(i, col);
                auto *itemJ = m_table->takeItem(j, col);
                m_table->setItem(j, col, itemI);
                m_table->setItem(i, col, itemJ);
            }
        }
    }
}

void OrionPlaylistView::onTableItemDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    if (row >= 0 && row < count()) {
        m_currentIndex = row;
        QString path = m_table->item(row, 0)->data(Qt::UserRole).toString();
        emit trackSelected(path);
    }
}

void OrionPlaylistView::onSearchChanged(const QString &text) {
    for (int r = 0; r < count(); ++r) {
        bool match = false;
        for (int c = 0; c < 4; ++c) {
            if (m_table->item(r, c)->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_table->setRowHidden(r, !match);
    }
}

void OrionPlaylistView::onAddFilesClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Select Media to Add to Playlist", QString(),
        "All Media (*.mkv *.mp4 *.webm *.avi *.mov *.flv *.ts *.mp3 *.flac *.opus *.ogg *.wav);;All Files (*)"
    );
    addFiles(files);
}

void OrionPlaylistView::onAddFolderClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder to Add");
    if (!dir.isEmpty()) {
        QDir directory(dir);
        QStringList filters;
        filters << "*.mp4" << "*.mkv" << "*.webm" << "*.avi" << "*.mov" << "*.flv" << "*.mp3" << "*.flac" << "*.wav";
        QStringList files = directory.entryList(filters, QDir::Files, QDir::Name);
        for (const auto &f : files) {
            addFile(directory.absoluteFilePath(f));
        }
    }
}

void OrionPlaylistView::onRemoveSelectedClicked() {
    int row = m_table->currentRow();
    if (row >= 0) {
        m_table->removeRow(row);
        if (m_currentIndex == row) m_currentIndex = -1;
        else if (m_currentIndex > row) m_currentIndex--;
        emit playlistChanged();
    }
}

void OrionPlaylistView::onClearClicked() {
    clear();
}

void OrionPlaylistView::onSortByNameClicked() {
    m_table->sortItems(0, Qt::AscendingOrder);
}

QString OrionPlaylistView::formatDuration(double seconds) {
    int total = static_cast<int>(seconds);
    int m = total / 60;
    int s = total % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}
