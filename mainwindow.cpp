#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <QInputDialog>
#include <iostream>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Available resolutions
    resolutions = {{640, 480}, {1280, 720}, {1920, 1080}};
    for (const auto &res : resolutions) {
        ui->resolutionDropdown->addItem(QString::number(res.width) + "x" + QString::number(res.height));
    }

    // Open video capture device
    cap.open(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video capture device" << std::endl;
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, resolutions[0].width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, resolutions[0].height);

    // Connect dropdown change event
    connect(ui->resolutionDropdown, SIGNAL(currentIndexChanged(int)), this, SLOT(changeResolution(int)));

    // Setup video update timer
    videoTimer = new QTimer(this);
    connect(videoTimer, &QTimer::timeout, this, &MainWindow::updateFrame);
    videoTimer->start(30);

    // Initialize SDL Audio
    SDL_Init(SDL_INIT_AUDIO);

    // List available audio input devices
    int numDevices = SDL_GetNumAudioDevices(1);
    for (int i = 0; i < numDevices; i++) {
        availableDevices.push_back(SDL_GetAudioDeviceName(i, 1));
    }

    // Select default audio device
    selectAudioDevice();

    // Add VU Meter
    vuMeter = new QProgressBar(this);
    vuMeter->setRange(0, 100);
    vuMeter->setValue(0);
    ui->verticalLayout->addWidget(vuMeter);

    // Add button to change audio device
    deviceSelectButton = new QPushButton("Select Audio Device", this);
    connect(deviceSelectButton, &QPushButton::clicked, this, &MainWindow::selectAudioDevice);
    ui->verticalLayout->addWidget(deviceSelectButton);
}

MainWindow::~MainWindow() {
    cap.release();
    SDL_CloseAudioDevice(audioDevice);
    SDL_Quit();
    delete ui;
}

// Update video frame
void MainWindow::updateFrame() {
    if (!cap.isOpened()) return;

    cap.read(frame);
    if (frame.empty()) return;

    // Convert to Qt image format
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

    // Display image in QLabel
    ui->videoLabel->setPixmap(QPixmap::fromImage(img).scaled(ui->videoLabel->size(), Qt::KeepAspectRatio));
}

// Change resolution (Reopen capture to prevent freezing)
void MainWindow::changeResolution(int index) {
    if (index >= 0 && index < resolutions.size()) {
        cap.release();
        cap.open(0);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, resolutions[index].width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, resolutions[index].height);
        std::cout << "Resolution changed to " << resolutions[index].width << "x" << resolutions[index].height << std::endl;
    }
}

// SDL Audio Callback Function (Runs in a separate thread)
void MainWindow::audioCallback(void* userdata, Uint8* stream, int len) {
    MainWindow *self = static_cast<MainWindow*>(userdata);
    int samples = len / sizeof(Sint16);
    std::vector<Sint16> buffer(samples);

    // Capture audio samples
    int dequeued = SDL_DequeueAudio(self->audioDevice, buffer.data(), len);
    if (dequeued > 0) {
        // Debugging: Print that we are receiving audio
        std::cout << "Captured " << dequeued << " bytes of audio." << std::endl;

        // Compute RMS (Root Mean Square) for VU Meter
        double sum = 0;
        for (Sint16 sample : buffer) {
            sum += sample * sample;
        }
        double rms = std::sqrt(sum / samples);
        int volumeLevel = std::min(100, static_cast<int>(rms / 32767.0 * 100));

        // Update VU meter (thread-safe method via Qt signal)
        QMetaObject::invokeMethod(self->vuMeter, "setValue", Qt::QueuedConnection, Q_ARG(int, volumeLevel));

        // Queue audio for playback
        SDL_QueueAudio(self->audioDevice, buffer.data(), len);
    }
}

// Select an audio device
void MainWindow::selectAudioDevice() {
    bool ok;
    QStringList deviceList;
    for (const std::string &device : availableDevices) {
        deviceList << QString::fromStdString(device);
    }

    QString selectedDevice = QInputDialog::getItem(this, "Select Audio Device", "Audio Input:", deviceList, 0, false, &ok);

    if (ok && !selectedDevice.isEmpty()) {
        SDL_CloseAudioDevice(audioDevice);

        SDL_AudioSpec desiredSpec;
        SDL_memset(&desiredSpec, 0, sizeof(desiredSpec));
        desiredSpec.freq = 44100;
        desiredSpec.format = AUDIO_S16LSB;
        desiredSpec.channels = 1;
        desiredSpec.samples = 4096;
        desiredSpec.callback = audioCallback;
        desiredSpec.userdata = this;

        audioDevice = SDL_OpenAudioDevice(selectedDevice.toStdString().c_str(), 1, &desiredSpec, nullptr, 0);
        if (!audioDevice) {
            std::cerr << "Error: Cannot open selected audio device" << std::endl;
        } else {
            SDL_PauseAudioDevice(audioDevice, 0); // Start audio playback
        }
    }
}
