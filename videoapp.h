#ifndef VIDEOAPP_H
#define VIDEOAPP_H

#include <opencv2/opencv.hpp>
#include <gtkmm.h>
#include <vector>
#include <iostream>

class VideoApp : public Gtk::Window {
public:
    VideoApp();
    virtual ~VideoApp();

private:
    void on_resolution_changed();
    bool update_frame();

    // GUI Components
    Gtk::Box vbox;
    Gtk::ComboBoxText resolutionDropdown;
    Gtk::Image videoDisplay;

    // OpenCV Video Capture
    cv::VideoCapture cap;
    cv::Mat frame;

    // Available resolutions
    struct Resolution {
        int width;
        int height;
    };
    std::vector<Resolution> resolutions;
};

#endif // VIDEOAPP_H