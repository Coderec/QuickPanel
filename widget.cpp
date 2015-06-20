#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>
#include <Windows.h>
#include <shellapi.h>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QBitmap>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    #pragma comment(lib,"shell32.lib")               // to use ShellExecute()
    ui->setupUi(this);
    QPixmap mask(":/img/QPMask.png");
    setMask(mask.mask());
    ui->bindButton->setIcon(QIcon(":/img/non_bind.png"));
    ui->bindButton->setIconSize(QSize(ui->bindButton->width(),ui->bindButton->height()));
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint| Qt::WindowStaysOnTopHint|Qt::Tool);
    ifOnTop = true;
    setAcceptDrops(true);
    setMouseTracking(true);
    setMinimumWidth(200);
    setAutoFillBackground(true);

    spacer = new QSpacerItem(20,500);
    grid   = new QGridLayout();
    grid->setSpacing(10);
    grid->setVerticalSpacing(100);
    flag   = 1;
    ifMousePressed = false;
    ifCopyLnk = false;
    menu = new QMenu();
    labelButtonlist = new QList<labelButton*>();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(isSingleClick()));

    readSettings();
    setGeometry(lastX,lastY,windowWidth,windowHeight);
    initLayOut();

    QPixmap pixmap;
    if(QFileInfo(img).exists())
        pixmap.load(img);
    else{
        if(!img.isEmpty()){
            QMessageBox msgBox(this);
            QString str("壁纸"+img+"已被删除或更改，即将恢复初始壁纸");
            msgBox.setText(str);
            msgBox.exec();
        }
        pixmap.load(":/img/background .jpg");
    }
    QPalette palette=this->palette();
    palette.setBrush(QPalette::Background,QBrush(pixmap.scaled(windowWidth,windowHeight))); //picture auto adapt the window's size
    this->setPalette(palette);

    trayIcon = new QSystemTrayIcon(QIcon(":/img/tray.png"),this);
    trayIcon->setToolTip(tr("快捷方式"));
    trayIcon->setVisible(true);
    trayIcon->setContextMenu(CreateTrayMenu());
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(iconActived(QSystemTrayIcon::ActivationReason)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onClicked(myButton *me)
{
    if(!me->toolTip().isEmpty()){
        if(QFileInfo(me->toolTip()).exists()){
            ShellExecute(NULL,QString("open").toStdWString().c_str(),      //open normal file by default program
                          me->toolTip().toStdWString().c_str(),NULL,NULL,SW_SHOW);
        }else{
            QMessageBox msgBox(this);
            msgBox.setText(tr("该快捷方式所指向的文件已被移动或删除，即将删除该快捷方式"));
            msgBox.exec();
            this->me=me;
            onRemove();
        }
    }
}

void Widget::onCreateMenu(QPoint pos,myButton *me)
{
    this->me = me;
    QMenu menu;
    menu.addAction(tr("修改名称"),this,SLOT(onRename()));
    menu.addAction(tr("打开文件位置"),this,SLOT(onOpenButtonDir()));
    menu.addAction(tr("删除"),this,SLOT(onRemove()));
    menu.addAction(tr("删除全部"),this,SLOT(onRemoveAll()));
    menu.exec(pos);
}

void Widget::onCreateLabelMenu(QPoint pos, labelButton *labelMe)
{
    this->labelMe = labelMe;
    QMenu menu;
    menu.addAction(tr("插入栏目"),this,SLOT(onInsertLabel()));
    if(labelButtonlist->indexOf(labelMe)!=0)
        menu.addAction(tr("上移"),this,SLOT(onLabelUp()));
    if(labelButtonlist->lastIndexOf(labelMe)!= labelButtonlist->count()-1)
        menu.addAction(tr("下移"),this,SLOT(onLabelDown()));
    menu.addAction(tr("修改名称"),this,SLOT(onLabelRename()));
    menu.addAction(tr("添加栏目"),this,SLOT(onNewLabel()));
    menu.addAction(tr("删除该栏"),this,SLOT(onDelLabel()));
    menu.addAction(tr("删除所有栏"),this,SLOT(onDelAllLabel()));
    menu.exec(pos);
}
/*void Widget::onInsertLabel() introduction
 *
 *foreach (lb, *labelButtonlist) {
        ui->verticalLayout_part->addWidget(lb);
        if(flag==labelButtonlist->indexOf(lb)+1){
            insertListToGrid(*lb->getButtonList());
            ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(lb)+1,grid);
            ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(lb)+2,spacer);
        }
    }

  *flag==labelButtonlist->indexOf(lb)+1
  * *labelButtonlist->at(flag-1)->getButtonList()  ==  *lb->getButtonList();
  *insertLayout(flag,grid); == insertLayout(labelButtonlist->indexOf(lb)+1,grid);
  *insertItem(flag+1,spacer) == insertItem(labelButtonlist->indexOf(lb)+2,spacer)
  *
  *labelButtonlist->indexOf(labelMe)+1   index to insert
*/

void Widget::onInsertLabel()
{
    ui->verticalLayout_part->removeItem(spacer);
    ui->verticalLayout_part->removeItem(grid);
    int index = labelButtonlist->indexOf(labelMe)+1;
    labelButton *newLabel = new labelButton();
    labelButtonlist->insert(index,newLabel);
    newLabel->setText(QVariant(labelButton::NUM).toString());
    connect(newLabel,SIGNAL(createMenu(QPoint,labelButton*)),this,SLOT(onCreateLabelMenu(QPoint,labelButton*)));
    connect(newLabel,SIGNAL(toClicked(labelButton*)),this,SLOT(onLabelButtonClicked(labelButton*)));
    ui->verticalLayout_part->insertWidget(index,newLabel);
    ui->verticalLayout_part->insertLayout(flag,grid);
    ui->verticalLayout_part->insertItem(flag+1,spacer);
}

void Widget::onLabelUp()
{
    labelButtonlist->move(labelButtonlist->indexOf(labelMe),labelButtonlist->indexOf(labelMe)-1);

    ui->verticalLayout_part->removeItem(spacer);      //remove all labels
    ui->verticalLayout_part->removeItem(grid);
    while(ui->verticalLayout_part->count()>0) {
        QWidget* widget = ui->verticalLayout_part->itemAt(0)->widget();
        ui->verticalLayout_part->removeWidget(widget);
        }

    labelButton*lb;                                  //recover all labels
    foreach (lb, *labelButtonlist) {
        ui->verticalLayout_part->addWidget(lb);
        if(flag==labelButtonlist->indexOf(lb)+1){
            insertListToGrid(*lb->getButtonList());
            ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(lb)+1,grid);
            ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(lb)+2,spacer);
        }
    }
}

void Widget::onLabelDown()
{
    labelButtonlist->move(labelButtonlist->indexOf(labelMe),labelButtonlist->indexOf(labelMe)+1);

    ui->verticalLayout_part->removeItem(spacer);      //remove all labels
    ui->verticalLayout_part->removeItem(grid);
    while(ui->verticalLayout_part->count()>0) {
        QWidget* widget = ui->verticalLayout_part->itemAt(0)->widget();
        ui->verticalLayout_part->removeWidget(widget);
        }

    labelButton*lb;                                  //recover all labels
    foreach (lb, *labelButtonlist) {
        ui->verticalLayout_part->addWidget(lb);
        if(flag==labelButtonlist->indexOf(lb)+1){
            insertListToGrid(*lb->getButtonList());
            ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(lb)+1,grid);
            ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(lb)+2,spacer);
        }
    }
}

void Widget::onRemove()
{
    QMessageBox msgBox(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("删除图标,你认真的?"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret==QMessageBox::Yes){
        if(me->toolTip().contains("copiedLnk"))
            QFile::remove(me->toolTip());
        labelButton*lb;
        foreach (lb, *labelButtonlist) {
            if(flag==labelButtonlist->indexOf(lb)+1){
                if(lb->getButtonList()->indexOf(me) >= 0){
                    lb->getButtonList()->removeAt(lb->getButtonList()->indexOf(me));
                    delete me;
                    insertListToGrid(*lb->getButtonList());
                }
            }
        }
    }
}

void Widget::onRemoveAll()
{
    QMessageBox msgBox(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("删除全部图标,你认真的?"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret==QMessageBox::Yes){
        labelButton*lb;
        foreach (lb, *labelButtonlist) {
            if(flag==labelButtonlist->indexOf(lb)+1){
                while(lb->getButtonList()->count()>0){
                    if(lb->getButtonList()->first()->toolTip().contains("copiedLnk"))
                        QFile::remove(lb->getButtonList()->first()->toolTip());
                    lb->getButtonList()->removeFirst();
                }
            }
        }
        removeGridWidget();
    }
}

void Widget::onRename()
{
    QString str = QInputDialog::getText(this,tr("修改名称"),tr("更改图标名字"),QLineEdit::Normal,me->text());
    if(!str.isEmpty())
        me->setText(str);
}

void Widget::onOpenButtonDir()
{
    QString str;
    if(QFileInfo(me->toolTip()).isSymLink())
        str = "/select,"+QFileInfo(me->toolTip()).symLinkTarget().replace("/","\\");
    else
        str = "/select,"+me->toolTip().replace("/","\\");
    ShellExecuteW(0,QString("open").toStdWString().c_str(),QString("explorer.exe").toStdWString().c_str(),str.toStdWString().c_str(),NULL,true);
}

void Widget::onLabelRename()
{
    QString str = QInputDialog::getText(this,tr("修改名称"),tr("更改栏目名字"),QLineEdit::Normal,labelMe->text());
    if(!str.isEmpty())
        labelMe->setText(str);
}

void Widget::iconActived(QSystemTrayIcon::ActivationReason r)
{
    switch(r){
    case QSystemTrayIcon::Trigger:
        timer->start(qApp->doubleClickInterval());
        trayCurPos = cursor().pos();
        break;
    case QSystemTrayIcon::DoubleClick:
        timer->stop();
        if(isHidden())
            show();
        raise();
        if(geometry().y()<0)
            move(geometry().x(),0);
        if(geometry().x()<0)
            move(0,geometry().y());
        if(geometry().x()>qApp->desktop()->width()-width())
            move(qApp->desktop()->width()-width(),geometry().y());
        break;
    }
}

void Widget::dragEnterEvent(QDragEnterEvent *e)
{
    if(!ifMousePressed && geometry().y()<0){      //recover the hide(top)
        for(int x=25;geometry().y()<0;){
            move(geometry().x(),geometry().y()+x);
            Sleep(10);
        }
        move(geometry().x(),0);
    }
    if(!ifMousePressed && geometry().x()<0){      //recover the hide(left)
        for(int x=15;geometry().x()<0;){
            move(geometry().x()+x,geometry().y());
            Sleep(10);
        }
        move(0,geometry().y());
    }
    if(!ifMousePressed && geometry().x()>QApplication::desktop()->width()-width()){      //recover the hide(right)
        for(int x=15;geometry().x()>QApplication::desktop()->width()-width();){
            move(geometry().x()-x,geometry().y());
            Sleep(10);
        }
        move(QApplication::desktop()->width()-width(),geometry().y());
    }
    if(e->mimeData()->hasUrls())                          //if has url,then accept the action
        e->acceptProposedAction();
    else
        e->ignore();
}

void Widget::dropEvent(QDropEvent *e)
{
    const QMimeData *mimeData = e->mimeData();
    if(mimeData->hasUrls()){
        QList<QUrl> urlList = mimeData->urls();
        QString fileName = urlList.at(0).toLocalFile();   //convert url to local-file-name
        if(!fileName.isEmpty()){
            foreach (QUrl url,urlList) {
                QString fileName = url.toLocalFile();     //tempButton is a temperary value,
                                                          //save information to append in buttonlist
                QFileInfo file_info(fileName);
                QFileIconProvider icon_provider;
                QIcon icon = icon_provider.icon(file_info);

                myButton *tempButton = new myButton();

                tempButton->setIcon(icon);
                tempButton->setText(file_info.fileName().section(".",0,0));

                if (file_info.isSymLink()){
                    if(ifCopyLnk){                                           //if click bind_button copy the lnk
                        QString str="./copiedLnk/"+file_info.fileName();
                        while(QFileInfo(str).exists()){
                            str = str.insert(str.lastIndexOf("/")+1,"+");
                        }
                        QFile::copy(file_info.absoluteFilePath(),str);
                        tempButton->setToolTip(QFileInfo(str).absoluteFilePath());
                    }else
                        tempButton->setToolTip(QFileInfo(file_info.symLinkTarget()).absoluteFilePath());
                }
                else{
                    if(ifCopyLnk){
                        QString str="./copiedLnk/"+file_info.fileName().section(".",0,0)+".lnk";
                        while(QFileInfo(str).exists()){
                            str = str.insert(str.lastIndexOf("/")+1,"+");
                        }
                        QFile::link(file_info.absoluteFilePath(),str);
                        tempButton->setToolTip(QFileInfo(str).absoluteFilePath());
                    }else
                        tempButton->setToolTip(file_info.absoluteFilePath());
                }
                ifCopyLnk = false;                                           //nomatter lnk or not change the flag
                ui->bindButton->setIcon(QIcon(":/img/non_bind.png"));
                ui->bindButton->setIconSize(QSize(ui->bindButton->width(),ui->bindButton->height()));

                labelButton*lb;
                foreach (lb, *labelButtonlist) {
                    if(flag==labelButtonlist->indexOf(lb)+1){
                        lb->getButtonList()->append(tempButton);
                        insertListToGrid(*lb->getButtonList());
                        ui->verticalLayout_part->removeItem(spacer);
                        ui->verticalLayout_part->removeItem(grid);
                        ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(lb)+1,grid);
                        ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(lb)+2,spacer);
                    }
                }
            }
        }
    }
    writeSettings();
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        ifMousePressed = true;
        dragPosition = e->globalPos()-this->geometry().topLeft();
        e->accept();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *e)
{
    if(ifMousePressed){
        move(e->globalPos()-dragPosition);
        e->accept();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
    ifMousePressed = false;
    if(geometry().x()<1)                                //make sure the window is in screen(top)
        move(QPoint(0,geometry().y()));
    if(geometry().y()<1){                               //make sure the window is in screen(left)
        move(QPoint(geometry().x(),0));
    }
    if(geometry().x() > QApplication::desktop()->width()-width()){        //make sure the window is in screen(right)
        move(QPoint(QApplication::desktop()->width()-width(),geometry().y()));
    }
}

void Widget::leaveEvent(QEvent *e)                     // auto hide if cursor is out of window
{
    Sleep(300);
    bool ifIn = cursor().pos().x()>=geometry().x() &&
            cursor().pos().x()<=geometry().x()+width() &&
                cursor().pos().y()<=geometry().y()+height() &&
                    cursor().pos().y()>=geometry().y();
    qDebug()<<QVariant(cursor().pos().y()).toString()+"::"+QVariant(geometry().y()).toString();
    if(!ifMousePressed && !ifIn && geometry().y()==0){  //hide top
        for(int x=15;geometry().y()>-height()+1;){
            move(geometry().x(),geometry().y()-x);
            Sleep(10);
        }
        move(geometry().x(),1-height());
    }
    if(!ifMousePressed && !ifIn && geometry().x()==0 && geometry().y()>0){  //hide left
        for(int x=15;geometry().x()>-width()+1;){
            move(geometry().x()-x,geometry().y());
            Sleep(10);
        }
        move(1-width(),geometry().y());
    }
    if(!ifMousePressed && !ifIn && geometry().x()==QApplication::desktop()->width()-width() && geometry().y()>0){  //hide right
        for(int x=15;geometry().x()<QApplication::desktop()->width()-1;){
            move(geometry().x()+x,geometry().y());
            Sleep(10);
        }
        move(QApplication::desktop()->width()-1,geometry().y());
    }
}

void Widget::enterEvent(QEvent *e)
{
    if(!ifMousePressed && geometry().y()<0){      //recover the hide(top)
        for(int x=25;geometry().y()<0;){
            move(geometry().x(),geometry().y()+x);
            Sleep(10);
        }
        move(geometry().x(),0);
    }
    if(!ifMousePressed && geometry().x()<0){      //recover the hide(left)
        for(int x=15;geometry().x()<0;){
            move(geometry().x()+x,geometry().y());
            Sleep(10);
        }
        move(0,geometry().y());
    }
    if(!ifMousePressed && geometry().x()>QApplication::desktop()->width()-width()){      //recover the hide(right)
        for(int x=15;geometry().x()>QApplication::desktop()->width()-width();){
            move(geometry().x()-x,geometry().y());
            Sleep(10);
        }
        move(QApplication::desktop()->width()-width(),geometry().y());
    }
}

void Widget::contextMenuEvent(QContextMenuEvent *e)
{
    QSettings *reg=new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    ifAutoRun = reg->contains("QuickPanel");
    QMenu menu;
    if(ifAutoRun){
        menu.addAction(tr("取消随机启动"),this,SLOT(changeAutoRun()));
    }else{
        menu.addAction(tr("设置随机启动"),this,SLOT(changeAutoRun()));
    }

    if(ifOnTop){
        menu.addAction(tr("设置底端显示"),this,SLOT(changeFront()));
    }else{
        menu.addAction(tr("设置前端显示"),this,SLOT(changeFront()));
    }
    menu.addAction(tr("更换壁纸"),this,SLOT(changeWallPaper()));
    menu.exec(e->globalPos());
}

void Widget::insertListToGrid(const QList<myButton*> &list)
{
    removeGridWidget();                               //remove all widget everytime
    if(!list.isEmpty()){
        int x=0,y=0;

        myButton *button;
        foreach(button, list) {
            disconnect(button,SIGNAL(toClicked(myButton*)),
                       this,SLOT(onClicked(myButton*)));
            disconnect(button,SIGNAL(createMenu(QPoint,myButton*)),
                       this,SLOT(onCreateMenu(QPoint,myButton*)));
            if(y<3){
                grid->addWidget(button,x,y);
                ++y;
            }else{
                ++x;
                y=0;
                grid->addWidget(button,x,y);
                ++y;
            }
            button->show();                                       //because hiding widget
            connect(button,SIGNAL(toClicked(myButton*)),
                    this,SLOT(onClicked(myButton*)));
            connect(button,SIGNAL(createMenu(QPoint,myButton*)),this,SLOT(onCreateMenu(QPoint,myButton*)));
                                                                    //on  signal of myButton::contextMenuEvent
        }
    }
}

void Widget::removeGridWidget()
{
    while(grid->count() > 0)
    {
        QWidget* widget = grid->itemAt(0)->widget();                  //remove the widget in the QGridLayout
        grid->removeWidget(widget);
        widget->hide();                                               //hide the widget
    }
}

void Widget::readSettings()
{
    QSettings settings("CBS","Fast Run");
    lastX = settings.value("GeometryX").toInt();
    lastY = settings.value("GeometryY").toInt();
    img = settings.value("WallPaper").toString();

    int t = 0;
    int num=settings.value("labelButtonListNum").toInt();
    QString str1,str2,str3;
    for(int j=1;j<=num;j++){
        labelButton *lb = new labelButton();
        labelButtonlist->append(lb);
        connect(lb,SIGNAL(createMenu(QPoint,labelButton*)),this,SLOT(onCreateLabelMenu(QPoint,labelButton*)));
        connect(lb,SIGNAL(toClicked(labelButton*)),this,SLOT(onLabelButtonClicked(labelButton*)));

        lb->setText(settings.value("Label"+QVariant(j).toString()+"Name").toString()); // label name

        t = settings.value("List"+QVariant(j).toString()+"NUM").toInt();
        flag=settings.value("lastList").toInt();                                  //read which LABEL when last closing
        for(int i=1;i<=t;i++){
            str1="list"+QVariant(j).toString()+"/b"+QVariant(i).toString()+"tooltip";
            str2="list"+QVariant(j).toString()+"/b"+QVariant(i).toString()+"icon";
            str3="list"+QVariant(j).toString()+"/b"+QVariant(i).toString()+"text";
            myButton *tempButton = new myButton();
            tempButton->setToolTip(settings.value(str1).toString());
            tempButton->setIcon(settings.value(str2).value<QIcon>());
            tempButton->setText(settings.value(str3).toString());
            lb->getButtonList()->append(tempButton);
        }
    }
}

void Widget::writeSettings()
{
    QSettings settings("CBS","Fast Run");
    settings.clear();
    settings.setValue("GeometryX",geometry().x());
    settings.setValue("GeometryY",geometry().y());
    settings.setValue("WallPaper",img);
    settings.setValue("labelButtonListNum",labelButtonlist->count()); // number of labels
    settings.setValue("lastList",flag);                 //write LABEL when closing

    labelButton*lb;
    myButton *button;
    int n=1;
    QString str1,str2,str3;
    foreach (lb, *labelButtonlist) {
        settings.setValue("Label"+QVariant(labelButtonlist->indexOf(lb)+1).toString()+"Name",lb->text()); //remember label name
        settings.setValue("List"+QVariant(labelButtonlist->indexOf(lb)+1).toString()+"NUM",lb->getButtonList()->count());  //buttons' number of label
        n=1;
        foreach(button, *lb->getButtonList()) {
            str1="list"+QVariant(labelButtonlist->indexOf(lb)+1).toString()+"/b"+QVariant(n).toString()+"tooltip";
            str2="list"+QVariant(labelButtonlist->indexOf(lb)+1).toString()+"/b"+QVariant(n).toString()+"icon";
            str3="list"+QVariant(labelButtonlist->indexOf(lb)+1).toString()+"/b"+QVariant(n).toString()+"text";
            settings.setValue(str1,button->toolTip());
            settings.setValue(str2,button->icon());
            settings.setValue(str3,button->text());
            ++n;
        }
    }
}

void Widget::initLayOut()                                 //recover the last close label
{
    labelButton*lb;
    foreach (lb, *labelButtonlist) {
        ui->verticalLayout_part->addWidget(lb);
        myButton *button;
        foreach(button, *lb->getButtonList()) {
            connect(button,SIGNAL(toClicked(myButton*)),
                    this,SLOT(onClicked(myButton*)));
            connect(button,SIGNAL(createMenu(QPoint,myButton*)),this,SLOT(onCreateMenu(QPoint,myButton*)));
                                                                    //on  signal of myButton::contextMenuEvent
        }

        if(flag==labelButtonlist->indexOf(lb)+1){
            insertListToGrid(*lb->getButtonList());
            ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(lb)+1,grid);
            ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(lb)+2,spacer);
        }
    }
}

void Widget::initTrayIconMenu()
{
    menu->clear();
    labelButton*lb;
    myButton *button;
    QAction *action;
    foreach (lb, *labelButtonlist) {
        action = menu->addAction(lb->text());
        action->setFont(QFont(QFont().family(),-1,600));
        menu->addAction(action);
        menu->addAction(menu->addSeparator());
        foreach(button, *lb->getButtonList()) {
            menu->addAction(button->icon(),button->text(),button,SLOT(click()));
        }
        menu->addAction(menu->addSeparator());
    }
}

void Widget::changeFront()
{
    if(ifOnTop){
        setWindowFlags(Qt::FramelessWindowHint |Qt::X11BypassWindowManagerHint |Qt::WindowStaysOnBottomHint |Qt::Tool );
        show();
        ifOnTop = false;
    }else{
        setWindowFlags(Qt::FramelessWindowHint |Qt::X11BypassWindowManagerHint |Qt::WindowStaysOnTopHint |Qt::Tool);
        show();
        ifOnTop = true;
    }
}

void Widget::changeAutoRun()
{
    QSettings *reg=new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    if(ifAutoRun){
        reg->remove("QuickPanel");
        ifAutoRun = false;
    }else{
        reg->setValue("QuickPanel",QApplication::applicationFilePath().replace("/","\\"));
        ifAutoRun = true;
    }
}

void Widget::changeWallPaper()
{
    img = QFileDialog::getOpenFileName(this,tr("更换壁纸"),"./img","Images (*.jpg)");
    if(!img.isEmpty()){
        QPixmap pixmap(img);
        QPalette palette=this->palette();
        palette.setBrush(QPalette::Background,QBrush(pixmap.scaled(windowWidth,windowHeight))); //picture auto adapt the window's size
        this->setPalette(palette);
        QSettings settings("CBS","Fast Run");
        settings.setValue("WallPaper",img);
    }
}

void Widget::on_closeButton_clicked()
{
    writeSettings();
    qApp->quit();
}

void Widget::on_minizeButton_clicked()
{
    hide();
}

void Widget::isSingleClick()
{
    initTrayIconMenu();
    if(menu->isHidden())
        menu->exec(trayCurPos);
    else
        menu->hide();
}

QMenu *Widget::CreateTrayMenu()
{
    QMenu *menu=new QMenu;
    menu->addAction(tr("离开"),this,SLOT(on_closeButton_clicked()));
    return menu;
}

void QMenu::leaveEvent(QEvent *){   //when leave trayMenu hide it
    QMenu::hide();
}

void Widget::on_bindButton_clicked()
{
    ifCopyLnk = !ifCopyLnk;
    if(ifCopyLnk==true){
        ui->bindButton->setIcon(QIcon(":/img/bind.png"));
        ui->bindButton->setIconSize(QSize(ui->bindButton->width(),ui->bindButton->height()));
    }else{
        ui->bindButton->setIcon(QIcon(":/img/non_bind.png"));
        ui->bindButton->setIconSize(QSize(ui->bindButton->width(),ui->bindButton->height()));
    }
}

void Widget::onLabelButtonClicked(labelButton*me)
{
    flag = labelButtonlist->indexOf(me)+1;
    insertListToGrid(*me->getButtonList());
    ui->verticalLayout_part->removeItem(spacer);
    ui->verticalLayout_part->removeItem(grid);
    ui->verticalLayout_part->insertLayout(labelButtonlist->indexOf(me)+1,grid);
    ui->verticalLayout_part->insertItem(labelButtonlist->indexOf(me)+2,spacer);
}

void Widget::onNewLabel()
{
    labelButton *newLabel = new labelButton();
    labelButtonlist->append(newLabel);
    newLabel->setText(QVariant(labelButton::NUM).toString());
    ui->verticalLayout_part->addWidget(newLabel);
    connect(newLabel,SIGNAL(createMenu(QPoint,labelButton*)),this,SLOT(onCreateLabelMenu(QPoint,labelButton*)));
    connect(newLabel,SIGNAL(toClicked(labelButton*)),this,SLOT(onLabelButtonClicked(labelButton*)));
    if(labelButtonlist->indexOf(newLabel)==0){
        insertListToGrid(*newLabel->getButtonList());
        ui->verticalLayout_part->removeItem(spacer);
        ui->verticalLayout_part->removeItem(grid);
        ui->verticalLayout_part->insertLayout(1,grid);
        ui->verticalLayout_part->insertItem(2,spacer);
    }
}


void Widget::onDelLabel()
{
    QMessageBox msgBox(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("删除该栏,你认真的?"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret==QMessageBox::Yes){
        while(labelMe->getButtonList()->count()>0){
            if(labelMe->getButtonList()->first()->toolTip().contains("copiedLnk"))
                QFile::remove(labelMe->getButtonList()->first()->toolTip());
            labelMe->getButtonList()->removeFirst();
        }
        labelMe->hide();
        ui->verticalLayout_part->removeWidget(labelMe);
        labelButtonlist->removeAt(labelButtonlist->indexOf(labelMe));
        delete labelMe;
        if(labelButtonlist->count()>0)
            onLabelButtonClicked(labelButtonlist->first());
        else
            removeGridWidget();
    }
}

void Widget::onDelAllLabel()
{
    QMessageBox msgBox(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("删除所有栏,你认真的?"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret==QMessageBox::Yes){
        labelButton*lb;
        foreach(lb,*labelButtonlist){
            while(lb->getButtonList()->count()>0){
                if(lb->getButtonList()->first()->toolTip().contains("copiedLnk"))
                    QFile::remove(lb->getButtonList()->first()->toolTip());
                lb->getButtonList()->removeFirst();
            }
            lb->hide();
            ui->verticalLayout_part->removeWidget(lb);
            labelButtonlist->removeAt(labelButtonlist->indexOf(lb));
            delete lb;
        }
        removeGridWidget();
    }
}

void Widget::on_newLabelButton_clicked()
{
    onNewLabel();
}
