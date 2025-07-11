#pragma once
#ifndef AUDIOMODEL_H
#define AUDIOMODEL_H

#include <QObject>
#include <QString>
#include <QVector>

/**
 * Модель для работы с WAV-файлом.
 *
 * Отвечает за:
 * - Загрузку WAV-файла и парсинг его структуры;
 * - Выделение аудиосемплов;
 * - Расчет спектра (FFT) и спектрограммы;
 * - Эмиссию сигналов с готовыми данными.
 */
class AudioModel : public QObject {
    Q_OBJECT

public:
    /// Структура с метаданными загруженного WAV-файла
    struct Meta {
        double    durationSeconds = 0.0; ///< Продолжительность в секундах
        quint32   sampleRate      = 0;   ///< Частота дискретизации (Гц)
        quint32   byteRate        = 0;   ///< Байтрейт (байт/с)
        quint16   channels        = 0;   ///< Кол-во каналов (1 — моно, 2 — стерео)
        quint16   bitsPerSample   = 0;   ///< Битность каждого сэмпла
        quint32   bitRate         = 0;   ///< Битрейт (бит/с)
    };

    explicit AudioModel(QObject* parent = nullptr);

    /**
     * Загружает WAV-файл с диска, парсит его и извлекает данные.
     *
     * filePath Путь к WAV-файлу.
     * outMeta Выходная структура с метаданными.
     * errorString Описание ошибки, если возникла.
     * true — если файл успешно загружен.
     */
    bool loadWav(const QString& filePath, Meta& outMeta, QString& errorString);

signals:
    void metadataReady(const AudioModel::Meta& m);                        ///< Сигнал после успешной загрузки метаданных
    void waveformReady(const QVector<double>& samples, quint32 rate);     ///< Сигнал с массивом амплитуд
    void spectrumReady(const QVector<double>& freq, const QVector<double>& amp); ///< Сигнал с частотами и амплитудами спектра
    void spectrogramReady(const QVector<QVector<double>>& frames);        ///< Сигнал с 2D-спектрограммой (кадр/амплитуда)
    void errorOccurred(const QString& error);                             ///< Сигнал об ошибке (например, неверный формат файла)

private:
    void calculateSpectrum(const QVector<double>& samples, quint32 sampleRate);     ///< Расчет спектра (быстрое преобразование Фурье)
    void calculateSpectrogram(const QVector<double>& samples, quint32 sampleRate);  ///< Расчет спектрограммы (кадры по времени)
};

#endif // AUDIOMODEL_H
