#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cctype>
#include <algorithm>
#include <fcntl.h>
#include <cstdarg>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>


bool fileExists(const std::string& filePath);//文件是否存在
bool createDirectory(const std::string& directoryPath);//创建目录
bool removeDirectory(const std::string& directoryPath);//递归删除目录
bool removeDirectoryFiles(const std::string& directoryPath);//删除目录下文件
bool copyFile(const std::string& file_src, const std::string& file_dst);//复制文件
bool copyFile_02(const std::string& file_src, const std::string& file_dst);//复制文件
bool moveFile(const std::string& file_src, const std::string& file_dst);//移动文件
std::streampos getFileSize(const std::string& filePath);//获取文件大小
std::vector<std::string> getFilesInDirectory(const std::string& directoryPath);//获取目录下所有文件
std::string getFileExtension(const std::string& filePath);//获取文件扩展名
std::string getFileName(const std::string& filePath);//获取文件名,不包括路径和扩展名
std::string getBaseName(const std::string& filePath);//获取文件名,包括扩展名
std::string getParentDirectory(const std::string& filePath);//获取父目录路径（不包括斜杠）
std::string joinPaths(const std::string& path1, const std::string& path2);//将两个路径组合成一个新的路径
std::string getCurrentDirectory();//获取当前工作目录
std::string readFile(const std::string& filePath);//读取文件的内容
std::vector <std::string> readFile_02(const std::string& filePath);//读取文件的内容
bool writeFile(const std::string& filePath, const std::string& content);//写入内容到文件
void mergeFiles(const std::string& outputFileName, const std::initializer_list<std::string>& inputFiles);//合并文件
std::string trim(const std::string& str);//将给定字符串前后的空格去除
std::vector<std::string> split(const std::string& str, char delimiter);//将给定的字符串按照指定的分隔符进行分割
int countDirectories(const std::string& path, const std::string& datePattern);//检索目录path下符合yyyymmdd日期格式的文件夹的数量
int createSymbolicLink(const char* targetPath, const char* linkPath);//创建链接文件
bool hasExtension(const std::string& filename, const std::string& extension);//判断文件后缀
uintmax_t getFileSize(const std::string& filePath);//获取文件大小
uintmax_t getFolderSize(const std::string& folderPath);//获取文件夹大小


void mprintf(const char* fmt, ...);//重新封装printf，例如记录日志
void setPrintDevice();//printf输出重定向
void printArray(unsigned char* data, int size);//打印数组数据
void setBit(char *c, int offset, bool flag);//设置输入字节某一位的值
bool getBit(char c, int offset);//获取输入字节某一位的值

#endif // FILE_UTILS_H
