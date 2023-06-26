#include <iostream>
#include <unistd.h>
#include "ffmux.h"
using namespace std;

int main()
{
    FFMPEG ffmux;
    ffmux.start();
    while(1){
        cout << "... ..." << endl;
        sleep(1);
    }
    return 0;
}
