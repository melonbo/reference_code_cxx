#include <QtCore/QCoreApplication>
#include "camera.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Camera myCamera;
//    myCamera.init("172.16.41.1", "admin", "waycom12345");
    myCamera.init("172.16.41.250", "admin", "admin123");

    int ret = myCamera.getCameraInfo();

    myCamera.setMainReslution(1280, 720);
    myCamera.setSubReslution(640, 480);

    myCamera.setMainFrameRateLimit(15);
    myCamera.setSubFrameRateLimit(15);

    myCamera.setMainBitrateLimit(1024);
    myCamera.setSubBitrateLimit(1024);

    myCamera.setMainFrameGovLength(15);
    myCamera.setSubFrameGovLength(15);

//    myCamera.setVideoConfig(MAINSTREAM);
//    myCamera.setVideoConfig(SUBSTREAM);

/****************find all camera***************/
//    IP *ip_sum;
//    int num = 30;
//    camera_findAll(&ip_sum, &num);
/**********************************************/

//    myCamera.setSystemDateAndTime(2019, 06, 6, 00, 00, 00);


//    ret = myCamera.getOSDOption();
//    ret = myCamera.getOSDs();
//    myCamera.deleteOSD();
//    myCamera.createOSD();

//    myCamera.getSnapshot();
    printf("ret = %d\n", ret);
exit(0);
    return a.exec();
}
