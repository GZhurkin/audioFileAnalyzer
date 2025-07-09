#include "audiomodel.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

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
            // пропустить содержимое чанка
            f.seek(f.pos() + chunkSize);
        }
    }
    if (!fmtFound) {
        errorString = tr("Чанк fmt не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // 2.1) Читаем параметры fmt-чана
    in >> audioFormat;              // аудио формат (1 = PCM)
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

    // Если fmtChunkSize > 16, пропустить доп. байты
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

    // 4) Вычисляем метаданные
    double durationSec = double(dataSize) / double(byteRate);
    quint32 bitRate = byteRate * 8;

    outMeta.durationSeconds = durationSec;
    outMeta.sampleRate      = sampleRate;
    outMeta.byteRate        = byteRate;
    outMeta.channels        = numChannels;
    outMeta.bitsPerSample   = bitsPerSample;
    outMeta.bitRate         = bitRate;

    emit metadataReady(outMeta);
    return true;
}
