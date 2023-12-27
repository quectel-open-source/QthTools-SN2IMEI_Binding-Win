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
#include "csvfilemange.h"
#include "ui_csvfilemange.h"
#include <QMessageBox>
#include <qDebug>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "csvfilemange.h"
#include "mythead.h"
#include <QTime>
#include <QScrollBar>
#include "third/include/progresswater.h"
#include "user_configure.h"
#include "QSimpleUpdater/include/updateConfig.h"
enum
{
    CHECK_NOMAL_MODE=0,
    CHECK_SAVE_MODE,
};
bool useFirstAPP = false;
int g_readSqlMaxLines = 10000;
int g_currentFileOffset = 0;
csvFileMange::csvFileMange(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::csvFileMange)
{
    ui->setupUi(this);
    qDebug()<<__FUNCTION__;
    getHeaderTextList();
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderText);
    ui->tableWidget_2->setHorizontalHeaderLabels(horizontalHeaderText);
    ui->tableWidget_3->setHorizontalHeaderLabels(horizontalHeaderText);
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    ui->gridLayout_3->addWidget(pageNavigator,2,1);
    setQTableWidget(ui->page,&stackWidgetInfo.leftWidget);
    setQTableWidget(ui->page_2,&stackWidgetInfo.currentWidget);
    setQTableWidget(ui->page_3,&stackWidgetInfo.rightWidget);
    ui->stackedWidget->setCurrentIndex(1);


    connect(ui->tableWidget,SIGNAL(deleteSqlInfo(QList<QString>)),this,SLOT(delTabWiagetData(QList<QString>)));
    connect(ui->tableWidget,SIGNAL(runNextPage()),this,SLOT(runNextPageHandle()));
    connect(ui->tableWidget,&newTableWidget::writeToTabHandle,this,&csvFileMange::writeToTabHandle,Qt::DirectConnection);


    connect(ui->tableWidget_2,SIGNAL(deleteSqlInfo(QList<QString>)),this,SLOT(delTabWiagetData(QList<QString>)));
    connect(ui->tableWidget_2,SIGNAL(runNextPage()),this,SLOT(runNextPageHandle()));
    connect(ui->tableWidget_2,&newTableWidget::writeToTabHandle,this,&csvFileMange::writeToTabHandle,Qt::DirectConnection);

    connect(pageNavigator,SIGNAL(currentPageChanged(int)),this,SLOT(pageNavigator_currentPageChanged(int)));
//    connect(this,SIGNAL(updateTableWidget(int)),this,SLOT(pageNavigator_currentPageChanged(int)));
    openSqlDataBase();

    progressBar = new QWidget(ui->stackedWidget);
    QGridLayout *layout = new QGridLayout();
    progressWater= new ProgressWater(ui->stackedWidget);
    progressWater->setMaximumSize(300,300);
    ((ProgressWater *)progressWater)->setRange(0,100);
    ((ProgressWater *)progressWater)->setBgColor(QColor("#000000"));
    layout->addWidget(progressWater);
    progressBarLabel = new QLabel();
    layout->addWidget(progressBarLabel);
    progressBar->setLayout(layout);
    progressBar->setWindowFlag(Qt::FramelessWindowHint);
    progressBar->setWindowFlags(progressBar->windowFlags()  | Qt::Dialog);
    progressBar->setAutoFillBackground(true);
    progressBar->setWindowOpacity(0.9);
    progressBar->setAttribute(Qt::WA_QuitOnClose,false);
    progressBar->setWindowModality(Qt::ApplicationModal);
    progressBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

}

csvFileMange::~csvFileMange()
{
    delete ui;
    if(NULL != pageNavigator)
    {
        delete pageNavigator;
        pageNavigator = NULL;
    }
    if(NULL != userInfoSql)
    {
        delete userInfoSql;
        userInfoSql = NULL;
    }
}
/**************************************************************************
** 功能	@brief : 获取当前用户的历史保存路径
** 输入	@param :
** 输出	@retval:
***************************************************************************/
QString csvFileMange::getCurrentPath()
{
    qDebug()<<__FUNCTION__;
    return g_currentSavePath;
}

/**************************************************************************
** 功能	@brief : 获取当前用户数据是否为空
** 输入	@param :
** 输出	@retval:
***************************************************************************/
bool csvFileMange::getTabIsEmpty()
{
    bool isEmpty = false;
    // 判断页数大于1，则数据不为空

    if(pageNavigator->getMaxPage() > 1)
    {
        return true;
    }

    // 判断当前页数据是否为空
    if(NULL != stackWidgetInfo.currentWidget)
    {
#if 0
        int rows = stackWidgetInfo.currentWidget->rowCount();
        for(int i =0;i<rows;i++)
        {
            if((NULL != stackWidgetInfo.currentWidget->item(i,0) && !stackWidgetInfo.currentWidget->item(i,0)->text().isEmpty())
                    || (NULL != stackWidgetInfo.currentWidget->item(i,1) && !stackWidgetInfo.currentWidget->item(i,1)->text().isEmpty())
                    || (NULL != stackWidgetInfo.currentWidget->item(i,2) && !stackWidgetInfo.currentWidget->item(i,2)->text().isEmpty())
                    || (NULL != stackWidgetInfo.currentWidget->item(i,3) && !stackWidgetInfo.currentWidget->item(i,3)->text().isEmpty()))
            {
                isEmpty = true;
                break;
            }
        }
#else
        if((NULL != stackWidgetInfo.currentWidget->item(0,0) && !stackWidgetInfo.currentWidget->item(0,0)->text().isEmpty()) || (NULL != stackWidgetInfo.currentWidget->item(0,1) && !stackWidgetInfo.currentWidget->item(0,1)->text().isEmpty()) ||
                (NULL != stackWidgetInfo.currentWidget->item(0,2) && !stackWidgetInfo.currentWidget->item(0,2)->text().isEmpty()))
        {
            isEmpty = true;
        }

#endif
    }
    return isEmpty;
}
/**************************************************************************
** 功能	@brief : 清空用户数据
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void csvFileMange::clearData()
{
    // 删除所有数据  // 清空多少个stackWidget 如何清空数据库
    pageNavigator->setMaxPage(1);
    stackWidgetInfo.currentWidget->clearContents();
    if(NULL != userInfoSql)
    {
        QString delInfo = QString("delete from device_userInfo;");
        ((mySqlite *)userInfoSql)->delSqlData(delInfo);
        QString delInfoSeq = QString("DELETE FROM sqlite_sequence WHERE name = 'device_userInfo';");
        ((mySqlite *)userInfoSql)->delSqlData(delInfoSeq);
    }
    QScrollBar *vScrollbar = stackWidgetInfo.currentWidget->verticalScrollBar();
    vScrollbar->setSliderPosition(0);
    stackWidgetInfo.currentWidget->setTabCurrentRow(0);
}

//int csvFileMange::getTableWidgetRow()
//{
//    qDebug()<<__FUNCTION__;
//    return stackWidgetInfo.currentWidget->getCurrentRow();
//}

void csvFileMange::uiAttributeSet()
{

}




/*********************************************************************************************************************************************************************

                                                                    文件操作

*********************************************************************************************************************************************************************/

bool csvFileMange::fileVaildCheck(QTextStream *in,int fileSize,int *lineCount)
{
    int currentFileOffset = 0;
    // 数据有效性检测
    bool fileValid = true;
    bool dataValid = true;
    int lineCountsCheck = 0;
    int oldPercent = 0;
    ((ProgressWater *)progressWater)->setValue(0);
    progressBarLabel->setText(QString::fromLocal8Bit("文件校验中..."));
    qDebug()<<ui->stackedWidget->size();
    progressBar->resize(ui->stackedWidget->size()/2);
    progressBar->show();
    nonBlockingSleep(4);


    QString fileData = in->readLine();
    QStringList list = fileData.split(',');
    if(list.size() != 4)
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("文件标题出错，请修正后重新导入"),QString::fromLocal8Bit("确认"));
        progressBar->hide();
        return false;
    }
    while(in->readLineInto(&fileData))
    {
        if(!fileData.isEmpty() && fileData.size() > 4)
        {
            QStringList list = fileData.split(',');
            if(list.size() != 4)
            {
                fileValid = false;
                break;
            }
            QString imeiString = list[0];
            QString pkString = list[1];
            QString snString = list[2];
            if((imeiString.size() >= 4) && (imeiString.size() <= 32))
            {
                QByteArray byteArray = imeiString.toLatin1();
                const char *s = byteArray.data();
                while(*s)
                {
                    if((*s>='0' && *s<='9'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s++;
                }
            }
            else
            {
                dataValid = false;
                break;
            }
            if(pkString.size() != 6 || snString.size() > 64)
            {
                dataValid = false;
                break;
            }
            else
            {
                QByteArray byteArray = snString.toLatin1();
                const char *s = byteArray.data();
                while(*s)
                {
                    if((*s>='0' && *s<='9') || (*s>='a' && *s<='z')|| (*s>='A' && *s<='Z'));
                    else
                    {
                        dataValid = false;
                        break;
                    }
                    s++;
                }
            }
            if(false == dataValid)
                break;
            lineCountsCheck++;
            currentFileOffset+=fileData.size();
            int percent = (currentFileOffset *1.0 / fileSize)*100;
            if (oldPercent != percent)
            {
                oldPercent = percent;
                ((ProgressWater *)progressWater)->setValue(percent);
                nonBlockingSleep(1);
            }

        }
        else
        {
            qDebug()<<"error";
            dataValid = false;
            break;
        }
    }
    qDebug()<<"lineCountsCheck:"<<lineCountsCheck;
    if(false == fileValid || false == dataValid || (fileValid && lineCountsCheck > 50000))
    {
        if(dataValid)
            QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("引入错误文件，请重新导入"),QString::fromLocal8Bit("确认"));
        else
            QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("文件第%1行出现错误，请检查后重新导入").arg(lineCountsCheck+2),QString::fromLocal8Bit("确认"));
        progressBar->hide();
        return false;
    }
    *lineCount = lineCountsCheck;
    progressBarLabel->setText(QString::fromLocal8Bit("文件校验完成"));
    nonBlockingSleep(2);
    return true;
}

QStringList csvFileMange::getHeaderTextList()
{
    qDebug()<<__FUNCTION__;
    QFile headerFile(vitalFilePath+"info.txt");
    if(headerFile.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QStringList data = QString(headerFile.readAll()).split("\r\n");
        qDebug()<<"data:"<<data.at(0);
        QStringList headString = QString(data.at(0)).split("_!!_");
        headString.replace(headString.size()-1,QString(headString.last()).simplified());
        qDebug()<<"headString:"<<headString;
        if(!headString.isEmpty())
        {
           horizontalHeaderText = headString;
        }
        headerFile.close();
    }
    else
    {
        qDebug()<<headerFile.errorString();
    }
    return horizontalHeaderText;
}

void csvFileMange::setHeaderTextList(QStringList list)
{
    qDebug()<<__FUNCTION__;
    QFile headerFile(vitalFilePath+"info.txt");
    if(headerFile.open(QIODevice::ReadWrite| QIODevice::Text))
    {

        QStringList data = QString(headerFile.readAll()).split("\r\n");
        if(list.isEmpty())
        {
            list = horizontalHeaderText;
        }
        QString newHeader;
        for(int i =0;i<ui->tableWidget->columnCount();i++)
        {
            if(i != ui->tableWidget->columnCount()-1)
            {
                newHeader.append(list.at(i)+"_!!_");
            }
            else
            {
                newHeader.append(list.at(i)+"\r\n");
            }
        }

        if(data.isEmpty())
        {
            data.append(newHeader);
        }
        else
        {
            data.replace(0,newHeader);
        }
        headerFile.close();
        QFile headerFile2(vitalFilePath+"info.txt");
        headerFile2.open(QIODevice::WriteOnly);
        QTextStream stream(&headerFile2);
        stream.setCodec("UTF-8");
        for(int i =0;i<data.size();i++)
            stream<<data[i];
        headerFile2.close();
    }
    else
        qDebug()<<headerFile.errorString();
}


/**************************************************************************
** 功能	@brief : 打开csv文件
** 输入	@param :
** 输出	@retval:
***************************************************************************/
void csvFileMange::openCsvFile(QString fileType)
{
    qDebug()<<__FUNCTION__;
    QString fileName;
    if(fileType.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(NULL,QString::fromLocal8Bit("选取文件"),".","");
    }
    else
    {
        fileName = QFileDialog::getOpenFileName(NULL,QString::fromLocal8Bit("选取文件"),".",fileType+" (*."+fileType+")");
    }
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadWrite))
    {
        QMessageBox::information(0,QString::fromLocal8Bit("文件管理"),QString::fromLocal8Bit("无法打开文件"),QString::fromLocal8Bit("确认"));
        return;
    }
    if(NULL != userInfoSql)
    {
        // 删除sql表
        clearData();
//        ((mySqlite *)userInfoSql)->delSqlData("DELETE FROM device_userInfo;");
        // 重新创建表
        createSqlTable();
    }
    else
    {
        qDebug()<<"error in openCsvFile";
    }
    QFileInfo info(fileName);
    int fileSize = info.size();
    int totalTimes = 6;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    g_filePath = QDir::currentPath()+"/";
    currentFile = &file;
    QString fileData = NULL;
    int j= 0;
//    QString insertData;
    bool titleFlag = false;
    int lineCounts = 0;
    if(false ==fileVaildCheck(&in,fileSize,&lineCounts))
    {
        return;
    }
    qDebug()<<"lineCounts:"<<lineCounts;

    ((ProgressWater *)progressWater)->setValue(0);
    ((ProgressWater *)progressWater)->setUsedColor(QColor("#DAA520"));
    progressBarLabel->setText(QString::fromLocal8Bit("文件写入中..."));
    int oldPercent = 0;
    in.seek(0);
//    createThread(fileSize);
    QList<structInfo_t>infoList;
    for(int i = 0;i<lineCounts+1;i++)
    {
        fileData = in.readLine();
        if(!fileData.isEmpty())
        {
            QStringList list = fileData.split(',');
            if(titleFlag)
            {
                // 当前不做数据正确性导入
                structInfo_t info;
                info.dk = list[0];
                info.pk = list[1];
                info.sn = list[2];
                info.text = list[3];
                infoList.append(info);
                j++;
                if(j >= 500)
                {
                    int line = ((mySqlite *)userInfoSql)->batchInsertSqlData(infoList);
                    infoList.clear();
                    if(line > 0)
                    {
                        QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("文件第%1行出现重复错误，请检查后重新导入").arg(line+i-500+2),QString::fromLocal8Bit("确认"));
                        file.close();
                        progressBar->hide();
                        return;
                    }
                    j = 0;
                }
            }
            else
            {
                titleFlag = true;
                qDebug()<<"list:"<<list;
                bool isSame = true;
                for(int i=0;i<stackWidgetInfo.currentWidget->model()->columnCount();i++)
                {
                    QString name = stackWidgetInfo.currentWidget->model()->headerData(i,Qt::Horizontal).toString();
                    qDebug()<<"name:"<<name;
                    if(0 != name.compare(list[i]))
                    {
                        isSame = false;
                        break;
                    }
                }
                if(false == isSame)
                {
                    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("当前文件标题与原设定的不一致，是否覆盖新标题"), QMessageBox::Yes | QMessageBox::No);
                    msgBox->setDefaultButton(QMessageBox::No);
                    msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
                    msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
                    if( QMessageBox::No == msgBox->exec())
                    {
                        break;
                    }
                }
                horizontalHeaderText.replace(0,list[0]);
                horizontalHeaderText.replace(1,list[1]);
                horizontalHeaderText.replace(2,list[2]);
                horizontalHeaderText.replace(3,list[3]);
                setHeaderTextList(horizontalHeaderText);
                ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderText);
                ui->tableWidget_2->setHorizontalHeaderLabels(horizontalHeaderText);
                ui->tableWidget_3->setHorizontalHeaderLabels(horizontalHeaderText);
            }
//            g_currentFileOffset+=fileData.size();
//            currentFileOffset+=fileData.size();
            if(totalTimes <= 0)
            {
                totalTimes = 6;
                int percent = (i *1.0 /lineCounts)*100;
                if(oldPercent != percent)
                {
                    ((ProgressWater *)progressWater)->setValue(percent);
                    nonBlockingSleep(1);
                }
            }
            else
            {
                totalTimes--;
            }
        }
    }
    file.close();
    progressBarLabel->setText(QString::fromLocal8Bit("文件写入完成"));
    if(j !=0)
    {
        int line = ((mySqlite *)userInfoSql)->batchInsertSqlData(infoList);
        infoList.clear();
        if(line > 0)
        {
            QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("文件第%1行出现重复错误，请检查后重新导入").arg(lineCounts>500?lineCounts-500+line+2:line+2),QString::fromLocal8Bit("确认"));
            progressBar->hide();
            return;
        }
    }
    QStringList list = ((mySqlite *)userInfoSql)->execSqlDataList("SELECT count(idex) AS number FROM device_userInfo;");
    qDebug()<<QString(list.at(0));
    progressBar->hide();
    initWidget();
//    g_currentFileOffset = 0;
}


/*********************************************************************************************************************************************************************

                                                                   表格操作

*********************************************************************************************************************************************************************/




/*******************************************************************************************
                                    数据保存
*******************************************************************************************/

bool csvFileMange::saveTablewidgetData(QFile &file)
{
    qDebug()<<__FUNCTION__;
    QStringList list = ((mySqlite *)userInfoSql)->execSqlDataList("SELECT count(idex) AS number FROM device_userInfo;");
    int counts = QString(list.at(0)).toInt();
    int times = counts/g_readSqlMaxLines + (counts%g_readSqlMaxLines>0?1:0);
    for(int time_i=0;time_i<times;time_i++)
    {
        int lastShowBegin = time_i*g_readSqlMaxLines;
        int lastShowOffset = ((time_i+1)*g_readSqlMaxLines > counts)?(counts-time_i*g_readSqlMaxLines):g_readSqlMaxLines;
        QVector<structInfo_t>list=((mySqlite *)userInfoSql)->findSqlAllList(QString("select * from device_userInfo limit '%1','%2'").arg(lastShowBegin).arg(lastShowOffset));
        int sum = list.size();
        if(time_i == times-1)
        {
            sum--;
        }
        for(int i =0;i<sum;i++)
        {
          QString lineData;
          lineData.append(list[i].dk+",");
          lineData.append(list[i].pk+",");
          lineData.append(list[i].sn+",");
          lineData.append(list[i].text+"\r\n");
          file.write(lineData.toUtf8());
          file.flush();
        }
        if(time_i == times-1)
        {
            QString lineData;
            lineData.append(list[sum].dk+",");
            lineData.append(list[sum].pk+",");
            lineData.append(list[sum].sn+",");
            lineData.append(list[sum].text);
            file.write(lineData.toUtf8());
            file.flush();
        }
    }
    return true;
}

void csvFileMange::saveTabDataToFile(QString fileName,bool noNeedTip)
{
    qDebug()<<__FUNCTION__;
    //打开文件
    QFile file(fileName);
    if(false == file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("文件保存失败"),QString::fromLocal8Bit("确认"));
        return;
    }
    else
    {
        QTextStream streamFileOut(&file);
        streamFileOut.setCodec("UTF-8");
        // 写入标题
        if( 0 == file.size() )
        {
            QStringList list;
            streamFileOut<<stackWidgetInfo.currentWidget->headerTextListToString(list)+"\r\n";
            streamFileOut.flush();
        }
        // 保存历史记录
        g_currentSavePath = fileName;
    }
    qDebug()<<"saveTablewidgetData";
    if(false == saveTablewidgetData(file))
    {
        file.close();
        QFile::remove(fileName);
        return;
    }
    if(noNeedTip)
    {
        QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("文件导出成功"),QString::fromLocal8Bit("确认"));
    }
    file.close();
}

//void csvFileMange::resizeEvent(QResizeEvent *event)
//{
//    qDebug()<<__FUNCTION__;
//    Q_UNUSED(event);
//    qDebug()<<"resize in csvfile";
//    qDebug()<<"objectName:"<<stackWidgetInfo.currentWidget->objectName();
//    qDebug()<<stackWidgetInfo.currentWidget->size();
//    qDebug()<<stackWidgetInfo.currentWidget->contentsRect();
////    stackWidgetInfo.currentWidget->horizontalHeader()->resize(stackWidgetInfo.currentWidget->contentsRect().width(),stackWidgetInfo.currentWidget->horizontalHeader()->height());
//}

/**************************************************************************************************************************************************************

                                                                            SQL

  **************************************************************************************************************************************************************/




void csvFileMange::saveFile()
{
    qDebug()<<__FUNCTION__;

    // 请选择保存文件的路径和输入文件名字
    QString fileName;
    if(g_currentSavePath == NULL || g_currentSavePath.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(NULL,QString::fromLocal8Bit("选取文件"),".","(*.csv)");
        if(fileName.isEmpty())
        {
            return;
        }
    }
    else
    {
        fileName = QFileDialog::getSaveFileName(NULL,QString::fromLocal8Bit("选取文件"),g_currentSavePath,"(*.csv)");
        if(fileName.isEmpty())
        {
            return;
        }
    }
    qDebug()<<"fileName:"<<fileName;
    saveTabDataToFile(fileName,true);


}


void csvFileMange::saveFile_clicked()
{
    qDebug()<<__FUNCTION__;
    // 请选择保存文件的路径和输入文件名字
    QString fileName;
    if(g_currentSavePath == NULL || g_currentSavePath.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(NULL,QString::fromLocal8Bit("选取文件"),".","(*.csv)");
        if(fileName.isEmpty())
        {
            return;
        }
    }
    else
    {
        fileName = g_currentSavePath;
    }
    qDebug()<<"fileName:"<<fileName;
    saveTabDataToFile(fileName,true);
}


/*******************************************************************************************
                                    界面退出
*******************************************************************************************/


void csvFileMange::closeEvent(QCloseEvent *event)
{
    qDebug()<<__FUNCTION__;
    Q_UNUSED(event);
    if(g_currentSavePath.isEmpty())
    {
        int rowCount = stackWidgetInfo.currentWidget->getCurrentRow();
        if(rowCount <= 0)
        {
            emit closeWidget();
            return;

        }
    }
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("是否需要保存文件"), QMessageBox::Yes | QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
    msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
    if( QMessageBox::Yes == msgBox->exec())
    {
        saveFile_clicked();
    }
    emit closeWidget();
}




void csvFileMange::stackWidgetChange(int mode)
{
    Q_UNUSED(mode);
//    if(NULL != stackWidgetInfo.rightWidget)
//    {

//    }
//    stackWidgetInfo.currentWidget = stackWidgetInfo.rightWidget;

}

void csvFileMange::setQTableWidget(QWidget *parentWidget,newTableWidget **newWidget)
{
    qDebug()<<__FUNCTION__;
    QObjectList list = parentWidget->children();
    foreach (QObject *var, list) {
        if(0 == QString(var->metaObject()->className()).compare("newTableWidget"))
        {
//            qDebug()<<"className:"<<var->metaObject()->className();
//            qDebug()<<"objectName:"<<var->objectName();
            ((newTableWidget *)var)->clearContents();
            ((newTableWidget *)var)->setRowCount(MAX_LINE);
            *newWidget = (newTableWidget *)var;
        }
    }



}

void csvFileMange::updateCurrentPage()
{
    qDebug()<<__FUNCTION__;
    if(ui->stackedWidget->currentIndex() == 1)
    {
        if(stackWidgetInfo.currentWidget->isHidden())
            stackWidgetInfo.currentWidget->show();
        ui->stackedWidget->setCurrentIndex(0);
        qDebug()<<"show stackedWidget 0";
    }
    else if(ui->stackedWidget->currentIndex() == 0)
    {
        stackWidgetInfo.currentWidget->show();
        ui->stackedWidget->setCurrentIndex(1);
        qDebug()<<"show stackedWidget 1";
    }
}


bool csvFileMange::runNextPageHandle(bool isShow)
{
    bool result = true;
    if(pageNavigator->getMaxPage() == pageNavigator->getCurrentPage())
    {
        if(pageNavigator->getCurrentPage()*MAX_LINE >= 50000)
        {
            QMessageBox::information(0,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("已达到最大行数"),QString::fromLocal8Bit("确认"));
            stackWidgetInfo.currentWidget->inputWidgetHide();
            result= false;
        }
        else
        {
            pageNavigator->setMaxPage(pageNavigator->getMaxPage()+1);
        }
    }
    if(result)
    {
        pageNavigator->setCurrentPage(pageNavigator->getCurrentPage()+1);
        changeWidget(isShow);
    }
    return result;
}

void csvFileMange::changeWidget(bool isShow)
{
    qDebug()<<__FUNCTION__;
    if(ui->stackedWidget->currentIndex() == 1)
    {
      setQTableWidget(ui->page,&stackWidgetInfo.currentWidget);
      stackWidgetInfo.currentWidget->hide();
    }
    else if(ui->stackedWidget->currentIndex() == 0)
    {
      setQTableWidget(ui->page_2,&stackWidgetInfo.currentWidget);
      stackWidgetInfo.currentWidget->hide();
    }
    qDebug()<<"isShow:"<<isShow;
    QScrollBar *vScrollbar = stackWidgetInfo.currentWidget->verticalScrollBar();
    vScrollbar->setSliderPosition(0);
    if(isShow)
        updateCurrentPage();
}



/***************************************************************************************************************************************************************************
                                                                sql
***************************************************************************************************************************************************************************/
void csvFileMange::openSqlDataBase()
{
    // 新建/打开 数据库
    userInfoSql = new mySqlite();
    ((mySqlite *)userInfoSql)->createSqlDataBase("tools_sqlite3","device_info");
    // 新建/打开表
    createSqlTable();

    // 写入数据
    initWidget();
}

void csvFileMange::createSqlTable()
{
    qDebug()<<__FUNCTION__;
    if(NULL !=userInfoSql)
    {
        ((mySqlite *)userInfoSql)->createSqlQuery("device_userInfo", \
            "create table device_userInfo (idex integer UNIQUE,dk TEXT(32) UNIQUE,pk TEXT(16),sn TEXT(16) UNIQUE,text TEXT(100),ver TEXT(10),PRIMARY KEY(idex AUTOINCREMENT))");
        // 创建索引
        // ((mySqlite *)userInfoSql)->execSqlDataList("CREATE INDEX index_name ON device_userInfo (dk);");
    }
}

void csvFileMange::initWidget()
{
    qDebug()<<__FUNCTION__;
    QStringList list = ((mySqlite *)userInfoSql)->execSqlDataList("SELECT count(idex) AS number FROM device_userInfo;");
    int counts;
    if(list.size() > 0)
        counts=QString(list.at(0)).toInt();
    else
    {
        qDebug()<<"error";
        counts = 0;
    }
    qDebug()<<"counts:"<<counts;
//    if(counts > 0){
    int integerPage = counts/MAX_LINE;
    int remainingPage = counts%MAX_LINE;
    int currentPages = integerPage+(remainingPage>0?1:0);
    int maxPages = (integerPage+1) > 25?integerPage:(integerPage+1);
    pageNavigator->setMaxPage(maxPages);

    if(currentPages > 1)
    {
        pageNavigator->setCurrentPage(maxPages,true);
    }
    else
    {
        pageNavigator_currentPageChanged(1);
    }
    if(useFirstAPP == false)
    {
        QTimer *showTiptimer = new QTimer();
        connect(showTiptimer,&QTimer::timeout,this,[=](){
            QMessageBox *box = new QMessageBox(QMessageBox::Information,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("鼠标双击%1或%2或%3列后，方可进行扫码").arg(horizontalHeaderText.at(0)).arg(horizontalHeaderText.at(1)).arg(horizontalHeaderText.at(2)),QMessageBox::Yes);
            box->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
            box->exec();
        });
        showTiptimer->setSingleShot(true);
        showTiptimer->start(200);
        useFirstAPP = true;
    }
}

QString csvFileMange::createInsertSqlWord(structInfo_t info)
{
    // 保存设备信息
    QString insertInfo = QString("insert into device_userInfo(dk,pk,sn,text,ver) values('%1','%2','%3','%4','%5');")
            .arg(info.dk)
            .arg(info.pk)
            .arg(info.sn)
            .arg(info.text.isEmpty()?"":info.text)
            .arg(info.ver.isEmpty()?"":info.ver);
    return insertInfo;
}

int csvFileMange::insertToSql(structInfo_t structInfo)
{
    qDebug()<<__FUNCTION__;
    int errorColumn = 0;
    if(false == ((mySqlite *)userInfoSql)->insertSqlData(createInsertSqlWord(structInfo),&errorColumn))
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("历史存在此DK:%1或SN:%2,是否覆盖"), QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
        msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
        if(QMessageBox::Yes == msgBox->exec())
        {
            // 保存设备信息
            QString updateInfo;
            if(errorColumn == TAB_DEVICEKEY_COLUMN)
            {
                if(structInfo.text.isEmpty())
                {
                    updateInfo = QString("update device_userInfo set pk='%2',sn='%3' where dk='%1'") //,ver='%4'
                            .arg(structInfo.dk)
                            .arg(structInfo.pk)
                            .arg(structInfo.sn);
//                            .arg(structInfo.ver);
                }
                else
                {
                    updateInfo = QString("update device_userInfo set pk='%2',sn='%3',text='%4' where dk='%1'") // ,ver='%5'
                            .arg(structInfo.dk)
                            .arg(structInfo.pk)
                            .arg(structInfo.sn)
                            .arg(structInfo.text);
//                            .arg(structInfo.ver);
                }
            }
            else if(errorColumn == TAB_DEVICESN_COLUMN)
            {
                if(structInfo.text.isEmpty())
                {
                    updateInfo = QString("update device_userInfo set dk='%2',pk='%3' where sn='%1'") //,ver='%4'
                            .arg(structInfo.sn)
                            .arg(structInfo.dk)
                            .arg(structInfo.pk);
//                            .arg(structInfo.ver);
                }
                else
                {
                    updateInfo = QString("update device_userInfo set dk='%2',pk='%3',text='%4' where sn='%1'") //,ver='%5'
                            .arg(structInfo.sn)
                            .arg(structInfo.dk)
                            .arg(structInfo.pk)
                            .arg(structInfo.text);
//                            .arg(structInfo.ver);
                }
            }
            ((mySqlite *)userInfoSql)->modifySqlData(updateInfo);
            return errorColumn;
        }
        return -1;
    }
    return -2;
}

bool csvFileMange::insertToSql(QString info)
{
    qDebug()<<__FUNCTION__;
    int errorColumn = 0;
    return((mySqlite *)userInfoSql)->insertSqlData(info,&errorColumn);
}


void csvFileMange::delFromSql(QList<QString> structInfo)
{
    qDebug()<<__FUNCTION__;
    if(!structInfo.isEmpty())
    {
        ((mySqlite *)userInfoSql)->batchDelSqlData(structInfo);
    }
}


void csvFileMange::writeToTabHandle(structInfo_t info,int row, bool &flag)
{
    qDebug()<<__FUNCTION__;
    // 这里后期使用多线程去优化 "QTableViem +QStandardltemModel"方案未测试过
    int result = insertToSql(info);
    qDebug()<<"result:"<<result;
    if(-1 == result)
    {
        stackWidgetInfo.currentWidget->removeRow(row);
        stackWidgetInfo.currentWidget->setTabCurrentRow(row);
        stackWidgetInfo.currentWidget->insertRow(MAX_LINE-1);
    }
    else if(-2 == result)
    {
        if(row >= (MAX_LINE-1))
        {
            if(false ==runNextPageHandle(true))
                flag = false;
        }
        else
        {
            stackWidgetInfo.currentWidget->setTabCurrentRow(row+1);
        }

    }
    else
    {
        // 这里需要做替换
        stackWidgetInfo.currentWidget->removeRow(row);
        stackWidgetInfo.currentWidget->insertRow(MAX_LINE-1);
        stackWidgetInfo.currentWidget->setCurrentCell(row,TAB_DEVICEKEY_COLUMN);
        int idex = -1;
        // 这里需要使用搜索
        QStringList list = ((mySqlite *)userInfoSql)->findSqlDataList("select ROW_NUMBER() over(order by idex) as idex,dk,sn from device_userInfo;");
        if(list.size() > 0)
        {
            QString findData;
            if(TAB_DEVICEKEY_COLUMN== result)
            {
                findData = info.dk;
            }
            else
                findData = info.sn;
            if(QString(list.at(0)).contains(','))
            {
                qDebug()<<list.at(0);
                if(TAB_DEVICEKEY_COLUMN== result)
                {
                    for(int i=0;i<list.size();i++)
                    {
                        if(QString(list.at(i)).split(",").at(1)==findData)
                        {
                            idex = QString(QString(list.at(i)).split(',').at(0)).toInt();
                        }
                    }
                }
                else
                {
                    for(int i=0;i<list.size();i++)
                    {
                        if(QString(list.at(i)).split(",").at(2)==findData)
                        {
                            idex = QString(QString(list.at(i)).split(',').at(0)).toInt();
                        }
                    }
                }
            }
        }

        if(idex < 0)
        {
            QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("程序异常导致无法定位，请联系开发人员"),QString::fromLocal8Bit("确认"));
            return;
        }
        int page = idex/MAX_LINE + (idex%MAX_LINE>0?1:0);
        int line = ((idex%MAX_LINE)>0)?(idex%MAX_LINE)-1:((idex>0)?MAX_LINE-1:0);
        qDebug()<<"line:"<<line;
        if(page == pageNavigator->getCurrentPage())
        {
            QTableWidgetItem *itemDk = new QTableWidgetItem(info.dk);
            stackWidgetInfo.currentWidget->setItem(line,TAB_DEVICEKEY_COLUMN,itemDk);
            QTableWidgetItem *itemPk = new QTableWidgetItem(info.pk);
            stackWidgetInfo.currentWidget->setItem(line,TAB_PRODUCTKEY_COLUMN,itemPk);
            QTableWidgetItem *itemSn = new QTableWidgetItem(info.sn);
            stackWidgetInfo.currentWidget->setItem(line,TAB_DEVICESN_COLUMN,itemSn);
            if(!info.text.isEmpty())
            {
                QTableWidgetItem *itemText = new QTableWidgetItem(info.text);
                stackWidgetInfo.currentWidget->setItem(line,TAB_DEVICENAME_COLUMN,itemText);
            }
        }
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("是否需要跳转至替换行"), QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::No);
        msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
        msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
        if( QMessageBox::Yes == msgBox->exec())
        {
            stackWidgetInfo.currentWidget->inputWidgetHide();
            if(page == pageNavigator->getCurrentPage())
            {
                if(line == 0)
                {
                    QScrollBar *vScrollbar = stackWidgetInfo.currentWidget->verticalScrollBar();
                    vScrollbar->setSliderPosition(0);
                }
                stackWidgetInfo.currentWidget->selectRow(line);
            }
            else
            {
                pageNavigator_currentPageChanged(page);
                if(line == 0)
                {
                    QScrollBar *vScrollbar = stackWidgetInfo.currentWidget->verticalScrollBar();
                    vScrollbar->setSliderPosition(0);
                }
                stackWidgetInfo.currentWidget->selectRow(line);
            }

            qDebug()<<"hide";
            flag = false;
        }

    }
}

void csvFileMange::handleResults(QVector<structInfo_t>data)
{

}

void csvFileMange::pageNavigator_currentPageChanged(int page)
{
    qDebug()<<__FUNCTION__<<page;
    // 这里的SQL尚未可行
    QStringList list = ((mySqlite *)userInfoSql)->execSqlDataList("SELECT count(idex) AS number FROM device_userInfo;");
    int lastShowBegin = 0;
    int lastShowOffset = 0;
    int counts = QString(list.at(0)).toInt();
    lastShowBegin   = ((page-1)*MAX_LINE==0)?0:(page-1)*MAX_LINE;
    lastShowOffset  = (page*MAX_LINE)>counts?counts-((page-1)*MAX_LINE):MAX_LINE;
    qDebug()<<"lastShowBegin:"<<lastShowBegin<<","<<lastShowOffset;
    int maxPage = counts/MAX_LINE+(counts%MAX_LINE?1:0);
    if(maxPage < pageNavigator->getMaxPage())
    {
        pageNavigator->setMaxPage(maxPage);
    }
    QVector<structInfo_t>info;
    if(lastShowBegin >= counts)
    {

    }
    else
    {
        info = ((mySqlite *)userInfoSql)->findSqlAllList(QString("select * from device_userInfo limit '%1','%2'").arg(lastShowBegin).arg(lastShowOffset));
    }

    // 切换widget
    changeWidget(false);
    stackWidgetInfo.currentWidget->setTabCurrentRow(0,false);
    int i = 0;
    for(;i<info.size();i++)
    {
        stackWidgetInfo.currentWidget->tableWidgetAddRow(info[i],i,0,false);
    }
    QScrollBar *vScrollbar = stackWidgetInfo.currentWidget->verticalScrollBar();
    vScrollbar->setSliderPosition(i);
    pageNavigator->setCurrentPage(page);
    // 显示出来
    updateCurrentPage();
}

void csvFileMange::delTabWiagetData(QList<QString> structInfo)
{
    qDebug()<<__FUNCTION__;
    delFromSql(structInfo);
    QTimer *timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(timeOutHandle()));
    timer->setSingleShot(true);
    timer->start(500);
            //    emit updateTableWidget(pageNavigator->getCurrentPage());
}

void csvFileMange::timeOutHandle()
{
    pageNavigator_currentPageChanged(pageNavigator->getCurrentPage());
}

void csvFileMange::changePercentHandle(int percent)
{
    qDebug()<<"here";
    if(NULL != progressWater)
    {
        ((ProgressWater *)progressWater)->setValue(percent);
    }
    progressBar->update();
}

void csvFileMange::nonBlockingSleep(int sectime)
{
    QTime dieTime = QTime::currentTime().addMSecs(sectime);

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, g_maxTime);
    }
}

void csvFileMange::createThread(int fileSize)
{
    Q_UNUSED(fileSize);
    myThead *workerThread = new myThead();
    connect(workerThread, &myThead::resultReady, this, &csvFileMange::handleResults);
    connect(workerThread, &myThead::changePercent, this, &csvFileMange::changePercentHandle);
    connect(workerThread, &myThead::finished, workerThread, &QObject::deleteLater);
    workerThread->start();
}


