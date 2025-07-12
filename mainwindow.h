#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAudioOutput>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QSlider>
#include <QLabel>
#include <QString>

#include "audiomodel.h"
#include "spectrogramview.h"
#include "waveformview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

    void onOpenFile();

    void onMetadataReady(const AudioModel::Meta &meta);

    void onWaveformReady(const QVector<double> &samples, quint32 sampleRate);

    void onSpectrogramReady(const QVector<QVector<double>> &frames);

    void onError(const QString &err);

    void onPositionChanged(qint64 pos);

private:
    AudioModel *m_model;

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    WaveformView *m_waveform;
    SpectrogramView *m_spectrogram;

    QSlider *m_progressSlider;
    QLabel *m_metadatalabel;
};

#endif // MAINWINDOW_H
