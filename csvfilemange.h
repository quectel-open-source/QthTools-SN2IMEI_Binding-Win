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
#ifndef CSVFILEMANGE_H
#define CSVFILEMANGE_H

#include <QWidget>
#include <QFileDialog>
#include <QTimer>
#include <QTableWidgetItem>
#include "kernel/mysqlite.h"
#include "kernel/pagenavigator.h"
#include "kernel/newtablewidget.h"
#include <QGridLayout>
typedef struct
{
    newTableWidget *leftWidget;
    newTableWidget *currentWidget;
    newTableWidget *rightWidget;
}
stackWidgetInfoStruct;



namespace Ui {
class csvFileMange;
}

class csvFileMange : public QWidget
{
    Q_OBJECT

public:
    explicit csvFileMange(QWidget *parent = nullptr);
    ~csvFileMange();
    QString getCurrentPath();
//    int getTableWidgetRow();
    bool getTabIsEmpty();
    void clearData();
    void openCsvFile(QString fileType);
    void saveFile();

    void openSqlDataBase();
    void initWidget();
    int insertToSql(structInfo_t structInfo);
    bool insertToSql(QString info);
    void delFromSql(QList<QString> structInfo);
    QStringList getHeaderTextList();
private:
    void uiAttributeSet();
    void createSqlTable();
    void saveTabDataToFile(QString fileName,bool noNeedTip);
//    void resizeEvent(QResizeEvent *event);
    bool saveTablewidgetData(QFile &file);
    void saveFile_clicked();
    void stackWidgetChange(int mode);
    void setQTableWidget(QWidget *parentWidget,newTableWidget **newWidget);
    void changeWidget(bool isShow);
    void updateCurrentPage();
    QString createInsertSqlWord(structInfo_t info);
    void nonBlockingSleep(int sectime);
    void createThread(int fileSize);
    bool fileVaildCheck(QTextStream *in,int fileSize,int *lineCount);
    void setHeaderTextList(QStringList list);
private slots:
    void closeEvent(QCloseEvent *event);
    bool runNextPageHandle(bool isShow=true);
    void writeToTabHandle(structInfo_t info,int row,bool &flag);
    void handleResults(QVector<structInfo_t> data);
    void pageNavigator_currentPageChanged(int page);
    void delTabWiagetData(QList<QString> structInfo);
    void timeOutHandle();
    void changePercentHandle(int percent);
signals:
    void closeWidget();
    void updateTableWidget(int);
private:
    Ui::csvFileMange *ui;
    QString g_filePath = NULL;
    QString g_currentSavePath = NULL;
    QFile *currentFile= NULL;
    QObject *userInfoSql = NULL;
    PageNavigator *pageNavigator = new PageNavigator();
    stackWidgetInfoStruct stackWidgetInfo;
    int g_maxTime=1;
    QWidget *progressBar = NULL;
    QWidget *progressWater;
    QLabel *progressBarLabel;
};

#endif // CSVFILEMANGE_H
