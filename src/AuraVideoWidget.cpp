#include "AuraVideoWidget.h"
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

void *AuraVideoWidget::getProcAddress(void *ctx, const char *name) {
    auto *glCtx = static_cast<QOpenGLContext *>(ctx);
    if (!glCtx) {
        glCtx = QOpenGLContext::currentContext();
    }
    if (!glCtx) return nullptr;
    return reinterpret_cast<void *>(glCtx->getProcAddress(QByteArray(name)));
}

void AuraVideoWidget::onMpvUpdateCallback(void *ctx) {
    auto *widget = static_cast<AuraVideoWidget *>(ctx);
    if (widget) {
        QMetaObject::invokeMethod(widget, "onMpvRenderUpdate", Qt::QueuedConnection);
    }
}

AuraVideoWidget::AuraVideoWidget(AuraEngine *engine, QWidget *parent)
    : QOpenGLWidget(parent), m_engine(engine) {
    setMouseTracking(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);

    m_clickTimer.setSingleShot(true);
    m_clickTimer.setInterval(220);
    connect(&m_clickTimer, &QTimer::timeout, this, &AuraVideoWidget::singleClicked);

    if (m_engine) {
        connect(m_engine, &AuraEngine::playbackStarted, this, [this]() {
            m_hasActiveVideo = true;
            update();
        });
        connect(m_engine, &AuraEngine::playbackStopped, this, [this]() {
            m_hasActiveVideo = false;
            update();
        });
    }
}

AuraVideoWidget::~AuraVideoWidget() {
    makeCurrent();
    if (m_renderCtx) {
        mpv_render_context_set_update_callback(m_renderCtx, nullptr, nullptr);
        mpv_render_context_free(m_renderCtx);
        m_renderCtx = nullptr;
    }
    doneCurrent();
}

void AuraVideoWidget::initializeGL() {
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
}

void AuraVideoWidget::resizeGL(int w, int h) {
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void AuraVideoWidget::paintGL() {
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
        // Draw modern idle / splash screen
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Center typography and icon
        QRect r = rect();
        painter.setPen(QColor(180, 190, 210));
        QFont titleFont("Inter", 22, QFont::Bold);
        painter.setFont(titleFont);
        painter.drawText(r.adjusted(0, -50, 0, -50), Qt::AlignCenter, "⚡ AuraPlayer");

        painter.setPen(QColor(130, 140, 160));
        QFont subFont("Inter", 12);
        painter.setFont(subFont);
        painter.drawText(r.adjusted(0, 20, 0, 20), Qt::AlignCenter, 
                         "Drop video or audio files here\nor click Open Media (Ctrl+O)");

        painter.setPen(QColor(80, 95, 120));
        QFont hintFont("Inter", 10);
        painter.setFont(hintFont);
        painter.drawText(r.adjusted(0, 80, 0, 80), Qt::AlignCenter,
                         "Hardware Accelerated (VA-API / NVDEC) • 4K/8K 60fps Ready");
    }
}

void AuraVideoWidget::onMpvRenderUpdate() {
    update();
}

void AuraVideoWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_clickTimer.start();
    }
    emit userActivity();
    QOpenGLWidget::mousePressEvent(event);
}

void AuraVideoWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_clickTimer.stop();
        emit doubleClicked();
    }
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void AuraVideoWidget::mouseMoveEvent(QMouseEvent *event) {
    emit userActivity();
    QOpenGLWidget::mouseMoveEvent(event);
}

void AuraVideoWidget::wheelEvent(QWheelEvent *event) {
    emit userActivity();
    emit wheelScrolled(event->angleDelta().y());
    QOpenGLWidget::wheelEvent(event);
}

void AuraVideoWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void AuraVideoWidget::dropEvent(QDropEvent *event) {
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
