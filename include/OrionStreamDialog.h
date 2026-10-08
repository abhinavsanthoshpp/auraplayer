#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class OrionStreamDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionStreamDialog(QWidget *parent = nullptr);

    QString streamUrl() const;

private:
    QLineEdit *m_urlEdit = nullptr;
    QPushButton *m_playBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
};
