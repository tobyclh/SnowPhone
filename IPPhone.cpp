#include <opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <math.h> 
using namespace dlib;
using namespace std;
// using namespace cv;
string nekomimi_left = "";
string nekomimi_right = "";
string nose_image = "nose.jpg";

void overlayImage(cv::Mat *src, cv::Mat *overlay, const cv::Point &location)
{
    for (int y = max(location.y - overlay->rows / 2, 0); y < src->rows; ++y)
    {
        int fY = y - (location.y - overlay->cols / 2);
        if (fY >= overlay->rows)
            break;

        for (int x = max(location.x - overlay->cols / 2, 0); x < src->cols; ++x)
        {
            int fX = x - (location.x  - overlay->cols / 2);

            if (fX >= overlay->cols)
                break;

            double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;

            for (int c = 0; opacity > 0 && c < src->channels(); ++c)
            {
                unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
                unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
                src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
            }
        }
    }
}

int main()
{
    int DEBUG = 0;
    cv::VideoCapture cap(1);
    if (!cap.isOpened())
    {
        cerr << "Unable to connect to camera" << endl;
        return 1;
    }

    image_window win;
    cv::Mat _nose_mat = cv::imread(nose_image, CV_LOAD_IMAGE_COLOR);
    cv::Mat nose_mat;
    cv::Size size(200, 200);

    // Load face detection and pose estimation models.
    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor pose_model;
    deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

    cv::Mat edges;
    cv::namedWindow("edges", 1);
    // while (!win.is_closed())
    while (true)
    {
        // Grab a frame
        cv::Mat temp;
        cap >> temp;
        // cv::cvtColor(temp, edges, cv::COLOR_BGR2GRAY);
        // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
        // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
        // long as temp is valid.  Also don't do anything to temp that would cause it
        // to reallocate the memory which stores the image as that will make cimg
        // contain dangling pointers.  This basically means you shouldn't modify temp
        // while using cimg.
        cv_image<bgr_pixel> cimg(temp);

        // Detect faces
        std::vector<rectangle> faces = detector(cimg);
        // Find the pose of each face.
        std::vector<full_object_detection> shapes;

        for (unsigned long i = 0; i < faces.size(); ++i)
        {
            auto shape = pose_model(cimg, faces[i]);
            if (DEBUG)
            {

                for (unsigned long i = 0; i < shape.num_parts(); ++i)
                {

                    int radius = 5;
                    // std::cout << "i: "<< (int) i << " x : " << (int)shape.part(i).x() << ", y: " << (int)shape.part(i).y() << "\n";
                    for (int _y = 0; _y < radius; _y++)
                    {
                        for (int _x = 0; _x < radius; _x++)
                        {
                            auto _color = temp.at<cv::Vec3b>(cv::Point((int)shape.part(i).x() + _x, (int)shape.part(i).y() + _y));
                            _color[0] = 255;
                            _color[1] = 255;
                            _color[2] = 255;
                            temp.at<cv::Vec3b>(cv::Point((int)shape.part(i).x() + _x, (int)shape.part(i).y() + _y)) = _color;
                        }
                    }
                    cv::putText(temp, std::to_string(i), cv::Point((int)shape.part(i).x(), (int)shape.part(i).y()),
                                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200, 200, 250), 1, CV_AA);
                }
            }
            float face_width, face_height;
            face_width = hypot(abs(shape.part(0).x()-shape.part(16).x()), abs(shape.part(0).y() - shape.part(16).y()));
            size =  cv::Size(face_width, face_width);
            // std::cout << "Width : " << face_width;
            resize(_nose_mat, nose_mat, size);
            overlayImage(&temp, &nose_mat, cv::Point((int)shape.part(30).x(), (int)shape.part(30).y()));
            shapes.push_back(shape);
        }

        cv::imshow("edges", temp);
        if (cv::waitKey(30) >= 0)
            break;
        // // Display it all on the screen
        // win.clear_overlay();
        // win.set_image(cimg);
        // win.add_overlay(render_face_detections(shapes));
    }

    
}
