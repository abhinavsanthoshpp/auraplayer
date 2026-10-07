#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include "AuraEngine.h"

struct mpv_render_context;

class AuraVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit AuraVideoWidget(AuraEngine *engine, QWidget *parent = nullptr);
    ~AuraVideoWidget() override;

signals:
    void doubleClicked();
    void singleClicked();
    void userActivity();
    void fileDropped(const QString &filePath);
    void wheelScrolled(int delta);

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

    AuraEngine *m_engine = nullptr;
    mpv_render_context *m_renderCtx = nullptr;
    bool m_hasActiveVideo = false;
    QTimer m_clickTimer;
};
