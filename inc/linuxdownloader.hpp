#ifndef LINUXDOWNLOADER_HPP
#define LINUXDOWNLOADER_HPP

#ifdef QT_NETWORK_LIB
#define LINUXDOWNLOADER_ENABLED
#endif

#include <QDebug>
#include <QDialog>
#include <QDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPointer>
#include <QFileDialog>
#include <QTimer>
#include <QCloseEvent>
#ifdef LINUXDOWNLOADER_ENABLED
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#endif

class LinuxDownloader : public QDialog
{
    Q_OBJECT

signals:

    void
    fileDownloaded(const QString &file);

public:

    LinuxDownloader(QWidget *parent = 0);

#ifdef LINUXDOWNLOADER_ENABLED

private slots:

    void
    closeEvent(QCloseEvent *event);

    void
    setInitialSelection();

    void
    handleSelection(QListWidgetItem *item);

    void
    askDownload(const QString &path);

    void
    addItem(const QString &distro, const QString &path, const QString &label);

    void
    loadListing(const QString &distro, const QString &path);

    void
    listFedora(const QString &path);

private:

    QListWidget
    *m_selection_list;

    QProgressBar
    *m_progress;

    QPointer<QFile>
    m_file;

    QNetworkAccessManager
    m_network;

#endif

};

#endif
