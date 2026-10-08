#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include "AuraEngine.h"

class AuraPreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit AuraPreferencesDialog(AuraEngine *engine, QWidget *parent = nullptr);

private slots:
    void onCategoryChanged(int index);
    void onSaveClicked();
    void onResetDefaultsClicked();

private:
    void setupUi();
    QWidget *createInterfacePage();
    QWidget *createAudioPage();
    QWidget *createVideoPage();
    QWidget *createSubtitlesPage();
    QWidget *createCodecsPage();

    AuraEngine *m_engine = nullptr;

    QListWidget *m_categoryList = nullptr;
    QStackedWidget *m_pages = nullptr;

    // Settings elements
    QComboBox *m_themeCombo = nullptr;
    QCheckBox *m_integrateMpris = nullptr;
    QCheckBox *m_saveRecent = nullptr;

    QSpinBox *m_defaultVolume = nullptr;
    QComboBox *m_audioOutputCombo = nullptr;

    QComboBox *m_hwdecCombo = nullptr;
    QComboBox *m_deinterlaceCombo = nullptr;
    QComboBox *m_aspectDefaultCombo = nullptr;

    QComboBox *m_subSizeCombo = nullptr;
    QComboBox *m_subEncodingCombo = nullptr;
};
