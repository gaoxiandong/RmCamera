#include "hkcamera.h"
HWND h = NULL;
LONG nPort=-1;
LONG lUserID;

static cv::Mat dst_hk;
pthread_mutex_t mutex_hk;
std::list<cv::Mat> g_frameList_hk;

char *hk_ip;
int hk_port;
char *hk_user;
char *hk_pwd;

void CALLBACK DecCBFunHK(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void* nReserved1, LONG nReserved2)
{
   long lFrameType = pFrameInfo->nType;

     if (lFrameType == T_YV12)
     {
      //cv::Mat dst(pFrameInfo->nHeight, pFrameInfo->nWidth,
      //            CV_8UC3);  // 8UC3表示8bit uchar无符号类型,3通道值
           dst_hk.create(pFrameInfo->nHeight, pFrameInfo->nWidth,
                 CV_8UC3);

           cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (uchar *)pBuf);
           cv::cvtColor(src, dst_hk, CV_YUV2RGB_YV12);
           pthread_mutex_lock(&mutex_hk);
           if(g_frameList_hk.size()>50)
               g_frameList_hk.clear();
           g_frameList_hk.push_back(dst_hk);
           pthread_mutex_unlock(&mutex_hk);
     }
    usleep(1000);

   //cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (uchar *)pBuf);
   //cv::cvtColor(src, dst, CV_YUV2BGR_YV12);
   //cv::imshow("bgr", dst);
   //pthread_mutex_lock(&mutex);
   //g_frameList.push_back(dst);
   //pthread_mutex_unlock(&mutex);
   //vw << dst;
   //cv::waitKey(10);

}

void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,void* dwUser)
{
   /*
   if (dwDataType == 1)
   {
       PlayM4_GetPort(&nPort);
       PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME);
       PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024);
       PlayM4_SetDecCallBackEx(nPort, DecCBFun, NULL, NULL);
       PlayM4_Play(nPort, h);
   }
   else
   {
       BOOL inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
   }*/
//   printf("send buffer size %d\n",dwBufSize);
   DWORD dRet;
   switch (dwDataType)
   {
     case NET_DVR_SYSHEAD:           //系统头
       if (!PlayM4_GetPort(&nPort))  //获取播放库未使用的通道号
       {
         break;
       }
       if (dwBufSize > 0) {
         if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME)) {
           dRet = PlayM4_GetLastError(nPort);
           break;
         }
         if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024)) {
           dRet = PlayM4_GetLastError(nPort);
           break;
         }
         //设置解码回调函数 只解码不显示
        //  if (!PlayM4_SetDecCallBack(nPort, DecCBFun)) {
        //     dRet = PlayM4_GetLastError(nPort);
        //     break;
        //  }

         //设置解码回调函数 解码且显示
         if (!PlayM4_SetDecCallBackEx(nPort, DecCBFunHK, NULL, NULL))
         {
           dRet = PlayM4_GetLastError(nPort);
           break;
         }

         //打开视频解码
         if (!PlayM4_Play(nPort, h))
         {
           dRet = PlayM4_GetLastError(nPort);
           break;
         }

         //打开音频解码, 需要码流是复合流
         if (!PlayM4_PlaySound(nPort)) {
           dRet = PlayM4_GetLastError(nPort);
           break;
         }
       }
       break;
       //usleep(500);
     case NET_DVR_STREAMDATA:  //码流数据
       if (dwBufSize > 0 && nPort != -1) {
         BOOL inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
         while (!inData) {
           sleep(100);
           inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
           std::cerr << "PlayM4_InputData failed \n" << std::endl;
         }
       }
       break;
   }
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
   char tempbuf[256] = {0};
   std::cout << "EXCEPTION_RECONNECT = " << EXCEPTION_RECONNECT << std::endl;
   switch(dwType)
   {
   case EXCEPTION_RECONNECT:    //预览时重连
       printf("pyd----------reconnect--------%d\n", time(NULL));
       break;
   default:
       break;
   }
}

void *RunIPCameraInfoHK(void *)
{
//    char IP[]         = "172.16.0.239";   //海康威视网络摄像头的ip
//    char UName[] = "admin";                 //海康威视网络摄像头的用户名
//    char PSW[]      = "rm123456";           //海康威视网络摄像头的密码
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(1000, true);
    NET_DVR_SetLogToFile(3, "./sdkLog");
    NET_DVR_DEVICEINFO_V30 struDeviceInfo = {0};
    NET_DVR_SetRecvTimeOut(5000);
//    lUserID = NET_DVR_Login_V30(IP, 8000, UName, PSW, &struDeviceInfo);

   lUserID = NET_DVR_Login_V30(hk_ip,(WORD)hk_port,hk_user,hk_pwd, &struDeviceInfo);

   NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

   long lRealPlayHandle;
   NET_DVR_CLIENTINFO ClientInfo = {0};

   ClientInfo.lChannel       = 1;
   ClientInfo.lLinkMode     = 0;
   ClientInfo.hPlayWnd     = 0;
   ClientInfo.sMultiCastIP = NULL;


   //lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, PsDataCallBack, NULL, 0);
   lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, g_RealDataCallBack_V30, NULL, 0);
   //NET_DVR_SaveRealData(lRealPlayHandle, "/home/lds/source/yuntai.mp4");
   if (lRealPlayHandle < 0)
   {
       printf("pyd1---NET_DVR_RealPlay_V30 error\n");
   }
   sleep(-1);

   NET_DVR_Cleanup();
}


HkCamera::HkCamera(char *ip,int port,char *userName,char *userPwd)
{
    hk_ip = ip;
    hk_port = port;
    hk_user = userName;
    hk_pwd = userPwd;

    int ret;
    pthread_mutex_init(&mutex_hk, NULL);
    ret = pthread_create(&getframe, NULL, RunIPCameraInfoHK, NULL);

    if(ret!=0)
    {
        printf("Create pthread error!\n");
    }

}

cv::Mat * HkCamera::getNewestFrame(){
        int flag =0;
        pthread_mutex_lock(&mutex_hk);
        if(g_frameList_hk.size())
        {
//            printf("framelist_Hk.size=%d\n",g_frameList_hk.size());
            std::list<cv::Mat>::iterator it;
            it = g_frameList_hk.end();
            it--;
            globleMat = (*(it));
//            g_frameList_hk.pop_front();
            g_frameList_hk.clear(); // 丢掉旧的帧
            flag =1;
        }
        pthread_mutex_unlock(&mutex_hk);
        if(flag != 0)
            return &globleMat;
        else{
            return NULL;
        }
//        usleep(100000);

}

int HkCamera::next(){
        int ret =0;
        pthread_mutex_lock(&mutex_hk);
        ret = g_frameList_hk.size();
        pthread_mutex_unlock(&mutex_hk);
        return ret;
}

HkCamera::~HkCamera(){
    printf("HKCamera destroy\n");
    pthread_mutex_destroy(&mutex_hk);

}
