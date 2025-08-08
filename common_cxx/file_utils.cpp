#include "file_utils.h"

bool fileExists(const std::string& filePath){
    std::ifstream file(filePath);
    return file.good();
}

bool createDirectory(const std::string& directoryPath) {
    int result = mkdir(directoryPath.c_str(), 0777);

    if (result == 0) {
        return true;
    } else {
        std::cout << "create directory failed, errno " << errno << std::endl;
        return false;
    }
}

bool folderExists(const std::string& folderPath) {
    struct stat info;
    return stat(folderPath.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

bool createFolder(const std::string& folderPath) {
    return mkdir(folderPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

bool removeDirectory(const std::string& directoryPath) {
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == NULL) {
        std::cerr << "Failed to open directory, errno " << errno << std::endl;
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string filePath = directoryPath + "/" + entry->d_name;

        struct stat fileStat;
        if (lstat(filePath.c_str(), &fileStat) == -1) {
            std::cerr << "Failed to get file stat, errno %d" << errno << std::endl;
            closedir(dir);
            return false;
        }

        if (S_ISDIR(fileStat.st_mode)) {
            if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                // 递归删除子目录
                if (!removeDirectory(filePath)) {
                    std::cerr << "Failed to removeDirectory, errno " << errno << std::endl;
                    closedir(dir);
                    return false;
                }
            }
        } else {
            // 删除文件
            if (unlink(filePath.c_str()) == -1) {
                std::cerr << "Failed to remove file, errno " << errno << std::endl;
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    // 删除空目录
    if (rmdir(directoryPath.c_str()) == -1) {
        std::cerr << "Failed to remove directory" << std::endl;
        return false;
    }

    return true;
}

bool removeDirectoryFiles(const std::string& directoryPath) {
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory" << std::endl;
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filePath = directoryPath + "/" + entry->d_name;

        struct stat fileStat;
        if (lstat(filePath.c_str(), &fileStat) == -1) {
            std::cerr << "Failed to get file stat" << std::endl;
            closedir(dir);
            return false;
        }

        if (!S_ISDIR(fileStat.st_mode)) {
            // 删除文件
            if (unlink(filePath.c_str()) == -1) {
                std::cerr << "Failed to remove file" << std::endl;
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    return true;
}

bool copyFile(const std::string& file_src, const std::string& file_dst) {
    std::ifstream src(file_src, std::ios::binary);
    std::ofstream dst(file_dst, std::ios::binary);

    if (!src || !dst) {
        return false;
    }

    dst << src.rdbuf();

    return true;
}

bool copyFile_02(const std::string& file_src, const std::string& file_dst) {
    std::ifstream src(file_src, std::ios::binary);
    std::ofstream dst(file_dst, std::ios::binary);

    if (!src.is_open()) {
        std::cerr << "Failed to open" << file_src << std::endl;
        return false;
    }else{
        if (!dst.is_open()) {
            std::cerr << "Failed to open " << file_dst << std::endl;
            src.close();
            return false;
        }else{
            // 逐行将源文件内容写入目标文件
            std::string line;
            while (std::getline(src, line)) {
                dst << line << '\n';
            }
            // 关闭源文件
            src.close();
            dst.close();
            return true;
        }
    }
}

bool moveFile(const std::string& file_src, const std::string& file_dst) {
    if (std::rename(file_src.c_str(), file_dst.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

std::streampos getFileSize(const std::string& filePath) {
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        std::cerr << "Failed to get file information: " << filePath << std::endl;
        return -1;
    }
    return fileInfo.st_size;
}

std::vector<std::string> getFilesInDirectory(const std::string& directoryPath) {
    std::vector<std::string> files;

    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory" << std::endl;
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filePath = directoryPath + "/" + entry->d_name;

        struct stat fileStat;
        if (lstat(filePath.c_str(), &fileStat) == -1) {
            std::cerr << "Failed to get file stat" << std::endl;
            continue;
        }

        if (!S_ISDIR(fileStat.st_mode)) {
            files.push_back(entry->d_name);
            std::cout << "find file: " << entry->d_name << std::endl;
        }
    }

    closedir(dir);

    return files;
}

std::string getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        return filePath.substr(dotPos + 1);
    }
    return "";
}

std::string getFileName(const std::string& filePath) {
    // 查找最后一个斜杠（/）在路径中的位置
    size_t lastSlashPos = filePath.find_last_of('/');

    // 在路径中获取文件名（不包括斜杠）
    std::string fileName = filePath.substr(lastSlashPos + 1);

    // 查找最后一个点（.）在文件名中的位置
    size_t dotPos = fileName.find_last_of('.');

    // 如果存在扩展名，则返回文件名去除扩展名部分，否则返回整个文件名
    if (dotPos != std::string::npos) {
        return fileName.substr(0, dotPos);
    }
    return fileName;
}

std::string getParentDirectory(const std::string& filePath) {
    // 查找最后一个斜杠（/）在路径中的位置
    size_t lastSlashPos = filePath.find_last_of('/');

    // 在路径中获取父目录路径（不包括斜杠）
    std::string parentDir = filePath.substr(0, lastSlashPos);

    return parentDir;
}

std::string joinPaths(const std::string& path1, const std::string& path2) {
    // 如果第一个路径为空，则直接返回第二个路径
    if (path1.empty()) {
        return path2;
    }

    // 如果第二个路径为空，则直接返回第一个路径
    if (path2.empty()) {
        return path1;
    }

    // 确保第一个路径以斜杠（/）结尾
    std::string newPath1 = path1;
    if (newPath1.back() != '/') {
        newPath1 += '/';
    }

    // 确保第二个路径不以斜杠（/）开头
    std::string newPath2 = path2;
    if (newPath2.front() == '/') {
        newPath2.erase(newPath2.begin());
    }

    // 组合两个路径并返回
    return newPath1 + newPath2;
}

std::string getCurrentDirectory() {
    char buffer[PATH_MAX];

    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        return "";
    }
}

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);

    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    } else {
        return "";
    }
}

std::vector <std::string> readFile_02(const std::string& filePath) {
    std::ifstream file(filePath);
    std::vector<std::string> lines;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    } else {
        printf("open file %s failed\n", filePath.c_str());
    }
    return lines;
}

bool writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);

    if (file.is_open()) {
        file << content;
        file.close();

        return true;
    } else {
        return false;
    }
}

//usage: mergeFiles("out.txt", {"1.txt", "2.txt", "3.txt"});
void mergeFiles(const std::string& outputFileName, const std::initializer_list<std::string>& inputFiles) {
    // 打开目标文件以输出方式
    std::fstream outFile(outputFileName, std::ios::out);

    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        std::cerr << "Failed to open the output file for writing!" << std::endl;
        return;
    }

    // 遍历需要合并的文件列表
    for (const std::string& inputFile : inputFiles) {
        // 打开源文件以输入方式
        std::ifstream inFile(inputFile);

        // 检查文件是否成功打开
        if (!inFile.is_open()) {
            std::cerr << "Failed to open " << inputFile << " for reading." << std::endl;
            return;
        }

        // 逐行将源文件内容写入目标文件
        std::string line;
        while (std::getline(inFile, line)) {
            outFile << line << '\n';
        }

        // 关闭源文件
        inFile.close();
    }

    // 关闭目标文件
    outFile.close();

    std::cout << "Files merged successfully." << std::endl;
}

std::string trim(const std::string& str) {
    std::string trimmed = str;

    // 从字符串前面开始查找第一个非空白字符
    auto it = std::find_if(trimmed.begin(), trimmed.end(), [](char ch) {
        return !std::isspace(ch);
    });

    // 如果找到了非空白字符，则从该位置开始修剪字符串
    if (it != trimmed.end()) {
        trimmed.erase(trimmed.begin(), it);
    }

    // 如果字符串还不为空，则从字符串后面开始查找第一个非空白字符
    if (!trimmed.empty()) {
        it = std::find_if(trimmed.rbegin(), trimmed.rend(), [](char ch) {
            return !std::isspace(ch);
        }).base();

        // 如果找到了非空白字符，则从该位置开始修剪字符串
        if (it != trimmed.begin()) {
            trimmed.erase(it, trimmed.end());
        }
    }

    return trimmed;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int countDirectories(const std::string& path, const std::string& datePattern) {
    int count = 0;
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory: " << path << std::endl;
        return count;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        if (name != "." && name != "..") {
            std::string fullPath = path + "/" + name;
            struct stat fileInfo;
            if (stat(fullPath.c_str(), &fileInfo) == 0) {
                if (S_ISDIR(fileInfo.st_mode)) {
                    // 检查文件夹名称是否匹配指定日期格式
                    if (name.size() == datePattern.size() && name.find_first_not_of("0123456789") == std::string::npos) {
                        count++;
                    }

                    // 递归检索子目录
                    count += countDirectories(fullPath, datePattern);
                }
            }
        }
    }

    closedir(dir);
    return count;
}

int createSymbolicLink(const char* targetPath, const char* linkPath) {
    if (remove(linkPath) == 0){
        printf("remove file %s\n", linkPath);
    }
    else{
        printf("remove file %s failed, errno %d\n", linkPath, errno);
    }

    if (-1 == symlink(targetPath, linkPath)) {
        // 创建链接文件失败
        perror("symlink");
        return -1;
    }
    return 0;
}

bool hasExtension(const std::string& filename, const std::string& extension) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos && filename.substr(dotPos + 1) == extension) {
        return true;  // 文件具有指定的后缀名
    }
    return false;  // 文件没有指定的后缀名
}

void mprintf(const char *fmt, ...)
{
    char myprintf_buf[1024];
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(myprintf_buf, sizeof(myprintf_buf), fmt, args);
    va_end(args);

    if(myprintf_buf[strlen(myprintf_buf)-1] == '\n')
        myprintf_buf[strlen(myprintf_buf)-1] = '\0';

    printf("%s\n", myprintf_buf);
    //LOG4CPLUS_INFO(Logger::getRoot(), myprintf_buf);
}

void setPrintDevice()
{
    int fd;
    time_t cur_time;
    struct tm *local_time;
    cur_time = time(NULL);
    local_time = localtime(&cur_time);

    char logName[100]={0};
    sprintf(logName, "/userdata/log/%04d%02d%02d/printf.log", local_time->tm_year+1900, local_time->tm_mon+1, local_time->tm_mday);
    printf("log name is %s\n", logName);


    if((fd=open(logName, O_WRONLY|O_CREAT|O_APPEND)))
    {
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        printf("\n\n\n%04d-%02d-%02d %02d:%02d:%02d \nstart recording printf message ...\n", local_time->tm_year+1900, local_time->tm_mon+1, local_time->tm_mday, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    }
}


#define PRINT_DATA_SIZE 1024
void printArray(unsigned char* data, int size){
    char buf[PRINT_DATA_SIZE];
    sprintf(buf, "data size %d, content : ", size);
    int len = strlen(buf);
    for(int i=0; i<size; i++)
    {
        sprintf(&buf[len+i*3], " %02x", data[i]);
    }

    mprintf("%s\n", buf);
}

void setBit(char *c, int offset, bool flag)
{
    if(flag)
    {
        *c |= (0x01<<offset);
    }
    else
    {
        *c &= ~(0x01<<offset);
    }
}

bool getBit(char c, int offset)
{
    return c & (0x01<<offset);
}

