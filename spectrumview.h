#pragma once
#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QChartView>
#include <QSplineSeries>
#include <QValueAxis>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVector>

/**
 * Виджет для отображения спектра амплитуд по частотам.
 *
 * Использует QtCharts для построения графика с использованием
 * сглаженной кривой (QSplineSeries). Обеспечивает масштабирование
 * графика, а также сброс зума правой кнопкой мыши и управление
 * с клавиатуры.
 */
class SpectrumView : public QChartView
{
    Q_OBJECT

public:
    explicit SpectrumView(QWidget* parent = nullptr);
    ~SpectrumView() override = default;

public slots:
    /**
     * Установить данные спектра для отображения.
     * freq Массив частот (ось X).
     * amp Массив амплитуд (ось Y).
     *
     * Ожидается, что freq и amp имеют одинаковую длину.
     * Данные отображаются сглаженной кривой.
     */
    void setSpectrum(const QVector<double>& freq, const QVector<double>& amp);

protected:
    /// Обработка отпускания кнопки мыши (для сброса зума правой кнопкой)
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// Обработка нажатия клавиш (+/- для зума)
    void keyPressEvent(QKeyEvent* event) override;

private:
    QSplineSeries* m_series; ///< Серия данных для графика (сглаженная линия)
    QValueAxis*    m_axisX;  ///< Ось X (частота)
    QValueAxis*    m_axisY;  ///< Ось Y (амплитуда)

    /**
     * Обеспечить ненулевой диапазон оси, чтобы избежать проблем с отрисовкой.
     * Если min == max, расширяет диапазон на padding с обеих сторон.
     */
    void ensureNonZeroRange(double& min, double& max, double padding = 1.0);
};

#endif // SPECTRUMVIEW_H
