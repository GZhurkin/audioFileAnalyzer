#pragma once
#ifndef AUDIOMODEL_H
#define AUDIOMODEL_H

#include <QObject>
#include <QString>
#include <QVector>

class AudioModel : public QObject
{
    Q_OBJECT

public:
    struct Meta
    {
        double durationSeconds = 0.0;
        quint32 sampleRate = 0;
        quint32 byteRate = 0;
        quint16 channels = 0;
        quint16 bitsPerSample = 0;
        quint32 bitRate = 0;
    };

    explicit AudioModel(QObject *parent = nullptr);

    bool loadWav(const QString &filePath, Meta &outMeta, QString &errorString);

signals:
    void metadataReady(const AudioModel::Meta &m);
    void waveformReady(const QVector<double> &samples, quint32 rate);
    void spectrumReady(const QVector<double> &freq, const QVector<double> &amp);
    void spectrogramReady(const QVector<QVector<double>> &frames);
    void errorOccurred(const QString &error);

private:
    void calculateSpectrum(const QVector<double> &samples, quint32 sampleRate);
    void calculateSpectrogram(const QVector<double> &samples, quint32 sampleRate);
};

#endif // AUDIOMODEL_H
