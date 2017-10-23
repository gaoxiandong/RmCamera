#include "rmcamera.h"


RmCamera::RmCamera(char *ip,int port,char *userName,char *pwd,int cameraType):
    ip(ip),
    port(port),
    userName(userName),
    pwd(pwd),
    cameraType(cameraType)
{
    if(cameraType == 1){

    }
}

RmCamera::~RmCamera(){

}

void RmCamera::getNewestFrame(char *pBuff,int *buffSize,int *height,int *width )
{

}

