#include "audiomodel.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <random>

AudioModel::AudioModel(QObject* parent)
    : QObject(parent)
{}

bool AudioModel::loadWav(const QString& filePath, Meta& outMeta, QString& errorString)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        errorString = QObject::tr("Не удалось открыть файл %1").arg(filePath);
        emit errorOccurred(errorString);
        return false;
    }

    QDataStream in(&f);
    in.setByteOrder(QDataStream::LittleEndian);

    // 1) RIFF header
    char riff[4];
    in.readRawData(riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0) {
        errorString = tr("Это не WAV (нет RIFF).");
        emit errorOccurred(errorString);
        return false;
    }
    quint32 riffSize;
    in >> riffSize;
    char wave[4];
    in.readRawData(wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0) {
        errorString = tr("Это не WAV (нет WAVE).");
        emit errorOccurred(errorString);
        return false;
    }

    // 2) Ищем подчанк "fmt "
    bool fmtFound = false;
    quint32 fmtChunkSize = 0;
    quint16 audioFormat = 0;
    while (!in.atEnd()) {
        char chunkId[4];
        in.readRawData(chunkId, 4);
        quint32 chunkSize;
        in >> chunkSize;

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            fmtFound = true;
            fmtChunkSize = chunkSize;
            break;
        } else {
            f.seek(f.pos() + chunkSize);
        }
    }
    if (!fmtFound) {
        errorString = tr("Чанк fmt не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // 2.1) Читаем параметры fmt-чана
    in >> audioFormat;
    quint16 numChannels;
    in >> numChannels;
    quint32 sampleRate;
    in >> sampleRate;
    quint32 byteRate;
    in >> byteRate;
    quint16 blockAlign;
    in >> blockAlign;
    quint16 bitsPerSample;
    in >> bitsPerSample;

    if (audioFormat != 1) { // 1 = PCM
        errorString = tr("Поддерживается только несжатый формат PCM.");
        emit errorOccurred(errorString);
        return false;
    }

    if (fmtChunkSize > 16) {
        f.seek(f.pos() + (fmtChunkSize - 16));
    }

    // 3) Ищем подчанк "data"
    bool dataFound = false;
    quint32 dataSize = 0;
    while (!in.atEnd()) {
        char chunkId[4];
        in.readRawData(chunkId, 4);
        quint32 chunkSize;
        in >> chunkSize;
        if (strncmp(chunkId, "data", 4) == 0) {
            dataFound = true;
            dataSize = chunkSize;
            break;
        } else {
            f.seek(f.pos() + chunkSize);
        }
    }
    if (!dataFound) {
        errorString = tr("Чанк data не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // 4) Вычисляем метаданные и сообщаем о них
    double durationSec = double(dataSize) / double(byteRate);
    quint32 bitRate = byteRate * 8;

    outMeta.durationSeconds = durationSec;
    outMeta.sampleRate      = sampleRate;
    outMeta.byteRate        = byteRate;
    outMeta.channels        = numChannels;
    outMeta.bitsPerSample   = bitsPerSample;
    outMeta.bitRate         = bitRate;
    emit metadataReady(outMeta);

    // 5) Читаем аудиоданные и генерируем осциллограмму
    QVector<double> samples;
    qint64 numSamples = (dataSize / (numChannels * (bitsPerSample / 8)));
    samples.reserve(numSamples);

    for (qint64 i = 0; i < numSamples; ++i) {
        double currentSample = 0.0;
        for(int j = 0; j < numChannels; ++j) {
            if (bitsPerSample == 16) {
                qint16 sampleValue;
                in >> sampleValue;
                currentSample += sampleValue;
            } else if (bitsPerSample == 8) {
                quint8 sampleValue;
                in >> sampleValue;
                currentSample += (sampleValue - 128) * 256; // Приводим к 16 битам
            } else {
                f.seek(f.pos() + (bitsPerSample / 8)); // Пропускаем
            }
        }
        currentSample /= numChannels; // Простое микширование каналов в моно

        // Нормализация в диапазон [-1.0, 1.0]
        if (bitsPerSample == 16) {
            samples.append(currentSample / 32768.0);
        } else if (bitsPerSample == 8) {
            samples.append(currentSample / 32768.0);
        }
    }
    emit waveformReady(samples, sampleRate);


    // 6) Расчет спектра (ЗАГЛУШКА)
    // В реальном проекте здесь должен быть вызов библиотеки FFT (например, kissfft)
    // Для демонстрации генерируем случайные данные
    QVector<double> frequencies;
    QVector<double> amplitudes;
    int spectrumSize = 1024;
    frequencies.reserve(spectrumSize);
    amplitudes.reserve(spectrumSize);
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> distrib(0, 0.8);

    for(int k = 0; k < spectrumSize; ++k) {
        frequencies.append(k * double(sampleRate) / (spectrumSize * 2));
        double amp = distrib(gen);
        if (k > 50 && k < 200) { // Имитируем пик
            amp *= (1 - (abs(k - 125)/75.0)) * 1.2;
        }
        amplitudes.append(amp);
    }
    emit spectrumReady(frequencies, amplitudes);


    return true;
}
