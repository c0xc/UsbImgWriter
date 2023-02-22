#include "linuxdownloader.hpp"

LinuxDownloader::LinuxDownloader(QWidget *parent)
               : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);

#ifdef LINUXDOWNLOADER_ENABLED

    QLabel *lbl_title = new QLabel(tr("Download Live Linux image"));
    lbl_title->setStyleSheet("font-weight:bold; color:gray; font-size:18pt;");
    vbox->addWidget(lbl_title);
    QFrame *hline1 = new QFrame;
    hline1->setFrameShape(QFrame::HLine);
    vbox->addWidget(hline1);

    m_selection_list = new QListWidget;
    m_selection_list->setStyleSheet("font-weight:bold; font-size:16pt;");
    m_selection_list->setCursor(Qt::PointingHandCursor);
    connect(m_selection_list, SIGNAL(itemActivated(QListWidgetItem*)),
        SLOT(handleSelection(QListWidgetItem*)));
    vbox->addWidget(m_selection_list);

    m_progress = new QProgressBar;
    vbox->addWidget(m_progress);

    QHBoxLayout *hbox_btn = new QHBoxLayout;
    QPushButton *btn_cancel = new QPushButton(tr("Cancel"));
    connect(btn_cancel, SIGNAL(clicked()), SLOT(close()));
    hbox_btn->addStretch();
    hbox_btn->addWidget(btn_cancel);
    vbox->addLayout(hbox_btn);

    setInitialSelection();

#else

    QMessageBox::critical(this, tr("Downloader not available"),
        tr("The Live Linux downloader is not available."));
    close();

#endif
}

#ifdef LINUXDOWNLOADER_ENABLED

void
LinuxDownloader::closeEvent(QCloseEvent *event)
{
    //Remove partially downloaded file
    if (m_file)
    {
        m_file->remove();
    }

    event->accept();
}

void
LinuxDownloader::setInitialSelection()
{
    m_selection_list->clear();

    QListWidgetItem *itm_debian = new QListWidgetItem("Debian Linux");
    itm_debian->setData(QListWidgetItem::UserType + 0, "DEBIAN");
    m_selection_list->addItem(itm_debian);

    QListWidgetItem *itm_fedora = new QListWidgetItem("Fedora Linux");
    itm_fedora->setData(QListWidgetItem::UserType + 0, "FEDORA");
    m_selection_list->addItem(itm_fedora);

}

void
LinuxDownloader::handleSelection(QListWidgetItem *item)
{
    QString act = item->data(QListWidgetItem::UserType + 0).toString();
    QString a_path = item->data(QListWidgetItem::UserType + 1).toString();

    if (act == "FILE")
    {
        askDownload(a_path);
        return;
    }

    m_selection_list->clear();

    //FEDORA
    //List of releases:
    //https://dl.fedoraproject.org/pub/fedora/linux/releases/
    //Release download location:
    //https://dl.fedoraproject.org/pub/fedora/linux/releases/37/Workstation/x86_64/iso/
    if (act == "FEDORA")
    {
        return listFedora(a_path);
    }

    //DEBIAN
    //https://cdimage.debian.org/debian-cd/current-live/amd64/iso-hybrid/SHA512SUMS
    //File contains checksums and file names, e.g.:
    //debian-live-11.6.0-amd64-mate.iso
    if (act == "DEBIAN")
    {
        //... NOT IMPLEMENTED ...
    }

}

void
LinuxDownloader::askDownload(const QString &path)
{
    QString filename = QFileInfo(path).fileName();
    if (QMessageBox::question(this, tr("Start download?"),
        tr("Do you want to download this file now?\n%1").arg(path)) != QMessageBox::Yes)
        return;
    QString file_path = QFileDialog::getSaveFileName(this, tr("Save file"),
        QDir::home().absoluteFilePath(filename),
        tr("ISO images (*.iso);;All files (*)"));
    if (file_path.isEmpty()) return;
    QFileInfo fi(file_path);
    //if (fi.exists())
    //{
    //    QMessageBox::critical(this, tr("File exists"),
    //        tr("This file already exists. Please enter a new file name."));
    //    return;
    //}

    m_file = new QFile(file_path, this);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::critical(this, tr("Cannot save file"),
            tr("Failed to open file for writing."));
        return;
    }

    m_selection_list->setDisabled(true);
    m_progress->setValue(0);

    QNetworkReply *reply = m_network.get(QNetworkRequest(path));
    connect(reply, &QNetworkReply::downloadProgress, this, [this, reply]
    (qint64 received, qint64 total)
    {
        qint64 written = m_file->property("written").toLongLong();
        m_file->setProperty("written", received);
        double p = std::div(written * 100, total).quot;
        m_progress->setValue(p);

        m_file->write(reply->read(received));
    }
    );
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        m_file->write(reply->readAll());
        m_file->close();

        QMessageBox::information(this, tr("Done"),
            tr("The selected image file has been downloaded: %1").arg(m_file->fileName()));
        emit fileDownloaded(m_file->fileName());

        QTimer::singleShot(0, this, SLOT(close()));

        m_file->deleteLater(); //not fast enough for close/this->delete
        m_file = 0; //clear fh immediately because we've scheduled close
        reply->deleteLater();
    }
    );
}

void
LinuxDownloader::addItem(const QString &distro, const QString &path, const QString &label)
{
    QListWidgetItem *itm = new QListWidgetItem(label);
    itm->setData(QListWidgetItem::UserType + 0, distro);
    itm->setData(QListWidgetItem::UserType + 1, path);
    m_selection_list->addItem(itm);
}

void
LinuxDownloader::loadListing(const QString &distro, const QString &path)
{
    QString base_addr = "https://dl.fedoraproject.org/pub/fedora/linux/releases/";
    QUrl url(base_addr);
    if (!path.isEmpty())
    {
        if (path.endsWith("/"))
        {
            url = QUrl(base_addr + path);
        }
    }

    QNetworkReply *reply = m_network.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, distro, base_addr, path]()
    {
        QUrl url = reply->url();
        QString raw = reply->readAll();
        qInfo() << "downloaded download section:" << path;
        QRegExp rx("href=\"([^\"]*)\"");

        addItem(distro, "", distro);
        int count = 0;

        QStringList dir_links;
        QStringList iso_links;
        int pos = 0;
        while ((pos = rx.indexIn(raw, pos)) != -1)
        {
            pos += rx.matchedLength();
            QString target = rx.cap(1);
            qInfo() << "found link:" << target;
            QString full_addr = url.toString() + target;
            if (target.endsWith("/"))
            {
                target.chop(1);
                dir_links << target;
            }
            else if (target.endsWith(".iso"))
                iso_links << target;
        }

        if (distro == "FEDORA")
        {
            if (path.isEmpty())
            {
                //Get list of releases
                dir_links.sort();
                for (int i = dir_links.size(); --i >= 0;)
                    if (dir_links[i].toInt() < 10) dir_links.removeAt(i);
                dir_links = dir_links.mid(dir_links.size() - 4);
                foreach (QString link, dir_links)
                {
                    QString sub_path = link + "/Spins/x86_64/iso/";
                    addItem(distro, sub_path, QString("Fedora %1").arg(link));
                    count++;
                }
            }
            else
            {
                //Show ISO links
                foreach (QString link, iso_links)
                {
                    QString file_path = base_addr + "/" + path + link;
                    addItem("FILE", file_path, link);
                    count++;
                }
            }
        }

        if (!count)
        {
            QMessageBox::warning(this, tr("No results"),
                tr("No downloads found in the selected location."));
        }

        reply->deleteLater();
    }
    );

}

void
LinuxDownloader::listFedora(const QString &path)
{
    loadListing("FEDORA", path);
}

#endif
