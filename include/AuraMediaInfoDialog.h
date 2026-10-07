#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "AuraEngine.h"

class AuraMediaInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit AuraMediaInfoDialog(AuraEngine *engine, QWidget *parent = nullptr);

    void refresh();

private:
    void addRow(int &row, const QString &property, const QString &value);
    static QString formatBytes(int64_t bytes);

    AuraEngine *m_engine = nullptr;
    QTableWidget *m_table = nullptr;
};
