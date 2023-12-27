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
#ifndef NEWTABLEWIDGET_H
#define NEWTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QMessageBox>
#include "mysqlite.h"

#define MAX_LINE 2000
#define MIN_LINE 20
enum
{
    TAB_DEVICEKEY_COLUMN = 0,
    TAB_PRODUCTKEY_COLUMN,
    TAB_DEVICESN_COLUMN,
    TAB_DEVICENAME_COLUMN,
};
extern QStringList horizontalHeaderText;

class newTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit newTableWidget(QWidget *parent = nullptr);
    explicit newTableWidget(QStringList headItems, QWidget *parent = nullptr);
    ~newTableWidget();
    bool dataHandle(QString data,int currentColumn);
    int getCurrentRow();
    int checkItemValue(int row,int mode);
    void inputWidgetCreate(QString data);
    void inputWidgetChangeMess();
    void inputWidgetSetMess(QString LabelData, QString LineEditData, QString PushButtonData,int colorChange=0);
    void inputWidgetHide();
    void setTabCurrentRow(int row,bool skipMode=true);
    void tableWidgetAddRow(structInfo_t structInfo, int row,int mode=1,bool changePageFlag=true);
    QString headerTextListToString(QStringList info);
signals:
    void runNextPage();
    void lineDataChange(structInfo_t info);
    void deleteSqlInfo(QList<QString> info);
    void writeToTabHandle(structInfo_t info,int row,bool &flag);
private slots:
    void returnInputEvent();
    void lineEdit_editFinished();
    void itemClickefHandle(int row, int column);
    void tableWidget_customContextMenuRequested(const QPoint &pos);
    void onTaskBoxContextMenuEvent();
    void timerCheckTimeOut();
private:
    int checkTableWidgetData();
    void writeToTabwidget(QString data,int row,int colume);
    void resizeEvent(QResizeEvent *event);
    void createRightMenu();
    void timerMessageBox(QMessageBox *mess,int times);
private:
    QWidget *inputWidget = NULL;
    int g_currentRowCount = 0;
    QMenu *rightMenu = NULL;
    QString g_inputData;
    QTimer *g_timerCheck = new QTimer();
    int g_currentRow;
};

#endif // NEWTABLEWIDGET_H
