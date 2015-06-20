#ifndef LABELBUTTON_H
#define LABELBUTTON_H

#include "myButton.h"
#include <QToolButton>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QMenu>
#include <QPoint>
#include <QTimer>
#include <QCursor>

class labelButton : public QToolButton
{
    Q_OBJECT
    QList<myButton*> *buttonList;
public:
    explicit labelButton(QWidget *parent = 0);
    ~labelButton();
    QList<myButton*> *getButtonList();
    static int NUM;
signals:
    void createMenu(QPoint pos,labelButton *labelMe);
    void toClicked(labelButton *me);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);

    
public slots:
    void onTimeOut();
    void ifClicked();
    
};

#endif // LABELBUTTON_H
