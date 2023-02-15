/****************************************************************************
**
** Copyright (C) 2023 Philip Seeger <philip@c0xc.net>
** This file is part of UsbImgWriter.
**
** CapacityTester is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** CapacityTester is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with CapacityTester. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

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

    QString
    m_path_sudo;

};

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

    //FileWriter(QFileInfo input, QFileInfo output);
    FileWriter(QIODevice *input, QIODevice *output);
    FileWriter(QIODevice *input, QProcess *output);

public slots:

    void
    run();

private slots:

    void
    handleFinished(int rc, QProcess::ExitStatus status);

private:

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

};

#endif
