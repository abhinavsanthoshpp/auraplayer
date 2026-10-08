#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QRandomGenerator>

class OrionPlaylist : public QWidget {
    Q_OBJECT

public:
    enum class LoopMode {
        None,
        RepeatAll,
        RepeatOne
    };

    explicit OrionPlaylist(QWidget *parent = nullptr);

    void addFile(const QString &filePath);
    void addFiles(const QStringList &filePaths);
    void clear();
    int count() const { return m_listWidget->count(); }
    int currentIndex() const { return m_currentIndex; }
    QString currentFilePath() const;
    QString playNext();
    QString playPrevious();

signals:
    void trackSelected(const QString &filePath);
    void playlistChanged();

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onAddClicked();
    void onRemoveClicked();
    void onClearClicked();
    void onShuffleClicked();
    void onLoopModeClicked();

private:
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_addBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_shuffleBtn = nullptr;
    QPushButton *m_loopBtn = nullptr;

    int m_currentIndex = -1;
    LoopMode m_loopMode = LoopMode::RepeatAll;
};
