#include "mainwindow.h"

#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new AudioModel(this))
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_waveform(new WaveformView(this))
    , m_spectrum(new SpectrumView(this))
    , m_spectrogram(new SpectrogramView(this))
{
    m_player->setAudioOutput(m_audioOutput);

    auto* tb = addToolBar("Controls");
    QAction* openAct  = tb->addAction("Open WAV");
    QAction* playAct  = tb->addAction("Play");
    QAction* pauseAct = tb->addAction("Pause");
    QAction* stopAct  = tb->addAction("Stop");

    connect(openAct,  &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(playAct,  &QAction::triggered, m_player, &QMediaPlayer::play);
    connect(pauseAct, &QAction::triggered, m_player, &QMediaPlayer::pause);
    connect(stopAct,  &QAction::triggered, m_player, &QMediaPlayer::stop);

    m_progressSlider = new QSlider(Qt::Horizontal, this);
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setValue(0);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(m_waveform, 2);
    layout->addWidget(m_progressSlider, 0);
    layout->addWidget(m_spectrum, 1);
    layout->addWidget(m_spectrogram, 2);

    setCentralWidget(central);

    connect(m_model, &AudioModel::metadataReady,    this, &MainWindow::onMetadataReady);
    connect(m_model, &AudioModel::waveformReady,    this, &MainWindow::onWaveformReady);
    connect(m_model, &AudioModel::spectrumReady,    this, &MainWindow::onSpectrumReady);
    connect(m_model, &AudioModel::spectrogramReady, this, &MainWindow::onSpectrogramReady);
    connect(m_model, &AudioModel::errorOccurred,    this, &MainWindow::onError);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);

    connect(m_waveform, &WaveformView::markerPositionChanged, this, [this](double seconds) {
        qint64 posMs = static_cast<qint64>(seconds * 1000);
        if (m_player->position() != posMs) {
            m_player->setPosition(posMs);
        }
        if (m_player->duration() > 0) {
            int sliderVal = static_cast<int>((posMs * 100) / m_player->duration());
            if (m_progressSlider->value() != sliderVal)
                m_progressSlider->setValue(sliderVal);
        }
    });

    connect(m_progressSlider, &QSlider::sliderMoved, this, [this](int value){
        if (m_player->duration() > 0) {
            qint64 newPos = (value * m_player->duration()) / 100;
            if (m_player->position() != newPos) {
                m_player->setPosition(newPos);
            }
            double seconds = newPos / 1000.0;
            m_waveform->setMarkerPosition(seconds);
        }
    });
}

MainWindow::~MainWindow()
{
}


void MainWindow::onOpenFile()
{
    const QString file = QFileDialog::getOpenFileName(this, "Select WAV", {}, "WAV Files (*.wav)");
    if (file.isEmpty())
        return;

    m_waveform->setSamples({}, 0);
    m_spectrum->setSpectrum({}, {});
    m_spectrogram->setSpectrogramData({});
    m_progressSlider->setValue(0);
    setWindowTitle("Audio File Analyzer");

    AudioModel::Meta meta;
    QString err;
    if (!m_model->loadWav(file, meta, err)) {
        return;
    }

    m_player->setSource(QUrl::fromLocalFile(file));
}

void MainWindow::onMetadataReady(const AudioModel::Meta& m)
{
    setWindowTitle(QString("Dur: %1 s  SR: %2 Hz  BR: %3 bps  Ch: %4  Bits: %5")
                       .arg(m.durationSeconds, 0, 'f', 2)
                       .arg(m.sampleRate)
                       .arg(m.bitRate)
                       .arg(m.channels)
                       .arg(m.bitsPerSample));
}

void MainWindow::onWaveformReady(const QVector<double>& samples, quint32 sampleRate)
{
    m_waveform->setSamples(samples, sampleRate);
}

void MainWindow::onSpectrumReady(const QVector<double>& freq, const QVector<double>& amp)
{
    m_spectrum->setSpectrum(freq, amp);
}

void MainWindow::onSpectrogramReady(const QVector<QVector<double>>& frames)
{
    m_spectrogram->setSpectrogramData(frames);
}

void MainWindow::onError(const QString& err)
{
    QMessageBox::critical(this, "Error", err);
}

void MainWindow::onPositionChanged(qint64 pos)
{
    double seconds = pos / 1000.0;

    m_waveform->setMarkerPosition(seconds);

    if (m_player->duration() > 0) {
        int sliderVal = static_cast<int>((pos * 100) / m_player->duration());
        if (m_progressSlider->value() != sliderVal)
            m_progressSlider->setValue(sliderVal);
    }
}
