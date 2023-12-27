﻿/*
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
#include "toolupdate.h"
#include "ui_toolupdate.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qDebug>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QTextCodec>
#include "third/QSimpleUpdater/include/updateConfig.h"
QString softwareName = "QthTools-SN2IMEI_Binding-Win";

toolUpdate::toolUpdate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::toolUpdate)
{
    ui->setupUi(this);
    m_updater = QSimpleUpdater::getInstance();
    /* Check for updates when the "Check For Updates" button is clicked */
    connect(m_updater, SIGNAL(checkingFinished(QString)), this, SLOT(updateChangelog(QString)));
    connect(m_updater, SIGNAL(appcastDownloaded(QString, QByteArray)), this, SLOT(displayAppcast(QString, QByteArray)));
    connect(m_updater,&QSimpleUpdater::updateEventSend,this,&toolUpdate::updataExeEvent);

    /* Resize the dialog to fit */
    setMinimumSize(minimumSizeHint());
    resize(minimumSizeHint());
}

toolUpdate::~toolUpdate()
{
    delete ui;
}

void toolUpdate::resetFields()
{

}

void toolUpdate::checkForUpdates()
{
    qDebug()<<"checkForUpdates";
    if(softwareVersion.isEmpty())
    {
        return;
    }
    qDebug()<<"softwareName:"<<softwareName;
    if(softwareName.isEmpty())
    {
        softwareName=QApplication::applicationDisplayName();
    }
    QString url = DEFS_URL+softwareName+"/update.json";
    QString version = softwareVersion;
    bool downloaderEnabled = true; // 可下载
    bool notifyOnUpdate =false;
    bool customAppcast = false;     // 是否使用客户自己的url解析
    bool notifyOnFinish;    // 更新下载完成提示
    if(false == firstUse)
    {
        notifyOnFinish = false;    // 是否弹出更新提示
        firstUse = true;
    }
    else
    {
        notifyOnFinish= true;
    }
    bool mandatoryUpdate = false;   // 强制升级

    m_updater->setModuleVersion(url, version);
    m_updater->setNotifyOnFinish(url, notifyOnFinish);
    m_updater->setNotifyOnUpdate(url, notifyOnUpdate);
    m_updater->setUseCustomAppcast(url, customAppcast);
    m_updater->setDownloaderEnabled(url, downloaderEnabled);
    m_updater->setMandatoryUpdate(url, mandatoryUpdate);
    m_updater->checkForUpdates(url);
}

void toolUpdate::updateChangelog(const QString &url)
{
    qDebug()<<__FUNCTION__<<"--url:"<<url;
}

void toolUpdate::displayAppcast(const QString &url, const QByteArray &reply)
{
    qDebug()<<__FUNCTION__<<"url:"<<url;
    qDebug()<<__FUNCTION__<<QString::fromUtf8(reply);
}

static void runBat(QString dstBatPath,QStringList arg)
{
    QProcess *p= new QProcess();
    p->startDetached(dstBatPath,arg);
    QApplication::quit();
}

#include <QThread>
void toolUpdate::updataExeEvent(QString newSoftName)
{
    qDebug()<<QApplication::applicationFilePath().split("/").last();
    QString moveExePath = vitalFilePath;
    qDebug()<<"moveExePath:"<<moveExePath<<"newSoftName:"<<newSoftName;
//    QString tagetExePath = QDir::currentPath()+"/"+newSoftName; // tagetExePath
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    std::string StringData = code->fromUnicode(QApplication::applicationDirPath()).data();
    QString tagetExePath = QString::fromLocal8Bit(QByteArray::fromRawData(StringData.c_str(),StringData.size()));
    QFileInfo dstExe(moveExePath+"/"+newSoftName);
    if(!dstExe.exists())
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("目标文件丢失"),"ok");
        checkForUpdates();
        return;
    }

    QString dstBatPath = QDir::currentPath()+"/restart.bat";
    if(!QFile::copy(":/restart.bat",dstBatPath) || !QFile(dstBatPath).exists())
    {
        qDebug()<<QString::fromLocal8Bit("bat文件丢失");
    }
    qDebug().noquote()<<"moveExePath2:"<<QDir::toNativeSeparators(moveExePath);
    qDebug().noquote()<<"tagetExePath2:"<<QDir::toNativeSeparators(tagetExePath);
    QStringList arg;
    arg<<QDir::toNativeSeparators(moveExePath)<<newSoftName<<QDir::toNativeSeparators(tagetExePath)<<QApplication::applicationFilePath().split("/").last();
    qDebug()<<"arg:"<<arg;
    runBat(dstBatPath,arg);

}



