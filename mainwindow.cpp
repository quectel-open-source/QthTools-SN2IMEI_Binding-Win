/*
Copyright (C) 2023  Quectel Wireless Solutions Co.,Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include "third/QSimpleUpdater/include/updateConfig.h"
QString softwareVersion = "1.2.0";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    ui->setupUi(this);
    qDebug()<<__FUNCTION__;
    connect(ui->menubar,SIGNAL(triggered(QAction*)),this,SLOT(action_handle(QAction*)));

    action_createFile_clicked();
    MainWindow::setWindowTitle(QString::fromLocal8Bit("SN&IMEI绑定工具 V")+softwareVersion);
    MainWindow::setWindowIcon(QIcon(":/image/quectel.ico"));
    QPalette pen;
    pen.setColor(QPalette::WindowText,Qt::blue);
    ui->statusBar->setPalette(pen);
    updateVersion = new toolUpdate();
    updateVersion->checkForUpdates();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::closeTabWidget()
{
    if(NULL != g_csvFile)
    {
        disconnect(g_csvFile,SIGNAL(closeWidget()),this,SLOT(closeTabWidget()));
        g_csvFile->close();
        g_csvFile->deleteLater();
        g_csvFile = NULL;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
}

bool MainWindow::action_createFile_clicked()
{
    qDebug()<<__FUNCTION__;
    if(false ==checkTabIsEmpty())
    {
        return false;
    }
    QObjectList list = ui->page_main->children();
    foreach (QObject *var, list) {
            delete var;
    }
    QGridLayout *layoutPass = new QGridLayout();
    layoutPass->setContentsMargins(0,0,0,0);
    g_csvFile = new csvFileMange(this);
    connect(g_csvFile,SIGNAL(closeWidget()),this,SLOT(closeTabWidget()));
    layoutPass->addWidget(this->g_csvFile);
    ui->page_main->setLayout(layoutPass);
    ui->stackedWidget->setCurrentIndex(1);
    QStringList headerList = g_csvFile->getHeaderTextList();
    ui->statusBar->showMessage(QString::fromLocal8Bit("请双击%1或%2或%3进行扫码输入").arg(headerList.at(0)).arg(headerList.at(1)).arg(headerList.at(2)));
    return true;

}

void MainWindow::action_openFile_clicked()
{
    if(action_createFile_clicked())
    {
        g_csvFile->openCsvFile("csv");
    }
}

void MainWindow::action_saveFile_clicked()
{
    qDebug()<<"save data start";
    if(NULL != g_csvFile)
        g_csvFile->saveFile();
}

void MainWindow::action_clearData_clicked()
{
    if(NULL != g_csvFile)
    {
        if(g_csvFile->getTabIsEmpty())
        {
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请确认是否清除当前记录"), QMessageBox::Yes | QMessageBox::No);
            msgBox->setDefaultButton(QMessageBox::No);
            msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
            msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
            if( QMessageBox::No == msgBox->exec())
            {
                return;
            }
        }
    }
    g_csvFile->clearData();
}

QDateTime MainWindow::getFileTime(QString path)
{
    QFileInfo file(":/help.chm");
    return file.lastModified();
}


void MainWindow::action_helpFile_clicked()
{
    if(helpProcess != NULL)
    {
        helpProcess->close();
        helpProcess = NULL;
    }
    helpProcess = new QProcess(this);
    qDebug()<<QDir::currentPath();
    QString docFilePath = QDir::currentPath()+"/doc/help.chm";

    QFileInfo file(docFilePath);
    qDebug()<<"time:"<<getFileTime(":/help.chm");
    qDebug()<<"time2:"<<file.lastModified();
    if((!file.isFile()) || (getFileTime(":/help.chm") > file.lastModified()))
    {
        QDir dir(QDir::currentPath()+"/doc/");
        if(!dir.exists())
        {
            if(!dir.mkdir(QDir::currentPath()+"/doc/"))
            {
                QMessageBox::critical(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("<FONT size=4><div><b>程序无法使用该路径</b><br/></div>"),QString::fromLocal8Bit("确认"));
                helpProcess->close();
                return;
            }
        }
        QFile::remove(docFilePath);
        QFile::copy(":/help.chm",docFilePath);
    }
    QStringList manuals(docFilePath);
    helpProcess->start("hh.exe",manuals);
    helpProcess->waitForStarted();
}

bool MainWindow::checkTabIsEmpty()
{
    if(NULL != g_csvFile)
    {
        if(g_csvFile->getTabIsEmpty())
        {
            if(QMessageBox::No == QMessageBox::warning(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("是否覆盖当前记录"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No))
            {
                return false;
            }
        }
    }
    return true;
}





void MainWindow::action_handle(QAction *action)
{
    qDebug()<<"name:"<<action->objectName();
    QString name = action->objectName();
    if(0 == name.compare("action_importFile"))
    {
        action_openFile_clicked();
    }
    else if(0 == name.compare("action_exportFile"))
    {
        action_saveFile_clicked();
    }
    else if(0 == name.compare("action_clearData"))
    {
        action_clearData_clicked();
    }
    else if(0 == name.compare("action_helpFile"))
    {
        action_helpFile_clicked();
    }
    else if(0 == name.compare("action_tool"))
    {
        updateVersion->checkForUpdates();
    }
}

