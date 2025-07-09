#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new AudioModel(this))
{
    auto* central = new QWidget(this);
    auto* vlay    = new QVBoxLayout(central);

    m_btnOpen = new QPushButton(tr("Открыть WAV..."), this);
    vlay->addWidget(m_btnOpen);

    // Метаданные
    m_lblDuration    = new QLabel(tr("Длительность: —"), this);
    m_lblSampleRate  = new QLabel(tr("Частота дискретизации: —"), this);
    m_lblBitRate     = new QLabel(tr("Битрейт: —"), this);
    m_lblChannels    = new QLabel(tr("Каналы: —"), this);

    vlay->addWidget(m_lblDuration);
    vlay->addWidget(m_lblSampleRate);
    vlay->addWidget(m_lblBitRate);
    vlay->addWidget(m_lblChannels);

    setCentralWidget(central);
    setWindowTitle(tr("Audio Meta Viewer"));

    // Сигналы
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(m_model, &AudioModel::metadataReady, this, &MainWindow::onMetadataReady);
    connect(m_model, &AudioModel::errorOccurred, this, &MainWindow::onError);
}

MainWindow::~MainWindow() = default;

void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Выберите WAV-файл"), QString(), tr("WAV Files (*.wav)"));
    if (path.isEmpty())
        return;

    AudioModel::Meta meta;
    QString err;
    if (!m_model->loadWav(path, meta, err)) {
        // ошибка уже эмитится из модели, здесь можно ничего не делать
        return;
    }
}

void MainWindow::onMetadataReady(const AudioModel::Meta& m)
{
    m_lblDuration->setText(
        tr("Длительность: %1 с")
            .arg(QString::number(m.durationSeconds, 'f', 2))
        );
    m_lblSampleRate->setText(
        tr("Частота дискретизации: %1 Гц")
            .arg(m.sampleRate)
        );
    m_lblBitRate->setText(
        tr("Битрейт: %1 бит/с")
            .arg(m.bitRate)
        );
    m_lblChannels->setText(
        tr("Каналы: %1 (бит/семпл: %2)")
            .arg(m.channels)
            .arg(m.bitsPerSample)
        );
}

void MainWindow::onError(const QString& err)
{
    QMessageBox::critical(this, tr("Ошибка"), err);
}
