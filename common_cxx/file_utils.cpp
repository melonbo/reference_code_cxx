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
