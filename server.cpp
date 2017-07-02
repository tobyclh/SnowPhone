
#include "opencv2/opencv.hpp"
#include <iostream>
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
using namespace cv;
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

    //accept connection from an incoming client

    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------

    return 0;
}
void *display(void *ptr)
{
    int sokt = *(int *)ptr;
    Mat img;
    img = Mat::zeros(480, 640, CV_8UC3);
    int imgSize = img.total() * img.elemSize();
    uchar *iptr = img.data;
    int bytes = 0;
    int key;

    namedWindow("CV Video Client", 1);
    char echo_buffer[BUF_LEN];
    while (key != 'q')
    {
        while (bytes > sizeof(int) || bytes == 0)
        {
            bytes = recv(sokt, echo_buffer, BUF_LEN, MSG_DONTWAIT);
        }
        int total_pack = ((int *)echo_buffer)[0];
        std::cout << "expecting length of packs:" << total_pack << std::endl;

        char *longbuf = new char[PACK_SIZE * total_pack];
        for (int i = 0; i < total_pack; i++)
        {
            bytes = recv(sokt, echo_buffer, PACK_SIZE, MSG_WAITALL);
            if (bytes != PACK_SIZE)
            {
                std::cerr << "Received unexpected size pack:" << bytes << std::endl;
                continue;
            }
            memcpy(&longbuf[i * PACK_SIZE], echo_buffer, PACK_SIZE);
        }
        std::cout << "Received packet" << std::endl;
        Mat rawData = cv::Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
        Mat frame = cv::imdecode(rawData, 1);
        if (frame.size().width == 0)
        {
            std::cerr << "decode failure!" << std::endl;
            continue;
        }
        cv::imshow("recv", frame);
        free(longbuf);

        if (key = cv::waitKey(1) >= 0)
            break;
    }
}