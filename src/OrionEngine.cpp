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

#include "OrionEngine.h"
#include "mpv/client.h"

#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <clocale>
#include <cmath>

OrionEngine::OrionEngine(QObject *parent)
    : QObject(parent) {
}

OrionEngine::~OrionEngine() {
    if (m_mpv) {
        mpv_set_wakeup_callback(m_mpv, nullptr, nullptr);
        mpv_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

void OrionEngine::wakeupCallback(void *ctx) {
    auto *engine = static_cast<OrionEngine *>(ctx);
    if (engine) {
        QMetaObject::invokeMethod(engine, "onMpvEvents", Qt::QueuedConnection);
    }
}

bool OrionEngine::initialize() {
    // mpv strictly requires LC_NUMERIC set to "C" for consistent float formatting
    std::setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv) {
        qCritical() << "Failed to allocate mpv handle";
        return false;
    }

    // High performance defaults
    setupHardwareAcceleration();

    // Zero-drop audio pitch correction (scaletempo2)
    mpv_set_option_string(m_mpv, "audio-pitch-correction", "yes");
    mpv_set_option_string(m_mpv, "audio-channels", "auto-safe");
    mpv_set_option_string(m_mpv, "volume-max", "200.0");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    mpv_set_option_string(m_mpv, "idle", "yes");
    mpv_set_option_string(m_mpv, "input-default-bindings", "no");
    mpv_set_option_string(m_mpv, "input-vo-keyboard", "no");
    mpv_set_option_string(m_mpv, "terminal", "no");
    // Strictly force mpv to use the libmpv render API — NEVER create external windows!
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "wid", "0");

    // Fast memory caching pipeline
    mpv_set_option_string(m_mpv, "cache", "yes");
    mpv_set_option_string(m_mpv, "demuxer-max-bytes", "150M");
    mpv_set_option_string(m_mpv, "demuxer-readahead-secs", "20");

    int res = mpv_initialize(m_mpv);
    if (res < 0) {
        qCritical() << "Failed to initialize mpv:" << mpv_error_string(res);
        mpv_destroy(m_mpv);
        m_mpv = nullptr;
        return false;
    }

    mpv_set_wakeup_callback(m_mpv, wakeupCallback, this);
    observeProperties();
    return true;
}

void OrionEngine::setupHardwareAcceleration() {
    if (!m_mpv) return;

    // Detect and prioritize VA-API for Intel Tiger Lake / AMD, NVDEC for NVIDIA
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    mpv_set_option_string(m_mpv, "hwdec-codecs", "all");
    mpv_set_option_string(m_mpv, "scale", "bilinear");
}

void OrionEngine::observeProperties() {
    if (!m_mpv) return;

    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "mute", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "speed", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "dwidth", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "dheight", MPV_FORMAT_INT64);
}

void OrionEngine::onMpvEvents() {
    if (!m_mpv) return;

    while (true) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (!event || event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleMpvEvent(event);
    }
}

void OrionEngine::handleMpvEvent(void *eventPtr) {
    auto *event = static_cast<mpv_event *>(eventPtr);

    switch (event->event_id) {
    case MPV_EVENT_START_FILE:
        emit playbackStarted();
        break;

    case MPV_EVENT_END_FILE:
        emit playbackStopped();
        break;

    case MPV_EVENT_PROPERTY_CHANGE: {
        auto *prop = static_cast<mpv_event_property *>(event->data);
        if (!prop || !prop->name) break;

        QString name = QString::fromUtf8(prop->name);
        if (name == "time-pos" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
            m_position = *static_cast<double *>(prop->data);
            emit positionChanged(m_position);
        } else if (name == "duration" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
            m_duration = *static_cast<double *>(prop->data);
            emit durationChanged(m_duration);
        } else if (name == "pause" && prop->format == MPV_FORMAT_FLAG && prop->data) {
            m_paused = (*static_cast<int *>(prop->data) != 0);
            emit playbackPaused(m_paused);
        } else if (name == "volume" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
            m_volume = *static_cast<double *>(prop->data);
            emit volumeChanged(m_volume);
        } else if (name == "mute" && prop->format == MPV_FORMAT_FLAG && prop->data) {
            m_muted = (*static_cast<int *>(prop->data) != 0);
            emit muteChanged(m_muted);
        } else if (name == "speed" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
            m_speed = *static_cast<double *>(prop->data);
            emit speedChanged(m_speed);
        } else if (name == "track-list") {
            emit tracksChanged();
        } else if (name == "dwidth" || name == "dheight") {
            int64_t w = 0, h = 0;
            mpv_get_property(m_mpv, "dwidth", MPV_FORMAT_INT64, &w);
            mpv_get_property(m_mpv, "dheight", MPV_FORMAT_INT64, &h);
            emit videoReconfigured(static_cast<int>(w), static_cast<int>(h));
        }
        break;
    }

    case MPV_EVENT_FILE_LOADED:
        emit fileLoaded(m_currentFile);
        break;

    default:
        break;
    }
}

void OrionEngine::loadFile(const QString &pathOrUrl, bool append) {
    if (!m_mpv) return;

    m_currentFile = pathOrUrl;
    QByteArray utf8 = pathOrUrl.toUtf8();
    const char *mode = append ? "append-play" : "replace";
    const char *args[] = {"loadfile", utf8.constData(), mode, nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::play() {
    if (!m_mpv) return;
    int flag = 0;
    mpv_set_property_async(m_mpv, 0, "pause", MPV_FORMAT_FLAG, &flag);
}

void OrionEngine::pause() {
    if (!m_mpv) return;
    int flag = 1;
    mpv_set_property_async(m_mpv, 0, "pause", MPV_FORMAT_FLAG, &flag);
}

void OrionEngine::togglePause() {
    if (!m_mpv) return;
    const char *args[] = {"cycle", "pause", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::stop() {
    if (!m_mpv) return;
    const char *args[] = {"stop", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::seek(double seconds, bool exact) {
    if (!m_mpv) return;
    QByteArray posStr = QByteArray::number(seconds, 'f', 2);
    const char *mode = exact ? "exact" : "keyframes";
    const char *args[] = {"seek", posStr.constData(), "absolute", mode, nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::seekRelative(double deltaSeconds) {
    if (!m_mpv) return;
    QByteArray deltaStr = QByteArray::number(deltaSeconds, 'f', 2);
    const char *args[] = {"seek", deltaStr.constData(), "relative", "exact", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::frameStep() {
    if (!m_mpv) return;
    const char *args[] = {"frame-step", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::frameBackStep() {
    if (!m_mpv) return;
    const char *args[] = {"frame-back-step", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::setVolume(double volume) {
    if (!m_mpv) return;
    volume = std::clamp(volume, 0.0, 200.0);
    mpv_set_property_async(m_mpv, 0, "volume", MPV_FORMAT_DOUBLE, &volume);
}

void OrionEngine::setMuted(bool mute) {
    if (!m_mpv) return;
    int flag = mute ? 1 : 0;
    mpv_set_property_async(m_mpv, 0, "mute", MPV_FORMAT_FLAG, &flag);
}

void OrionEngine::toggleMute() {
    if (!m_mpv) return;
    const char *args[] = {"cycle", "mute", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::setSpeed(double speed) {
    if (!m_mpv) return;
    speed = std::clamp(speed, 0.25, 4.0);
    mpv_set_property_async(m_mpv, 0, "speed", MPV_FORMAT_DOUBLE, &speed);
}

QVector<MediaTrack> OrionEngine::getTracks() const {
    QVector<MediaTrack> result;
    if (!m_mpv) return result;

    mpv_node node;
    if (mpv_get_property(m_mpv, "track-list", MPV_FORMAT_NODE, &node) >= 0) {
        if (node.format == MPV_FORMAT_NODE_ARRAY && node.u.list) {
            for (int i = 0; i < node.u.list->num; ++i) {
                mpv_node *item = &node.u.list->values[i];
                if (item->format == MPV_FORMAT_NODE_MAP && item->u.list) {
                    MediaTrack track;
                    for (int k = 0; k < item->u.list->num; ++k) {
                        char *key = item->u.list->keys[k];
                        mpv_node *val = &item->u.list->values[k];
                        if (strcmp(key, "id") == 0 && val->format == MPV_FORMAT_INT64) {
                            track.id = static_cast<int>(val->u.int64);
                        } else if (strcmp(key, "type") == 0 && val->format == MPV_FORMAT_STRING) {
                            track.type = QString::fromUtf8(val->u.string);
                        } else if (strcmp(key, "title") == 0 && val->format == MPV_FORMAT_STRING) {
                            track.title = QString::fromUtf8(val->u.string);
                        } else if (strcmp(key, "lang") == 0 && val->format == MPV_FORMAT_STRING) {
                            track.language = QString::fromUtf8(val->u.string);
                        } else if (strcmp(key, "codec") == 0 && val->format == MPV_FORMAT_STRING) {
                            track.codec = QString::fromUtf8(val->u.string);
                        } else if (strcmp(key, "selected") == 0 && val->format == MPV_FORMAT_FLAG) {
                            track.isSelected = (val->u.flag != 0);
                        } else if (strcmp(key, "default") == 0 && val->format == MPV_FORMAT_FLAG) {
                            track.isDefault = (val->u.flag != 0);
                        }
                    }
                    result.append(track);
                }
            }
        }
        mpv_free_node_contents(&node);
    }
    return result;
}

void OrionEngine::setAudioTrack(int trackId) {
    if (!m_mpv) return;
    if (trackId < 0) {
        const char *val = "no";
        mpv_set_property_string(m_mpv, "aid", val);
    } else {
        int64_t id = trackId;
        mpv_set_property_async(m_mpv, 0, "aid", MPV_FORMAT_INT64, &id);
    }
}

void OrionEngine::setSubtitleTrack(int trackId) {
    if (!m_mpv) return;
    if (trackId < 0) {
        const char *val = "no";
        mpv_set_property_string(m_mpv, "sid", val);
    } else {
        int64_t id = trackId;
        mpv_set_property_async(m_mpv, 0, "sid", MPV_FORMAT_INT64, &id);
    }
}

void OrionEngine::setSubtitleDelay(double deltaSeconds) {
    if (!m_mpv) return;
    mpv_set_property_async(m_mpv, 0, "sub-delay", MPV_FORMAT_DOUBLE, &deltaSeconds);
}

double OrionEngine::subtitleDelay() const {
    if (!m_mpv) return 0.0;
    double d = 0.0;
    mpv_get_property(m_mpv, "sub-delay", MPV_FORMAT_DOUBLE, &d);
    return d;
}

void OrionEngine::setAudioDelay(double deltaSeconds) {
    if (!m_mpv) return;
    mpv_set_property_async(m_mpv, 0, "audio-delay", MPV_FORMAT_DOUBLE, &deltaSeconds);
}

double OrionEngine::audioDelay() const {
    if (!m_mpv) return 0.0;
    double d = 0.0;
    mpv_get_property(m_mpv, "audio-delay", MPV_FORMAT_DOUBLE, &d);
    return d;
}

void OrionEngine::setBrightness(int value) {
    if (!m_mpv) return;
    int64_t v = std::clamp(value, -100, 100);
    mpv_set_property_async(m_mpv, 0, "brightness", MPV_FORMAT_INT64, &v);
}

void OrionEngine::setContrast(int value) {
    if (!m_mpv) return;
    int64_t v = std::clamp(value, -100, 100);
    mpv_set_property_async(m_mpv, 0, "contrast", MPV_FORMAT_INT64, &v);
}

void OrionEngine::setSaturation(int value) {
    if (!m_mpv) return;
    int64_t v = std::clamp(value, -100, 100);
    mpv_set_property_async(m_mpv, 0, "saturation", MPV_FORMAT_INT64, &v);
}

void OrionEngine::setGamma(int value) {
    if (!m_mpv) return;
    int64_t v = std::clamp(value, -100, 100);
    mpv_set_property_async(m_mpv, 0, "gamma", MPV_FORMAT_INT64, &v);
}

void OrionEngine::setAspectRatio(const QString &ratio) {
    if (!m_mpv) return;
    m_currentAspectRatio = ratio;
    static const QStringList ratios = {"default", "16:9", "4:3", "1:1", "16:10", "2.21:1", "2.35:1"};
    int idx = ratios.indexOf(ratio);
    if (idx >= 0) m_aspectRatioIndex = idx;

    if (ratio == "default" || ratio == "-1") {
        const char *val = "-1";
        mpv_set_property_string(m_mpv, "video-aspect-override", val);
    } else {
        QByteArray b = ratio.toUtf8();
        mpv_set_property_string(m_mpv, "video-aspect-override", b.constData());
    }
}

QString OrionEngine::cycleAspectRatio() {
    static const QStringList ratios = {"default", "16:9", "4:3", "1:1", "16:10", "2.21:1", "2.35:1"};
    m_aspectRatioIndex = (m_aspectRatioIndex + 1) % ratios.size();
    QString chosen = ratios[m_aspectRatioIndex];
    setAspectRatio(chosen);
    return chosen;
}

QString OrionEngine::cycleAudioTrack() {
    if (!m_mpv) return QString();
    QVector<MediaTrack> tracks = getTracks();
    QVector<MediaTrack> audioTracks;
    int currentIndex = -1;
    for (const auto &t : tracks) {
        if (t.type == "audio") {
            if (t.isSelected) currentIndex = audioTracks.size();
            audioTracks.append(t);
        }
    }
    if (audioTracks.isEmpty()) return "None";

    // Cycle: 0, 1, ..., N-1, Disable (-1)
    int nextIndex = (currentIndex + 1) % (audioTracks.size() + 1);
    if (nextIndex == audioTracks.size()) {
        setAudioTrack(-1);
        return "Disabled";
    }

    const auto &nextTrack = audioTracks[nextIndex];
    setAudioTrack(nextTrack.id);
    if (!nextTrack.title.isEmpty()) return nextTrack.title;
    if (!nextTrack.language.isEmpty()) return QString("Track %1 [%2]").arg(nextTrack.id).arg(nextTrack.language);
    return QString("Track %1 (%2)").arg(nextTrack.id).arg(nextTrack.codec);
}

QString OrionEngine::cycleSubtitleTrack() {
    if (!m_mpv) return QString();
    QVector<MediaTrack> tracks = getTracks();
    QVector<MediaTrack> subTracks;
    int currentIndex = -1;
    for (const auto &t : tracks) {
        if (t.type == "sub") {
            if (t.isSelected) currentIndex = subTracks.size();
            subTracks.append(t);
        }
    }
    if (subTracks.isEmpty()) return "None";

    // Cycle: 0, 1, ..., N-1, Disable (-1)
    int nextIndex = (currentIndex + 1) % (subTracks.size() + 1);
    if (nextIndex == subTracks.size()) {
        setSubtitleTrack(-1);
        return "Disabled";
    }

    const auto &nextTrack = subTracks[nextIndex];
    setSubtitleTrack(nextTrack.id);
    if (!nextTrack.title.isEmpty()) return nextTrack.title;
    if (!nextTrack.language.isEmpty()) return QString("Track %1 [%2]").arg(nextTrack.id).arg(nextTrack.language);
    return QString("Track %1 (%2)").arg(nextTrack.id).arg(nextTrack.codec);
}

void OrionEngine::loadSubtitleFile(const QString &path) {
    if (!m_mpv || path.isEmpty()) return;
    QByteArray bytes = path.toUtf8();
    const char *args[] = {"sub-add", bytes.constData(), "select", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void OrionEngine::setHue(int value) {
    if (!m_mpv) return;
    int64_t v = std::clamp(value, -100, 100);
    mpv_set_property_async(m_mpv, 0, "hue", MPV_FORMAT_INT64, &v);
}

void OrionEngine::setDeinterlace(bool enable) {
    if (!m_mpv) return;
    m_deinterlace = enable;
    const char *val = enable ? "yes" : "no";
    mpv_set_property_string(m_mpv, "deinterlace", val);
}

bool OrionEngine::toggleDeinterlace() {
    m_deinterlace = !m_deinterlace;
    setDeinterlace(m_deinterlace);
    return m_deinterlace;
}

void OrionEngine::setEqualizerBands(const QVector<double> &bands, double preamp) {
    if (!m_mpv) return;
    static const double freqs[] = {31.25, 62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
    QStringList filters;

    if (std::abs(preamp) > 0.1) {
        filters << QString("volume=volume=%1dB").arg(preamp, 0, 'f', 1);
    }

    for (int i = 0; i < std::min(10, static_cast<int>(bands.size())); ++i) {
        double gain = std::clamp(bands[i], -20.0, 20.0);
        filters << QString("equalizer=f=%1:w=1:g=%2").arg(freqs[i]).arg(gain, 0, 'f', 1);
    }
    QString filterChain = filters.join(",");
    QByteArray bytes = filterChain.toUtf8();
    mpv_set_property_string(m_mpv, "af", bytes.constData());
}

void OrionEngine::takeScreenshot(const QString &destinationPath) {
    if (!m_mpv) return;
    QString target = destinationPath;
    if (target.isEmpty()) {
        QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
        target = QDir::homePath() + QString("/Pictures/OrionPlayer_%1.png").arg(timeStr);
    }
    QDir().mkpath(QFileInfo(target).absolutePath());
    QByteArray bytes = target.toUtf8();
    const char *args[] = {"screenshot-to-file", bytes.constData(), "video", nullptr};
    mpv_command_async(m_mpv, 0, args);
    emit screenshotTaken(target);
}

MediaMetadata OrionEngine::getMetadata() const {
    MediaMetadata meta;
    if (!m_mpv) return meta;

    char *title = mpv_get_property_string(m_mpv, "media-title");
    if (title) { meta.title = QString::fromUtf8(title); mpv_free(title); }

    char *vcodec = mpv_get_property_string(m_mpv, "video-codec");
    if (vcodec) { meta.videoCodec = QString::fromUtf8(vcodec); mpv_free(vcodec); }

    char *acodec = mpv_get_property_string(m_mpv, "audio-codec");
    if (acodec) { meta.audioCodec = QString::fromUtf8(acodec); mpv_free(acodec); }

    char *fmt = mpv_get_property_string(m_mpv, "file-format");
    if (fmt) { meta.format = QString::fromUtf8(fmt); mpv_free(fmt); }

    int64_t w = 0, h = 0, size = 0, vbit = 0, abit = 0, achans = 0, asrate = 0;
    double fps = 0.0, dur = 0.0;

    mpv_get_property(m_mpv, "dwidth", MPV_FORMAT_INT64, &w);
    mpv_get_property(m_mpv, "dheight", MPV_FORMAT_INT64, &h);
    mpv_get_property(m_mpv, "file-size", MPV_FORMAT_INT64, &size);
    mpv_get_property(m_mpv, "video-bitrate", MPV_FORMAT_INT64, &vbit);
    mpv_get_property(m_mpv, "audio-bitrate", MPV_FORMAT_INT64, &abit);
    mpv_get_property(m_mpv, "audio-params/channel-count", MPV_FORMAT_INT64, &achans);
    mpv_get_property(m_mpv, "audio-params/samplerate", MPV_FORMAT_INT64, &asrate);
    mpv_get_property(m_mpv, "container-fps", MPV_FORMAT_DOUBLE, &fps);
    mpv_get_property(m_mpv, "duration", MPV_FORMAT_DOUBLE, &dur);

    meta.videoWidth = static_cast<int>(w);
    meta.videoHeight = static_cast<int>(h);
    meta.fileSize = size;
    meta.videoBitrate = vbit;
    meta.audioBitrate = abit;
    meta.audioChannels = static_cast<int>(achans);
    meta.audioSampleRate = static_cast<int>(asrate);
    meta.videoFps = fps;
    meta.duration = dur;

    return meta;
}

int OrionEngine::executeCommand(const QStringList &args) {
    if (!m_mpv || args.isEmpty()) return -1;
    QVector<QByteArray> utf8Args;
    QVector<const char *> cArgs;
    utf8Args.reserve(args.size());
    cArgs.reserve(args.size() + 1);
    for (const auto &arg : args) {
        utf8Args.append(arg.toUtf8());
        cArgs.append(utf8Args.last().constData());
    }
    cArgs.append(nullptr);
    return mpv_command(m_mpv, cArgs.data());
}
