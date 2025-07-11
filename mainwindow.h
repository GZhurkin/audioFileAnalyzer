#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>

#include "audiomodel.h"
#include "waveformview.h"
#include "spectrumview.h"
#include "spectrogramview.h"

/**
 * Главное окно приложения визуализации аудиофайлов.
 *
 * Отвечает за:
 * - Отображение интерфейса;
 * - Связь между моделью и представлениями;
 * - Обработку взаимодействия пользователя;
 * - Управление воспроизведением.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    /// Обработка открытия нового WAV-файла
    void onOpenFile();

    /// Обработка готовности метаданных
    void onMetadataReady(const AudioModel::Meta& meta);

    /// Обработка готовности осциллограммы
    void onWaveformReady(const QVector<double>& samples, quint32 sampleRate);

    /// Обработка готовности спектра
    void onSpectrumReady(const QVector<double>& freq, const QVector<double>& amp);

    /// Обработка готовности спектрограммы
    void onSpectrogramReady(const QVector<QVector<double>>& frames);

    /// Отображение ошибки
    void onError(const QString& err);

    /// Обработка изменения позиции воспроизведения
    void onPositionChanged(qint64 pos);

private:
    // Модель аудио-данных
    AudioModel*      m_model;

    // Компоненты для воспроизведения звука
    QMediaPlayer*    m_player;
    QAudioOutput*    m_audioOutput;

    // Виджеты визуализации
    WaveformView*    m_waveform;
    SpectrumView*    m_spectrum;
    SpectrogramView* m_spectrogram;

    // Слайдер для отображения/изменения позиции воспроизведения
    QSlider*         m_progressSlider;
};
