#include "AuraStreamDialog.h"

AuraStreamDialog::AuraStreamDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Open Network Stream — AuraPlayer");
    resize(480, 160);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *descLabel = new QLabel(
        "Enter a network URL or live stream address (HTTP, HTTPS, RTSP, RTMP, HLS, or YouTube):", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #8b949e;");
    layout->addWidget(descLabel);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://... or rtsp://...");
    m_urlEdit->setStyleSheet(
        "QLineEdit { background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 8px; color: #ffffff; }"
        "QLineEdit:focus { border: 1px solid #58a6ff; }"
    );
    layout->addWidget(m_urlEdit);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_playBtn = new QPushButton("▶ Play Stream", this);
    m_playBtn->setStyleSheet("background: #1f6feb; color: white; font-weight: bold; padding: 6px 14px;");

    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_playBtn);
    layout->addLayout(btnRow);

    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_playBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &QDialog::accept);
}

QString AuraStreamDialog::streamUrl() const {
    return m_urlEdit->text().trimmed();
}
