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

#include "OrionVideoWidget.h"
#include "mpv/render.h"
#include "mpv/render_gl.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QOpenGLContext>
#include <QDebug>

void *OrionVideoWidget::getProcAddress(void *ctx, const char *name) {
    auto *glCtx = static_cast<QOpenGLContext *>(ctx);
    if (!glCtx) {
        glCtx = QOpenGLContext::currentContext();
    }
    if (!glCtx) return nullptr;
    return reinterpret_cast<void *>(glCtx->getProcAddress(QByteArray(name)));
}

void OrionVideoWidget::onMpvUpdateCallback(void *ctx) {
    auto *widget = static_cast<OrionVideoWidget *>(ctx);
    if (widget) {
        QMetaObject::invokeMethod(widget, "onMpvRenderUpdate", Qt::QueuedConnection);
    }
}

OrionVideoWidget::OrionVideoWidget(OrionEngine *engine, QWidget *parent)
    : QOpenGLWidget(parent), m_engine(engine) {
    setMouseTracking(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);

    m_clickTimer.setSingleShot(true);
    m_clickTimer.setInterval(220);
    connect(&m_clickTimer, &QTimer::timeout, this, &OrionVideoWidget::singleClicked);

    if (m_engine) {
        connect(m_engine, &OrionEngine::playbackStarted, this, [this]() {
            m_hasActiveVideo = true;
            update();
        });
        connect(m_engine, &OrionEngine::playbackStopped, this, [this]() {
            m_hasActiveVideo = false;
            update();
        });
        connect(m_engine, &OrionEngine::videoReconfigured, this, [this](int w, int h) {
            m_hasActiveVideo = (w > 0 && h > 0);
            update();
        });
    }
}

OrionVideoWidget::~OrionVideoWidget() {
    makeCurrent();
    if (m_renderCtx) {
        mpv_render_context_set_update_callback(m_renderCtx, nullptr, nullptr);
        mpv_render_context_free(m_renderCtx);
        m_renderCtx = nullptr;
    }
    doneCurrent();
}

void OrionVideoWidget::ensureRenderContextInitialized() {
    if (m_renderCtx) return;
    if (!isValid() || !context()) return;
    makeCurrent();
    initializeGL();
    doneCurrent();
}

void OrionVideoWidget::initializeGL() {
    if (m_renderCtx) return;
    initializeOpenGLFunctions();

    if (!m_engine || !m_engine->handle()) return;

    mpv_opengl_init_params glInitParams{
        getProcAddress,
        context()
    };

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    int err = mpv_render_context_create(&m_renderCtx, m_engine->handle(), params);
    if (err < 0) {
        qWarning() << "Failed to create mpv OpenGL render context:" << mpv_error_string(err);
        return;
    }

    mpv_render_context_set_update_callback(m_renderCtx, onMpvUpdateCallback, this);
    emit renderContextReady();
}

void OrionVideoWidget::resizeGL(int w, int h) {
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void OrionVideoWidget::paintGL() {
    if (m_renderCtx && m_hasActiveVideo) {
        qreal dpr = devicePixelRatioF();
        int fboWidth = static_cast<int>(width() * dpr);
        int fboHeight = static_cast<int>(height() * dpr);

        mpv_opengl_fbo fbo{
            static_cast<int>(defaultFramebufferObject()),
            fboWidth,
            fboHeight,
            0
        };

        int flipY = 1;
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flipY},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        mpv_render_context_render(m_renderCtx, params);
    } else {
        glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect r = rect();
        QPoint center = r.center();

        // Subtle ambient radial backdrop
        QRadialGradient glow(center, std::min(r.width(), r.height()) * 0.45);
        glow.setColorAt(0.0, QColor(0, 229, 255, 25));
        glow.setColorAt(0.5, QColor(37, 99, 235, 12));
        glow.setColorAt(1.0, QColor(10, 14, 23, 0));
        painter.fillRect(r, glow);

        // Draw official Orion vector logo
        QIcon appIcon(":/icon.svg");
        if (!appIcon.isNull()) {
            QPixmap iconPix = appIcon.pixmap(110, 110);
            painter.drawPixmap(center.x() - 55, center.y() - 95, iconPix);
        }

        // Project title
        painter.setPen(QColor(240, 246, 252));
        QFont titleFont = font();
        titleFont.setPointSize(20);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(r.adjusted(0, 35, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "Orion Player");

        // Drag and drop guidance
        painter.setPen(QColor(139, 148, 158));
        QFont subFont = font();
        subFont.setPointSize(11);
        painter.setFont(subFont);
        painter.drawText(r.adjusted(0, 70, 0, 0), Qt::AlignHCenter | Qt::AlignTop,
                         "Drop a media file or stream here to start playback");

        // Shortcut hint line
        painter.setPen(QColor(95, 105, 120));
        QFont hintFont = font();
        hintFont.setPointSize(9);
        painter.setFont(hintFont);
        painter.drawText(r.adjusted(0, 105, 0, 0), Qt::AlignHCenter | Qt::AlignTop,
                         "Space: Play/Pause  •  Ctrl+O: Open File  •  Ctrl+L: Playlist  •  Ctrl+E: Effects  •  F11: Fullscreen");
    }
}

void OrionVideoWidget::onMpvRenderUpdate() {
    update();
}

void OrionVideoWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_clickTimer.start();
    }
    emit userActivity();
    QOpenGLWidget::mousePressEvent(event);
}

void OrionVideoWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_clickTimer.stop();
        emit doubleClicked();
    }
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void OrionVideoWidget::mouseMoveEvent(QMouseEvent *event) {
    emit userActivity();
    QOpenGLWidget::mouseMoveEvent(event);
}

void OrionVideoWidget::wheelEvent(QWheelEvent *event) {
    emit userActivity();
    emit wheelScrolled(event->angleDelta().y());
    QOpenGLWidget::wheelEvent(event);
}

void OrionVideoWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void OrionVideoWidget::dropEvent(QDropEvent *event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString localPath = urls.first().toLocalFile();
        if (localPath.isEmpty()) {
            localPath = urls.first().toString();
        }
        emit fileDropped(localPath);
        event->acceptProposedAction();
    }
}
