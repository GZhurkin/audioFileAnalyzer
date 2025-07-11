#pragma once
#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QWidget>
#include <QVector>
#include <QScrollBar>
#include <QPainterPath>

/**
 * Виджет для отображения осциллограммы аудиофайла с поддержкой зума, скролла и маркера времени.
 */
class WaveformView : public QWidget {
    Q_OBJECT

public:
    /**
     * Конструктор виджета.
     * parent Родительский виджет.
     */
    explicit WaveformView(QWidget* parent = nullptr);

public slots:
    /**
     * Устанавливает аудиоданные и частоту дискретизации.
     * samples Массив отсчётов сигнала, нормированных в диапазоне [-1, 1].
     * sampleRate Частота дискретизации в Гц.
     */
    void setSamples(const QVector<double>& samples, quint32 sampleRate);

    /**
     * Устанавливает положение маркера в секундах.
     * seconds Время в секундах (ограничено длиной файла).
     */
    void setMarkerPosition(double seconds);

signals:
    /**
     * Сигнал об изменении положения маркера.
     * seconds Новое положение маркера в секундах.
     */
    void markerPositionChanged(double seconds);

protected:
    /// Отрисовка осциллограммы и маркера.
    void paintEvent(QPaintEvent* ev) override;

    /// Обработка изменения размера окна.
    void resizeEvent(QResizeEvent* ev) override;

    /// Обработка нажатия мыши (для перемещения маркера).
    void mousePressEvent(QMouseEvent* ev) override;

    /// Обработка перемещения мыши (перетаскивание маркера).
    void mouseMoveEvent(QMouseEvent* ev) override;

    /// Обработка отпускания кнопки мыши.
    void mouseReleaseEvent(QMouseEvent* ev) override;

    /// Обработка колесика мыши (зум при Ctrl).
    void wheelEvent(QWheelEvent* ev) override;

private:
    QVector<double> m_samples;        ///< Отсчёты аудиосигнала.
    quint32 m_sampleRate = 0;         ///< Частота дискретизации.
    double m_markerSec = 0.0;         ///< Положение маркера (в секундах).

    double m_zoom = 1.0;              ///< Текущий коэффициент зума.
    const double m_minZoom = 0.5;     ///< Минимальный зум.
    const double m_maxZoom = 100.0;   ///< Максимальный зум.

    QScrollBar* m_hScroll = nullptr;  ///< Горизонтальный скроллбар.
    bool m_draggingMarker = false;    ///< Флаг перетаскивания маркера.

    QPainterPath m_cachedPath;        ///< Кэшированный путь осциллограммы.
    int m_cachedOffset = -1;          ///< Последнее смещение, для проверки актуальности кэша.
    QSize m_cachedSize;               ///< Последний размер области отрисовки.

    /**
     * Обновляет параметры скроллбара в зависимости от текущего зума и размера виджета.
     */
    void updateScroll();

    /**
     * Обновляет положение маркера на основе координаты мыши.
     * x Позиция X в пикселях в пределах виджета.
     */
    void updateMarkerFromPos(int x);

    /**
     * Перестраивает кэшированный путь осциллограммы.
     */
    void updateCachedPath();
};

#endif // WAVEFORMVIEW_H
