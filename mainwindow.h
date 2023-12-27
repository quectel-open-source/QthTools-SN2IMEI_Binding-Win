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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "csvfilemange.h"
#include <QProcess>
#include "third/QSimpleUpdater/toolupdate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:


    void closeTabWidget();
    void closeEvent(QCloseEvent *event);
    void action_handle(QAction *action);

private:
    bool action_createFile_clicked();
    void action_openFile_clicked();
    void action_saveFile_clicked();
    void action_clearData_clicked();
    void action_helpFile_clicked();
    bool checkTabIsEmpty();
    QDateTime getFileTime(QString path);
private:
    Ui::MainWindow *ui;
    csvFileMange *g_csvFile = NULL;
    QProcess *helpProcess= NULL;
    toolUpdate *updateVersion;
//    seriel *currentSeriel = new seriel();
};
#endif // MAINWINDOW_H
