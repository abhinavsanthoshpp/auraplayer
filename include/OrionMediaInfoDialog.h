#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "OrionEngine.h"

class OrionMediaInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit OrionMediaInfoDialog(OrionEngine *engine, QWidget *parent = nullptr);

    void refresh();

private:
    void addRow(int &row, const QString &property, const QString &value);
    static QString formatBytes(int64_t bytes);

    OrionEngine *m_engine = nullptr;
    QTableWidget *m_table = nullptr;
};
