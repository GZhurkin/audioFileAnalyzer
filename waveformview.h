#pragma once
#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QWidget>
#include <QVector>
#include <QScrollBar>
#include <QPainterPath>

/**
 * Виджет для отображения осциллограммы аудиофайла.
 *
 * Поддерживает:
 * - горизонтальный скроллинг;
 * - масштабирование (зум) с помощью колесика мыши;
 * - отображение и перемещение маркера (позиции воспроизведения);
 * - кэширование пути для оптимальной отрисовки.
 */
class WaveformView : public QWidget {
    Q_OBJECT

public:
    /// Конструктор
    explicit WaveformView(QWidget* parent = nullptr);

public slots:
    /**
     * Устанавливает аудиоданные и частоту дискретизации.
     *
     * samples Массив значений амплитуды от -1.0 до 1.0
     *  sampleRate Частота дискретизации в Гц
     */
    void setSamples(const QVector<double>& samples, quint32 sampleRate);

    /**
     * Устанавливает положение маркера в секундах.
     * Значение будет ограничено длиной сигнала.
     */
    void setMarkerPosition(double seconds);

signals:
    /**
     * Сигнал, испускаемый при изменении положения маркера (например, пользователем).
     *
     * seconds Новая позиция в секундах.
     */
    void markerPositionChanged(double seconds);

protected:
    // Переопределение событий Qt:
    void paintEvent(QPaintEvent* ev) override;          ///< Отрисовка осциллограммы и маркера
    void resizeEvent(QResizeEvent* ev) override;        ///< Обновление скроллбара при изменении размера
    void mousePressEvent(QMouseEvent* ev) override;     ///< Начало перетаскивания маркера
    void mouseMoveEvent(QMouseEvent* ev) override;      ///< Перемещение маркера
    void mouseReleaseEvent(QMouseEvent* ev) override;   ///< Завершение перетаскивания
    void wheelEvent(QWheelEvent* ev) override;          ///< Зумирование (Ctrl + колесо)

private:
    QVector<double> m_samples;       ///< Аудиоданные в нормализованном виде
    quint32 m_sampleRate = 0;        ///< Частота дискретизации
    double m_markerSec = 0.0;        ///< Позиция маркера в секундах

    // Управление масштабом
    double m_zoom = 1.0;             ///< Текущий зум
    const double m_minZoom = 0.5;   ///< Минимальное приближение
    const double m_maxZoom = 100.0; ///< Максимальное приближение (расширено)

    QScrollBar* m_hScroll = nullptr;///< Горизонтальный скроллбар
    bool m_draggingMarker = false;  ///< Признак перетаскивания маркера

    // Кэширование для оптимизации отрисовки
    QPainterPath m_cachedPath;      ///< Предрасчитанный путь осциллограммы
    int m_cachedOffset = -1;        ///< Последний offset скролла, для сравнения
    QSize m_cachedSize;             ///< Последний размер области отрисовки

    // Вспомогательные методы
    void updateScroll();            ///< Перерасчет параметров скроллбара
    void updateMarkerFromPos(int x);///< Обновление позиции маркера по координате X
    void updateCachedPath();        ///< Перестроение формы сигнала для текущего масштаба и смещения
};

#endif // WAVEFORMVIEW_H
