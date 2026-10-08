#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>

class AuraPlaylistView : public QWidget {
    Q_OBJECT

public:
    enum class LoopMode {
        None,
        RepeatAll,
        RepeatOne
    };

    explicit AuraPlaylistView(QWidget *parent = nullptr);

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
