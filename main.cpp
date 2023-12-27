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

//#pragma execution_character_set("utf-8")
#include <QApplication>
#include <QTextCodec>
#include <qDebug>
//这里需要include掉下面两个头文件
#include "Windows.h"
#include "WinUser.h"
#include <QStandardPaths>
#include <QMessageBox>
#include <QDateTime>
#include <QReadWriteLock>
#include <DbgHelp.h>
#include "user_configure.h"
#include "third/QSimpleUpdater/include/updateConfig.h"
#include <QSystemSemaphore>
#include <QSharedMemory>


#pragma comment(lib,"Dbghelp.lib")

int debugLevel=0;
QReadWriteLock readWriteLock;
QtMessageHandler gDefaultHandler = NULL;
QString vitalFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-SN2IMEI_Binding-Win/";
void customMessageHandler(QtMsgType type,
                          const QMessageLogContext &context,
                          const QString &msg)
{
    //Q_UNUSED(context)


    QDateTime _datetime = QDateTime::currentDateTime();
    QString szDate = _datetime.toString("yyyy-MM-dd hh:mm:ss.zzz");//"yyyy-MM-dd hh:mm:ss ddd"
    QString txt(szDate);
    if(type < debugLevel)
    {
        return;
    }
//    if(debugLogFile.size() > 10485760)
//    {
//        debugLogFile.resize(0);
//    }
    switch (type)
    {
        case QtDebugMsg://调试信息提示
        {
            txt += QString(" [Debug] ");
            break;
        }
        case QtInfoMsg://信息输出
        {
            txt += QString(" [Info] ");
            break;
        }
        case QtWarningMsg://一般的warning提示
        {
            txt += QString(" [Warning] ");
            break;
        }
        case QtCriticalMsg://严重错误提示
        {
            txt += QString(" [Critical] ");
            break;
        }
        case QtFatalMsg://致命错误提示
        {
            txt += QString(" [Fatal] ");
            //abort();
            break;
        }
        default:
        {
            txt += QString(" [Trace] ");
            break;
        }
    }
    txt.append( QString(" %1").arg(context.file) );
    txt.append( QString("<%1>: ").arg(context.line) );
    txt.append(msg);

    readWriteLock.lockForWrite();
    QFile debugLogFile;
    QString configfile = vitalFilePath+".debug.ini";
    debugLogFile.setFileName(configfile);
    debugLogFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&debugLogFile);
    text_stream << txt << "\r\n";
    text_stream.flush();
    debugLogFile.close();
    readWriteLock.unlock();
    if(gDefaultHandler)
    {
        gDefaultHandler(type,context,msg);
    }
}


void clearLogFile()
{
    readWriteLock.lockForWrite();
    QString configfile = vitalFilePath+".debug.ini";
    QFile file(configfile);
    file.resize(0);
    file.close();
    readWriteLock.unlock();
}


long  __stdcall CrashInfocallback(_EXCEPTION_POINTERS *pexcp)
{
    //创建 Dump 文件
    QString hDumpFileName = vitalFilePath+".dmp";
    std::wstring wlpstrstd = hDumpFileName.toStdWString();
    LPCWSTR strName = wlpstrstd.c_str();
    HANDLE hDumpFile = ::CreateFile(
        strName,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        //Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pexcp;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        //写入Dump文件内容
        ::MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hDumpFile,
            MiniDumpNormal,
            &dumpInfo,
            NULL,
            NULL
        );
    }
    QMessageBox::critical(NULL,"Dump",QString::fromLocal8Bit("<FONT size=4><div><b>对于发生的错误，表示诚挚的歉意</b><br/></div>"),QString::fromLocal8Bit("确认"));
    return 0;
}




int main(int argc, char *argv[])
{

    //添加Qt的对高分屏的支持
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    debugLevel = 0;
    // 更新dump和debug信息输出驱动
    ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CrashInfocallback);
    gDefaultHandler = qInstallMessageHandler(customMessageHandler);
//    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//    QTextCodec::setCodecForLocale(codec);

    QApplication a(argc, argv);

    // 创建信号量
    QSystemSemaphore semaphore("SingleAppTest2Semaphore", 1);
    // 启用信号量，禁止其他实例通过共享内存一起工作
    semaphore.acquire();

#ifndef Q_OS_WIN32
    // 在linux / unix 程序异常结束共享内存不会回收
    // 在这里需要提供释放内存的接口，就是在程序运行的时候如果有这段内存 先清除掉
    QSharedMemory nix_fix_shared_memory("SingleAppTest2");
    if (nix_fix_shared_memory.attach())
    {undefined
        nix_fix_shared_memory.detach();
    }
#endif
    // 创建一个共享内存  “SingleAppTest2”表示一段内存的标识key 可作为唯一程序的标识
    QSharedMemory sharedMemory("QthTools-SN2IMEI_Binding-Win");
    // 测试是否已经运行
    bool isRunning = false;
    // 试图将共享内存的副本附加到现有的段中。
    if (sharedMemory.attach())
    {
        // 如果成功，则确定已经存在运行实例
        isRunning = true;
    }
    else
    {
        // 否则申请一字节内存
        sharedMemory.create(1);
        // 确定不存在运行实例
        isRunning = false;
    }
    semaphore.release();

    // 如果您已经运行了应用程序的一个实例，那么我们将通知用户。
    if (isRunning)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
            "Allowed to run only one instance of the application.");
        msgBox.exec();
        return 1;
    }



    //从系统函数中获取显示器的分辨率，设置字体跟随缩放比例来变化
    const float DEFAULT_DPI = 96.0;
    HDC screen = GetDC(nullptr);
    FLOAT dpiX = static_cast<FLOAT>(GetDeviceCaps(screen, LOGPIXELSX));
    ReleaseDC(nullptr, screen);
    float fontSize = dpiX / DEFAULT_DPI;


    QFont font = a.font();
    font.setPointSize(font.pointSize()*fontSize);
    a.setFont(font);

    QDir dir(vitalFilePath);
    if(!dir.exists())
    {
        if(!dir.mkdir(vitalFilePath))
        {
            QMessageBox::critical(NULL,"Dump",QString::fromLocal8Bit("<FONT size=4><div><b>程序无法使用</b><br/></div>"),QString::fromLocal8Bit("确认"));
            return 0;
        }
    }
    // 清空日志文件
    clearLogFile();
    MainWindow w;
    w.show();
    return a.exec();
}
