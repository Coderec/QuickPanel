#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QToolButton>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPoint>

class myButton : public QToolButton
{
    Q_OBJECT
public:
    explicit myButton(QWidget *parent = 0);

signals:
    void toClicked(myButton *me);
    void createMenu(QPoint pos,myButton *me);

public slots:
    void ifClicked();
protected:
    void contextMenuEvent(QContextMenuEvent *e);

};

#endif // MYBUTTON_H
