#include "audiomodel.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <cmath>
#include <cstring>

extern "C" {
#include "kissfft/kiss_fft.h"
#include "kissfft/kiss_fftr.h"
}

AudioModel::AudioModel(QObject *parent)
    : QObject(parent)
{}

// Загрузка WAV-файла и извлечение данных
bool AudioModel::loadWav(const QString &filePath, Meta &outMeta, QString &errorString)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        errorString = tr("Не удалось открыть файл %1").arg(filePath);
        emit errorOccurred(errorString);
        return false;
    }

    QDataStream in(&f);
    in.setByteOrder(QDataStream::LittleEndian); // WAV использует little-endian

    // Проверка RIFF заголовка
    char riff[4];
    in.readRawData(riff, 4);
    if (std::strncmp(riff, "RIFF", 4) != 0) {
        errorString = tr("Это не WAV (нет RIFF).");
        emit errorOccurred(errorString);
        return false;
    }

    quint32 riffSize;
    in >> riffSize;

    char wave[4];
    in.readRawData(wave, 4);
    if (std::strncmp(wave, "WAVE", 4) != 0) {
        errorString = tr("Это не WAV (нет WAVE).");
        emit errorOccurred(errorString);
        return false;
    }

    // Поиск чанка 'fmt '
    bool fmtFound = false;
    quint32 fmtChunkSize = 0;
    quint16 audioFormat = 0;

    while (!in.atEnd()) {
        char chunkId[4];
        in.readRawData(chunkId, 4);
        quint32 chunkSize;
        in >> chunkSize;
        if (std::strncmp(chunkId, "fmt ", 4) == 0) {
            fmtFound = true;
            fmtChunkSize = chunkSize;
            break;
        } else {
            f.seek(f.pos() + chunkSize); // Пропуск неизвестных чанков
        }
    }
    if (!fmtFound) {
        errorString = tr("Чанк fmt не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // Чтение параметров аудио
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

    // Поддерживается только PCM
    if (audioFormat != 1) {
        errorString = tr("Поддерживается только несжатый формат PCM.");
        emit errorOccurred(errorString);
        return false;
    }

    // Пропуск дополнительных данных в fmt чанке
    if (fmtChunkSize > 16) {
        f.seek(f.pos() + (fmtChunkSize - 16));
    }

    // Поиск чанка с аудиоданными ('data')
    bool dataFound = false;
    quint32 dataSize = 0;

    while (!in.atEnd()) {
        char chunkId[4];
        in.readRawData(chunkId, 4);
        quint32 chunkSize;
        in >> chunkSize;
        if (std::strncmp(chunkId, "data", 4) == 0) {
            dataFound = true;
            dataSize = chunkSize;
            break;
        } else {
            f.seek(f.pos() + chunkSize); // Пропуск других чанков
        }
    }
    if (!dataFound) {
        errorString = tr("Чанк data не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // Формирование метаданных
    double durationSec = double(dataSize) / byteRate;
    quint32 bitRate = byteRate * 8;
    outMeta = {durationSec, sampleRate, byteRate, numChannels, bitsPerSample, bitRate};
    emit metadataReady(outMeta);

    // Чтение и обработка сэмплов
    QVector<double> samples;
    qint64 numSamples = dataSize / (numChannels * (bitsPerSample / 8));
    samples.reserve(numSamples);

    for (qint64 i = 0; i < numSamples; ++i) {
        double currentSample = 0.0;
        // Смешивание каналов (если аудио многоканальное)
        for (int ch = 0; ch < numChannels; ++ch) {
            if (bitsPerSample == 16) {
                qint16 val;
                in >> val;
                currentSample += val;
            } else if (bitsPerSample == 8) {
                quint8 val;
                in >> val;
                currentSample += (val - 128) * 256; // Конвертация 8-bit в signed
            } else {
                f.seek(f.pos() + (bitsPerSample / 8)); // Пропуск неподдерживаемых форматов
            }
        }
        currentSample /= numChannels;            // Усреднение по каналам
        samples.append(currentSample / 32768.0); // Нормализация [-1.0, 1.0]
    }

    emit waveformReady(samples, sampleRate);

    // Вычисление спектральных характеристик
    calculateSpectrum(samples, sampleRate);
    calculateSpectrogram(samples, sampleRate);

    return true;
}

// ИЗМЕНЕН calculateSpectrum
void AudioModel::calculateSpectrum(const QVector<double> &samples, quint32 sampleRate)
{
    const int fftSize = 2048;
    int n = qMin(samples.size(), fftSize);

    kiss_fft_cfg cfg = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);
    if (!cfg) {
        emit errorOccurred(tr("Не удалось инициализировать kissfft"));
        return;
    }

    QVector<kiss_fft_cpx> input(fftSize);
    QVector<kiss_fft_cpx> output(fftSize);

    // Применяем оконную функцию Ханна для уменьчения артефактов
    for (int i = 0; i < fftSize; ++i) {
        double window = 0.5 * (1 - cos(2 * M_PI * i / (fftSize - 1)));
        input[i].r = (i < n) ? samples[i] * window : 0.0;
        input[i].i = 0.0;
    }

    kiss_fft(cfg, input.data(), output.data());

    QVector<double> frequencies;
    QVector<double> amplitudes;

    frequencies.reserve(fftSize / 2);
    amplitudes.reserve(fftSize / 2);

    for (int i = 0; i < fftSize / 2; ++i) {
        double freq = i * double(sampleRate) / fftSize;
        double amp = std::sqrt(output[i].r * output[i].r + output[i].i * output[i].i);

        // Правильный расчет dB (без инверсии)
        double dB = 20 * log10(amp + 1e-12); // +1e-12 чтобы избежать log(0)

        frequencies.append(freq);
        amplitudes.append(dB);
    }

    free(cfg);

    emit spectrumReady(frequencies, amplitudes);
}

// Вычисление спектрограммы
void AudioModel::calculateSpectrogram(const QVector<double> &samples, quint32 sampleRate)
{
    const int fftSize = 512;
    const int hopSize = fftSize / 2; // 50% перекрытие окон
    int numFrames = (samples.size() - fftSize) / hopSize;
    if (numFrames <= 0)
        return;

    kiss_fft_cfg cfg = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);
    if (!cfg) {
        emit errorOccurred(tr("Не удалось инициализировать kissfft"));
        return;
    }

    QVector<QVector<double>> spectrogram;
    spectrogram.reserve(numFrames);

    // Подготовка оконной функции (Ханна)
    QVector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (fftSize - 1)));
    }

    // Обработка кадров
    QVector<kiss_fft_cpx> input(fftSize);
    QVector<kiss_fft_cpx> output(fftSize);

    for (int frame = 0; frame < numFrames; ++frame) {
        int offset = frame * hopSize;

        // Применение оконной функции
        for (int i = 0; i < fftSize; ++i) {
            input[i].r = samples[offset + i] * window[i];
            input[i].i = 0.0;
        }

        // Выполнение FFT для текущего кадра
        kiss_fft(cfg, input.data(), output.data());

        // Вычисление магнитуд
        QVector<double> magnitudes(fftSize / 2);
        for (int i = 0; i < fftSize / 2; ++i) {
            double re = output[i].r;
            double im = output[i].i;
            magnitudes[i] = std::sqrt(re * re + im * im);
        }

        spectrogram.append(magnitudes);
    }

    free(cfg);
    emit spectrogramReady(spectrogram);
}
