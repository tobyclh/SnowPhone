#include "opencv2/opencv.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#define PACK_SIZE 4096
#define BUF_LEN 65540
#define ENCODE_QUALITY 30
using namespace cv;

void *display(void *);

int capDev = 1;

VideoCapture cap(capDev); // open the default camera
int main(int argc, char **argv)
{

    //--------------------------------------------------------
    //networking stuff: socket , connect
    //--------------------------------------------------------
    int sokt;
    char *serverIP;
    int serverPort;

    if (argc < 3)
    {
        std::cerr << "Usage: cv_video_cli <serverIP> <serverPort> " << std::endl;
    }

    serverIP = argv[1];
    serverPort = atoi(argv[2]);

    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(struct sockaddr_in);

    if ((sokt = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "socket() failed" << std::endl;
    }

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);

    if (connect(sokt, (sockaddr *)&serverAddr, addrLen) < 0)
    {
        std::cerr << "connect() failed!" << std::endl;
    }

    Mat img, imgGray;
    img = Mat::zeros(480, 640, CV_8UC1);
    //make it continuous
    if (!img.isContinuous())
    {
        img = img.clone();
    }

    int imgSize = img.total() * img.elemSize();
    int bytes = 0;
    int key;
    //make img continuos
    if (!img.isContinuous())
    {
        img = img.clone();
        imgGray = img.clone();
    }

    std::cout << "Image Size:" << imgSize << std::endl;

    // Destination Buffer and JPEG Encode Parameter
    std::vector<uchar> buf;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, ENCODE_QUALITY};
    while (1)
    {

        /* get a frame from camera */
        cap >> img;
        int offset_x = img.size().width/6;
        int offset_y = img.size().height/6;
        cv::Rect roi;
        roi.x = offset_x;
        roi.y = offset_y;
        roi.width = img.size().width - (offset_x*2);
        roi.height = img.size().height - (offset_y*2);
        img = img(roi);
        //do video processing here
        // cvtColor(img, imgGray, CV_BGR2GRAY);
        cv::Size size(img.size().width/2, img.size().height/2);
        cv::resize(img, img, size);
        //do video processing here
        // cvtColor(img, imgGray, CV_BGR2GRAY);

        cv::imencode(".jpg", img, buf, params);
        std::cout << "Encoded Size:" << buf.size() << std::endl;
        int total_pack = 1 + (buf.size() - 1) / PACK_SIZE;

        // notify the size of image in terms of package size
        int ibuf[1];
        ibuf[0] = total_pack;
        if ((bytes = send(sokt, ibuf, sizeof(int), 0)) < 0)
        {
            std::cerr << "bytes = " << bytes << std::endl;
            std::cout << "Retrying" << std::endl;
            continue;
        }

        //send processed image in batches
        for (int i = 0; i < total_pack; i++)
        {
            if ((bytes = send(sokt, &buf[i * PACK_SIZE], PACK_SIZE, 0)) < 0)
            {
                std::cerr << "bytes = " << bytes << std::endl;
                break;
            }
        }
        cv::waitKey(100);

    }

    return 0;
}
