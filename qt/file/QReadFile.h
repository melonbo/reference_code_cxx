#ifndef QREADFILE_H
#define QREADFILE_H

#include <QObject>
#include <QStringList>

class QReadFile : public QObject
{
    Q_OBJECT
public:
    explicit QReadFile(QObject *parent = 0);

public:
    //QReadFile(QString);
    int CheckFileExit();
    int ReadFileInfo();//读取文件内
    QString GetFileString(QString Section, QString Item, QString Value);//读取字符串型数据
    int GetFileInt(QString Section, QString Item, int Value);//读取整型数据
    void SetFileName(QString FileName);//设置配置文件内容
private:
    bool m_is_file_exit;
    bool m_file_open;
    QStringList FileContainer; //存储文件内容的数组
    QString m_file_name;       //文件名称



    int m_read_value;

signals:

public slots:
};

#endif // QREADFILE_H
