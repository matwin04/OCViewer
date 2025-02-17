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
    int numInputDevices = SDL_GetNumAudioDevices(1);
    for (int i = 0; i < numInputDevices; i++) {
        availableInputDevices.push_back(SDL_GetAudioDeviceName(i, 1));
    }

    // List available audio output devices
    int numOutputDevices = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < numOutputDevices; i++) {
        availableOutputDevices.push_back(SDL_GetAudioDeviceName(i, 0));
    }

    // Select default audio devices
    selectInputDevice();
    selectOutputDevice();

    // Add VU Meter
    vuMeter = new QProgressBar(this);
    vuMeter->setRange(0, 100);
    vuMeter->setValue(0);
    ui->verticalLayout->addWidget(vuMeter);

    // Add buttons for input & output selection
    inputDeviceButton = new QPushButton("Select Input Device", this);
    connect(inputDeviceButton, &QPushButton::clicked, this, &MainWindow::selectInputDevice);
    ui->verticalLayout->addWidget(inputDeviceButton);

    outputDeviceButton = new QPushButton("Select Output Device", this);
    connect(outputDeviceButton, &QPushButton::clicked, this, &MainWindow::selectOutputDevice);
    ui->verticalLayout->addWidget(outputDeviceButton);
}

MainWindow::~MainWindow() {
    cap.release();
    SDL_CloseAudioDevice(inputDevice);
    SDL_CloseAudioDevice(outputDevice);
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

// SDL Audio Callback
void MainWindow::audioCallback(void* userdata, Uint8* stream, int len) {
    MainWindow *self = static_cast<MainWindow*>(userdata);
    std::vector<Sint16> buffer(len / sizeof(Sint16));

    int dequeued = SDL_DequeueAudio(self->inputDevice, buffer.data(), len);
    if (dequeued > 0) {
        SDL_QueueAudio(self->outputDevice, buffer.data(), dequeued);

        double sum = 0;
        for (Sint16 sample : buffer) {
            sum += sample * sample;
        }
        double rms = std::sqrt(sum / buffer.size());
        int volumeLevel = std::min(100, static_cast<int>(rms / 32767.0 * 100));

        QMetaObject::invokeMethod(self->vuMeter, "setValue", Qt::QueuedConnection, Q_ARG(int, volumeLevel));
    }
}

// Select input device
void MainWindow::selectInputDevice() {
    bool ok;
    QStringList deviceList;
    for (const std::string &device : availableInputDevices) {
        deviceList << QString::fromStdString(device);
    }

    QString selectedDevice = QInputDialog::getItem(this, "Select Input Device", "Microphone:", deviceList, 0, false, &ok);

    if (ok && !selectedDevice.isEmpty()) {
        SDL_CloseAudioDevice(inputDevice);

        SDL_AudioSpec spec = {};
        spec.freq = 44100;
        spec.format = AUDIO_S16LSB;
        spec.channels = 1;
        spec.samples = 4096;
        spec.callback = audioCallback;
        spec.userdata = this;

        inputDevice = SDL_OpenAudioDevice(selectedDevice.toStdString().c_str(), 1, &spec, nullptr, 0);
        SDL_PauseAudioDevice(inputDevice, 0);
    }
}

// Select output device
void MainWindow::selectOutputDevice() {
    bool ok;
    QStringList deviceList;
    for (const std::string &device : availableOutputDevices) {
        deviceList << QString::fromStdString(device);
    }

    QString selectedDevice = QInputDialog::getItem(this, "Select Output Device", "Speaker:", deviceList, 0, false, &ok);

    if (ok && !selectedDevice.isEmpty()) {
        SDL_CloseAudioDevice(outputDevice);
        outputDevice = SDL_OpenAudioDevice(selectedDevice.toStdString().c_str(), 0, nullptr, nullptr, 0);
        SDL_PauseAudioDevice(outputDevice, 0);
    }
}
