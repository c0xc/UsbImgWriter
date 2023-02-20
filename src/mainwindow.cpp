#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent)
          : QMainWindow(parent),
            _file_selected(false),
            m_file_size(0),
            m_drive_size(0)
{

    QWidget *wid_main = new QWidget;
    setCentralWidget(wid_main);
    setWindowTitle(tr("USB IMG writer"));

    //TODO planned feature ideas:
    //- check format of input file
    //- allow gzip compressed files to be loaded
    //- add complex search&download dialog for Live Linux systems...
    //  or for a custom image?
    //- ask to create backup if device clicked before file selected
    //  we'd use our ExtFileReaderWriter in read mode

    //Top layout
    QVBoxLayout *vbox = new QVBoxLayout;
    wid_main->setLayout(vbox);
    QLabel *lbl_title = new QLabel(tr("USB IMG writer"));
    lbl_title->setStyleSheet("font-weight:bold; color:gray; font-size:18pt;");
    vbox->addWidget(lbl_title);
    QFrame *hline1 = new QFrame;
    hline1->setFrameShape(QFrame::HLine);
    vbox->addWidget(hline1);
    //Optional text frame with warning or something
    frm_warn_top = new QLabel;
    frm_warn_top->setStyleSheet("border:3px outset orange; background-color:orange;");
    frm_warn_top->hide();
    vbox->addWidget(frm_warn_top);

    //Image selection frame
    QHBoxLayout *hbox = new QHBoxLayout;
    //---
    QGroupBox *frm_image = new QGroupBox(tr("Image file"));
    QVBoxLayout *vbox_image = new QVBoxLayout;
    frm_image->setLayout(vbox_image);
    //Title/status/button, separator
    lbl_img_title = new QLabel(tr("Select image file"));
    QPushButton *btn_img_browse = new QPushButton(tr("Select..."));
    m_all_buttons << btn_img_browse;
    connect(btn_img_browse, SIGNAL(clicked()), SLOT(selectFile()));
    QMenu *file_menu = new QMenu;
    QAction *act1 = file_menu->addAction(tr("Open local file"));
    connect(act1, SIGNAL(triggered()), SLOT(selectFile()));
    QAction *act2 = file_menu->addAction(tr("Download Live Linux"));
    connect(act2, SIGNAL(triggered()), SLOT(showLinuxDownloader()));
    btn_img_browse->setMenu(file_menu);
    QHBoxLayout *hbox_img_top = new QHBoxLayout;
    hbox_img_top->addWidget(lbl_img_title);
    hbox_img_top->addWidget(btn_img_browse);
    vbox_image->addLayout(hbox_img_top);
    QFrame *hline2 = new QFrame;
    hline2->setFrameShape(QFrame::HLine);
    vbox_image->addWidget(hline2);
    //Form layout, fields with file info
    QGridLayout *grid_image = new QGridLayout;
    grid_image->addWidget(new QLabel(tr("File")), 0, 0);
    txt_img_file = new QLineEdit;
    txt_img_file->setReadOnly(true);
    grid_image->addWidget(txt_img_file, 0, 1);
    grid_image->addWidget(new QLabel(tr("Size")), 1, 0);
    txt_img_size = new QLineEdit;
    txt_img_size->setReadOnly(true);
    grid_image->addWidget(txt_img_size, 1, 1);
    grid_image->addWidget(new QLabel(tr("Modified")), 2, 0); //last modified / file date
    txt_img_mtime = new QLineEdit;
    txt_img_mtime->setReadOnly(true);
    grid_image->addWidget(txt_img_mtime, 2, 1);
    vbox_image->addLayout(grid_image);
    vbox_image->addStretch(); //use space at bottom, don't center vertically
    //Add drive frame to window's hbox
    hbox->addWidget(frm_image);
    hbox->setStretch(0, 50);

    //USB selection frame
    //---
    QGroupBox *frm_drive = new QGroupBox(tr("USB device"));
    QVBoxLayout *vbox_drive = new QVBoxLayout;
    frm_drive->setLayout(vbox_drive);
    //Title/status/button, separator
    lbl_usb_title = new QLabel(tr("Select USB drive"));
    QPushButton *btn_usb_refresh = new QPushButton(tr("Reload"));
    m_all_buttons << btn_usb_refresh;
    connect(btn_usb_refresh, SIGNAL(clicked()), SLOT(refreshDrives()));
    QHBoxLayout *hbox_usb_top = new QHBoxLayout;
    hbox_usb_top->addWidget(lbl_usb_title);
    hbox_usb_top->addWidget(btn_usb_refresh);
    vbox_drive->addLayout(hbox_usb_top);
    QFrame *hline3 = new QFrame;
    hline3->setFrameShape(QFrame::HLine);
    vbox_drive->addWidget(hline3);
    //Selection area with buttons for available devices
    wid_usb_buttons = new QWidget;
    vbox_drive->addWidget(wid_usb_buttons);
    QPushButton *btn_usb_start = new QPushButton(tr("Start"));
    m_all_buttons << btn_usb_start;
    connect(btn_usb_start, SIGNAL(clicked()), SLOT(askStart()));
    vbox_drive->addWidget(btn_usb_start);
    vbox_drive->addStretch(); //use space at bottom, don't center vertically
    //Add drive frame to window's hbox
    hbox->addWidget(frm_drive);
    hbox->setStretch(1, 50);

    //H layout -> top layout
    vbox->addLayout(hbox);

    //Load device list on startup
    QTimer::singleShot(0, this, SLOT(refreshDrives()));

    //Tux area
    vbox->addStretch();
    QPixmap pixmap(":/tux2.png");
    pixmap = pixmap.scaledToWidth(100);
    m_pix_tux = pixmap;
    lbl_tux = new QLabel;
    lbl_tux->setFixedSize(pixmap.size());
    lbl_tux->setPixmap(pixmap);
    vbox->addWidget(lbl_tux);

    //Misc

}

void
MainWindow::selectFile(const QString &file_path)
{
    //Check if file is readable/valid
    QFileInfo fi(file_path);
    if (!fi.isReadable())
    {
        QMessageBox::critical(this, tr("Cannot open file"),
            tr("This file cannot be opened (it's not readable): %1").arg(file_path));
        return;
    }

    //Sanity checks
    //---
    //Is the selected file empty?
    if (fi.size() <= 0)
    {
        QMessageBox::warning(this, tr("Invalid file selected"),
            tr("This file is empty: %1").arg(file_path));
    }

    //Fill file info fields
    txt_img_file->setText(fi.fileName());
    txt_img_file->setToolTip(file_path);
    Size file_size = fi.size();
    txt_img_size->setText(QString("%1 / %2 B").
        arg(file_size.formatted()).
        arg(file_size.bytes()));
    QDateTime file_date = fi.lastModified();
    //TODO QLocale::system().toString()
    QString file_date_str = file_date.toString("dd.MM.yyyy hh:mm:ss");
    txt_img_mtime->setText(file_date_str);

    //Check file type?
    //QMimeType("application/octet-stream")

    //Remember file selected, selection valid
    m_fi = fi;
    m_file_size = fi.size();
    _file_selected = true;

}

void
MainWindow::selectFile()
{
    _file_selected = false;

    //Show file dialog (user may select any file)
    QString file_path = QFileDialog::getOpenFileName(
        this, tr("Open File"),
        QDir::homePath(),
        tr("Image files (*.iso *.img);;All files (*)"));
    if (file_path.isEmpty()) return;
    selectFile(file_path);
}

void
MainWindow::showLinuxDownloader()
{
    LinuxDownloader *downloader = new LinuxDownloader(this);
    connect(downloader, SIGNAL(fileDownloaded(const QString&)),
        SLOT(selectFile(const QString&)));
    downloader->setModal(true);
    downloader->show();
}

void
MainWindow::refreshDrives()
{
    //Clear and reset layout
    m_sel_drive.clear();
    foreach (QObject *obj, wid_usb_buttons->children())
        obj->deleteLater();
    if (wid_usb_buttons->layout()) delete wid_usb_buttons->layout();
    QVBoxLayout *vbox = new QVBoxLayout;
    wid_usb_buttons->setLayout(vbox);

    //Query devices (provided by UDiskManager), filter/list block devices
    //without partitions (i.e., without nested block devices)
    QStringList found_devs;
    m_dev_button_refs.clear();
    foreach (QString dev, m_diskmanager.blockDevices())
    {
        //dev is a block device which may or may not have a parent device
        QString parent_dev = m_diskmanager.underlyingBlockDevice(dev); //TODO provide hasUnderlyingBlockDevice
        if (!parent_dev.isEmpty() && parent_dev != dev) continue; //skip partitions
        //filter usb devices
        if (!m_diskmanager.isUsbDevice(dev)) continue;

        //Collect device info
        QVariantMap data = m_diskmanager.deviceData(dev);
        QStringList drive_parts = data.value("Drive").toString().split("/");
        QString drive_name;
        if (!drive_parts.isEmpty()) drive_name = drive_parts.last();
        Size capacity = data.value("Size").toLongLong();
        QString dev_path = data.value("Device").toString();
        //Skip if device has 0 capacity (e.g., card reader ports on USB hub)
        if (capacity.bytes() == 0) continue;

        //Add button
        QString dev_label = dev + " [" + capacity.formatted() + "]";
        if (!m_diskmanager.idLabel(dev).isEmpty()) //never true because top level devices don't have labels
            dev_label += " " + m_diskmanager.idLabel(dev);
        ItemButton *btn = new ItemButton(dev_label);
        btn->setSubtitle(drive_name);
        btn->setToolTip(QString("Device: %1 [capacity: %2 B]").arg(dev_path).arg(capacity.bytes()));
        btn->setSignalName(dev_path);
        connect(btn, SIGNAL(clicked(const QString&)),
            SLOT(selectDevice(const QString&)));

        found_devs << dev;
        m_dev_button_refs.append(btn);
        vbox->addWidget(btn);
    }

    //Add note if no devices found
    if (found_devs.isEmpty())
    {
        QLabel *lbl = new QLabel(tr("(no USB devices found)"));
        vbox->addWidget(lbl);
    }
}

void
MainWindow::selectDevice(const QString &dev)
{
    QString dev_path = dev;
    QString dev_name = dev.split('/').last(); //TODO fix UDiskManager to get an object with those properties

    //Hide all other devices, except selected one
    bool found_selection = false;
    ItemButton *btn = 0;
    foreach (QPointer<QWidget> w, m_dev_button_refs)
    {
        ItemButton *other_btn = qobject_cast<ItemButton*>(w);
        if (!other_btn) continue; //this would be a bug
        QString other_dev = other_btn->signalName();
        if (other_dev == dev)
        {
            found_selection = true;
            btn = other_btn;
        }
        else
        {
            other_btn->hide();
        }
    }
    if (!found_selection)
    {
        QMessageBox::critical(this, tr("Cannot select device"),
            tr("Cannot select device, not in our list: %1").arg(dev));
        return;
    }

    //Check if drive is writable (as root)
    //TODO unmount ... test /dev/sdX permissions ...
    if (!QFileInfo(m_sel_drive).isWritable())
    {
        frm_warn_top->setText(tr("Unable to access the device without root privileges. An attempt will be made to gain root privileges."));
        frm_warn_top->show();
        QTimer::singleShot(60000, frm_warn_top, SLOT(hide()));
    }

    //Check if selected device is mounted or contains mounted filesystems
    //This is important because many desktops auto-mount everything
    QStringList dev_with_partitions = QStringList() << dev_name;
    //TODO ugly hack: find sub devices by name TODO add this to UDiskManager!
    foreach (QString other_dev, m_diskmanager.blockDevices())
    {
        QString parent_dev = m_diskmanager.underlyingBlockDevice(other_dev);
        parent_dev = parent_dev.split("/").last();
        if (parent_dev != dev_name) continue; //only child devices

        dev_with_partitions << other_dev;
    }
    foreach (QString dev, dev_with_partitions)
    {
        //Check if (selected device and/or its partition devices) mounted
        if (m_diskmanager.mountpoints(dev).isEmpty())
            continue;
        //Ask before unmounting
        if (QMessageBox::question(this, tr("Device is mounted"),
            tr("This device is currently mounted: %1. Unmount it now?").arg(dev)) != QMessageBox::Yes)
        {
            frm_warn_top->setText(tr("Aborted: Device is mounted."));
            frm_warn_top->show();
            QTimer::singleShot(60000, frm_warn_top, SLOT(hide()));
            return;
        }
        //Now unmount it
        if (!m_diskmanager.umount(dev))
        {
            QMessageBox::critical(this, tr("Device is mounted"),
                tr("Failed to unmount device %1.").arg(dev));
            return;
        }
    }

    //Remember capacity of selected drive
    QVariantMap dev_data = m_diskmanager.deviceData(dev_name);
    Size capacity = dev_data.value("Size").toLongLong();

    //Disable, highlight selected button
    btn->setBgColor("steelblue");
    btn->setFgColor("yellow");
    btn->updateColor();
    btn->setDisabled(true);
    //Remember drive selected, selection valid
    m_sel_drive = dev_path; //note dev vs. dev_path
    m_drive_size = capacity.bytes();

    askStart(true);
}

void
MainWindow::start()
{
    //Cannot start without input + output selected
    if (!_file_selected) return; //input file selected
    if (m_sel_drive.isEmpty()) return; //output drive selected
    if (!m_sel_drive.contains('/')) return;

    //Open input file
    QFile *file = new QFile(m_fi.filePath());
    if (!file->open(QIODevice::ReadOnly | QIODevice::Unbuffered))
    {
        QMessageBox::critical(this, tr("Cannot open file"),
            tr("Failed to open file for reading: %1").arg(m_fi.filePath()));
        return;
    }

    //Reset state
    m_bytes_written = 0;

    //Block window
    foreach (QPushButton *btn, m_all_buttons)
        btn->setDisabled(true);

    //Create separate thread
    QThread *thread = new QThread;

    //Use sudo equivalent if not running as root: pkexec
    //pkexec is our choice because gksu/kdesudo may not be installed by default
    //Note that this will show a password prompt (unless running as root),
    //which can be aborted.

    //Initialize write process
    //But not QProcess because we can't track its progress due to writeBuffer
    //Initialize external file writer
    QString out_file = m_sel_drive;
    ExtFileReaderWriter *writer;
    writer = new ExtFileReaderWriter(out_file); //could have thread as parent

    //Initialize our worker which will run in parallel
    FileWriter *worker = new FileWriter(file, writer);
    worker->moveToThread(thread);
    //Hook up start/stop signals
    //https://doc.qt.io/qt-5/qthread.html#dtor.QThread
    //Deleting a running QThread will result in a program crash.
    //Wait for the finished() signal before deleting the QThread.
    connect(worker, SIGNAL(finished()), thread, SLOT(quit())); //finish event loop when done
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater())); //delete worker after loop
    connect(worker, SIGNAL(destroyed()), thread, SLOT(deleteLater())); //delete thread after worker
    //Connect to worker signals
    connect(worker, SIGNAL(bytesWritten(qint64)), SLOT(handleDataWritten(qint64)));
    connect(worker, SIGNAL(readFailed()), SLOT(handleReadError()));
    connect(worker, SIGNAL(writeFailed()), SLOT(handleWriteError()));
    connect(worker, SIGNAL(writeProcFailed(int)), SLOT(handleWriteProcError(int)));
    connect(worker, SIGNAL(finished()), SLOT(handleDone()));
    //Start process
    thread->start();
}

void
MainWindow::askStart(bool only_if_selected)
{
    if (!_file_selected) //input file selected
    {
        if (!only_if_selected)
        {
            QMessageBox::critical(this, tr("Cannot start copy process"),
                tr("Cannot start scan. You have not selected a source image file. Please open the source image file (or download a Live Linux iso)."));
        }
        return;
    }
    if (m_sel_drive.isEmpty()) //output drive selected
    {
        if (!only_if_selected)
        {
            QMessageBox::critical(this, tr("Cannot start copy process"),
                tr("Cannot start scan. You have not selected a target device. Please click on the USB device that you want to overwrite. "
                "If you don't have a USB device, order one from China."));
        }
        return;
    }

    if (QMessageBox::question(this, tr("Start copy process?"),
        tr("Do you want to start the copy process now? This will wipe the device %1.").arg(m_sel_drive)) != QMessageBox::Yes)
        return;

    start();
}

void
MainWindow::unblock()
{
    foreach (QPushButton *btn, m_all_buttons)
        btn->setDisabled(false);
}

void
MainWindow::handleDataWritten(qint64 count)
{
    //qInfo() << count << "bytes written";

    //Handle error
    if (count <= 0)
    {
        //Display error message
        QMessageBox::critical(this, tr("Error"),
            tr("Failed to transfer data (bytes written: %1)").arg(m_bytes_written));
        return;
    }

    //Update progress
    m_bytes_written += count;
    double p = std::div(m_bytes_written * 100, m_file_size).quot;
    lbl_tux->setPixmap(progressedTux(p));
    qInfo().noquote() << QString("transfer block ... %1%").arg(p, 0, 'f', 1);

}


void
MainWindow::handleReadError()
{
    QMessageBox::critical(this, tr("Error"),
        tr("The copy process has failed due to a read error! Bytes written: %1").arg(m_bytes_written));

    lbl_tux->setPixmap(progressedTux(-1));
    unblock();
}

void
MainWindow::handleWriteError()
{
    QMessageBox::critical(this, tr("Error"),
        tr("The copy process has failed due to a write error! Bytes written: %1").arg(m_bytes_written));

    lbl_tux->setPixmap(progressedTux(-1));
    unblock();
}

void
MainWindow::handleWriteProcError(int rc)
{
    QMessageBox::critical(this, tr("Error"),
        tr("The copy process has failed (process returned %1)! Bytes written: %2").arg(rc).arg(m_bytes_written));

    lbl_tux->setPixmap(progressedTux(-1));
    unblock();
}

void
MainWindow::handleDone()
{
    //Check if the worker has actually written the whole image
    if (m_bytes_written != m_file_size)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("The copy process has finished early! Bytes written: %1 (expected: %2)").arg(m_bytes_written).arg(m_file_size));
        lbl_tux->setPixmap(progressedTux(-1));
        return;
    }

    //Show success message
    qInfo().noquote() << tr("finished successfully: bytes written: %1 (total: %2)").arg(m_bytes_written).arg(m_file_size);
    QMessageBox::information(this, tr("Done"),
        tr("The process was completed successfully! Bytes written: %1").arg(m_bytes_written));

    //Reset tux
    unblock();
    lbl_tux->setPixmap(m_pix_tux);

}

QPixmap
MainWindow::progressedTux(double percentage)
{
    QPixmap tux = m_pix_tux;

    if (percentage >= 0)
    {
        //Write percentage on tux
        QPainter painter(&tux);
        painter.setPen(Qt::blue);
        QString p_text = QString("%1%").arg(percentage, 0, 'f', 1);
        painter.drawText(tux.rect(), Qt::AlignCenter, p_text);
    }
    else if (percentage == -1)
    {
        //Turn tux upside down to indicate error
        QImage img = tux.toImage();
        tux = QPixmap::fromImage(img.mirrored());
    }

    return tux;
}

FileWriter::FileWriter(QIODevice *input, QIODevice *output)
          : QObject(),
            m_read(0),
            m_written(0),
            m_auto_blocksize(0),
            m_done(false),
            m_failed(false),
            m_input(input),
            m_output(output),
            m_out_proc(0),
            m_out_writer(0)
{
    m_input = input;
    m_output = output;
    if (m_input)
        if (!m_input->parent()) m_input->setParent(this);
    if (m_output)
    {
        if (!m_output->parent()) m_output->setParent(this);
        connect(m_output, SIGNAL(bytesWritten(qint64)), SLOT(confirmWritten(qint64)));
    }

    if (m_input && m_output)
        qInfo() << "worker initialized!";
}

FileWriter::FileWriter(QIODevice *input, QProcess *output)
          : FileWriter(input, (QIODevice*)output)
{
    qInfo() << "worker got control over write qprocess";

    m_out_proc = output;
    connect(output, SIGNAL(finished(int, QProcess::ExitStatus)),
        SLOT(handleFinished(int, QProcess::ExitStatus)));
    connect(output, SIGNAL(started()), SLOT(run()));

    output->start(QIODevice::WriteOnly);
}

FileWriter::FileWriter(QIODevice *input, ExtFileReaderWriter *output)
          : FileWriter(input, (QIODevice*)0)
{
    m_out_writer = output;
    if (!m_out_writer->parent()) m_out_writer->setParent(this);
    qInfo() << "worker got control over external writer";

    QTimer::singleShot(0, this, SLOT(run()));
}

void
FileWriter::run()
{
    qInfo() << "starting transfer";
    if (m_out_proc)
        qInfo() << "writing to process" << m_out_proc->processId();

    //Read/write loop
    int last_written = 0;
    while (!m_failed && !m_done && last_written != -1)
    {
        last_written = transfer();
    }

}

qint64
FileWriter::transfer()
{
    //NOTE Qt's QFile and QProcess classes show a weird buffering behavior,
    //they keep buffering all input from m_input to m_output
    //though in blocks of 64KB (65535B internal write buffer)
    //but that way, more than 100 MB of data are "written" in under a second,
    //apparently stuck in yet another internal buffer somewhere in memory,
    //without waiting for the process (to accept more data),
    //rendering our logic useless as the GUI shows 100% progress,
    //it stays there the whole time and after the fh's are closed,
    //the data is actually written out, so we see no progress.
    //QProcess has a method called waitForBytesWritten()
    //but calling it (-1) does NOT help except for the very first block,
    //from then on it once again fills whatever buffer with 64KB blocks
    //until the whole file has been consumed before the output process
    //has even committed 1%.
    //Where are more than 100 MB of data buffered?
    //It seems like it's all written to a private QByteArray
    //which is hidden in some buffer struct called writeBuffer.
    //And if we count the exact number of bytes written as received in
    //confirmWritten() from the bytesWritten signal and
    //only read/transfer the next block when ALL pending bytes have been
    //confirmed/written, we get stuck at this point because
    //there will be exactly 65535 bytes pending which are not committed,
    //not confirmed written, so our confirmWritten() waits forever
    //and the program just gets stuck in an awkward way.
    //There does not seem to be a flush method in QProcess either...(?)
    //So we determine the size of the write buffer in confirmWritten()
    //by the number of bytes confirmed written in one go
    //and assume that this is the maximum number of bytes
    //that can get stuck in the buffer.
    //With that, our confirmWritten() will know that if up to this number
    //of bytes are not yet confirmed (i.e., > 0), it needs to call transfer
    //to read more data for these currently buffered bytes to come out.
    //So, our alternative to QProcess is ExtFileReaderWriter.

    //Transfer (loop) iteration: read chunk of bytes, write them to m_output
    int bs = 1024 * 1024 * 1;
    if (m_failed) return -1;
    {
        //Read block
        QByteArray block = m_input->read(bs); //TODO handle read error
        if (block.isEmpty())
        {
            qInfo().noquote() << tr("empty read: done");
            m_done = true;
            finish();
            return 0;
        }
        m_read += block.size();
        qInfo() << "read:" << block.size();

        //Write block
        qint64 written;
        if (m_output)
            written = m_output->write(block);
        else if (m_out_writer)
            written = m_out_writer->write(block);
        if (written != block.size())
        {
            qInfo().noquote() << tr("write failed: %1 written (out of %2)").arg(written).arg(block.size());
            m_failed = true;
            emit writeFailed();
            return -1;
        }
        qInfo() << "written:" << written;
        if (m_out_proc)
        {
            //QProcess is our target - it buffers internally without blocking
            //This should wait/block until all data are written/synced
            //...but it doesn't.
            qInfo() << "waiting for output process...";
            bool ok = m_out_proc->waitForBytesWritten(-1);
            qInfo() << "output process confirmed bytes written" << ok;
        }

        emit bytesWritten(written);
        return written;
    }

    //Next iteration called by slot after bytesWritten()
}

void
FileWriter::confirmWritten(qint64 count)
{
    //UNUSED due to Qt's private writeBuffer (and count grows: 54K, 128K...)
}

/**
 * finish() will close the file handles and schedule the finished signal
 * to be emitted, except if an error has occurred in the meantime.
 *
 * The finished signal is not emitted immediately because
 * m_output might be a QProcess which can fail
 * after waitForFinished() returned, in the handleFinished() slot.
 */
void
FileWriter::finish()
{
    if (m_failed || !m_done) return;

    //Close pipe
    m_input->close();
    if (m_out_proc)
    {
        //Wait for QProcess to finish writing
        //Due to the nature of QProcess and its internal buffer,
        //trying to write a file that's 4x larger than the target device
        //may still fail after waitForFinished() returned,
        //in the handleFinished() slot.
        //So that slot will send the finished signal.
        qInfo() << "closing pipe and waiting for process" << m_out_proc->processId();
        m_out_proc->closeWriteChannel();
        m_out_proc->waitForFinished(-1); //"If msecs is -1, this function will not time out."
        qInfo() << "done waiting for process";
    }
    else if (m_out_writer)
    {
        m_out_writer->close();
    }
    else if (m_output)
    {
        m_output->close();
    }

    //Confirm process finished + this may be deleted
    //Confirm that we're done now, except if QProcess is involved,
    //in which case this happens in the handleFinished slot called
    //when QProcess itself is finished.
    //Otherwise, we'd have a race condition causing
    //a message to be shown to the user confirming success
    //before QProcess actually sends all the data to the process
    //which then fails.
    if (!m_out_proc)
    {
        confirmFinished();
    }
}

void
FileWriter::confirmFinished()
{
    //Do not confirm if an error has occurred (error signal already sent)
    if (m_failed || !m_done) return;

    //Now finally confirm that we're done
    emit finished();
}

void
FileWriter::handleFinished(int rc, QProcess::ExitStatus status)
{
    //Write process has finished (or crashed?)
    qInfo() << "write process has finished with return code" << rc;
    if (!m_done)
    {
        //too early
        qInfo() << "worker's write process has exited too early";
        m_failed = true;
        m_output->close();
        emit writeProcFailed(rc);
        return;
    }

    //Handle error
    if (status != QProcess::NormalExit || rc != 0)
    {
        qInfo() << "worker's write process has failed";
        m_failed = true;
        m_output->close();
        emit writeProcFailed(rc);
        return;
    }

    //Send finished signal to confirm we're done
    //(in other cases, i.e., if not writing to QProcess, this was already done)
    confirmFinished();
}

ExtFileReaderWriter::ExtFileReaderWriter(QObject *parent)
                   : QObject(parent),
                     m_write_mode(true),
                     m_pid(0)
{
    openPipe();
}

ExtFileReaderWriter::ExtFileReaderWriter(QString file, bool write, QObject *parent)
                   : ExtFileReaderWriter(parent)
{
    m_file = file;

    start();

}

ExtFileReaderWriter::~ExtFileReaderWriter()
{
    //Close write pipeline
    closePipe();

    //Wait for child process to finish
    if (m_pid)
    {
        int wstatus;
        waitpid(m_pid, &wstatus, 0);
    }

}

bool
ExtFileReaderWriter::needSudo()
{
    bool need_sudo = !QFileInfo(m_file).isWritable();
    return need_sudo;
}

QStringList
ExtFileReaderWriter::command()
{

    //Check for special characters which may cause harm if run in a shell
    //(even though we don't...)
    if (m_file.contains(' ') || m_file.contains('"') || m_file.contains("'"))
        throw std::runtime_error("illegal file name");

    //Construct command: dd for writing; pkexec to gain root privileges
    //pkexec is preferred over kdesu/gksu which may not be preinstalled
    QStringList cmd_list;
    cmd_list << "dd";
    if (needSudo())
        cmd_list.prepend("pkexec"); //PolicyKit will show a password prompt
    cmd_list << "bs=4";
    //write or read mode - write by default
    if (m_write_mode)
        cmd_list << QString("of=") + m_file;
    else
        cmd_list << QString("if=") + m_file;

    return cmd_list;
}

std::vector<std::string>
ExtFileReaderWriter::cmd()
{
    std::vector<std::string> args_v;
    foreach (QString a, this->command())
        args_v.push_back(a.toLocal8Bit().constData());

    return args_v;
}

void
ExtFileReaderWriter::start()
{
    //Define external call
    std::vector<std::string> args_v = this->cmd();

    //Fork child process
    qInfo() << "ExtFileReaderWriter forking...";
    int p = fork();
    if (p == -1)
        throw std::runtime_error("failed to fork");
    if (p == 0)
    {
        //Connect stdin to write pipe of parent
        dup2(m_pipefd[0], STDIN_FILENO);
        closePipe();

        //Prepare arguments for exec call
        const char *prog = args_v[0].c_str();
        char **args = new char*[args_v.size() + 1]; //for (std::string s: args_v)
        for (int i = 0; i < (int)args_v.size(); i++)
        {
            char *str = args[i] = new char[args_v[i].length() + 1];
            strcpy(str, args_v[i].c_str());
        }
        args[args_v.size()] = 0;
        //Call external program
        qInfo() << "ExtFileReaderWriter calling external program" << QString(prog);
        qInfo() << QString(prog) << args[0];
        execvp(prog, const_cast<char* const*>(args));
        throw std::runtime_error("execvp failed");

        return;
    }
    else
    {
        qInfo() << "ExtFileReaderWriter working with" << p;
        m_pid = p;
    }
}

qint64
ExtFileReaderWriter::write(const char *data, qint64 max)
{
    return ::write(m_pipefd[1], data, max);
    //if written < max && written > 0:
    const char *dataptr = data;
    qint64 total = 0;
    while (true)
    {
        qint64 count = ::write(m_pipefd[1], dataptr, max);
        if (count == -1) return count;
        total += count;
        dataptr = dataptr + count;
    }
    return total;
}

qint64
ExtFileReaderWriter::write(const QByteArray &bytes)
{
    return write(bytes.constData(), bytes.size());
}

void
ExtFileReaderWriter::close()
{
    closePipe();
}

void
ExtFileReaderWriter::openPipe()
{
    if (pipe(m_pipefd) == -1)
        throw std::runtime_error("pipe failed");
}

void
ExtFileReaderWriter::closePipe()
{
    ::close(m_pipefd[0]);
    ::close(m_pipefd[1]);
    m_pipefd[0] = 0;
    m_pipefd[1] = 0;
}

