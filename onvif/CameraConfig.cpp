#include "CameraConfig.h"
#include <pthread.h>
#include "wsdd.h"
#include "soapStub.h"
#include "wsseapi.h"
#include <QDebug>
#include "onvif_comm.h"
#include "onvif_dump.h"
#include "config/util.h"
#include "config/platform.h"
#include <thread>
#include <sys/prctl.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

//构造函数
CameraConfig::CameraConfig()
{
    memset(camera_IP, 0, sizeof(camera_IP));
    memset(camera_userName, 0, sizeof(camera_userName));
    memset(camera_password, 0, sizeof(camera_password));
    memset(camera_serviceAddr, 0, sizeof(camera_serviceAddr));
    memset(camera_mediaAddr, 0, sizeof(camera_mediaAddr));

    camera_info.camera_configuration = NULL;
    camera_info.camera_configurationOption = NULL;
    camera_token = NULL;
    camera_isOnline = false;

    isAlarm = 0;
    alarm_type = 0;
    alarm_type_old = -1;
}
//析构函数
CameraConfig::~CameraConfig()
{

}
//初始化摄像机
int CameraConfig::init(char *IPAddr, char* UserName, char* Password)
{
    memcpy(camera_IP, IPAddr, strlen(IPAddr));
    memcpy(camera_userName, UserName, strlen(UserName));
    memcpy(camera_password, Password, strlen(Password));
    needAuthorization = true;

    camera_channel = platform->getChannelIndex(IPAddr);
    if(camera_channel == -1)
        return -1;

printf("%s %d\n", IPAddr, camera_channel);
    return CAMERA_OK;
}
//初始化摄像机
int CameraConfig::init(char *IPAddr)
{
    memcpy(camera_IP, IPAddr, strlen(IPAddr));
    needAuthorization = false;
    return CAMERA_OK;
}
//检查设备是否在线
int CameraConfig::checkDevice()
{
    int retval = CAMERA_OK;
    const char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    const char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";

    int result = 0;
    wsdd__ProbeType req;
    struct __wsdd__ProbeMatches resp;
    wsdd__ScopesType sScope;
    struct SOAP_ENV__Header header;
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 5;
    soap->send_timeout = 5;
    soap_set_namespaces(soap, namespaces);
    soap_default_SOAP_ENV__Header(soap, &header);

    header.wsa__MessageID = (char*)soap_wsa_rand_uuid(soap);
//    header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
//    header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    if(was_Action != NULL)
    {
        header.wsa__Action = (char*)malloc(1024);
        memset(header.wsa__Action, '\0', 1024);
        strncpy(header.wsa__Action, was_Action, 1024);//
    }
    if(was_To != NULL)
    {
        header.wsa__To = (char *)malloc(1024);
        memset(header.wsa__To, '\0', 1024);
        strncpy(header.wsa__To, was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    }

    soap->header = &header;
//    soap->user = (void*)ip;
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = NULL;
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;
    req.Types = NULL;//"dn:NetworkVideoTransmitter";
    result = soap_send___wsdd__Probe(soap, MULTICAST_ADDRESS, NULL, &req);

    while(result == SOAP_OK)
    {
        result = soap_recv___wsdd__ProbeMatches(soap, &resp);

        if(result == SOAP_OK)
        {
            if(soap->error)
            {
                printf("[%s][%d]:recv soap error :%d, %s, %s\n", camera_IP, __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
                soap_error_number = soap->error;
                retval = CAMERA_DEVICENOTONLINE;
                break;
            }
            else //we find a device
            {
                char XAddr_ip[32];
                memset(XAddr_ip, 0, sizeof(XAddr_ip));
                unsigned short XAddr_port = 0;
                if(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs != NULL)
                {
                    getIPFromXAddr(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs, XAddr_ip, &XAddr_port);

//                    printf("[%s]XAddr_ip is %s\n", camera_IP, XAddr_ip);
                }

                if(strcmp(camera_IP, XAddr_ip) == 0)
                {
                    strcpy(camera_serviceAddr, resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                    camera_isOnline = true;
                    mprintf("[%s]current device %s is online\n", camera_IP, XAddr_ip);
                    break;
                }
            }
        }
        else if(soap->error)
        {
            printf("[%s]\n", camera_IP);
            camera_isOnline = false;
            soap_error_number = soap->error;
            mprintf("[%s]Search end! Current device is not online! error = %d\n", camera_IP, soap_error_number);
            retval = CAMERA_DEVICENOTONLINE;
            break;
        }
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);

    return retval;
}
//生成随机UUID
const char* CameraConfig::soap_wsa_rand_uuid(soap *soap)
{
    const int uuidlen = 48;

    char* uuid = (char*)soap_malloc(soap, uuidlen);
    int r1, r2, r3, r4;
    r1 = soap_random;
    r2 = soap_random;
    r3 = soap_random;
    r4 = soap_random;
    sprintf(uuid, "urn:uuid:%8.8x-%4.4hx-4%3.3hx-%4.4hx-%4.4hx%8.8x", r1, (short)(r2 >> 16), ((short)r2 >> 4) & 0x0fff, ((short)(r3 >> 16) & 0x3fff) | 0x8000, (short)r3, r4);
//    unsigned char macaddr[6];
//    unsigned int Flagrand;
//    srand((int)time(0));
//    Flagrand = rand()%9000 + 8888;
//    macaddr[0] = 0x1;
//    macaddr[1] = 0x2;
//    macaddr[2] = 0x3;
//    macaddr[3] = 0x4;
//    macaddr[4] = 0x5;
//    macaddr[5] = 0x6;
//    sprintf(uuid, "urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", Flagrand, macaddr[0], macaddr[1], macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
//    printf("[%s]uuid is %s\n", camera_IP, uuid);
    return uuid;
}
//从服务地址中解析出设备IP地址
void CameraConfig::getIPFromXAddr(const char *XAddr, char* DeviceIP, unsigned short* DevicePort)
{
    char XAddrBuf[256] = {0};
//    unsigned short* DevicePort;
    strncpy(XAddrBuf, XAddr + 7, 255);
    XAddrBuf[255] = '\0';

    int j = 0;
    bool bReadPort = false;
    unsigned short l_Port = 0;
    unsigned int i;
    for(i = 0; i <= strlen(XAddrBuf); ++i)
    {
        if(XAddrBuf[i] == '/' && !bReadPort)
        {
            l_Port = 80;
            break;
        }
        else if(XAddrBuf[i] == ':')
        {
            bReadPort = true;
        }
        else if(bReadPort)
        {
            if(isdigit(XAddrBuf[i]))
            {
                l_Port = 10 * l_Port + (XAddrBuf[i] = '0');
            }
            else
            {
                break;
            }
        }
        else
        {
            DeviceIP[j++] = XAddrBuf[i];
        }
    }
    DeviceIP[j] = '\0';
    *DevicePort = l_Port;
}
//获取设备能力
int CameraConfig::getCapabilities()
{
    printf("[%s]function getCapabilities() ......\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    //capabilities
    struct _tds__GetCapabilities *tds__GetCapabilities = (_tds__GetCapabilities*)soap_malloc(soap, sizeof(_tds__GetCapabilities));
    memset(tds__GetCapabilities, 0, sizeof(struct _tds__GetCapabilities));
    tds__GetCapabilities->__sizeCategory = 0;
    tds__GetCapabilities->Category = (enum tt__CapabilityCategory*)soap_malloc(soap, sizeof(enum tt__CapabilityCategory));
    *(tds__GetCapabilities->Category) = (enum tt__CapabilityCategory)(tt__CapabilityCategory__Media);

    struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = (_tds__GetCapabilitiesResponse*)soap_malloc(soap, sizeof(_tds__GetCapabilitiesResponse));
    memset(tds__GetCapabilitiesResponse, 0, sizeof(struct _tds__GetCapabilitiesResponse));


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___tds__GetCapabilities(soap, camera_serviceAddr, NULL, tds__GetCapabilities, tds__GetCapabilitiesResponse);

    if(!soap->error)
    {
        if(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr != NULL)
        {
            memcpy(camera_mediaAddr, tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, strlen(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr));
            memcpy(camera_eventAddr, tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, strlen(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr));
            printf("[%s][%d][%s]get media addr success, mediaAddr is %s\n", camera_IP, __LINE__, __func__, camera_mediaAddr);
            printf("[%s][%d][%s]get media addr success, eventAddr is %s\n", camera_IP, __LINE__, __func__, camera_eventAddr);
        }
        else
        {

        }
    }
    else
    {

        retval = CAMERA_GETCAPABILICITYERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}
//获取设备配置
int CameraConfig::getProfiles()
{
    printf("[%s]function getProfiles() ......\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    //getProfile
    struct _trt__GetProfiles *trt__GetProfiles = (_trt__GetProfiles*)soap_malloc(soap, sizeof(_trt__GetProfiles));
    struct _trt__GetProfilesResponse *trt__GetProfilesResponse = (_trt__GetProfilesResponse*)soap_malloc(soap, sizeof(_trt__GetProfilesResponse));
    memset(trt__GetProfiles, 0, sizeof(struct _trt__GetProfiles));
    memset(trt__GetProfilesResponse, 0, sizeof(struct _trt__GetProfilesResponse));

    int ret = 0;

    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }
    ret = soap_call___trt__GetProfiles(soap, camera_mediaAddr, NULL, trt__GetProfiles, trt__GetProfilesResponse);


    if(!soap->error)
    {
        //get profile success
        camera_info.camera_configurationSize = trt__GetProfilesResponse->__sizeProfiles;

        if(camera_info.camera_configuration == NULL)
        {
            camera_info.camera_configuration = (CAMERA_CONFIGURATION*)malloc(camera_info.camera_configurationSize * sizeof(CAMERA_CONFIGURATION));
        }
        else if(camera_info.camera_configurationSize != trt__GetProfilesResponse->__sizeProfiles)
        {
            free(camera_info.camera_configuration);
            camera_info.camera_configuration = (CAMERA_CONFIGURATION*)malloc(camera_info.camera_configurationSize * sizeof(CAMERA_CONFIGURATION));
        }
//        if(profileToken == NULL)
//        {
//            profileToken = (char*)malloc(camera_info.camera_configurationSize * TOKEN_LENGTH);
//        }
//        if(configToken == NULL)
//        {
//            configToken = (char*)malloc(camera_info.camera_configurationSize * TOKEN_LENGTH);
//        }
//        memset(profileToken, 0, camera_info.camera_configurationSize * TOKEN_LENGTH);
//        memset(configToken, 0, camera_info.camera_configurationSize * TOKEN_LENGTH);
        if(camera_token == NULL)
        {
            camera_token = (TOKEN*)malloc(camera_info.camera_configurationSize * sizeof(TOKEN));
        }

        memset(camera_info.camera_configuration, 0, camera_info.camera_configurationSize * sizeof(CAMERA_CONFIGURATION));

        for(int i = 0; i < 2; i++)
        {
            printf("[%s]\n", camera_IP);
//            memset(camera_info[i], 0, sizeof(CAMERA_INFO));

//            printf("[%s]getConfigResponse.Configurations[%d]->Name = %s\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name);
//            printf("[%s]getConfigResponse.Configurations[%d]->UseCount = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->UseCount);
//            printf("[%s]getConfigResponse.Configurations[%d]->Token = %s \n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token);
//            printf("[%s]getConfigResponse.Configurations[%d]->Encoding = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Encoding);
//            printf("[%s]getConfigResponse.Configurations[%d]->resolution->Width = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Width);
//            printf("[%s]getConfigResponse.Configurations[%d]->resolution->Height = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Height);
//            printf("[%s]getConfigResponse.Configurations[%d]->Quality = %f \n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Quality);
//            printf("[%s]getConfigResponse.Configurations[%d]->RateControl->FrameRateLimit = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->FrameRateLimit);
//            printf("[%s]getConfigResponse.Configurations[%d]->RateControl->EncodingInterval = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->EncodingInterval);
//            printf("[%s]getConfigResponse.Configurations[%d]->RateControl->BitrateLimit = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->BitrateLimit);
//            printf("[%s]getConfigResponse.Configurations[%d]->H264->GovLength = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->GovLength);
//            printf("[%s]getConfigResponse.Configurations[%d]->H264->H264Profile = %d\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->H264Profile);
//            printf("[%s]trt__GetProfilesResponse->Profiles[%d].token = %s\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].token);
//            printf("[%s]trt__GetProfilesResponse->Profiles[%d].VideoEncoderConfiguration->token = %s\n", camera_IP, i, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token);

            camera_info.camera_configuration[i].RateControl.BitRateLimit = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->BitrateLimit;
            camera_info.camera_configuration[i].RateControl.EncodingInterval = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->EncodingInterval;
            camera_info.camera_configuration[i].RateControl.FrameRateLimit = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->FrameRateLimit;
            camera_info.camera_configuration[i].resolution.Height = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Height;
            camera_info.camera_configuration[i].resolution.Width = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Width;
            camera_info.camera_configuration[i].GovLength = trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->GovLength;
            memset(camera_info.camera_configuration[i].configurationName, 0, sizeof(camera_info.camera_configuration->configurationName));
            memcpy(camera_info.camera_configuration[i].configurationName, trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name, strlen(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name));

            memset(camera_info.camera_configuration[i].profileName, 0, sizeof(camera_info.camera_configuration->profileName));
            memcpy(camera_info.camera_configuration[i].profileName, trt__GetProfilesResponse->Profiles[i].Name, strlen(trt__GetProfilesResponse->Profiles[i].Name));

            memset(camera_token[i].configToken, 0, sizeof(camera_token->configToken));
            memset(camera_token[i].profileToken, 0, sizeof(camera_token->profileToken));
            memcpy(&(camera_token[i].profileToken), trt__GetProfilesResponse->Profiles[i].token, strlen(trt__GetProfilesResponse->Profiles[i].token));
            memcpy(&(camera_token[i].configToken), trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token, strlen(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token));
            memcpy(&(camera_token[i].videoSourceToken), trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token, strlen(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token));
            mprintf("camera %d config token is %s", i, camera_token[i].configToken);
            mprintf("camera %d profile token is %s", i, camera_token[i].profileToken);
            mprintf("camera %d videosource token is %s", i, camera_token[i].videoSourceToken);
        }
    }
    else
    {
        printf("[%s]getProfiles() failed, soap->error = %d\n", camera_IP, soap->error);
        //get profile failed

        retval = CAMERA_GETPROFILEERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}
//获取流媒体地址
int CameraConfig::getStreamUri()
{
    printf("[%s]function getStreamUri() ......\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    struct _trt__GetStreamUri *trt__GetStreamUri = (_trt__GetStreamUri*)soap_malloc(soap, sizeof(_trt__GetStreamUri));
    struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse = (_trt__GetStreamUriResponse*)soap_malloc(soap, sizeof(_trt__GetStreamUriResponse));
    struct tt__StreamSetup StreamSetup;
    memset(trt__GetStreamUri, 0, sizeof(_trt__GetStreamUri));
    memset(trt__GetStreamUriResponse, 0, sizeof(_trt__GetStreamUriResponse));


    StreamSetup.Transport = (struct tt__Transport*)soap_malloc(soap, sizeof(struct tt__Transport));
    memset(StreamSetup.Transport, 0, sizeof(struct tt__Transport));
    StreamSetup.Stream = tt__StreamType__RTP_Unicast;
    StreamSetup.Transport->Tunnel = 0;
    StreamSetup.__size = 1;
    StreamSetup.__any = NULL;
    StreamSetup.__anyAttribute = NULL;
    int i = 0;

    camera_info.camera_configurationSize = 2;
    printf("[%s]camera_configurationSize = %d\n", camera_IP, camera_info.camera_configurationSize);
    for(i = 0; i < camera_info.camera_configurationSize; i++)
    {
        trt__GetStreamUri->StreamSetup = &StreamSetup;
        trt__GetStreamUri->ProfileToken = (char*)soap_malloc(soap, sizeof(camera_token[i].profileToken));
        memset(trt__GetStreamUri->ProfileToken, 0, sizeof(camera_token[i].profileToken));
        memcpy(trt__GetStreamUri->ProfileToken, camera_token[i].profileToken, sizeof(camera_token[i].profileToken));
        if(needAuthorization)
        {
            soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
        }
        soap_call___trt__GetStreamUri(soap, camera_mediaAddr, NULL, trt__GetStreamUri, trt__GetStreamUriResponse);
        if(!soap->error)
        {
            memset(camera_info.camera_configuration[i].streamUri, 0, sizeof(camera_info.camera_configuration[i].streamUri));
            memcpy(camera_info.camera_configuration[i].streamUri, trt__GetStreamUriResponse->MediaUri->Uri, strlen(trt__GetStreamUriResponse->MediaUri->Uri));
            printf("[%s]camera_info.camera_configuration[%d].streamUri = %s\n", camera_IP, i, camera_info.camera_configuration[i].streamUri);
        }
        else
        {
            retval = CAMERA_GETSTREAMURIERROR;
        }
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}
//获取摄像机编码参数范围
int CameraConfig::getVideoConfigOption()
{
    printf("[%s]function getVideoConfigOption() ......\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions = (_trt__GetVideoEncoderConfigurationOptions*)soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationOptions));
    struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse = (_trt__GetVideoEncoderConfigurationOptionsResponse*)soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationOptionsResponse));
    memset(trt__GetVideoEncoderConfigurationOptions, 0, sizeof(_trt__GetVideoEncoderConfigurationOptions));
    memset(trt__GetVideoEncoderConfigurationOptionsResponse, 0, sizeof(_trt__GetVideoEncoderConfigurationOptionsResponse));

    trt__GetVideoEncoderConfigurationOptions->ProfileToken = (char*)soap_malloc(soap, 128);
    trt__GetVideoEncoderConfigurationOptions->ConfigurationToken = (char*)soap_malloc(soap, 128);

    memset(trt__GetVideoEncoderConfigurationOptions->ProfileToken, 0, 128);
    memset(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken, 0, 128);


    if(camera_info.camera_configurationOption == NULL)
    {
        camera_info.camera_configurationOption = (CAMERA_CONFIGURATIONOPTION*)malloc(camera_info.camera_configurationSize * sizeof(CAMERA_CONFIGURATIONOPTION));
    }

    //the number should be camera_info.camera_configurationSize, but if exceed 2, it will get a server error 12
    for(int i = 0; i < 2; i++)
    {
        //getCapabilitiesOptions
        memset(trt__GetVideoEncoderConfigurationOptions->ProfileToken, 0, 128);
        memset(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken, 0, 128);
        memcpy(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken, camera_token[i].configToken, strlen(camera_token[i].configToken));
        memcpy(trt__GetVideoEncoderConfigurationOptions->ProfileToken, camera_token[i].profileToken, strlen(camera_token[i].profileToken));
        if(needAuthorization)
        {
            soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
        }
        soap_call___trt__GetVideoEncoderConfigurationOptions(soap, camera_mediaAddr, NULL, trt__GetVideoEncoderConfigurationOptions, trt__GetVideoEncoderConfigurationOptionsResponse);
        if(!soap->error)
        {
            struct tt__VideoResolution *ResolationAvailable;
            ResolationAvailable = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable;
            int count = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable;
            memset(camera_info.camera_configurationOption[i].configurationName, 0, sizeof(camera_info.camera_configurationOption[i].configurationName));
            memset(camera_info.camera_configurationOption[i].profileName, 0, sizeof(camera_info.camera_configurationOption[i].profileName));
            memcpy(camera_info.camera_configurationOption[i].configurationName, camera_info.camera_configuration[i].configurationName, sizeof(camera_info.camera_configuration[i].configurationName));
            memcpy(camera_info.camera_configurationOption[i].profileName, camera_info.camera_configuration[i].profileName, sizeof(camera_info.camera_configuration[i].profileName));
            camera_info.camera_configurationOption[i].resolutionSize = count;
            camera_info.camera_configurationOption[i].resolution = (RESOLUTION*)malloc(count * sizeof(RESOLUTION));
            for(int j = 0; j < count; j++)
            {
                camera_info.camera_configurationOption[i].resolution[j].Height = ResolationAvailable->Height;
                camera_info.camera_configurationOption[i].resolution[j].Width  = ResolationAvailable->Width;
//                printf("[%s]getOptionsResponse.Options->H264->ResolutionAvailable[%d] = (%d x %d)\n", camera_IP, j, ResolationAvailable->Width, ResolationAvailable->Height);
                ResolationAvailable++;
            }

//            camera_info.camera_configurationOption[i].ratecontrol_option.BitRateLimit_max = trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max;
//            camera_info.camera_configurationOption[i].ratecontrol_option.BitRateLimit_min = trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min;
//            camera_info.camera_configurationOption[i].ratecontrol_option.EncodingInterval_max = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max;
//            camera_info.camera_configurationOption[i].ratecontrol_option.EncodingInterval_min = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min;
//            camera_info.camera_configurationOption[i].ratecontrol_option.FrameRateLimit_max = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max;
//            camera_info.camera_configurationOption[i].ratecontrol_option.FrameRateLimit_min = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min;

            if(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension != NULL)
            {
//                printf("[%s]getOptionsResponse.Options->Extension->H264->BitrateRange = (%d ~ %d)\n", camera_IP, trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min, trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max);
            }

        }
        else
        {
            printf("[%s]get configuration option error, soap->error = %d\n", camera_IP, soap->error);
            retval =  CAMERA_GETCONFIGURATIONOPTIONERROR;
        }
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;

}

void CameraConfig::setMainReslution(int width, int height)
{
    camera_info.camera_configuration[0].resolution.Width = width;
    camera_info.camera_configuration[0].resolution.Height = height;
}

void CameraConfig::setSubReslution(int width, int height)
{
    camera_info.camera_configuration[1].resolution.Width = width;
    camera_info.camera_configuration[1].resolution.Height = height;
}

void CameraConfig::setMainBitrateLimit(int rate)
{
    camera_info.camera_configuration[0].RateControl.BitRateLimit = rate;
}

void CameraConfig::setSubBitrateLimit(int rate)
{
    camera_info.camera_configuration[1].RateControl.BitRateLimit = rate;
}

void CameraConfig::setMainFrameRateLimit(int rate)
{
    camera_info.camera_configuration[0].RateControl.FrameRateLimit = rate;
}

void CameraConfig::setSubFrameRateLimit(int rate)
{
    camera_info.camera_configuration[1].RateControl.FrameRateLimit = rate;
}

void CameraConfig::setMainFrameGovLength(int len)
{
    camera_info.camera_configuration[0].GovLength = len;
}

void CameraConfig::setSubFrameGovLength(int len)
{
    camera_info.camera_configuration[1].GovLength = len;
}

void CameraConfig::setOSDString(char *str)
{
    if(str != NULL)
        strcpy(this->stringAddToOsd, str);
}

int CameraConfig::getOSDOption()
{
    printf("[%s]start getOSDOption() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;


    struct _trt__GetOSDOptions *trt__GetOSDOptions = (struct _trt__GetOSDOptions *)soap_malloc(soap, sizeof(_trt__GetOSDOptions));
    struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse = (struct _trt__GetOSDOptionsResponse *)soap_malloc(soap, sizeof(_trt__GetOSDOptionsResponse));
    memset(trt__GetOSDOptions, 0, sizeof(_trt__GetOSDOptions));
    memset(trt__GetOSDOptionsResponse, 0, sizeof(_trt__GetOSDOptionsResponse));
    struct tt__OSDConfigurationOptions *Configurations = NULL;
    trt__GetOSDOptions->ConfigurationToken = this->camera_token[0].videoSourceToken;
    trt__GetOSDOptions->__size = 0;

    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___trt__GetOSDOptions(soap, camera_mediaAddr, NULL, trt__GetOSDOptions, trt__GetOSDOptionsResponse);

    printf("[%s]trt__GetOSDOptions->ConfigurationToken = %s\n", camera_IP, trt__GetOSDOptions->ConfigurationToken);
//    printf("[%s]trt__GetOSDOptions->__size = %d\n", camera_IP, trt__GetOSDOptions->__size);
//    printf("[%s]trt__GetOSDOptions->__any = %s\n", camera_IP, trt__GetOSDOptions->__any);
//    printf("[%s]trt__GetOSDOptionsResponse->__size = %d\n", camera_IP, trt__GetOSDOptionsResponse->__size);
//    printf("[%s]trt__GetOSDOptionsResponse->__any = %s\n", camera_IP, trt__GetOSDOptionsResponse->__any);
//    printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->__sizeType = %d\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->__sizeType);
//    printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->__anyAttribute = %s\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->__anyAttribute);


    if(!soap->error)
    {
        Configurations = trt__GetOSDOptionsResponse->OSDOptions;

//        printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Total = %d\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Total);
//        printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText = %d\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->PlainText);
//        printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime = %d\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->DateAndTime);
//        printf("[%s]trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Image = %d\n", camera_IP, trt__GetOSDOptionsResponse->OSDOptions->MaximumNumberOfOSDs->Image);
    }
    else
    {
        printf("[%s]GetOSDOptions error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDOPTIONSERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::getOSDs()
{
    printf("[%s]start getOSDs() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;


    struct _trt__GetOSDs *trt__GetOSDs = (struct _trt__GetOSDs *)soap_malloc(soap, sizeof(_trt__GetOSDs));
    struct _trt__GetOSDsResponse *trt__GetOSDsResponse = (struct _trt__GetOSDsResponse *)soap_malloc(soap, sizeof(_trt__GetOSDsResponse));
    struct tt__OSDConfiguration *Configurations = NULL;
    memset(trt__GetOSDs, 0, sizeof(_trt__GetOSDs));
    memset(trt__GetOSDsResponse, 0, sizeof(_trt__GetOSDsResponse));
    trt__GetOSDs->ConfigurationToken = this->camera_token[0].videoSourceToken;
    trt__GetOSDs->ConfigurationToken = 0;
    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    SOAP_FMAC6 soap_call___trt__GetOSDs(soap, camera_mediaAddr, NULL, trt__GetOSDs, trt__GetOSDsResponse);

    if(!soap->error)
    {
        Configurations = trt__GetOSDsResponse->OSDs;

        for(int i=0; i<trt__GetOSDsResponse->__sizeOSDs; i++)
        {
            printf("[%s]the %d osd %s is : %s\n", camera_IP, i, trt__GetOSDsResponse->OSDs[i].token, trt__GetOSDsResponse->OSDs[i].TextString->PlainText);
            if(i>1)
            {
                sprintf(this->osdToken, "%s", trt__GetOSDsResponse->OSDs[i].token);
                printf("[%s]i will delete the %d osd\n", camera_IP, i);
                deleteOSD();
            }
        }

    }
    else
    {
        printf("[%s]GetOSDs error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDSERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::getOSD()
{
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;


    struct _trt__GetOSD *trt__GetOSD = (struct _trt__GetOSD *)soap_malloc(soap, sizeof(_trt__GetOSD));
    struct _trt__GetOSDResponse *trt__GetOSDResponse = (struct _trt__GetOSDResponse *)soap_malloc(soap, sizeof(_trt__GetOSDResponse));
    memset(trt__GetOSD, 0, sizeof(_trt__GetOSD));
    memset(trt__GetOSDResponse, 0, sizeof(_trt__GetOSDResponse));
    struct tt__OSDConfiguration *Configurations = NULL;
    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    SOAP_FMAC6 soap_call___trt__GetOSD(soap, camera_mediaAddr, NULL, trt__GetOSD, trt__GetOSDResponse);

    if(!soap->error)
    {
        Configurations = trt__GetOSDResponse->OSD;

        printf("[%s]trt__GetOSDsResponse->OSD->token = %s\n", camera_IP, trt__GetOSDResponse->OSD->token);
    }
    else
    {
        printf("[%s]GetOSD error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::createOSD()
{

    printf("[%s]start camera_createOSD() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;


    struct _trt__CreateOSD *trt__CreateOSD = (struct _trt__CreateOSD *)soap_malloc(soap, sizeof(_trt__CreateOSD));
    struct _trt__CreateOSDResponse *trt__CreateOSDResponse = (struct _trt__CreateOSDResponse *)soap_malloc(soap, sizeof(_trt__CreateOSDResponse));
    memset(trt__CreateOSD, 0, sizeof(_trt__CreateOSD));
    memset(trt__CreateOSDResponse, 0, sizeof(_trt__CreateOSDResponse));
    trt__CreateOSD->__size = 0;

    struct tt__OSDConfiguration osd;
    char* token = "";
    osd.token = token;
    tt__OSDReference VideoSourceConfigurationToken;
    VideoSourceConfigurationToken.__item = this->camera_token[0].videoSourceToken;
    osd.VideoSourceConfigurationToken = &VideoSourceConfigurationToken;
    osd.Type = tt__OSDType__Text;
    struct tt__OSDPosConfiguration pos;
    char *postype = "Custom";
    pos.Type = postype;
    float x = -0.8;
    float y = 0.0;
    struct tt__Vector coor;
    coor.x = &x;
    coor.y = &y;
    pos.Pos = &coor;
    struct tt__OSDPosConfigurationExtension posextension;
    posextension.__any = "";
    posextension.__size = 8;
    posextension.__anyAttribute = "";
    pos.__anyAttribute = "";
    pos.Extension = &posextension;
    struct tt__OSDTextConfiguration textstr;
    struct tt__OSDColor fontcolor;
    struct tt__OSDColor backgroundcolor;
    struct tt__OSDTextConfigurationExtension textstrExtension;
    textstrExtension.__any = "";
    textstrExtension.__size = 1;
    textstrExtension.__anyAttribute = "";

    struct tt__Color color;
    //<tt:Color X="16.000000" Y="128.000000" Z="128.000000"
        //Colorspace="http://www.onvif.org/ver10/colorspace/YCbCr"
    color.X = 16;
    color.Y = 128;
    color.Z = 128;
    color.Colorspace = "http://www.onvif.org/ver10/colorspace/YCbCr";
    fontcolor.Color = &color;
    int transparent = 0;
    fontcolor.Transparent = &transparent;
    fontcolor.__anyAttribute = "";

    backgroundcolor.Color = &color;
    backgroundcolor.Transparent = &transparent;
    backgroundcolor.__anyAttribute = "";
    textstr.Extension = &textstrExtension;
    textstr.FontColor = &fontcolor;
    textstr.BackgroundColor = &backgroundcolor;

    char *strtype = "Plain";
    int fontsize = 32;
    int *FontSize = &fontsize;
    char *plaintext = "ALARM test";
    plaintext = this->stringAddToOsd;
    textstr.Type = strtype;
    textstr.FontSize = FontSize;
    textstr.PlainText = plaintext;
    textstr.TimeFormat = "";
    textstr.DateFormat = "";
    textstr.__anyAttribute = "";
    osd.Extension = NULL;
    osd.Image = NULL;
    osd.TextString = &textstr;
    osd.Position = &pos;

    struct tt__OSDConfigurationExtension extension;
    extension.__any = "";
    extension.__size = 0;
    extension.__anyAttribute = "";
//    osd.Extension = &extension;
    trt__CreateOSD->OSD = &osd;


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    SOAP_FMAC6 soap_call___trt__CreateOSD(soap, camera_mediaAddr, NULL, trt__CreateOSD, trt__CreateOSDResponse);

    if(!soap->error)
    {
        printf("[%s]create osd token : %s\n", camera_IP, trt__CreateOSDResponse->OSDToken);
        memcpy(this->camera_token->createOSDToken, trt__CreateOSDResponse->OSDToken, strlen(trt__CreateOSDResponse->OSDToken));
        printf("[%s]this->camera_token->createOSDToken = %s\n", camera_IP, this->camera_token->createOSDToken);
    }
    else
    {
        printf("[%s]CreateOSD error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDSERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::deleteOSD()
{
    printf("[%s]start camera_deleteOSD() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;


    struct _trt__DeleteOSD *trt__DeleteOSD = (struct _trt__DeleteOSD *)soap_malloc(soap, sizeof(_trt__DeleteOSD));
    struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse = (struct _trt__DeleteOSDResponse *)soap_malloc(soap, sizeof(_trt__DeleteOSDResponse));
    memset(trt__DeleteOSD, 0, sizeof(_trt__DeleteOSD));
    memset(trt__DeleteOSDResponse, 0, sizeof(_trt__DeleteOSDResponse));
    struct tt__OSDConfiguration OSD;

    printf("[%s]this->camera_token->createOSDToken = %s\n", camera_IP, this->camera_token->createOSDToken);
    printf("[%s]delete osd token %s\n", camera_IP, this->camera_token->createOSDToken);
//    trt__DeleteOSD->OSDToken = this->camera_token->createOSDToken;
    trt__DeleteOSD->OSDToken = this->osdToken;

    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }


    SOAP_FMAC6 soap_call___trt__DeleteOSD(soap, camera_mediaAddr, NULL, trt__DeleteOSD, trt__DeleteOSDResponse);

    if(!soap->error)
    {
        printf("[%s]delete osd success\n", camera_IP);

    }
    else
    {
        printf("[%s]delete osd error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDSERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::getSnapshot()
{
    printf("[%s]start getSnapshot() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;
    char cmd[256];
    char uri_auth[256] = {0};


    struct _trt__GetSnapshotUri *trt__GetSnapshotUri = (struct _trt__GetSnapshotUri *)soap_malloc(soap, sizeof(_trt__GetSnapshotUri));
    struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse = (struct _trt__GetSnapshotUriResponse *)soap_malloc(soap, sizeof(_trt__GetSnapshotUriResponse));
    memset(trt__GetSnapshotUri, 0, sizeof(_trt__GetSnapshotUri));
    memset(trt__GetSnapshotUriResponse, 0, sizeof(_trt__GetSnapshotUriResponse));
    struct tt__OSDConfiguration OSD;
    trt__GetSnapshotUri->ProfileToken = this->camera_token->profileToken;


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }


    SOAP_FMAC6 soap_call___trt__GetSnapshotUri(soap, camera_mediaAddr, NULL, trt__GetSnapshotUri, trt__GetSnapshotUriResponse);

    if(!soap->error)
    {
        printf("[%s]get snapshot uri success\n", camera_IP);
        if(NULL != trt__GetSnapshotUriResponse->MediaUri)
        {
            if(NULL != trt__GetSnapshotUriResponse->MediaUri->Uri)
            {
                strcpy(this->snapshotUri, trt__GetSnapshotUriResponse->MediaUri->Uri);
                make_uri_withauth(snapshotUri, camera_userName, camera_password, uri_auth, sizeof(uri_auth));
                printf("[%s]uri = %s\n", camera_IP, uri_auth);
                sprintf(cmd, "wget -O out.jpeg '%s'", uri_auth);                        // ʹ��wget����ͼƬ
                system(cmd);
            }
        }

    }
    else
    {
        printf("[%s]delete osd error %d\n", camera_IP, soap->error);
        retval = CAMERA_GETOSDSERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

int CameraConfig::make_uri_withauth(char *src_uri, char *username, char *password, char *dest_uri, unsigned int size_dest_uri)
{
    int result = 0;
    unsigned int needBufSize = 0;

    memset(dest_uri, 0x00, size_dest_uri);

    needBufSize = strlen(src_uri) + strlen(username) + strlen(password) + 3;    // ��黺���Ƿ��㹻��������:���͡�@�����ַ���������
    if (size_dest_uri < needBufSize) {
        printf("[%s]dest uri buf size is not enough.\n", camera_IP);
        result = -1;
        goto EXIT;
    }

    if (0 == strlen(username) && 0 == strlen(password)) {                       // �����µ�uri��ַ
        strcpy(dest_uri, src_uri);
    } else {
        char *p = strstr(src_uri, "//");
        if (NULL == p) {
            printf("[%s]can't found '//', src uri is: %s.\n", camera_IP, src_uri);
            result = -1;
            goto EXIT;
        }
        p += 2;

        memcpy(dest_uri, src_uri, p - src_uri);
        sprintf(dest_uri + strlen(dest_uri), "%s:%s@", username, password);
        strcat(dest_uri, p);
    }

EXIT:

    return result;
}


//设置摄像机编码参数
int CameraConfig::setVideoConfig(int num)
{
    printf("[%s]start setVideoConfig() ...\n", camera_IP);
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);
    soap->connect_timeout = 2;
    soap->recv_timeout = 10;
    soap->send_timeout = 2;
    //getCapabilities
    struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration = (_trt__GetVideoEncoderConfiguration*)soap_malloc(soap, sizeof(_trt__GetVideoEncoderConfiguration));
    struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse = (_trt__GetVideoEncoderConfigurationResponse*)soap_malloc(soap, sizeof(_trt__GetVideoEncoderConfigurationResponse));
    struct tt__VideoEncoderConfiguration *Configurations = NULL;

    trt__GetVideoEncoderConfiguration->ConfigurationToken = this->camera_token[num].configToken;

    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___trt__GetVideoEncoderConfiguration(soap, camera_mediaAddr, NULL, trt__GetVideoEncoderConfiguration, trt__GetVideoEncoderConfigurationResponse);

    struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration = (_trt__SetVideoEncoderConfiguration*)soap_malloc(soap, sizeof(_trt__SetVideoEncoderConfiguration));
    struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse = (_trt__SetVideoEncoderConfigurationResponse*)soap_malloc(soap, sizeof(_trt__SetVideoEncoderConfigurationResponse));


    if(!soap->error)
    {
        Configurations = trt__GetVideoEncoderConfigurationResponse->Configuration;
    }
    else
    {
        retval = CAMERA_GETCAPABILICITYERROR;
        printf("[%s][%s]soap->error = %d\n", camera_IP, __FUNCTION__ , soap->error);
        goto EXIT;
    }

    if(Configurations == NULL)
    {
        printf("[%s][%s]Configurations is null\n", camera_IP, __FUNCTION__);
        goto EXIT;
    }

    Configurations->Resolution->Height = camera_info.camera_configuration[num].resolution.Height;
    Configurations->Resolution->Width = camera_info.camera_configuration[num].resolution.Width;
    Configurations->RateControl->BitrateLimit = camera_info.camera_configuration[num].RateControl.BitRateLimit;
    Configurations->RateControl->EncodingInterval = camera_info.camera_configuration[num].RateControl.EncodingInterval;
    Configurations->RateControl->FrameRateLimit = camera_info.camera_configuration[num].RateControl.FrameRateLimit;
    Configurations->H264->GovLength = camera_info.camera_configuration[num].GovLength;

    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    trt__SetVideoEncoderConfiguration->Configuration = Configurations;
    trt__SetVideoEncoderConfiguration->ForcePersistence = xsd__boolean__true_;
    soap_call___trt__SetVideoEncoderConfiguration(soap, camera_mediaAddr, NULL, trt__SetVideoEncoderConfiguration, trt__SetVideoEncoderConfigurationResponse);
    if(!soap->error)
    {
        printf("[%s]set video encoder configuration success!\n", camera_IP);
    }
    else
    {
        retval = CAMERA_SETCONFIGURATIONERROR;
    }

EXIT:
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}
//获取摄像机所有信息
int CameraConfig::getCameraInfo()
{
    int retval = CAMERA_OK;

    if((retval = checkDevice()) != CAMERA_OK)
    {
        return retval;
    }

    if((retval = getCapabilities()) != CAMERA_OK)
    {
        return retval;
    }

    if((retval = getProfiles()) != CAMERA_OK)
    {
        return retval;
    }

    if((retval = getVideoConfigOption()) != CAMERA_OK)
    {
        return retval;
    }

    if((retval = getStreamUri()) != CAMERA_OK)
    {
        return retval;
    }

    return retval;
}
//发现所有在线的摄像机并获取它们的IP地址
int camera_findAll(IP **all_ip, int* deviceCount)
{
    printf("start find all camera ...\n");
    IP tmp[*deviceCount];
    memset(tmp, '\0', sizeof(IP)*(*deviceCount));
    int count = 0;
    int retval = CAMERA_OK;
    const char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    const char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";

    int result = 0;
    wsdd__ProbeType req;
    struct __wsdd__ProbeMatches resp;
    wsdd__ScopesType sScope;
    struct SOAP_ENV__Header header;
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap->connect_timeout = 1;
    soap->recv_timeout = 5;
    soap->send_timeout = 1;
    soap_set_namespaces(soap, namespaces);
    soap_default_SOAP_ENV__Header(soap, &header);

    const int uuidlen = 48;
    char* uuid = (char*)soap_malloc(soap, uuidlen);
    int r1, r2, r3, r4;
    r1 = soap_random;
    r2 = soap_random;
    r3 = soap_random;
    r4 = soap_random;
    sprintf(uuid, "urn:uuid:%8.8x-%4.4hx-4%3.3hx-%4.4hx-%4.4hx%8.8x", r1, (short)(r2 >> 16), ((short)r2 >> 4) & 0x0fff, ((short)(r3 >> 16) & 0x3fff) | 0x8000, (short)r3, r4);

    header.wsa__MessageID = uuid;
//    header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
//    header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    if(was_Action != NULL)
    {
        header.wsa__Action = (char*)malloc(1024);
        memset(header.wsa__Action, '\0', 1024);
        strncpy(header.wsa__Action, was_Action, 1024);//
    }
    if(was_To != NULL)
    {
        header.wsa__To = (char *)malloc(1024);
        memset(header.wsa__To, '\0', 1024);
        strncpy(header.wsa__To, was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    }

    soap->header = &header;
//    soap->user = (void*)ip;
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = NULL;
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;
    req.Types = NULL;//"dn:NetworkVideoTransmitter";
    result = soap_send___wsdd__Probe(soap, MULTICAST_ADDRESS, NULL, &req);
    while(result == SOAP_OK)
    {
        result = soap_recv___wsdd__ProbeMatches(soap, &resp);
        if(result == SOAP_OK)
        {
            if(soap->error)
            {
                printf("[%d]:recv soap error :%d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
//                soap_error_number = soap->error;
                retval = CAMERA_DEVICENOTONLINE;
                break;
            }
            else //we find a device
            {
                printf("find a device xaddr is %s\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                char tmpAddr[256] = "";
                char delims1[] = "/";
                char delims2[] = ":";
                char *result = NULL;
                strcpy(tmpAddr, resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                result = strtok(tmpAddr, delims1);
                result = strtok(NULL, delims1);
                result = strtok(result, delims2);
                if(result == NULL) continue;
                printf("result is %s, strlen of result is %d\n", result, strlen(result));
                memcpy(&(tmp[count].ip), result, strlen(result));

                count++;
                if(count == *deviceCount)
                {
                    break;
                }
            }
        }
        else if(soap->error)
        {
//            camera_isOnline = false;
//            soap_error_number = soap->error;
            break;
        }

    }
    *all_ip = (IP*)malloc(count * sizeof(IP));
    int i = 0;
    for(i = 0; i < count; i++)
    {
        printf("tmp[%d].ip is %s\n", i, tmp[i].ip);
        memset((((*all_ip)[i]).ip), 0, sizeof(((*all_ip)[i]).ip));
        memcpy((((*all_ip)[i]).ip), tmp[i].ip, sizeof(tmp[i].ip));
    }
    *deviceCount = count;
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;

}


int CameraConfig::getSystemDateAndTime()
{
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    struct _tds__GetSystemDateAndTime         req;
    struct _tds__GetSystemDateAndTimeResponse rep;
    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___tds__GetSystemDateAndTime(soap, camera_serviceAddr, NULL,  &req, &rep);
    if(!soap->error)
    {
        int year = rep.SystemDateAndTime->LocalDateTime->Date->Year;
        int month = rep.SystemDateAndTime->LocalDateTime->Date->Month;
        int day = rep.SystemDateAndTime->LocalDateTime->Date->Day;
        int hour = rep.SystemDateAndTime->LocalDateTime->Time->Hour;
        int min = rep.SystemDateAndTime->LocalDateTime->Time->Minute;
        int sec = rep.SystemDateAndTime->LocalDateTime->Time->Second;
        printf("[%s]camera %s time is %d-%d-%d %d:%d:%d\n", camera_IP, camera_IP, year, month, day, hour, min, sec);
    }
    else
    {

        retval = CAMERA_GETSYSTEMDATEANDTIMEERROR;
    }
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}


int CameraConfig::setSystemDateAndTime(int sysYear, int sysMonth, int sysDay, int sysHour, int sysMinute, int sysSecond)
{
    printf("setSystemDateAndTime\n");
    int retval = CAMERA_OK;
    //soap
    struct soap soap_client;
    struct soap* soap = &soap_client;
    soap_init(soap);
    soap_set_namespaces(soap, namespaces);
    soap_header(soap);

    struct _tds__GetSystemDateAndTime         req;
    struct _tds__GetSystemDateAndTimeResponse rep;
    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___tds__GetSystemDateAndTime(soap, camera_serviceAddr, NULL,  &req, &rep);
    if(!soap->error)
    {
        int year = rep.SystemDateAndTime->LocalDateTime->Date->Year;
        int month = rep.SystemDateAndTime->LocalDateTime->Date->Month;
        int day = rep.SystemDateAndTime->LocalDateTime->Date->Day;
        int hour = rep.SystemDateAndTime->LocalDateTime->Time->Hour;
        int min = rep.SystemDateAndTime->LocalDateTime->Time->Minute;
        int sec = rep.SystemDateAndTime->LocalDateTime->Time->Second;
        mprintf("[%s]time of camera %s is %d-%d-%d %d:%d:%d\n", camera_IP, camera_IP, year, month, day, hour, min, sec);
    }
    else
    {
        mprintf("[%s]get camera time failed, error = %d\n", camera_IP, soap->error);
        retval = CAMERA_GETSYSTEMDATEANDTIMEERROR;

        goto stop;
    }


    struct _tds__SetSystemDateAndTime req_set;
    struct _tds__SetSystemDateAndTimeResponse rep_set;
    memset(&req_set, 0x00, sizeof(req_set));
    memset(&rep_set, 0x00, sizeof(rep_set));


    req_set.DateTimeType = rep.SystemDateAndTime->DateTimeType;
    req_set.DaylightSavings = rep.SystemDateAndTime->DaylightSavings;

    req_set.TimeZone          = (struct tt__TimeZone *)malloc(sizeof(struct tt__TimeZone));
    req_set.UTCDateTime       = (struct tt__DateTime *)malloc(sizeof(struct tt__DateTime));
    req_set.UTCDateTime->Date = (struct tt__Date *)malloc(sizeof(struct tt__Date));
    req_set.UTCDateTime->Time = (struct tt__Time *)malloc(sizeof(struct tt__Time));

//    req_set.TimeZone->TZ = rep.SystemDateAndTime->TimeZone->TZ;
    req_set.TimeZone->TZ = "CST-08:00:00";
    req_set.UTCDateTime->Date->Year   = sysYear;
    req_set.UTCDateTime->Date->Month  = sysMonth;
    req_set.UTCDateTime->Date->Day    = sysDay;
    req_set.UTCDateTime->Time->Hour   = sysHour;
    req_set.UTCDateTime->Time->Minute = sysMinute;
    req_set.UTCDateTime->Time->Second = sysSecond;


    if(needAuthorization)
    {
        soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);
    }

    soap_call___tds__SetSystemDateAndTime(soap, camera_serviceAddr, NULL,  &req_set, &rep_set);
    if(!soap->error)
    {
        printf("[%s]set camera time success !\n", camera_IP);
    }
    else
    {
        printf("[%s]set camera time failed, error = %d\n", camera_IP, soap->error);
        retval = CAMERA_SETSYSTEMDATEANDTIMEERROR;
    }

stop:
    soap_destroy(soap);
    soap_end(soap);
    soap_done(soap);
    return retval;
}

/* 遮挡报警 */
#define TAMPER_TOPIC            "tns1:RuleEngine/TamperDetector/Tamper"
#define TAMPER_NAME             "isTamper"
#define TAMPER_VALUE            "true"

#define MOTION_TOPIC            "tns1:RuleEngine/CellMotionDetector/Motion"
#define MOTION_NAME             "IsMotion"
#define MOTION_VALUE            "true"

//int find_event(struct _tev__PullMessagesResponse *rep, char *topic, char *name, char *value);
//int isMotion(struct _tev__PullMessagesResponse *rep);
//int ONVIF_CreatePullPointSubscription(const char *EventXAddr);
//int ONVIF_GetEventProperties(const char *EventXAddr);
//int ONVIF_GetServiceCapabilities(const char *EventXAddr);



/************************************************************************
**函数：find_event
**功能：查找指定主题、指定内容的事件
**参数：略
**返回：
        0表明未找到，非0表明找到
************************************************************************/
int CameraConfig::find_event(struct _tev__PullMessagesResponse *rep, const char *topic, const char *name, const char *value)
{
//    printf("[%s]get into find_event() ...\n", camera_IP);
    int i, j;

    printf("%s\n", topic);

    if(NULL == rep) {
        return 0;
    }

    for (i = 0; i < rep->__sizeNotificationMessage; i++) {
        struct wsnt__NotificationMessageHolderType *p = rep->wsnt__NotificationMessage + i;
        //printf("[%s]message %d : %s\n", camera_IP, i, p->Message.__any);
        //printf("[%s]p->Topic->__any : %s\n", camera_IP,p->Topic->__any);
        //printf("[%s]p->Topic->__anyAttribute : %s\n", camera_IP,p->Topic->__anyAttribute);
        //printf("[%s]p->Topic->__mixed : %s\n", camera_IP,p->Topic->__mixed);
        if (NULL == p->Topic) {
            printf("[%s]topic\n", camera_IP);
            continue;
        }

        if (NULL == p->Topic->__any ) {
            printf("[%s]topic any\n", camera_IP);
            continue;
        }

//        if (0 != strcmp(topic, p->Topic->__any)) {
//            printf("[%s]topic compare\n", camera_IP);
//            printf("[%s]message %d : %s\n", camera_IP, i, p->Topic->__any);
//            continue;
//        }

        if (NULL == p->Message.__any) {
            printf("[%s]Message.__any \n", camera_IP);
            continue;
        }


        if(strstr(p->Message.__any, "IsMotion"))
        {
            if(strstr(p->Message.__any, "true"))
            {
//                printf("[%s]message %d : %s\n", camera_IP, i, p->Message.__any);
//                printf("[%s]motion detect active ...\n", camera_IP);
                //motion start ...
                platform->HostDevice.IPC[camera_channel].isMotion = 1;
                return 1;
            }
            if(strstr(p->Message.__any, "false"))
            {
//                printf("[%s]message %d : %s\n", camera_IP, i, p->Message.__any);
//                printf("[%s]motion detect stop\n", camera_IP);
                //motion stop ...
                platform->HostDevice.IPC[camera_channel].isMotion = 0;
                return 3;
            }
        }
        else if(strstr(p->Topic->__any, "MotionAlarm"))
        {
//            printf("[%s]message %d : %s\n", camera_IP, i, p->Topic->__any);
            //motion continue ...
            return 2;
        }
        else
        {
            return -1;
        }



        /*the following code do not run for xml parsing problem*/
        struct _tt__Message *ttm = (struct _tt__Message*)p->Message.__any;

        if (NULL == ttm->Data) {
            printf("[%s]ttm->Data \n", camera_IP);
            continue;
        }
        if (NULL == ttm->Data->SimpleItem) {
            printf("[%s]ttm->Data->SimpleItem \n", camera_IP);
            continue;
        }


        for (j = 0; j < ttm->Data->__sizeSimpleItem; j++) {
            struct _tt__ItemList_SimpleItem *a = ttm->Data->SimpleItem + j;
            if (NULL == a->Name || NULL == a->Value) {
                printf("[%s](NULL == a->Name || NULL == a->Value) \n", camera_IP);
                continue;
            }
            if (0 != strcmp(name, a->Name)) {
                printf("[%s]a->Name \n", camera_IP);
                continue;
            }
            if (0 != strcmp(value, a->Value)) {
                printf("[%s]a->Value \n", camera_IP);
                continue;
            }
            printf("[%s]name %s, value %s\n", camera_IP, a->Name, a->Value);
            return 1;
        }
    }


    return 0;
}

/************************************************************************
**函数：isMotion
**功能：判断是否有遮挡报警
**参数：略
**返回：
        0表明没有，非0表明有
************************************************************************/
int CameraConfig::isMotion(struct _tev__PullMessagesResponse *rep)
{
    return find_event(rep, MOTION_TOPIC, MOTION_NAME, MOTION_VALUE);
}

/************************************************************************
**函数：ONVIF_CreatePullPointSubscription
**功能：使用Pull-Point方式订阅事件
**参数：
        [in] EventXAddr - 事件服务地址
**返回：
        0表明成功，非0表明失败
************************************************************************/
int CameraConfig::ONVIF_CreatePullPointSubscription(const char *EventXAddr)
{
    printf("[%s]get into ONVIF_CreatePullPointSubscription() ... ...\n", camera_IP);
    int result = 0;
    struct soap *soap = NULL;
    char *pullpoint = NULL;
    struct _tev__CreatePullPointSubscription         req;
    struct _tev__CreatePullPointSubscriptionResponse rep;

    struct _tev__PullMessages                        req_pm;
    struct _tev__PullMessagesResponse                rep_pm;

    struct _wsnt__Unsubscribe                        req_u;
    struct _wsnt__UnsubscribeResponse                rep_u;

    int PULLMSG_TIMEOUT_UNIT;
    if(strcmp(platform->HostDevice.IPC[camera_channel].factory, "hk") == 0)
        PULLMSG_TIMEOUT_UNIT = 1000;// 海康IPC
    else if(strcmp(platform->HostDevice.IPC[camera_channel].factory, "dahua") == 0)
        PULLMSG_TIMEOUT_UNIT = 5000000;// 大华IPC
    else
        PULLMSG_TIMEOUT_UNIT = 1000;

    if(strcmp(camera_IP, "172.16.41.250") == 0)
        PULLMSG_TIMEOUT_UNIT = 5000000;//test ipc


    LONG64 pullmsg_timeout = 5 * PULLMSG_TIMEOUT_UNIT;                          // PullMessages查询事件的超时时间，不同IPC厂家的单位不同
    int socket_timeout = 30;                                                    // 创建soap的socket超时时间，单位秒

    SOAP_ASSERT(pullmsg_timeout < socket_timeout * PULLMSG_TIMEOUT_UNIT);       // 要确保查询事件的超时时间比socket超时时间小，否则，事件没查询到就socket超时，导致PullMessages返回失败
    SOAP_ASSERT(NULL != EventXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(socket_timeout)));

    /*
    *
    * 可以通过主题过滤我们所订阅的事件，过滤规则在官方「ONVIF Core Specification」规格说明书「Topic Filter」章节里有详细的介绍。
    * 比如：
    * tns1:RuleEngine/TamperDetector/Tamper   只关心遮挡报警
    * tns1:RuleEngine/TamperDetector//.       只关心主题TamperDetector树下的事件
    * NULL                                    关心所有事件，即不过滤
    *                                         也可以通过 '|' 表示或的关系，即同时关心某几类事件
    *
    */
    memset(&req, 0x00, sizeof(req));                                            // 订阅事件
    memset(&rep, 0x00, sizeof(rep));
    req.Filter = (struct wsnt__FilterType *)ONVIF_soap_malloc(soap, sizeof(struct wsnt__FilterType));

//    req.Filter->TopicExpression = (struct wsnt__TopicExpressionType *)ONVIF_soap_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
//    req.Filter->TopicExpression->Dialect = "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
//    req.Filter->TopicExpression->__mixed = TAMPER_TOPIC;
    ONVIF_SetAuthInfo(soap, camera_userName, camera_password);

    result = soap_call___tev__CreatePullPointSubscription(soap, EventXAddr, NULL, &req, &rep);

    SOAP_CHECK_ERROR(result, soap, "CreatePullPointSubscription");
//    dump_tev__CreatePullPointSubscriptionResponse(&rep);

    pullpoint = rep.SubscriptionReference.Address;                              // 提取pull point地址
    if(pullpoint)
        printf("[%s]pollpoint addr is %s\n", camera_IP, pullpoint);

    // 轮询事件
    for (;;) {
        sleep(5);
        memset(&req_pm, 0x00, sizeof(req_pm));
        memset(&rep_pm, 0x00, sizeof(rep_pm));
        req_pm.Timeout      = pullmsg_timeout;
        req_pm.MessageLimit = 0;
        ONVIF_SetAuthInfo(soap, camera_userName, camera_password);
        result = soap_call___tev__PullMessages(soap, pullpoint, NULL, &req_pm, &rep_pm);
//        SOAP_CHECK_ERROR(result, soap, "PullMessages");

        if(result == -1)
            continue;
//        dump_tev__PullMessagesResponse(&rep_pm);

        result = isMotion(&rep_pm);
        platform->HostDevice.IPC[this->camera_channel].isMotion = result;
        switch(result)
        {
            case 1:
                mprintf("[%s]detect motion active ...\n", camera_IP);
//                setAlarmLed(MOTION_ERROR, true);
                break;
            case 2:
                printf("[%s]detect motion continue ...\n", camera_IP);
//                setAlarmLed(MOTION_ERROR, true);
                break;
            case 3:
                mprintf("[%s]detect motion stop ...\n", camera_IP);
//                setAlarmLed(MOTION_ERROR, false);
                break;
            default:
                printf("[%s]detect motion null ...\n", camera_IP);
                break;
        }
        sleep(1);
    }

EXIT:

    memset(&req_u, 0x00, sizeof(req_u));                                        // 退订事件
    memset(&rep_u, 0x00, sizeof(rep_u));
    ONVIF_SetAuthInfo(soap, camera_userName, camera_password);
    result = soap_call___tev__Unsubscribe(soap, pullpoint, NULL, &req_u, &rep_u);
    if (SOAP_OK != result || SOAP_OK != soap->error) {
        soap_perror(soap, "Unsubscribe");
        if (SOAP_OK == result) {
            result = soap->error;
        }
    }

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}

/************************************************************************
**函数：ONVIF_GetEventProperties
**功能：获取事件属性
**参数：
        [in] EventXAddr - 事件服务地址
**返回：
        0表明成功，非0表明失败
************************************************************************/
int CameraConfig::ONVIF_GetEventProperties(const char *EventXAddr)
{
    printf("[%s]get into ONVIF_GetEventProperties() ... ...\n", camera_IP);
    int result = 0;
    struct soap *soap = NULL;
    struct _tev__GetEventProperties         req;
    struct _tev__GetEventPropertiesResponse rep;

    SOAP_ASSERT(NULL != EventXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    soap_wsse_add_UsernameTokenDigest(soap, "user", camera_userName, camera_password);

    result = soap_call___tev__GetEventProperties(soap, EventXAddr, NULL, &req, &rep);
    SOAP_CHECK_ERROR(result, soap, "GetEventProperties");
//    dump_tev__GetEventPropertiesResponse(&rep);

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}

/************************************************************************
**函数：ONVIF_GetServiceCapabilities
**功能：获取服务功能
**参数：
        [in] EventXAddr - 事件服务地址
**返回：
        0表明成功，非0表明失败
************************************************************************/
int CameraConfig::ONVIF_GetServiceCapabilities(const char *EventXAddr)
{
    printf("[%s]get into ONVIF_GetServiceCapabilities() ... ...\n", camera_IP);
    int result = 0;
    struct soap *soap = NULL;
    struct _tev__GetServiceCapabilities         req;
    struct _tev__GetServiceCapabilitiesResponse rep;

    SOAP_ASSERT(NULL != EventXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    ONVIF_SetAuthInfo(soap, camera_userName, camera_password);
    result = soap_call___tev__GetServiceCapabilities(soap, EventXAddr, NULL, &req, &rep);
    SOAP_CHECK_ERROR(result, soap, "GetServiceCapabilities");
//    dump_tev__GetServiceCapabilitiesResponse(&rep);

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}

void CameraConfig::cb_discovery(char *DeviceXAddr)
{
//    struct tagCapabilities capa;
    if(DeviceXAddr == NULL)
    {}

//    ONVIF_GetCapabilities(DeviceXAddr, &capa);                                  // 获取设备能力信息（获取媒体服务地址）
    ONVIF_GetServiceCapabilities(camera_eventAddr);                              // 获取服务功能
    ONVIF_GetEventProperties(camera_eventAddr);                                  // 获取事件属性
    ONVIF_CreatePullPointSubscription(camera_eventAddr);                         // 使用Pull-Point方式订阅事件
}


void CameraConfig::startSetCameraTime()
{
    prctl(PR_SET_NAME, "setCameraTime");
    std::thread([&](CameraConfig *pthis){pthis->setCameraTime();}, this).detach();
}

void CameraConfig::setCameraTime()
{
    int ret;
    printf("%s : set camera time\n", camera_IP);

    ret = getCameraInfo();
    if(ret == 0)
    {
        mprintf("%s : time_task : getCameraInfo success\n", camera_IP);;
    }
    else
    {
        mprintf("%s : time_task : getCameraInfo failed, errno = %d\n", camera_IP, ret);
    }

    time_t timer;
    struct tm* t_tm;
    time(&timer);
    timer -= 8*60*60;
    t_tm = localtime(&timer);

    ret = setSystemDateAndTime(t_tm->tm_year+1900, t_tm->tm_mon+1, t_tm->tm_mday, t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec);

    if(ret == 0)
    {
        mprintf("%s : time_task setSystemDateAndTime success\n", camera_IP);
    }
    else
    {
        mprintf("%s : time_task setSystemDateAndTime failed, errno = %d\n", camera_IP, ret);
        int index = platform->getIndexOfCamera(camera_IP);
        platform->HostDevice.IPC[index].time_done = 0;
    }
}

void CameraConfig::setAlarm(bool flag, int type)
{

    if(flag)
        alarm_type |= (0x01<<type);
    else
        alarm_type &= ~(0x01<<type);

//    printf("%s : set alarm type %d, result is %02x\n", camera_IP, type, alarm_type);

}

void CameraConfig::startSetCameraAlarmOsd()
{
    prctl(PR_SET_NAME, "setCameraOsd");
    std::thread([&](CameraConfig *pthis){pthis->setCameraAlarmOsd();}, this).detach();
}

void CameraConfig::setCameraAlarmOsd()
{
    int ret;

    if(alarm_type_old == alarm_type)
        return;

    mprintf("%s : channel %02d alarm stat change from %02x to %02x\n", camera_IP, alarm_type_old, alarm_type);
    alarm_type_old = alarm_type;

    //delete osd
    ret = getCameraInfo();
    if(ret == 0)
    {
        mprintf("%s : alarm_task : getCameraInfo success\n", camera_IP);
    }
    else
    {
        mprintf("%s : alarm_task : getCameraInfo failed, errno = %d\n", camera_IP, ret);
        return;
    }

    ret = getOSDOption();
    if(ret == 0)
    {
        mprintf("%s : alarm_task : getOSDOption success\n", camera_IP);;
    }
    else
    {
        mprintf("%s : alarm_task : getOSDOption failed, errno = %d\n", camera_IP, ret);
        return;
    }

    ret = getOSDs();
    if(ret == 0)
    {
        mprintf("%s : alarm_task : getOSDs success\n", camera_IP);
    }
    else
    {
        mprintf("%s : alarm_task : getOSDs failed, errno = %d\n", camera_IP, ret);
        return;
    }



    if(alarm_type)
    {
        //create osd text
        char text[128] = {0};

        if(alarm_type & 0x01)
            sprintf(&text[strlen(text)], "EMERGENCYS");

        if(alarm_type & 0x02)
            sprintf(&text[strlen(text)], " PRE-FIRE");

        if(alarm_type & 0x04)
            sprintf(&text[strlen(text)], " FIRE");

        if(alarm_type & 0x08)
            sprintf(&text[strlen(text)], " UNLOCK");

        if(alarm_type & 0x10)
            sprintf(&text[strlen(text)], " CLOSE-BLOCK");

        if(alarm_type & 0x20)
            sprintf(&text[strlen(text)], " OPEN-BLOCK");

        mprintf("%s : create osd text : %s\n", camera_IP, text);


        //create osd
        setOSDString(text);
        ret = createOSD();
        if(ret == 0)
        {
            mprintf("%s : alarm_task : createOSD success\n", camera_IP);
            alarm_type_old = alarm_type;
        }
        else
        {
            mprintf("%s : alarm_task : createOSD failed, errno = %d\n", camera_IP, ret);
        }

        isAlarm = 1;
    }
    else
    {
        isAlarm = 0;
    }
}
