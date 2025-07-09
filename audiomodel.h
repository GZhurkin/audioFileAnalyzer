#ifndef AUDIOMODEL_H
#define AUDIOMODEL_H

#pragma once

#include <QObject>
#include <QString>

class AudioModel : public QObject {
    Q_OBJECT
public:
    struct Meta {
        double    durationSeconds = 0.0;
        quint32   sampleRate      = 0;
        quint32   byteRate        = 0;
        quint16   channels        = 0;
        quint16   bitsPerSample   = 0;
        quint32   bitRate         = 0; // в битах в секунду
    };

    explicit AudioModel(QObject* parent = nullptr);

    // Загружает WAV-файл, парсит заголовок, заполняет outMeta и возвращает true при успехе
    bool loadWav(const QString& filePath, Meta& outMeta, QString& errorString);

signals:
    void metadataReady(const AudioModel::Meta& m);
    void errorOccurred(const QString& error);

};

#endif // AUDIOMODEL_H
