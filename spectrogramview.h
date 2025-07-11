#pragma once
#ifndef SPECTROGRAMVIEW_H
#define SPECTROGRAMVIEW_H

#include <QWidget>
#include <QImage>
#include <QVector>
#include <QMutex>

/**
 * Виджет для отображения спектрограммы.
 *
 * Спектрограмма представляет собой визуализацию частотного спектра
 * аудио с течением времени. Каждый столбец — спектр на одном временном срезе.
 * Для отображения используется QImage, который обновляется при добавлении новых данных.
 */
class SpectrogramView : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrogramView(QWidget* parent = nullptr);
    ~SpectrogramView() override = default;

public slots:
    /**
     * Добавить один срез спектра (полосы частот и амплитуды).
     * freqBins - массив частотных полос (используется только размер)
     * magnitudes - амплитуды для каждой полосы частот
     *
     * Требует, чтобы freqBins и magnitudes имели одинаковую длину.
     * Сохраняет срез, поддерживает ограничение по числу срезов.
     * Обновляет изображение и виджет.
     */
    void addSpectrumSlice(const QVector<double>& freqBins, const QVector<double>& magnitudes);

    /// Очистить данные спектрограммы и изображение
    void clear();

    /// Установить всю спектрограмму целиком (вектор векторов амплитуд)
    void setSpectrogramData(const QVector<QVector<double>>& data);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QImage m_image; ///< Кэшированное изображение спектрограммы для отрисовки
    int m_maxTimeSlices = 500; ///< Максимальное число временных срезов (столбцов)

    int m_freqBinCount = 0; ///< Количество частотных полос (высота спектрограммы)

    QVector<QVector<double>> m_spectrogramData; ///< Данные амплитуд: [время][частота]
    QMutex m_mutex; ///< Мьютекс для потокобезопасности при обновлении данных

    /// Пересчитать изображение спектрограммы на основе данных m_spectrogramData
    void updateImage();

    /// Преобразовать амплитуду в цвет (например, оттенок желтого)
    QColor magnitudeToColor(double magnitude) const;
};

#endif // SPECTROGRAMVIEW_H
