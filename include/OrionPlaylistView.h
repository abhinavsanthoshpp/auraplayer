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

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>

class OrionPlaylistView : public QWidget {
    Q_OBJECT

public:
    enum class LoopMode {
        None,
        RepeatAll,
        RepeatOne
    };

    explicit OrionPlaylistView(QWidget *parent = nullptr);

    void addFile(const QString &filePath, double duration = 0.0);
    void addFiles(const QStringList &filePaths);
    void clear();
    int count() const;
    QString currentFilePath() const;
    QString playNext();
    QString playPrevious();
    void setLoopMode(LoopMode mode) { m_loopMode = mode; }
    void shuffle();

signals:
    void trackSelected(const QString &filePath);
    void playlistChanged();

private slots:
    void onTableItemDoubleClicked(int row, int column);
    void onSearchChanged(const QString &text);
    void onAddFilesClicked();
    void onAddFolderClicked();
    void onRemoveSelectedClicked();
    void onClearClicked();
    void onSortByNameClicked();

private:
    void setupUi();
    static QString formatDuration(double seconds);

    QLineEdit *m_searchEdit = nullptr;
    QTreeWidget *m_categoryTree = nullptr;
    QTableWidget *m_table = nullptr;

    QPushButton *m_addFileBtn = nullptr;
    QPushButton *m_addFolderBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_sortBtn = nullptr;

    int m_currentIndex = -1;
    LoopMode m_loopMode = LoopMode::None;
};
