#include "audiomodel.h"

#include <QFile>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>
#include <cmath>
#include <algorithm>

AudioModel::AudioModel(QObject* parent)
    : QObject(parent)
{}

bool AudioModel::loadWav(const QString& filePath, Meta& outMeta, QString& errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorString = tr("Не удалось открыть файл: %1").arg(filePath);
        emit errorOccurred(errorString);
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // Проверка заголовка RIFF
    char riff[4];
    in.readRawData(riff, 4);
    quint32 riffSize;
    in >> riffSize;
    char wave[4];
    in.readRawData(wave, 4);
    if (strncmp(riff, "RIFF", 4) != 0 || strncmp(wave, "WAVE", 4) != 0) {
        errorString = tr("Файл не является WAV (не найден RIFF/WAVE).");
        emit errorOccurred(errorString);
        return false;
    }

    // Поиск чанка fmt
    quint16 audioFormat = 0, numChannels = 0, bitsPerSample = 0;
    quint32 sampleRate = 0, byteRate = 0;
    while (!in.atEnd()) {
        char chunkId[4];
        if (in.readRawData(chunkId, 4) != 4)
            break;
        quint32 chunkSize;
        in >> chunkSize;

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            in >> audioFormat >> numChannels >> sampleRate >> byteRate;
            quint16 blockAlign;
            in >> blockAlign >> bitsPerSample;

            if (chunkSize > 16)
                file.seek(file.pos() + (chunkSize - 16));
            break;
        } else {
            file.seek(file.pos() + chunkSize);
        }
    }

    if (audioFormat != 1) {
        errorString = tr("Поддерживается только PCM WAV.");
        emit errorOccurred(errorString);
        return false;
    }

    // Поиск чанка data
    quint32 dataSize = 0;
    while (!in.atEnd()) {
        char chunkId[4];
        if (in.readRawData(chunkId, 4) != 4)
            break;
        quint32 chunkSize;
        in >> chunkSize;

        if (strncmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            break;
        } else {
            file.seek(file.pos() + chunkSize);
        }
    }

    if (dataSize == 0) {
        errorString = tr("Чанк data не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // Метаданные
    const double durationSec = double(dataSize) / byteRate;
    outMeta = { durationSec, sampleRate, byteRate, numChannels, bitsPerSample, byteRate * 8 };
    emit metadataReady(outMeta);

    // Чтение данных
    QVector<double> samples;
    const int bytesPerSample = bitsPerSample / 8;
    const qint64 totalSamples = dataSize / (numChannels * bytesPerSample);
    samples.reserve(totalSamples);

    for (qint64 i = 0; i < totalSamples; ++i) {
        double sampleSum = 0.0;

        for (int ch = 0; ch < numChannels; ++ch) {
            if (bitsPerSample == 16) {
                qint16 val;
                in >> val;
                sampleSum += val;
            } else if (bitsPerSample == 8) {
                quint8 val;
                in >> val;
                sampleSum += (val - 128) * 256;
            } else {
                file.seek(file.pos() + bytesPerSample);
            }
        }

        double normalized = sampleSum / (numChannels * 32768.0);
        samples.append(normalized);
    }

    emit waveformReady(samples, sampleRate);

    // Спектр — вычисляется отдельно (например, через kissfft).
    emit spectrumReady({}, {}); // пустой — если ещё не реализовано

    return true;
}
