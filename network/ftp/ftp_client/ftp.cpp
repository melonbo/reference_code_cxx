#include "ftp.h"

QFTP::QFTP(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}
QFTP::~QFTP()
{

}

void QFTP::downloadFile(const QString &url, const QString &filePath)
{
    QUrl uri(url);
    QNetworkRequest request(uri);
    QNetworkReply *reply = m_manager->get(request);

    connect(reply, &QNetworkReply::finished, [=]() {
        onDownloadFinished(reply);
    });

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(reply->readAll());
        file.close();
    }

}

void QFTP::uploadFile(const QString &url, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "file open error: " << file.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    const QByteArray array = file.readAll();
    QNetworkReply *reply = m_manager->put(request, array);

    connect(reply, &QNetworkReply::finished, [=]() {
        onUploadFinished(reply);
    });

    file.close();
}

void QFTP::onDownloadFinished(QNetworkReply *reply)
{
    emit downloadFinished(reply->url().fileName());
}

void QFTP::onUploadFinished(QNetworkReply *reply)
{
    emit uploadFinished();
}

