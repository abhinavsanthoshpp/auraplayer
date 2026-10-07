#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QMutex>

struct mpv_handle;

struct MediaTrack {
    int id = -1;
    QString type;      // "video", "audio", "sub"
    QString title;
    QString language;
    QString codec;
    bool isDefault = false;
    bool isSelected = false;
};

struct MediaMetadata {
    QString title;
    QString artist;
    QString album;
    QString format;
    int64_t fileSize = 0;
    double duration = 0.0;
    int videoWidth = 0;
    int videoHeight = 0;
    double videoFps = 0.0;
    QString videoCodec;
    QString audioCodec;
    int audioChannels = 0;
    int audioSampleRate = 0;
    int64_t videoBitrate = 0;
    int64_t audioBitrate = 0;
};

class AuraEngine : public QObject {
    Q_OBJECT

public:
    explicit AuraEngine(QObject *parent = nullptr);
    ~AuraEngine() override;

    bool initialize();
    mpv_handle *handle() const { return m_mpv; }

    // Playback control
    void loadFile(const QString &pathOrUrl, bool append = false);
    void play();
    void pause();
    void togglePause();
    void stop();
    void seek(double seconds, bool exact = false);
    void seekRelative(double deltaSeconds);
    void frameStep();
    void frameBackStep();

    // Audio & Speed
    void setVolume(double volume); // 0.0 to 200.0
    double volume() const { return m_volume; }
    void setMuted(bool mute);
    bool isMuted() const { return m_muted; }
    void toggleMute();
    void setSpeed(double speed);   // 0.25 to 4.0
    double speed() const { return m_speed; }

    // Track selection & sync
    QVector<MediaTrack> getTracks() const;
    void setAudioTrack(int trackId);
    void setSubtitleTrack(int trackId);
    void setSubtitleDelay(double deltaSeconds);
    double subtitleDelay() const;
    void setAudioDelay(double deltaSeconds);
    double audioDelay() const;

    // Video adjustments
    void setBrightness(int value); // -100 to 100
    void setContrast(int value);   // -100 to 100
    void setSaturation(int value); // -100 to 100
    void setGamma(int value);      // -100 to 100
    void setAspectRatio(const QString &ratio); // "default", "16:9", "4:3", "2.35:1", "fill"

    // Audio Equalizer (10 bands in dB, -12 to +12)
    void setEqualizerBands(const QVector<double> &bands);

    // Snapshot & Info
    void takeScreenshot(const QString &destinationPath = QString());
    MediaMetadata getMetadata() const;
    QString currentFilePath() const { return m_currentFile; }

    // Direct command execution
    int executeCommand(const QStringList &args);

signals:
    void playbackStarted();
    void playbackStopped();
    void playbackPaused(bool paused);
    void positionChanged(double seconds);
    void durationChanged(double seconds);
    void volumeChanged(double volume);
    void muteChanged(bool muted);
    void speedChanged(double speed);
    void fileLoaded(const QString &filePath);
    void tracksChanged();
    void videoReconfigured(int width, int height);
    void logMessage(const QString &prefix, const QString &text);
    void screenshotTaken(const QString &path);

private slots:
    void onMpvEvents();

private:
    static void wakeupCallback(void *ctx);
    void setupHardwareAcceleration();
    void observeProperties();
    void handleMpvEvent(void *eventPtr);

    mpv_handle *m_mpv = nullptr;
    QString m_currentFile;
    double m_position = 0.0;
    double m_duration = 0.0;
    double m_volume = 100.0;
    double m_speed = 1.0;
    bool m_paused = false;
    bool m_muted = false;
    QMutex m_mutex;
};
