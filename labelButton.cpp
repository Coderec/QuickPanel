#include "labelButton.h"
#include <QApplication>
int labelButton::NUM=0;
labelButton::labelButton(QWidget *parent) :
    QToolButton(parent)
{
    NUM++;
    buttonList = new QList<myButton*>();
    setStyleSheet("background-color: rgba(255, 255, 255, 150)");
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    setAcceptDrops(true);
    connect(this,SIGNAL(clicked()),this,SLOT(ifClicked()));
}

QList<myButton *> *labelButton::getButtonList()
{
    return buttonList;
}

labelButton::~labelButton(){
    delete buttonList;
    NUM--;
}

void labelButton::contextMenuEvent(QContextMenuEvent *e)
{
    emit createMenu(e->globalPos(),this);
}

void labelButton::dragEnterEvent(QDragEnterEvent *e)
{  
    QTimer::singleShot(500, this, SLOT(onTimeOut()));
}


void labelButton::onTimeOut()
{
    QRect rect = QRect(QWidget::mapFromParent(this->geometry().topLeft()),this->size());
    if(rect.contains(QWidget::mapFromGlobal(cursor().pos())))
        emit this->clicked();            //when drag to this label,open this label
}

void labelButton::ifClicked()
{
     emit toClicked(this);
}
