#ifndef HKCAMERA_H
#define HKCAMERA_H
#include "opencv2/opencv.hpp"


#include <fstream>
#include <unistd.h>
//#include "auto_entercs.h"

#include "HCNetSDK.h"
#include "PlayM4.h"
#include "LinuxPlayM4.h"


class HkCamera
{
public:
    HkCamera(char *ip,int port,char *userName,char *userPwd);

    ~HkCamera();

    cv::Mat * getNewestFrame();
    int next();
    cv::Mat globleMat;
private:
    pthread_t getframe;
};

#endif // HKCAMERA_H
