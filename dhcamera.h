#ifndef DHCAMERA_H
#define DHCAMERA_H
#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>
#include "dhnetsdk.h"
#include <stdio.h>
#include "dhplay.h"

#include <unistd.h>


class DHCamera
{
public:
    DHCamera(char *ip,int port,char *userName,char *userPwd);
    ~DHCamera();

    cv::Mat * getNewestFrame();
    int next();
    cv::Mat globleMat;
private:
    pthread_t getframe;

};

#endif // DHCAMERA_H
