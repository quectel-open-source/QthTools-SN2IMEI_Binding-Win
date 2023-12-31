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
#ifndef PAGENAVIGATOR_H
#define PAGENAVIGATOR_H

#include <QWidget>
//#include <QList>
#include <QLabel>
#include <QKeyEvent>
//class QLabel;
//class QEvent;
//
namespace Ui
{
    class PageNavigator;
}

class PageNavigator : public QWidget
{
    Q_OBJECT

public:
    explicit PageNavigator(int blockSize = 3, QWidget *parent = nullptr);
    ~PageNavigator();

    int getBlockSize() const;
    int getMaxPage() const;
    int getCurrentPage() const;

    // 其他组件只需要调用这两个函数即可
    void setMaxPage(int page);   // 当总页数改变时调用
    void setCurrentPage(int page, bool signalEmitted = false); // 修改当前页时调用

protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);

signals:
    void currentPageChanged(int page);

private:
    Ui::PageNavigator *ui;
    int m_blockSize;
    int m_maxPage;
    int m_currentPage;
    QList<QLabel *> *m_pageLabels;

    void setBlockSize(int blockSize);
    void updatePageLabels();
    void initialize();
};

#endif // PAGENAVIGATOR_H
