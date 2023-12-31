/***************************************************************************
                          ftplib.h  -  description
                             -------------------
    begin                : Son Jul 27 2003
    copyright            : (C) 2003 by mkulke
    email                : sikor_sxe@radicalapproach.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* original unmodified copyright notes from Thomas Pfau */

/***************************************************************************/
/*									   */
/* ftplib.c - callable ftp access routines				   */
/* Copyright (C) 1996, 1997, 1998 Thomas Pfau, pfau@cnj.digex.net	   */
/*	73 Catherine Street, South Bound Brook, NJ, 08880		   */
/*									   */
/* This library is free software; you can redistribute it and/or	   */
/* modify it under the terms of the GNU Library General Public		   */
/* License as published by the Free Software Foundation; either		   */
/* version 2 of the License, or (at your option) any later version.	   */
/* 									   */
/* This library is distributed in the hope that it will be useful,	   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   */
/* Library General Public License for more details.			   */
/* 									   */
/* You should have received a copy of the GNU Library General Public	   */
/* License along with this progam; if not, write to the			   */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,		   */
/* Boston, MA 02111-1307, USA.						   */
/* 									   */
/***************************************************************************/
 
#ifndef FTPLIB_H
#define FTPLIB_H
#define NOSSL
#if defined(_WIN32)

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

#endif

#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#ifdef NOLFS
#define off64_t long
#endif

#ifndef NOSSL
#include <openssl/ssl.h>
#endif

using namespace std;

/**
  *@author mkulke
  */

typedef int (*FtpCallbackXfer)(off64_t xfered, void *arg);
typedef int (*FtpCallbackIdle)(void *arg);
typedef void (*FtpCallbackLog)(char *str, void* arg, bool out);

#ifndef NOSSL
typedef bool (*FtpCallbackCert)(void *arg, X509 *cert);
#endif

struct ftphandle {
	char *cput,*cget;
	int handle;
	int cavail,cleft;
	char *buf;
	int dir;
	ftphandle *ctrl;
	int cmode;
	struct timeval idletime;
	FtpCallbackXfer xfercb;
	FtpCallbackIdle idlecb;
	FtpCallbackLog logcb;
	void *cbarg;
	off64_t xfered;
	off64_t cbbytes;
	off64_t xfered1;
	char response[256];
#ifndef NOSSL
	SSL* ssl;
	SSL_CTX* ctx;
	BIO* sbio;
	int tlsctrl;
	int tlsdata;
	FtpCallbackCert certcb;
#endif
	off64_t offset;
	bool correctpasv;
};

#if defined(_WIN32)  
class DLLIMPORT ftplib {
#else
class ftplib {
#endif
public:

	enum accesstype
	{
		dir = 1,
		dirverbose,
		fileread,
		filewrite,
		filereadappend,
		filewriteappend
	}; 

	enum transfermode
	{
		ascii = 'A',
		image = 'I'
	};

	enum connmode
	{
		pasv = 1,
		port
	};

	enum fxpmethod
	{
		defaultfxp = 0,
        alternativefxp
	};

    enum dataencryption
    {
        unencrypted = 0,
        secure
    };

	ftplib();
	~ftplib();
    char* LastResponse();		   //找回上回响应
    int Connect(const char *host); //连接到远程服务器
    int Login(const char *user, const char *pass);//登陆到远程机器
    int Site(const char *cmd);		//发送网站命令
    int Raw(const char *cmd);
    int SysType(char *buf, int max);//确定远程系统的类型
    int Mkdir(const char *path); //创建目录
    int Chdir(const char *path); //改变工作目录
    int Cdup();					 //改变父目录
    int Rmdir(const char *path); //删除目录
    int Pwd(char *path, int max);//查看当前工作目录
    int Nlst(const char *outputfile, const char *path); //列出远程目录下的文件
    int Dir(const char *outputfile, const char *path); //列出远程目录
    int Size(const char *path, int *size, transfermode mode);//确定远程文件大小
    int ModDate(const char *path, char *dt, int max);//确定文件的修改时间
    int Get(const char *outputfile, const char *path, transfermode mode, off64_t offset = 0); //获取远程文件
    int Put(const char *inputfile, const char *path, transfermode mode, off64_t offset = 0);  //发送本地文件到远程
    int Rename(const char *src, const char *dst); //重命名远程文件
    int Delete(const char *path);	//删除远程文件
#ifndef NOSSL    
	int SetDataEncryption(dataencryption enc);
    int NegotiateEncryption();
	void SetCallbackCertFunction(FtpCallbackCert pointer);
#endif
    int Quit();
    void SetCallbackIdleFunction(FtpCallbackIdle pointer);
    void SetCallbackLogFunction(FtpCallbackLog pointer);
	void SetCallbackXferFunction(FtpCallbackXfer pointer);
	void SetCallbackArg(void *arg);
    void SetCallbackBytes(off64_t bytes);
	void SetCorrectPasv(bool b) { mp_ftphandle->correctpasv = b; };
    void SetCallbackIdletime(int time);
    void SetConnmode(connmode mode);
    static int Fxp(ftplib* src, ftplib* dst, const char *pathSrc, const char *pathDst, transfermode mode, fxpmethod method);
    
	ftphandle* RawOpen(const char *path, accesstype type, transfermode mode);
	int RawClose(ftphandle* handle); 
	int RawWrite(void* buf, int len, ftphandle* handle);
	int RawRead(void* buf, int max, ftphandle* handle); 


private:
    ftphandle* mp_ftphandle;

    int FtpXfer(const char *localfile, const char *path, ftphandle *nControl, accesstype type, transfermode mode);
    int FtpOpenPasv(ftphandle *nControl, ftphandle **nData, transfermode mode, int dir, char *cmd);
    int FtpSendCmd(const char *cmd, char expresp, ftphandle *nControl);
    int FtpAcceptConnection(ftphandle *nData, ftphandle *nControl);
    int FtpOpenPort(ftphandle *nControl, ftphandle **nData, transfermode mode, int dir, char *cmd);
    int FtpRead(void *buf, int max, ftphandle *nData);
    int FtpWrite(void *buf, int len, ftphandle *nData);
    int FtpAccess(const char *path, accesstype type, transfermode mode, ftphandle *nControl, ftphandle **nData);
    int FtpClose(ftphandle *nData);
	
	int socket_wait(ftphandle *ctl);
    int readline(char *buf,int max,ftphandle *ctl);
    int writeline(char *buf, int len, ftphandle *nData);
    int readresp(char c, ftphandle *nControl);
	
	void ClearHandle();
	int CorrectPasvResponse(unsigned char *v);
};

#endif
