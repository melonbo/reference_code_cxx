#ifndef PROXY_H
#define PROXY_H
#include "config/util.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "live/proxyServer/DynamicRTSPServer.hh"

void proxyServer();
bool startProxyServer();
int transportStreamIndexer(char *input);

#endif // PROXY_H
