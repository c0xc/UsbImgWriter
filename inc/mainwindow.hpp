/****************************************************************************
**
** Copyright (C) 2023 Philip Seeger <philip@c0xc.net>
** This file is part of UsbImgWriter.
**
** UsbImgWriter is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** UsbImgWriter is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with UsbImgWriter. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

#include <QObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QStorageInfo>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QTimer>
#include <QMessageBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMainWindow>
#include <QFileDialog>
#include <QGroupBox>
#include <QDateTime>
#include <QProcess>
#include <QPainter>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "udiskmanager.hpp" //from CapacityTester project
#include "size.hpp" //from CapacityTester project
#include "itembutton.hpp"

class FileWriter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = 0);

public slots:

    void
    selectFile();

    void
    refreshDrives();

    void
    selectDevice(const QString &dev);

    void
    start();

    void
    askStart(bool only_if_selected = false);

private slots:

    void
    unblock();

    void
    handleDataWritten(qint64 count);

    void
    handleReadError();

    void
    handleWriteError();

    void
    handleWriteProcError(int rc);

    void
    handleDone();

private:

    QPixmap
    progressedTux(double percentage);

    QPixmap
    m_pix_tux;

    QLabel
    *lbl_tux;

    QLabel
    *frm_warn_top;

    QList<QPushButton*>
    m_all_buttons;

    QLabel
    *lbl_img_title;

    QLineEdit
    *txt_img_file;

    QLineEdit
    *txt_img_size;

    QLineEdit
    *txt_img_mtime;

    QLabel
    *lbl_usb_title;

    bool
    _file_selected;

    QFileInfo
    m_fi;

    qint64
    m_file_size;

    qint64
    m_drive_size;

    qint64
    m_bytes_written;

    QWidget
    *wid_usb_buttons;

    QList<QPointer<QWidget>>
    m_dev_button_refs;

    UDiskManager
    m_diskmanager;

    QString
    m_sel_drive;

};

/**
 * FileWriter takes an input and an output stream,
 * it reads all input data and writes each chunk of data to output in a loop.
 *
 * Originally written to also accept a QProcess instance (2nd ctor)
 * in which case it writes to that process and sends status signals
 * including bytesWritten(qint64).
 * This almost works except that QProcess::write() actually writes
 * to its internal writeBuffer which seems to have no limit (ex: 4GB RES).
 */
class ExtFileReaderWriter;
class FileWriter : public QObject
{
    Q_OBJECT

signals:

    void
    bytesWritten(qint64 count);

    void
    readFailed();

    void
    writeFailed();

    void
    writeProcFailed(int rc);

    void
    finished();

public:

    /**
     * FileWriter can be initialized with two io devices,
     * in which case it will just read from the first and write to the second.
     * If initialized with a QProcess, it will call some extra methods.
     *
     * In any case, if the input or output object has no parent,
     * FileWriter will become its parent,
     * so it will take care of closing and deleting it.
     */
    FileWriter(QIODevice *input, QIODevice *output);
    FileWriter(QIODevice *input, QProcess *output);
    FileWriter(QIODevice *input, ExtFileReaderWriter *output);
    //FileWriter(QIODevice *input, QFileInfo output); //convenience
    //FileWriter(QFileInfo input, QIODevice *output); //convenience

public slots:

    void
    run();

private slots:

    qint64
    transfer();

    void
    confirmWritten(qint64 count);

    void
    finish();

    void
    confirmFinished();

    void
    handleFinished(int rc, QProcess::ExitStatus status);

private:

    qint64
    m_read;

    qint64
    m_written;

    qint64
    m_auto_blocksize;

    bool
    m_done;

    bool
    m_failed;

    QIODevice
    *m_input;

    QIODevice
    *m_output;

    QProcess
    *m_out_proc;

    ExtFileReaderWriter
    *m_out_writer;

};

/**
 * ExtFileReaderWriter takes a file path and provides an interface
 * to write to that file (or read from it) through an external process.
 * This external process would usually be dd.
 * Without write permissions (not running as root),
 * the process may be run through pkexec.
 *
 * In write mode (the default), the target file will be truncated
 * if it's a regular file. But it shouldn't be:
 * The target file is expected to be a storage device file like:
 * /dev/sdd
 *
 * There's obviously a risk of some self-destruction involved.
 * In the context of the UsbImgWriter program,
 * the selected device will first be unmounted.
 *
 * TODO IDEA reimplement QIODevice - but without any hidden buffer
 */
class ExtFileReaderWriter : public QObject //TODO QIODevice
{
    Q_OBJECT

signals:

public:

    ExtFileReaderWriter(QObject *parent = 0);
    ExtFileReaderWriter(QString file, bool write = true, QObject *parent = 0);
    ~ExtFileReaderWriter();

    bool
    needSudo();

    QStringList
    command();

    void
    start();

    qint64
    write(const char *data, qint64 max);

    qint64
    write(const QByteArray &bytes);

    void
    close();

private:

    void
    openPipe();

    void
    closePipe();

    std::vector<std::string>
    cmd();

    bool
    m_write_mode;

    int
    m_pipefd[2];

    QString
    m_file;

    int
    m_pid;



};

#endif
