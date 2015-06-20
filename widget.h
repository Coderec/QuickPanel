#ifndef WIDGET_H
#define WIDGET_H

#include "myButton.h"
#include "labelButton.h"
#include <QWidget>
#include <QSpacerItem>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QUrl>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QIcon>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QProcess>
#include <QPoint>
#include <QMenu>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:


private slots:
    void onClicked(myButton *me);
    void onCreateMenu(QPoint pos,myButton *me);
    void onCreateLabelMenu(QPoint pos,labelButton *labelMe);
    void onInsertLabel();
    void onLabelUp();
    void onLabelDown();
    void onRemove();
    void onRemoveAll();
    void onRename();
    void onOpenButtonDir();
    void onLabelRename();
    void iconActived(QSystemTrayIcon::ActivationReason r);   // response to tray
    void on_closeButton_clicked();
    void on_minizeButton_clicked();
    void isSingleClick();                                    // set interval of click and doubleClick of tray
    void on_bindButton_clicked();
    void onLabelButtonClicked(labelButton*);
    void onNewLabel();
    void onDelLabel();
    void onDelAllLabel();
    void changeFront();
    void changeAutoRun();
    void changeWallPaper();

    void on_newLabelButton_clicked();

private:
    Ui::Widget *ui;
    QSpacerItem *spacer;
    QGridLayout *grid;
    int lastX;                        //x,y when last closing
    int lastY;
    static const int windowHeight=525;
    static const int windowWidth=280;
    QList<labelButton*> *labelButtonlist;
    myButton *me;
    labelButton *labelMe;
    QString img;
    int flag;
    bool ifMousePressed;               // if mouseCur out of the window
    bool ifCopyLnk;
    bool ifOnTop;                      //if setWindowFlags(Qt::WindowStaysOnTopHint)
    bool ifAutoRun;                    // if run when computer start
    QPoint dragPosition;

    QSystemTrayIcon *trayIcon;        //the trayIcon
    QMenu *CreateTrayMenu();
    QMenu *menu;

    QTimer *timer;                   // set interval of click and doubleClick of tray
    QPoint trayCurPos;

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void leaveEvent(QEvent *e);        // auto hide if cursor is out of window
    void enterEvent(QEvent *e);        // recover the hide
    void contextMenuEvent(QContextMenuEvent *e);

    void insertListToGrid(const QList<myButton*> &list);
    void removeGridWidget();
    void readSettings();
    void writeSettings();
    void initLayOut();
    void initTrayIconMenu();
};

#endif // WIDGET_H
