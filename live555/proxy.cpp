#include "proxy.h"
#include <pthread.h>
#include "config/util.h"
#include "config/platform.h"

char const* progName;
UsageEnvironment* env;
UserAuthenticationDatabase* authDB = NULL;
UserAuthenticationDatabase* authDBForREGISTER = NULL;

// Default values of command-line parameters:
int verbosityLevel = 0;
Boolean streamRTPOverTCP = False;
portNumBits tunnelOverHTTPPortNum = 0;
portNumBits rtspServerPortNum = 554;
char* username = NULL;
char* password = NULL;
Boolean proxyREGISTERRequests = False;
char* usernameForREGISTER = NULL;
char* passwordForREGISTER = NULL;

static RTSPServer* createRTSPServer(Port port) {
  if (proxyREGISTERRequests) {
    return RTSPServerWithREGISTERProxying::createNew(*env, port, authDB, authDBForREGISTER, 65, streamRTPOverTCP, verbosityLevel, username, password);
  } else {
    return RTSPServer::createNew(*env, port, authDB);
  }
}

void* proxyServer(void *) {
  // Increase the maximum size of video frames that we can 'proxy' without truncation.
  // (Such frames are unreasonably large; the back-end servers should really not be sending frames this large!)
  OutPacketBuffer::maxSize = 100000; // bytes

  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

//  if (streamRTPOverTCP) {
//    if (tunnelOverHTTPPortNum > 0) {
//      *env << "The -t and -T options cannot both be used!\n";
//    } else {
//      tunnelOverHTTPPortNum = (portNumBits)(~0); // hack to tell "ProxyServerMediaSession" to stream over TCP, but not using HTTP
//    }
//  }

#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
      // Repeat this line with each <username>, <password> that you wish to allow access to the server.
#endif

  // Create the RTSP server. Try first with the configured port number,
  // and then with the default port number (554) if different,
  // and then with the alternative port number (8554):
  RTSPServer* rtspServer;
  rtspServer = createRTSPServer(rtspServerPortNum);

  platform->HostDevice.rtspServer = rtspServer;
  if (rtspServer == NULL) {
    if (rtspServerPortNum != 554) {
      *env << "Unable to create a RTSP server with port number " << rtspServerPortNum << ": " << env->getResultMsg() << "\n";
      *env << "Trying instead with the standard port numbers (554 and 8554)...\n";

      rtspServerPortNum = 554;
      rtspServer = createRTSPServer(rtspServerPortNum);
    }
  }

  if (rtspServer == NULL) {
    rtspServerPortNum = 8554;
    rtspServer = createRTSPServer(rtspServerPortNum);
  }
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  // Create a proxy for each "rtsp://" URL specified on the command line:
  for(int i=0;i<platform->HostDevice.ipc_num;i++) {
    char const* proxiedStreamURL = platform->HostDevice.IPC[i].uri;
    char streamName[30];
    sprintf(streamName, "channel-%d", i); // there's more than one stream; distinguish them by name

    ServerMediaSession* sms
      = ProxyServerMediaSession::createNew(*env, rtspServer,
                                           proxiedStreamURL, streamName,
                                           username, password, tunnelOverHTTPPortNum, verbosityLevel);
    rtspServer->addServerMediaSession(sms);

    char* proxyStreamURL = rtspServer->rtspURL(sms);

    *env << "RTSP stream, proxying the stream \"" << proxiedStreamURL << "\"\n";
    *env << "\tPlay this stream using the URL: " << proxyStreamURL << "\n";

    delete[] proxyStreamURL;
  }

  if (proxyREGISTERRequests) {
    *env << "(We handle incoming \"REGISTER\" requests on port " << rtspServerPortNum << ")\n";
  }

  // Create the RTSP server.  Try first with the default port number (554),
  // and then with the alternative port number (8554):
  RTSPServer* rtspServer_2;
  rtspServer_2 = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
  if (rtspServer_2 == NULL) {
    rtspServerPortNum = 8554;
    rtspServer_2 = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
  }
  if (rtspServer_2 == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  // Now, enter the event loop:
  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}


bool startProxyServer()
{
    pthread_t pid;
    if(-1 == pthread_create(&pid, NULL, proxyServer, NULL))
    {
        printf("create proxy thread failed, err=%d\n", errno);
        return false;
    }
    else
    {
        printf("create proxy thread success\n");
        return true;
    }
}

void afterPlaying(void* /*clientData*/) {
    *env << "...done\n";
    exit(0);
}

int transportStreamIndexer(char *inputFileName)
{
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

//    inputFileName = "test.ts";
    // Check whether the input file name ends with ".ts":
    int len = strlen(inputFileName);
    if (len < 4 || strcmp(&inputFileName[len-3], ".ts") != 0) {
     *env << "ERROR: input file name \"" << inputFileName
      << "\" does not end with \".ts\"\n";
    }

    // Open the input file (as a 'byte stream file source'):
    FramedSource* input
     = ByteStreamFileSource::createNew(*env, inputFileName, TRANSPORT_PACKET_SIZE);
    if (input == NULL) {
     *env << "Failed to open input file \"" << inputFileName << "\" (does it exist?)\n";
     return -1;
    }

    // Create a filter that indexes the input Transport Stream data:
    FramedSource* indexer
     = MPEG2IFrameIndexFromTransportStream::createNew(*env, input);

    // The output file name is the same as the input file name, except with suffix ".tsx":
    char* outputFileName = new char[len+2]; // allow for trailing x\0
    sprintf(outputFileName, "%sx", inputFileName);

    // Open the output file (for writing), as a 'file sink':
    MediaSink* output = FileSink::createNew(*env, outputFileName);
    if (output == NULL) {
     *env << "Failed to open output file \"" << outputFileName << "\"\n";
     return -1;
    }

    // Start playing, to generate the output index file:
    *env << "Writing index file \"" << outputFileName << "\"...";
    output->startPlaying(*indexer, afterPlaying, NULL);

    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}
