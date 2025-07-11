#include "audiomodel.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <cmath>
#include <cstring>

// Подключение kissfft C-style
extern "C" {
#include "kissfft/kiss_fft.h"
#include "kissfft/kiss_fftr.h"
}

AudioModel::AudioModel(QObject* parent)
    : QObject(parent)
{}

/**
 * Загружает WAV-файл, извлекает метаданные и аудиосемплы, рассчитывает спектр и спектрограмму.
 */
bool AudioModel::loadWav(const QString& filePath, Meta& outMeta, QString& errorString)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        errorString = tr("Не удалось открыть файл %1").arg(filePath);
        emit errorOccurred(errorString);
        return false;
    }

    QDataStream in(&f);
    in.setByteOrder(QDataStream::LittleEndian);

    // Проверка RIFF-заголовка
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

    // Поиск чанка fmt
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
            f.seek(f.pos() + chunkSize); // пропустить ненужный чанк
        }
    }
    if (!fmtFound) {
        errorString = tr("Чанк fmt не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // Считываем параметры формата
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

    if (audioFormat != 1) {
        errorString = tr("Поддерживается только несжатый формат PCM.");
        emit errorOccurred(errorString);
        return false;
    }

    if (fmtChunkSize > 16) {
        f.seek(f.pos() + (fmtChunkSize - 16)); // Пропустить оставшиеся поля, если есть
    }

    // Поиск чанка data
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
            f.seek(f.pos() + chunkSize); // пропустить
        }
    }
    if (!dataFound) {
        errorString = tr("Чанк data не найден.");
        emit errorOccurred(errorString);
        return false;
    }

    // Подготовка метаданных
    double durationSec = double(dataSize) / byteRate;
    quint32 bitRate = byteRate * 8;

    outMeta = { durationSec, sampleRate, byteRate, numChannels, bitsPerSample, bitRate };
    emit metadataReady(outMeta);

    // Чтение аудиоданных
    QVector<double> samples;
    qint64 numSamples = dataSize / (numChannels * (bitsPerSample / 8));
    samples.reserve(numSamples);

    for (qint64 i = 0; i < numSamples; ++i) {
        double currentSample = 0.0;
        for (int ch = 0; ch < numChannels; ++ch) {
            if (bitsPerSample == 16) {
                qint16 val;
                in >> val;
                currentSample += val;
            } else if (bitsPerSample == 8) {
                quint8 val;
                in >> val;
                currentSample += (val - 128) * 256;
            } else {
                f.seek(f.pos() + (bitsPerSample / 8)); // skip unsupported format
            }
        }
        currentSample /= numChannels;
        samples.append(currentSample / 32768.0); // нормализация
    }

    emit waveformReady(samples, sampleRate);

    // Запуск анализа
    calculateSpectrum(samples, sampleRate);
    calculateSpectrogram(samples, sampleRate);

    return true;
}

/**
 *  Выполняет FFT и генерирует спектр.
 */
void AudioModel::calculateSpectrum(const QVector<double>& samples, quint32 sampleRate)
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

    for (int i = 0; i < fftSize; ++i) {
        input[i].r = (i < n) ? samples[i] : 0.0;
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
        frequencies.append(freq);
        amplitudes.append(amp);
    }

    free(cfg);

    emit spectrumReady(frequencies, amplitudes);
}

/**
 * Расчет спектрограммы с окном Хэннинга и перекрытием 50%.
 */
void AudioModel::calculateSpectrogram(const QVector<double>& samples, quint32 sampleRate)
{
    const int fftSize = 512;
    const int hopSize = fftSize / 2; // перекрытие 50%
    int numFrames = (samples.size() - fftSize) / hopSize;
    if (numFrames <= 0) return;

    kiss_fft_cfg cfg = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);
    if (!cfg) {
        emit errorOccurred(tr("Не удалось инициализировать kissfft"));
        return;
    }

    QVector<QVector<double>> spectrogram;
    spectrogram.reserve(numFrames);

    QVector<kiss_fft_cpx> input(fftSize);
    QVector<kiss_fft_cpx> output(fftSize);

    // Предрасчет окна Хэннинга
    QVector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (fftSize - 1)));
    }

    // Расчет кадров спектрограммы
    for (int frame = 0; frame < numFrames; ++frame) {
        int offset = frame * hopSize;
        for (int i = 0; i < fftSize; ++i) {
            input[i].r = samples[offset + i] * window[i];
            input[i].i = 0.0;
        }
        kiss_fft(cfg, input.data(), output.data());

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
