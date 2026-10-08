#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include "OrionEngine.h"

struct mpv_render_context;

class OrionVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit OrionVideoWidget(OrionEngine *engine, QWidget *parent = nullptr);
    ~OrionVideoWidget() override;

    void ensureRenderContextInitialized();
    bool isRenderContextReady() const { return m_renderCtx != nullptr; }

signals:
    void doubleClicked();
    void singleClicked();
    void userActivity();
    void fileDropped(const QString &filePath);
    void wheelScrolled(int delta);
    void renderContextReady();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onMpvRenderUpdate();

private:
    static void *getProcAddress(void *ctx, const char *name);
    static void onMpvUpdateCallback(void *ctx);

    OrionEngine *m_engine = nullptr;
    mpv_render_context *m_renderCtx = nullptr;
    bool m_hasActiveVideo = false;
    QTimer m_clickTimer;
};
