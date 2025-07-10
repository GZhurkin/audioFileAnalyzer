#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include "audiomodel.h"
#include "waveformview.h"
#include "spectrumview.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onOpenFile();
    void onMetadataReady(const AudioModel::Meta& m);
    void onWaveformReady(const QVector<double>& samples, quint32 sampleRate);
    void onSpectrumReady(const QVector<double>& freq, const QVector<double>& amp);
    void onError(const QString& err);
    void onPositionChanged(qint64 pos);

private:
    AudioModel*     m_model;
    QMediaPlayer*   m_player;
    QAudioOutput*   m_audioOutput;
    WaveformView*   m_waveform;
    SpectrumView*   m_spectrum;
    QSlider*        m_progressSlider;
    quint32         m_sampleRate = 0;
};

#endif // MAINWINDOW_H
