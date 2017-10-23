#include "dhcamera.h"

#define PLAYPORT 1


static cv::Mat dst;
pthread_mutex_t mutex;
std::list<cv::Mat> g_frameList;

char *dh_ip;
int dh_port;
char *dh_userName;
char *dh_userPwd;

void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    printf("设备断线.\n");
    return;
}

void CALLBACK AutoReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    printf("自动重连成功.\n");
    return;
}

//子连接自动重连回调函数
void CALLBACK SubHaveReConnectFunc(EM_INTERFACE_TYPE emInterfaceType, BOOL bOnline, LLONG lOperateHandle, LLONG lLoginID, LDWORD dwUser)
{
    printf("Device reconnecte.\n");
    return;
}


void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize, LONG lParam, LDWORD dwUser)
{

//    printf("send play %d,%d \n",dwDataType,dwBufSize);
    if (dwDataType == 0)  //原始视频流送播放库
    {
        PLAY_InputData(PLAYPORT,pBuffer,dwBufSize);
    }
}

void CALLBACK DecCBFun(LONG nPort,char * pBuf,LONG nSize,FRAME_INFO * pFrameInfo, void* pUserData, LONG nReserved2)
{
    //移走数据后，快速返回,即开始解码回调下一帧数据;不要在回调中运行长事务，否则阻塞解码下一帧数据
    // pbuf里的数据是YUV I420格式的数据

    if(pFrameInfo->nType == 3) //视频数据
    {
        //         printf("video CallBack\n");
        dst.create(pFrameInfo->nHeight, pFrameInfo->nWidth,
                   CV_8UC3);

        cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (uchar *)pBuf);
        cv::cvtColor(src, dst, CV_YUV2RGB_YV12);
        pthread_mutex_lock(&mutex);
        if(g_frameList.size()>50)
            g_frameList.clear();
        g_frameList.push_back(dst);
//        printf("video vallBack %d\n",g_frameList.size());
        pthread_mutex_unlock(&mutex);
    }
    else if(pFrameInfo->nType == T_AUDIO16)
    {
        //cout<<"Audio CallBack"<<endl;
    }
    else
    {
        printf("nType = %d\n",pFrameInfo->nType);
    }
    return;
}

void *RunIPCameraInfo(void *)
{
    static BOOL g_bNetSDKInitFlag = FALSE;

    //打开播放通道
    PLAY_OpenStream(PLAYPORT,0,0,1024*1024);
    PLAY_SetDecCallBackEx(PLAYPORT, DecCBFun,NULL);
    PLAY_Play(PLAYPORT,NULL);

    //    CLIENT_LogClose();
    //初始化sdk
    g_bNetSDKInitFlag = CLIENT_Init(DisConnectFunc,0);
    if (FALSE == g_bNetSDKInitFlag)
    {
        printf("Initialize client SDK fail; \n");
    }
    else
    {
        printf("Initialize client SDK done; \n");
    }
    //设置登录参数
    CLIENT_SetConnectTime(3000, 3);

    // 设置更多网络参数,NET_PARAM 的 nWaittime,nConnectTryNum 成员与 CLIENT_SetConnectTime 接口设置的登录设备超时时间和尝试次数意义相同
    // 此操作为可选操作
    NET_PARAM stuNetParm = {0};
    stuNetParm.nConnectTime = 1000; // 登录时尝试建立链接的超时时间
    CLIENT_SetNetworkParam(&stuNetParm);


    //设置自动重连回调函数
    CLIENT_SetAutoReconnect(AutoReConnectFunc, 0);
    //设置子连接自动重连回调函数
    CLIENT_SetSubconnCallBack(SubHaveReConnectFunc, 0);

    NET_DEVICEINFO_Ex deviceInfoEx ={0};
    int nError = 0;
    unsigned long lLogin = 0;
//    char ip[255] = {0};
//    WORD nPort = 37777;

//    strcpy(ip,"172.16.0.238");
    while(0 == lLogin)
    {
        // 登录设备
        lLogin = CLIENT_LoginEx2(dh_ip, (WORD)dh_port, dh_userName,dh_userPwd, EM_LOGIN_SPEC_CAP_TCP, NULL, &deviceInfoEx, &nError);//登陆设备，用户需另选设备。

        if(0 == lLogin)
        {
            // 根据错误码,可以在 dhnetsdk.h 中找到相应的解释,此处打印的是 16 进制,头文件中是十进制,其中的转换需注意
            // 例如:
            // #define NET_NOT_SUPPORTED_EC(23) // 当前 SDK 未支持该功能,对应的错误码为 0x80000017, 23 对应的 16 进制为 0x17
            printf("client %s[%d]Failed!Last Error[%x]\n" , dh_ip ,dh_port , CLIENT_GetLastError());
        }
        else
        {
            printf("CLIENT_LoginEx2 %s[%d] Success\n" , dh_ip , dh_port);
        }
        // 用户初次登录设备,需要初始化一些数据才能正常实现业务功能,建议登录后等待一小段时间,具体等待时间因设备而异
        sleep(1);
    }

    long lRealPlay = {0};
    if(lLogin != 0)
    {
        printf( "Login Success ,Start Real Play\n");
        lRealPlay = CLIENT_RealPlayEx(lLogin,0,0);
        printf("lRealPlay=%d\n",lRealPlay);

        if(lRealPlay != 0)
        {
            //窗口句柄传空值，网络库只回调原始数据
            CLIENT_SetRealDataCallBackEx(lRealPlay, RealDataCallBackEx, 0, 0x1f);
            sleep(-1);
        }else{
            printf("set realDataCallBack failed");
        }

    }
    else
    {
        printf(" Login Fail \n)");
    }

    //释放网络库
    CLIENT_StopRealPlayEx(lRealPlay);
    CLIENT_Logout(lLogin);
    CLIENT_Cleanup();

    //关闭播放通道，释放资源
    PLAY_Stop(PLAYPORT);
    PLAY_CloseStream(PLAYPORT);

    pthread_detach(pthread_self());//pthread_detach(pthread_self())，将状态改为unjoinable状态，确保资源的释放
    static int g;
    printf("children thread:%d \n", g++);
    pthread_exit(0) ;//pthread_exit时自动会被释放
}

DHCamera::DHCamera(char *ip,int port,char *userName,char *userPwd)
{
    dh_ip = ip;
    dh_port = port;
    dh_userName = userName;
    dh_userPwd = userPwd;
    int ret;
    pthread_mutex_init(&mutex, NULL);
    ret = pthread_create(&getframe, NULL, RunIPCameraInfo, NULL);

    if(ret!=0)
    {
        printf("Create pthread error!\n");
    }

}

cv::Mat * DHCamera::getNewestFrame(){
        int flag =0;
        pthread_mutex_lock(&mutex);
        if(g_frameList.size())
        {
//            printf("framelist.size=%d\n",g_frameList.size());
            std::list<cv::Mat>::iterator it;
            it = g_frameList.end();
            it--;
            globleMat = (*(it));
//            g_frameList.pop_front();
            g_frameList.clear(); // 丢掉旧的帧
            flag =1;
        }
        pthread_mutex_unlock(&mutex);
        if(flag != 0)
            return &globleMat;
        else
            return NULL;
//        usleep(100000);

}

int DHCamera::next(){
        int ret =0;
        pthread_mutex_lock(&mutex);
        ret = g_frameList.size();
        pthread_mutex_unlock(&mutex);
        return ret;
}

DHCamera::~DHCamera(){

    printf("DHCamera destroy\n");
    pthread_mutex_destroy(&mutex);

}

