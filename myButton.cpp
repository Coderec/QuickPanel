#include "mybutton.h"

myButton::myButton(QWidget *parent) :
    QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    setCursor(Qt::PointingHandCursor);
    setIconSize(QSize(30,30));
    setFixedSize(66,50);
    setStyleSheet("background-color:transparent");
    connect(this,SIGNAL(clicked()),this,SLOT(ifClicked()));
}

void myButton::ifClicked()
{
    emit toClicked(this);
}

void myButton::contextMenuEvent(QContextMenuEvent *e)
{
    emit createMenu(e->globalPos(),this);
}
