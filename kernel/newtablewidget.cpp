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
#include "newtablewidget.h"
#include <QHeaderView>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <qDebug>
#include <QResizeEvent>
#include <QMenu>
#include <QItemDelegate>

bool handleInputDataMutex;
QStringList horizontalHeaderText = {QString::fromLocal8Bit("*产品ProductKey"),QString::fromLocal8Bit("*设备DeviceKey"),QString::fromLocal8Bit("*设备SN"),QString::fromLocal8Bit("设备名称")};
enum
{
    CHECK_NOMAL_MODE=0,
    CHECK_SAVE_MODE,
};

class ReadOnlyDelegate: public QItemDelegate
{

public:
    ReadOnlyDelegate(QWidget *parent = NULL):QItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
const QModelIndex &index) const override //final
    {
        Q_UNUSED(parent)
        Q_UNUSED(option)
        Q_UNUSED(index)
        return NULL;
    }
};

newTableWidget::newTableWidget(QWidget *parent):QTableWidget(parent)
{
 this->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    this->setRowCount(MIN_LINE);
    this->setRowCount(MAX_LINE);
//    this->setSelectionMode(QAbstractItemView::ContiguousSelection);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(this,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(itemClickefHandle(int,int)));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}");
    createRightMenu();
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tableWidget_customContextMenuRequested(QPoint)));
//    g_timerCheck->setInterval(50);
//    connect(g_timerCheck,SIGNAL(timeout()),this,SLOT(timerCheckTimeOut()));
//    connect(this,SIGNAL(sendHandleDataMess(QString)),this,SLOT(dataHandle(QString)));
}

newTableWidget::newTableWidget(QStringList headItems, QWidget *parent):QTableWidget(parent)
{
    this->setColumnCount(headItems.size());
    this->setHorizontalHeaderLabels(headItems);
//    this->setRowCount(MIN_LINE);
    this->setRowCount(MAX_LINE);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(this,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(itemClickefHandle(int,int)));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}");
    createRightMenu();
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tableWidget_customContextMenuRequested(QPoint)));
//    g_timerCheck->setInterval(50);
//    connect(g_timerCheck,SIGNAL(timeout()),this,SLOT(timerCheckTimeOut()));
//    connect(this,SIGNAL(sendHandleDataMess(QString)),this,SLOT(dataHandle(QString)));
//    setSelectionMode(QAbstractItemView::ExtendedSelection);

}

newTableWidget::~newTableWidget()
{
    delete rightMenu;
}

void newTableWidget::inputWidgetCreate(QString data)
{
    if(inputWidget != NULL)
    {
        if(inputWidget->isActiveWindow())
            inputWidget->close();
        inputWidget->deleteLater();
        inputWidget = NULL;
    }
    inputWidget = new QWidget();
    QGridLayout *griLyout = new QGridLayout();
    QHBoxLayout *vLayout = new QHBoxLayout();
    QLineEdit *lineEdit;
    if(data != NULL)
        lineEdit = new QLineEdit(data);
    else
        lineEdit = new QLineEdit();

    lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(lineEdit,SIGNAL(editingFinished()),this,SLOT(lineEdit_editFinished()));
    QPushButton *button = new QPushButton(QString::fromLocal8Bit("退出"));
    connect(button,SIGNAL(clicked()),this,SLOT(returnInputEvent()));
    vLayout->addWidget(lineEdit);
    vLayout->addWidget(button);
    QLabel *label = new QLabel(QString::fromLocal8Bit("请扫描设备二维码"));
    label->setFont(QFont("宋体",10,QFont::Bold));
    griLyout->addWidget(label,0,0,Qt::AlignBottom| Qt::AlignLeft);
    griLyout->addLayout(vLayout,1,0,Qt::AlignTop| Qt::AlignHCenter);
    griLyout->setRowStretch(0,1);
    griLyout->setRowStretch(1,1);

    inputWidget->setLayout(griLyout);
    inputWidget->setWindowFlag(Qt::FramelessWindowHint);
    inputWidget->setWindowFlags(inputWidget->windowFlags()  | Qt::Dialog);
//    inputWidget->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    inputWidget->setAutoFillBackground(true);
    inputWidget->setWindowOpacity(0.7);
    inputWidget->setAttribute(Qt::WA_QuitOnClose,false);
    inputWidget->setGeometry(QWidget::mapToGlobal(this->pos()).x()+this->width()/4,QWidget::mapToGlobal(this->pos()).y()+this->height()/3,this->width()/2,this->height()/3);
    inputWidget->setWindowModality(Qt::ApplicationModal);
}

void newTableWidget::inputWidgetChangeMess()
{
//        if(NULL != inputWidget && inputWidget->isActiveWindow())
//        {
//            if(TAB_DEVICEKEY_COLUMN == currentColumn )
//            {
//                inputWidgetSetMess(QString::fromLocal8Bit("请扫描模组二维码"),NULL,NULL);
//            }
//            else if(TAB_DEVICEKEY_COLUMN == currentColumn )
//            {
//                inputWidgetSetMess(QString::fromLocal8Bit("请扫描设备二维码"),NULL,NULL);
//            }
//        }
}

void newTableWidget::itemClickefHandle(int row, int column)
{
    if(column != 3)
    {
        if(getCurrentRow() != row)
        {
            QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("请按照行顺序输入，不能跳行输入"),QString::fromLocal8Bit("确认"));
        }
        else
        {
            if(NULL == this->item(row,column))
            {
                if(column == 0)
                {
                    inputWidgetSetMess(QString::fromLocal8Bit("请扫描模组二维码"),NULL,NULL);
                }
                else
                {
                    if(NULL == this->item(row,0) || this->item(row,0)->text().isEmpty())
                    {
                        QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("请先点击%1列进行扫描").arg(horizontalHeaderText.at(0)),QString::fromLocal8Bit("确认"));
                        return;
                    }
                    else
                        inputWidgetSetMess(QString::fromLocal8Bit("请扫描设备二维码"),NULL,NULL);
                }
            }
            else
            {
                if(column == 0)
                {
                    inputWidgetSetMess(QString::fromLocal8Bit("请扫描模组二维码"),this->item(row,column)->text(),NULL);
                }
                else
                {
                    if(NULL == this->item(row,0) || this->item(row,0)->text().isEmpty())
                    {
                        QMessageBox::critical(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("请先点击%1列进行扫描").arg(horizontalHeaderText.at(0)),QString::fromLocal8Bit("确认"));
                        return;
                    }
                    else
                        inputWidgetSetMess(QString::fromLocal8Bit("请扫描设备二维码"),this->item(row,column)->text(),NULL);
                }
            }
            QPoint point ;
            point.setX(mapToGlobal(this->pos()).x() +this->width()/4);
            point.setY(mapToGlobal(this->pos()).y() +this->height()/3);
            inputWidget->move(point);
            inputWidget->show();
            QObjectList list = inputWidget->children();
            foreach (QObject *var, list) {
                if(0 == QString(var->metaObject()->className()).compare("QLineEdit"))
                {
                    ((QLineEdit *)var)->setFocus();
                }
            }
        }
    }
}

void newTableWidget::lineEdit_editFinished()
{
    QLineEdit *sigBtn = qobject_cast<QLineEdit*>(sender());
    if(sigBtn->hasFocus() && !sigBtn->text().isEmpty())
    {
//        g_inputData.append(sigBtn->text());
//        if(!g_timerCheck->isActive())
//        {
//            g_timerCheck->start();
//        }
        qDebug()<<"lineEdit data:"<<sigBtn->text();
        if(handleInputDataMutex == false)
        {
            dataHandle(sigBtn->text(),this->currentColumn());
            sigBtn->clear();
        }
    }
}

void newTableWidget::returnInputEvent()
{
    ((QLineEdit *)inputWidget->children().at(2))->clear();
    inputWidget->hide();
}

void newTableWidget::inputWidgetSetMess(QString LabelData, QString LineEditData, QString PushButtonData,int colorChange)
{
#ifdef ERROR_LEVEL
    qDebug()<<__FUNCTION__;
#endif
    if(NULL == inputWidget)
    {
        inputWidgetCreate(NULL);
    }
    QPalette pal=inputWidget->palette();

    //这里需要注意，输入性质的控件(如这里的lineEdit)用base(),如果是按钮用button()，不然获取不到
    if(colorChange == 0)
    {
        inputWidget->setStyleSheet("background-color:rgba(245,245,245,1);");
        inputWidget->show();
    }
    else if(colorChange == 1)
    {
        inputWidget->setStyleSheet("background-color:rgba(240,128,128,1);");
    }
    QObjectList list = inputWidget->children();
    if(!LabelData.isEmpty())
    {
        foreach (QObject *var, list) {
            if(0 == QString(var->metaObject()->className()).compare("QLabel"))
            {
                ((QLabel *)var)->setText(LabelData);
            }
        }
    }
    if(!LineEditData.isEmpty())
    {
        foreach (QObject *var, list) {
            if(0 == QString(var->metaObject()->className()).compare("QLineEdit"))
            {
                ((QLineEdit *)var)->setText(LineEditData);
            }
        }
    }
    if(!PushButtonData.isEmpty())
    {
        foreach (QObject *var, list) {
            if(0 == QString(var->metaObject()->className()).compare("QPushButton"))
            {
                ((QPushButton *)var)->setText(LineEditData);
            }
        }
    }
}

void newTableWidget::inputWidgetHide()
{
    if(inputWidget !=NULL && inputWidget->isActiveWindow())
    {
        // returnInputEvent();
        inputWidget->hide();
    }
    else
    {
        qCritical()<<"error hide";
    }
}

void newTableWidget::timerMessageBox(QMessageBox *mess,int times)
{
    QTimer::singleShot(times,mess,SLOT(accept())); //也可将accept改为close
    mess->exec();
}


bool newTableWidget::dataHandle(QString data,int currentColumn)
{
#ifdef ERROR_LEVEL
    qDebug()<<__FUNCTION__<<data;
#endif
    handleInputDataMutex = true;
    bool result = false;
    int row = this->currentRow();
    bool checkResult = false;

    if(TAB_DEVICEKEY_COLUMN == currentColumn)
    {
        QString imeiString;
        if(data.contains(';'))
        {
            QStringList list = data.split(';');
            imeiString=list[0];
        }
        else
            imeiString = data;
        if((imeiString.size() >= 4) && (imeiString.size() <= 32))
        {
            QByteArray byteArray = imeiString.toLatin1();
            const char *s = byteArray.data();
            while(*s)
            {
                if((*s>='0' && *s<='9'));
                else
                {
                    qDebug()<<"error:"<<*s;
                    inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描模组二维码"),NULL,NULL,1);
                    goto exit;
                }
                s++;
            }
            QTableWidgetItem *itemImei = new QTableWidgetItem(imeiString);
            this->setItem(row,TAB_DEVICEKEY_COLUMN,itemImei);
            this->setCurrentCell(row,TAB_DEVICESN_COLUMN);
            checkResult = true;
        }
        else
        {
            inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描模组二维码"),NULL,NULL,1);
            goto exit;
        }
    }
    else if(TAB_DEVICESN_COLUMN == currentColumn || TAB_PRODUCTKEY_COLUMN == currentColumn)
    {
        if(data.contains('&'))
        {
            QStringList list = data.split('&');
            QString pkString;
            QString snString;
            QString verString;
            int size = list.size();
            for(int i = 0;i<size;i++)
            {
                if(QString(list[i]).contains("pk="))
                {
                    pkString = QString(list[i]).split("pk=").at(1);
                }
                else if(QString(list[i]).contains("sn="))
                {
                    snString = QString(list[i]).split("sn=").at(1);
                }
                else if(QString(list[i]).contains("v="))
                {
                    verString = QString(list[i]).split("v=").at(1);
                }
            }

            if(!pkString.isEmpty() && pkString.size() == 6 && !snString.isEmpty() && snString.size() <=64 && !verString.isEmpty())
            {
                QByteArray byteArray = pkString.toLatin1();
                const char *s = byteArray.data();
                while(*s)
                {
                    if((*s>='0' && *s<='9') || (*s>='a' && *s<='z')|| (*s>='A' && *s<='Z'));
                    else
                    {
                        qDebug()<<"error:"<<*s;
                        inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描模组二维码"),NULL,NULL,1);
                        goto exit;
                    }
                    s++;
                }
                byteArray = snString.toLatin1();
                const char *s2 = byteArray.data();
                while(*s2)
                {
                    if((*s2>='0' && *s2<='9') || (*s2>='a' && *s2<='z')|| (*s2>='A' && *s2<='Z'));
                    else
                    {
                        qDebug()<<"error:"<<*s2;
                        inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描模组二维码"),NULL,NULL,1);
                        goto exit;
                    }
                    s2++;
                }
                QTableWidgetItem *itemPk = new QTableWidgetItem(pkString);
                this->setItem(row,TAB_PRODUCTKEY_COLUMN,itemPk);
                QTableWidgetItem *itemSn = new QTableWidgetItem(snString);
                this->setItem(row,TAB_DEVICESN_COLUMN,itemSn);
                checkResult = true;
            }
            else
            {
                inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描设备二维码"),NULL,NULL,1);
                goto exit;
            }
        }
        else
        {
            inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描设备二维码"),NULL,NULL,1);
            goto exit;
        }
    }
    else
    {
        inputWidgetSetMess(QString::fromLocal8Bit("内容错误，请重新扫描二维码"),NULL,NULL,1);
        goto exit;
    }
    if(checkResult)
    {
        bool nextLineFlag = false;
        bool resultFlag = true;
        if(NULL != this->item(row,TAB_DEVICEKEY_COLUMN) && NULL != this->item(row,TAB_PRODUCTKEY_COLUMN) && NULL != this->item(row,TAB_DEVICESN_COLUMN)
                && !this->item(row,TAB_DEVICEKEY_COLUMN)->text().isEmpty() && !this->item(row,TAB_PRODUCTKEY_COLUMN)->text().isEmpty() && !this->item(row,TAB_DEVICESN_COLUMN)->text().isEmpty())
        {

            structInfo_t info;
            info.dk = this->item(row,TAB_DEVICEKEY_COLUMN)->text();
            info.pk = this->item(row,TAB_PRODUCTKEY_COLUMN)->text();
            info.sn = this->item(row,TAB_DEVICESN_COLUMN)->text();
            if(NULL != this->item(row,TAB_DEVICENAME_COLUMN))
            {
                info.text = this->item(row,TAB_DEVICENAME_COLUMN)->text();
            }
            emit writeToTabHandle(info,row,resultFlag);
            qDebug()<<"resultFlag:"<<resultFlag;
            nextLineFlag = true;
        }
        if(resultFlag)
        {
            if(nextLineFlag)
            {
                inputWidgetSetMess(QString::fromLocal8Bit("请扫描模组二维码"),NULL,NULL);
            }
            else if(TAB_DEVICEKEY_COLUMN == currentColumn )
            {
                inputWidgetSetMess(QString::fromLocal8Bit("请扫描设备二维码"),NULL,NULL);
            }
        }
        result = true;
    }
    exit:
        handleInputDataMutex = false;
        return result;
}

void newTableWidget::writeToTabwidget(QString data,int row,int colume)
{
    QTableWidgetItem *item = new QTableWidgetItem(data);
    this->setItem(row,colume,item);
}

void newTableWidget::tableWidgetAddRow(structInfo_t structInfo, int row, int mode,bool changePageFlag)
{
    if(changePageFlag)
    {
        if(row >= (MAX_LINE-1))
        {
            emit runNextPage();
        }
    }
    writeToTabwidget(structInfo.dk,row,TAB_DEVICEKEY_COLUMN);
    writeToTabwidget(structInfo.pk,row,TAB_PRODUCTKEY_COLUMN);
    writeToTabwidget(structInfo.sn,row,TAB_DEVICESN_COLUMN);
    if(!structInfo.text.isEmpty())
    {
        writeToTabwidget(structInfo.text,row,TAB_DEVICENAME_COLUMN);
    }
    if(mode == 1)
    {
        bool flag;
        emit writeToTabHandle(structInfo,row,flag);
    }
    else
    {
        g_currentRow++;
    }

}

QString newTableWidget::headerTextListToString(QStringList info)
{
    QString headerName;
    if(info.isEmpty())
    {
        info = horizontalHeaderText;
    }
    for(int i = 0;i<info.size();i++)
    {
        headerName.append(info.at(i)+",");
    }
    headerName.append(QString::fromLocal8Bit("证书指纹"));
    return headerName;

}


/*******************************************************************************************
                                    数据检测
*******************************************************************************************/

int newTableWidget::checkTableWidgetData()
{
    int rowCount = getCurrentRow()+30;
    bool exsitData = false;
    // 获取实际使用行数
    do
    {
        rowCount--;
        if(NULL == this->item(rowCount,0) || NULL == this->item(rowCount,1) || NULL == this->item(rowCount,2) || NULL == this->item(rowCount,3))
        {
            continue;
        }
        // 不全为空，则为最后一行
        if(15 != checkItemValue(rowCount,CHECK_NOMAL_MODE))
        {
            exsitData = true;
            break;
        }
    }
    while(rowCount);
    qDebug()<<"rowCount:"<<rowCount;
    if(true == exsitData)
    {
        return rowCount+1;
    }
    return 0;
}

void newTableWidget::createRightMenu()
{
    //创建菜单对象
       rightMenu = new QMenu(this);
       QAction *pDelTask = new QAction(QString::fromLocal8Bit("删除行"), this);
       rightMenu->addAction(pDelTask);
    //连接鼠标右键点击信号
      connect(pDelTask, SIGNAL(triggered()), this, SLOT(onTaskBoxContextMenuEvent()));
}

int newTableWidget::checkItemValue(int row,int mode)
{
    int value = 0;
    QString devKey = this->item(row,0)->text();
    QString productKey = this->item(row,1)->text();
    QString devSN = this->item(row,2)->text();
    QString devName = this->item(row,3)->text();
    QString data;
    if(devKey.isEmpty())
    {
        value |= 1;
        data.append(horizontalHeaderText.at(0));
    }
    if(productKey.isEmpty())
    {
        value |= 2;
        if(data.size()>0)
            data.append(",");
        data.append(horizontalHeaderText.at(1));
    }
    if(devSN.isEmpty())
    {
        value |= 4;
        if(data.size()>0)
            data.append(",");
        data.append(horizontalHeaderText.at(2));
    }
    if(devName.isEmpty())
    {
        value |= 8;
        if(data.size()>0)
            data.append(",");
        data.append(horizontalHeaderText.at(3));
    }
    if(0 != value)
    {
        if(CHECK_SAVE_MODE == mode)
        {
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("当前第%1行：%2列为空,是否继续保存").arg(row).arg(data), QMessageBox::Yes | QMessageBox::No);
            msgBox->setDefaultButton(QMessageBox::Yes);
            msgBox->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("确认"));
            msgBox->button(QMessageBox::No)->setText(QString::fromLocal8Bit("取消"));
            if( QMessageBox::No == msgBox->exec())
            {
                return -1;
            }
        }
    }
    return value;
}




int newTableWidget::getCurrentRow()
{
    return g_currentRow >= MAX_LINE?MAX_LINE:g_currentRow;
}

void newTableWidget::setTabCurrentRow(int row,bool skipMode)
{
#ifdef ERROR_LEVEL
    qDebug()<<__FUNCTION__<<row;
#endif
    g_currentRow = row;
    if(row >= (MAX_LINE-1))
    {
        emit runNextPage();
    }
    else
    {
        if(skipMode)
            this->setCurrentCell(row,TAB_DEVICEKEY_COLUMN);
    }
}


bool firstUseResizeEvent = true;
void newTableWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(firstUseResizeEvent)
    {
        inputWidgetCreate(NULL);
        firstUseResizeEvent = false;
    }

    this->horizontalHeader()->resize(this->contentsRect().width(),this->horizontalHeader()->height());
}

void newTableWidget::tableWidget_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    qDebug()<<__FUNCTION__;
    bool isEmpty = true;
    for(int i=0;i<this->columnCount();i++)
    {
        if(NULL != this->item(this->currentRow(),i))
        {
             isEmpty = false;
        }
    }
    if(isEmpty)
    {
        return;
    }
//    this->selectRow(this->currentRow());

    //在鼠标右键点击的地方显示菜单
    qDebug()<<"cursor().pos():"<<cursor().pos();
    if(NULL == rightMenu)
    {
        qDebug()<<"ERROR";
    }
    else
        rightMenu->exec(cursor().pos());


}


void newTableWidget::onTaskBoxContextMenuEvent()
{

    QAction *pEven = qobject_cast<QAction *>(this->sender()); //this->sender()就是发信号者 QAction

    //获取发送信息类型 1:添加行 2:删除行
    int iType = pEven->data().toInt();
    qDebug()<<QString::number(iType);
    switch (iType)
    {
//        case 2:
//        {
//            // 增加行
//            int currentRow = this->currentRow();
//            qDebug()<<"currentRow:"<<currentRow;
//            if(currentRow < MAX_LINE) // 最后一行不允许操作
//            {
//                this->insertRow(currentRow);
//                // 删除最后一行
//                if((this->rowCount() +1) >= MAX_LINE)
//                {
//                    qDebug()<<"remove row:"<<this->rowCount();
//                    this->removeRow(this->rowCount()-1);
//                }
//                else
//                    qDebug()<<"current row:"<<this->rowCount() +1;
//                // 不增加到sqlite
//            }
//            break;
//        }
//        case 3:
//        {
//        // 增加行
//            int currentRow = this->currentRow();
//            if(currentRow+20 < MAX_LINE)
//            {
//                for(int i =0;i<20;i++)
//                    this->insertRow(currentRow);
//                if((this->rowCount() +20) >= MAX_LINE)
//                {
//                    int count = 20;
//                    while(count)
//                    {
//                        this->removeRow(this->rowCount()-1);
//                    }
//                }

//            }
//            else
//            {

//            }
//            // 不增加到sqlite

//            break;
//        }
        case 0:
        {
        // 删除行

        QModelIndexList selected = this->selectionModel()->selectedRows();
        QList<int>selectRowNum;
        for(int i = 0;i<selected.count();i++)
            selectRowNum.append(selected[i].row());
        qSort(selectRowNum.begin(),selectRowNum.end(), qGreater<int>());
        if(0 == selectRowNum.size())
        {
            selectRowNum.append(this->currentRow());
        }
        qDebug()<<"delete row:"<<selectRowNum;
        QList<QString>infoList;
        for(int i =0;i<selectRowNum.count();i++)
        {
            qDebug()<<"info.dk:"<<selectRowNum[i];
            infoList.append(this->item(selectRowNum[i],TAB_DEVICEKEY_COLUMN)->text());
            this->removeRow(selectRowNum[i]);
            this->insertRow(this->rowCount());
        }
        emit deleteSqlInfo(infoList);

//            int currentRow = this->currentRow();
//            structInfo_t info;
//            info.dk = this->item(currentRow,TAB_DEVICEKEY_COLUMN)->text();
//            this->removeRow(this->currentRow());
//            this->insertRow(this->rowCount());
//            // 从sqlite删除
//            emit deleteSqlInfo(info);
            break;
        }
        default:
            break;
    }
}

void newTableWidget::timerCheckTimeOut()
{
    if(false == handleInputDataMutex)
    {
        dataHandle(g_inputData,this->currentColumn());
    }
}
