#include "QReadFile.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>


QReadFile::QReadFile(QObject *parent) : QObject(parent)
{

}

//检查配置文件是否存在

int QReadFile::CheckFileExit()
{
    QFile config_file;
    int res;
    res = config_file.exists(m_file_name);
    if(res)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

//读取文件流内容
int QReadFile::ReadFileInfo()
{
    //add 2019-9-12

    FileContainer.clear();
    //
    QFile file(m_file_name);
    m_file_open=file.open(QIODevice::ReadOnly);
    if(m_file_open)
    {
        QTextStream stream(&file);
        QString line;
        while(!stream.atEnd())
        {
            line = stream.readLine();
           // qDebug()<<"line = "<<line<<endl;
            FileContainer.append(line);
        }
        file.close();
        return 1;
    }
    else
    {
        file.close();
        return 0;
    }
    return 0;

}

//查找文件内统，返回字符串类型数据
QString QReadFile::GetFileString(QString Section, QString Item, QString Value)
{

    if(FileContainer.size() <= 0)
    {
        return Value;//文件打开出错或文件为空，返回默认值
    }

    int i = 0;
    int iFileLines = FileContainer.size();
    //qDebug()<<"iFileLines = "<<iFileLines<<endl;
    QString strline,str;

    while(i<iFileLines)
    {
        strline = FileContainer.at(i++);
        //qDebug()<<"111---strline ==="<<strline<<endl;
        strline.simplified();
        if(strline.length()<1)
        {
            QMessageBox::warning(NULL,"Warning","配置文件字符串长度小1",QMessageBox::Yes);
            return NULL;
        }

        if(strline.at(0)=='[')//查找Section，第一个必须为[
        {
            str=strline.left(strline.indexOf("]"));//去掉]右边
            str=str.right(str.length()-str.indexOf("[")-1);//去掉[左边
            str.simplified();

            if(Section == str)//找到Section
            {
                while(i<iFileLines)
                {
                    strline = FileContainer.at(i++);
                    strline.simplified();

                    if(strline.at(0)=='[')
                        return Value;//如果到达下一个[]，即找不到,返回默认值

                    str = strline.left(strline.indexOf("="));//去掉=右边

                    str.simplified();

                    if(Item == str)//找到Item
                    {

                         str=strline.right(strline.length()-strline.indexOf("=")-1);//去掉=左边
                         str.simplified();
                         return str;
                    }
                 }
                         return Value;//找不到,返回默认值
            }

        }

    }

    return Value;//找不到,返回默认值
}

int QReadFile::GetFileInt(QString Section, QString Item, int Value)
{
    QString strtemp;
    strtemp.number(Value);
    return GetFileString(Section, Item, strtemp).toInt();
}

void QReadFile::SetFileName(QString FileName)
{
    m_file_name = FileName;
}
