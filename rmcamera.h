#ifndef RMCAMERA_H
#define RMCAMERA_H
#include "opencv2/opencv.hpp"

class  RmCamera
{

public:
    RmCamera(char *ip,int port,char *userName,char *pwd,int cameraType);
    ~RmCamera();
    void getNewestFrame(char *pBuff,int *buffSize,int *height,int *width);
private:
    int cameraType;//1 HK  2 DH
    char *ip;
    char *userName;
    char *pwd;
    int port;

};

#endif // RMCAMERA_H
