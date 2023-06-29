#ifndef FTP_H
#define FTP_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>

class QFTP : public QObject
{
    Q_OBJECT
public:
    explicit QFTP(QObject *parent = nullptr);
    ~QFTP();

    void downloadFile(const QString &url, const QString &filePath);
    void uploadFile(const QString &url, const QString &filePath);

signals:
    void downloadFinished(const QString &filePath);
    void uploadFinished();

private slots:
    void onDownloadFinished(QNetworkReply *reply);
    void onUploadFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};

#endif // FTP_H
