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

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QLabel>
#include "OrionEngine.h"

struct mpv_render_context;

class OrionVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit OrionVideoWidget(OrionEngine *engine, QWidget *parent = nullptr);
    ~OrionVideoWidget() override;

    void ensureRenderContextInitialized();
    bool isRenderContextReady() const { return m_renderCtx != nullptr; }

    void showOsd(const QString &text, int durationMs = 1200);

signals:
    void doubleClicked();
    void singleClicked();
    void contextMenuRequested(const QPoint &globalPos);
    void userActivity();
    void fileDropped(const QString &filePath);
    void wheelScrolled(int delta);
    void renderContextReady();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onMpvRenderUpdate();

private:
    static void *getProcAddress(void *ctx, const char *name);
    static void onMpvUpdateCallback(void *ctx);

    OrionEngine *m_engine = nullptr;
    mpv_render_context *m_renderCtx = nullptr;
    bool m_hasActiveVideo = false;
    QTimer m_clickTimer;

    QLabel *m_osdLabel = nullptr;
    QTimer m_osdTimer;
};
