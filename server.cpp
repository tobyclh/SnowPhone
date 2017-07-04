#include "opencv2/opencv.hpp"
#include <iostream>
#include <opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <opencv2/highgui/highgui.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define ENCODE_QUALITY 80
#define PACK_SIZE 4096
#define BUF_LEN 65540
//using namespace cv;
using namespace dlib;
using namespace std;
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


void *display(void *);

int main(int argc, char **argv)
{

    //--------------------------------------------------------
    //networking stuff: socket, bind, listen
    //--------------------------------------------------------
    int localSocket,
        remoteSocket,
        port = 4097;

    struct sockaddr_in localAddr,
        remoteAddr;
    pthread_t thread_id;

    int addrLen = sizeof(struct sockaddr_in);

    if ((argc > 1) && (strcmp(argv[1], "-h") == 0))
    {
        std::cerr << "usage: ./cv_video_srv [port] [capture device]\n"
                  << "port           : socket port (4097 default)\n"
                  << "capture device : (0 default)\n"
                  << std::endl;

        exit(1);
    }

    if (argc == 2)
        port = atoi(argv[1]);

    localSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (localSocket == -1)
    {
        perror("socket() call failed!!");
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(port);

    if (bind(localSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
    {
        perror("Can't bind() socket");
        exit(1);
    }



    //Listening
    listen(localSocket, 3);

    std::cout << "Waiting for connections...\n"
              << "Server Port:" << port << std::endl;
    while (1)
    {
        //if (remoteSocket < 0) {
        //    perror("accept failed!");
        //    exit(1);
        //}

        remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t *)&addrLen);
        //std::cout << remoteSocket<< "32"<< std::endl;
        if (remoteSocket < 0)
        {
            perror("accept failed!");
            exit(1);
        }
        std::cout << "Connection accepted" << std::endl;
        pthread_create(&thread_id, NULL, display, &remoteSocket);

        //pthread_join(thread_id,NULL);
    }
    return 0;
}
void *display(void *ptr)
{
    int DEBUG = 0;
    int sokt = *(int *)ptr;
    cv::Mat img;
    img = cv::Mat::zeros(480, 640, CV_8UC3);
    int imgSize = img.total() * img.elemSize();
    uchar *iptr = img.data;
    int bytes = 0;
    int key;
    cv::namedWindow("CV Video Client", 1);
    char echo_buffer[BUF_LEN];

    cv::Mat _nose_mat = cv::imread(nose_image, CV_LOAD_IMAGE_COLOR);
    cv::Mat nose_mat;
    cv::Size size(200, 200);
    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor pose_model;
    deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

    while (key != 'q')
    {
        while (bytes > sizeof(int) || bytes == 0)
        {
            // std::cout << "Sleep"  << std::endl;
            // usleep(100);
            std::cout << "recv "  << bytes << std::endl;
            bytes = recv(sokt, echo_buffer, BUF_LEN, MSG_DONTWAIT);
        }
        int total_pack = ((int *)echo_buffer)[0];
        std::cout << "expecting length of packs:" << total_pack << std::endl;

        char *longbuf = new char[PACK_SIZE * total_pack];
        for (int i = 0; i < total_pack; i++)
        {
            std::cout << "Waiting for pack" << std::endl;
            bytes = recv(sokt, echo_buffer, PACK_SIZE, MSG_WAITALL);
            if (bytes != PACK_SIZE)
            {
                std::cerr << "Received unexpected size pack:" << bytes << std::endl;
                continue;
            }
            memcpy(&longbuf[i * PACK_SIZE], echo_buffer, PACK_SIZE);
        }
        std::cout << "Received packet" << std::endl;
        cv::Mat rawData = cv::Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
        cv::Mat frame = cv::imdecode(rawData, 1);
        if (frame.size().width == 0)
        {
            std::cerr << "decode failure!" << std::endl;
            continue;
        }
        std::cout << "Showing" << std::endl;
        size = cv::Size(frame.size().width*2, frame.size().height*2);
        cv::resize(frame,frame,size);
        cv_image<bgr_pixel> cimg(frame);

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
                            auto _color = frame.at<cv::Vec3b>(cv::Point((int)shape.part(i).x() + _x, (int)shape.part(i).y() + _y));
                            _color[0] = 255;
                            _color[1] = 255;
                            _color[2] = 255;
                            frame.at<cv::Vec3b>(cv::Point((int)shape.part(i).x() + _x, (int)shape.part(i).y() + _y)) = _color;
                        }
                    }
                    cv::putText(frame, std::to_string(i), cv::Point((int)shape.part(i).x(), (int)shape.part(i).y()),
                                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200, 200, 250), 1, CV_AA);
                }
            }
            float face_width, face_height;
            face_width = hypot(abs(shape.part(0).x()-shape.part(16).x()), abs(shape.part(0).y() - shape.part(16).y()));
            size =  cv::Size(face_width, face_width);
            // std::cout << "Width : " << face_width;
            cv::resize(_nose_mat, nose_mat, size);
            overlayImage(&frame, &nose_mat, cv::Point((int)shape.part(30).x(), (int)shape.part(30).y()));
            shapes.push_back(shape);
        }



        cv::imshow("recv", frame);
        free(longbuf);

        if (key = cv::waitKey(1) >= 0)
            break;
    }
}