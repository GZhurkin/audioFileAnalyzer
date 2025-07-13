#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H

#include <QWidget>
#include <QVector>

class GridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridWidget(QWidget *parent = nullptr);

    void setSamples(const QVector<double>& samples, quint32 sampleRate);
    void setMarkerPosition(double seconds);

signals:
    void markerPositionChanged(double seconds);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_samples;
    quint32 m_sampleRate = 0;
    double m_markerPosition = 0.0; // в секундах
};

#endif // GRIDWIDGET_H
