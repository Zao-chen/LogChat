﻿#include "setlive2ddialogwidget.h"
#include "ui_setlive2ddialogwidget.h"


QProcess*   SetLive2DDialogWidget::     m_Live2dProcess =nullptr;
bool        SetLive2DDialogWidget::     live2DIsOpen    =false;
int         SetLive2DDialogWidget::     m_Live2dOpenId  =-1;

SetLive2DDialogWidget::SetLive2DDialogWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetLive2DDialogWidget)
{
    ui->setupUi(this);

    init();
    initConnect();


    QFile file(":/main.qss");
    if(file.open(QFile::ReadOnly)){
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
}

SetLive2DDialogWidget::~SetLive2DDialogWidget()
{
    delete ui;
}

void SetLive2DDialogWidget::init()
{
    this->resize(_Width,_Height);
    this->setWindowTitle("Live2D");
    this->setWindowIcon(QIcon(":/res/u77.svg"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //初始化ListWidget
    ui->listWidget_model->setStyleSheet(
                "QScrollBar:vertical {width: 10px;background-color: #F5F5F5;margin: 0px 0px 0px 0px;border-radius: 5px;}"
                "QScrollBar::handle:vertical {background-color: #CCCCCC;min-height: 20px;border-radius: 5px;}"
                "QScrollBar::handle:vertical:hover {background-color: #BBBBBB;border-radius: 5px;}"
                "QScrollBar::add-line:vertical {height: 0px; subcontrol-position: bottom; subcontrol-origin: margin;}"
                "QScrollBar::sub-line:vertical {height: 0 px;subcontrol-position: top; subcontrol-origin: margin;}"
                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background-color: #F5F5F5;border-radius: 5px;}"
                );
     ui->listWidget_model->setEditTriggers(QAbstractItemView::NoEditTriggers);         //禁止编辑
     ui->listWidget_model->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);       //禁用水平滑动条
     ui->listWidget_model->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);   //平滑效果


     for(int i=0;i<Config::get_LIVE2DMODELCONFIG_V().size();++i){
         addListItem(Config::get_LIVE2DMODELCONFIG_V().at(i));
     }

     ui->tabWidget_control->setCurrentIndex(0);

     //配置
     ui->radioButton_look_enable_yes->setChecked(Config::get_LIVE2DENABLEINIT_M()["look_enable"]);
     ui->radioButton_top_enable_yes->setChecked(Config::get_LIVE2DENABLEINIT_M()["top_enable"]);

     //几何
     ui->horizontalSlider_zoom_model_size->setRange(10,10000);      //0.1-100
     ui->horizontalSlider_zoom_model_X->setRange(-200000,200000);   //-2000-2000
     ui->horizontalSlider_zoom_model_Y->setRange(-200000,200000);   //-2000-2000

     ui->horizontalSlider_zoom_model_RX->setRange(-72000,72000);          //-720-720
     ui->horizontalSlider_zoom_model_RY->setRange(-72000,72000);          //-720-720
     ui->horizontalSlider_zoom_model_RZ->setRange(-72000,72000);          //-720-720

     //信息
     ui->horizontalSlider_zoom_mouse_speed->setRange(0,200);        //0-2
     ui->horizontalSlider_zoom_eye_time->setRange(100,1000);        //1-10
     ui->horizontalSlider_zoom_eye_deviation->setRange(50,500);     //0.5-5
     ui->horizontalSlider_zoom_eye_speed->setRange(100,2000);       //1-20
     ui->horizontalSlider_zoom_audio_add->setRange(100,1000);       //1-10
     ui->horizontalSlider_zoom_audio_smooth->setRange(0,100);       //0-1


     ui->horizontalSlider_zoom_model_size->setValue(Config::get_LIVE2DPARAMINIT_M()["model_size"]);
     ui->horizontalSlider_zoom_model_X->setValue(Config::get_LIVE2DPARAMINIT_M()["model_x"]);
     ui->horizontalSlider_zoom_model_Y->setValue(Config::get_LIVE2DPARAMINIT_M()["model_y"]);
     ui->horizontalSlider_zoom_model_RX->setValue(Config::get_LIVE2DPARAMINIT_M()["model_rx"]);
     ui->horizontalSlider_zoom_model_RY->setValue(Config::get_LIVE2DPARAMINIT_M()["model_ry"]);
     ui->horizontalSlider_zoom_model_RZ->setValue(Config::get_LIVE2DPARAMINIT_M()["model_rz"]);

     ui->horizontalSlider_zoom_mouse_speed->setValue(Config::get_LIVE2DPARAMINIT_M()["mouse_speed"]);
     ui->horizontalSlider_zoom_eye_time->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_time"]);
     ui->horizontalSlider_zoom_eye_deviation->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_deviation"]);
     ui->horizontalSlider_zoom_eye_speed->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_speed"]);
     ui->horizontalSlider_zoom_audio_add->setValue(Config::get_LIVE2DPARAMINIT_M()["audio_add"]);
     ui->horizontalSlider_zoom_audio_smooth->setValue(Config::get_LIVE2DPARAMINIT_M()["audio_smooth"]);

     ui->lineEdit_zoom_model_size->setEnabled(false);
     ui->lineEdit_zoom_model_X->setEnabled(false);
     ui->lineEdit_zoom_model_Y->setEnabled(false);
     ui->lineEdit_zoom_model_RX->setEnabled(false);
     ui->lineEdit_zoom_model_RY->setEnabled(false);
     ui->lineEdit_zoom_model_RZ->setEnabled(false);
     ui->lineEdit_zoom_mouse_speed->setEnabled(false);
     ui->lineEdit_zoom_eye_time->setEnabled(false);
     ui->lineEdit_zoom_eye_deviation->setEnabled(false);
     ui->lineEdit_zoom_eye_speed->setEnabled(false);
     ui->lineEdit_zoom_audio_add->setEnabled(false);
     ui->lineEdit_zoom_audio_smooth->setEnabled(false);


     updateLineEdit();



     if(SetLive2DDialogWidget::live2DIsOpen){
         if(SetLive2DDialogWidget::m_Live2dOpenId!=-1){
             updateForUi();

         }

     }




}

void SetLive2DDialogWidget::initConnect()
{

    SliderChange slider=&QSlider::sliderMoved;

    //启动模型
    connect(ui->pushButton_start,&QPushButton::clicked,this,[=](){
        if(NetLive2D::getIsConnect()==false){
            live2DIsOpen=false;
        }
        if(ui->listWidget_model->count()<=0){
            QMessageBox::warning(this,"Error occurred","没有可用模型！");
            return ;
        }
        if(m_Live2dPassId==-1){
            QMessageBox::warning(this,"Error occurred","请选择模型！");
            return ;
        }
        if(Config::get_LIVE2DMODELCONFIG_V()[m_Live2dPassId].getFilePath().size()<QString("model3.json").size()){

            QMessageBox::warning(this,"Error occurred","模型路径有误！");
            return ;
        }
        m_Live2dOpenId=m_Live2dPassId;

        if(live2DIsOpen==true){
            Config::set_UNITY_STARTMODELPATH(Config::get_LIVE2DMODELCONFIG_V()[SetLive2DDialogWidget::m_Live2dOpenId].getFilePath());
            sendModelHandle("Init:null");
            //同步刷新Ui
            updateForUi();
            //同步配置数据
            QTimer* timer=new QTimer(this);
            connect(timer,&QTimer::timeout,this,[=](){
                if(NetLive2D::getIsConnect()){

                    //同步配置数据
                    QTimer::singleShot(500,this,[=](){
                        updateForUnity();
                    });
                    timer->stop();
                }

            });
            timer->start(10);


            return;
        }
        SetLive2DDialogWidget::live2DIsOpen=true;
        m_Live2dProcess=new QProcess(ConfigWindow::_WindowPointer);
        m_Live2dProcess->setWorkingDirectory(Config::get_UNITY_DEMO_WORK_WAY());
        m_Live2dProcess->start(Config::get_UNITY_DEMO_WAY());
            //检查外部进程是否成功启动
        if (!m_Live2dProcess->waitForStarted()) {
            qDebug() << "Failed to start the process";
            return;
        }
        else {
            //同步刷新Ui
            updateForUi();
            QTimer* timer=new QTimer(this);
            connect(timer,&QTimer::timeout,this,[=](){
                if(NetLive2D::getIsConnect()){

                    //同步配置数据
                    QTimer::singleShot(100,this,[=](){
                        updateForUnity();
                    });
                    timer->stop();
                }

            });
            timer->start(10);
        }

    });

    //退出模型
    connect(ui->pushButton_quit,&QPushButton::clicked,this,[=](){

        SetLive2DDialogWidget::live2DIsOpen=false;
        SetLive2DDialogWidget::m_Live2dOpenId=-1;
        if(m_Live2dProcess!=nullptr){
            delete m_Live2dProcess;
            m_Live2dProcess=nullptr;
        }
    });

    //添加模型
    connect(ui->pushButton_add,&QPushButton::clicked,this,[=](){
        QString jsonPath = QFileDialog::getOpenFileName(nullptr, "Open File", "", "All Files (*model3.json)");
        if(jsonPath.size()<QString("model3.json").size()){return ;}

        QVector<ModelConfigItem> modV=Config::get_LIVE2DMODELCONFIG_V();

        //model初始化
        ModelConfigItem modelConfigItem;
        modelConfigItem.setModelId(Config::get_LIVE2DPARAMINIT_M()["model_id"]);
        modelConfigItem.setModelName(Config::get_LIVE2DSTRING_M()["model_name"]);
        modelConfigItem.setFilePath(Config::get_LIVE2DSTRING_M()["file_path"]);
        modelConfigItem.setLookEnable(Config::get_LIVE2DENABLEINIT_M()["look_enable"]);
        modelConfigItem.setTopEnable(Config::get_LIVE2DENABLEINIT_M()["top_enable"]);
        modelConfigItem.setModelSize(Config::get_LIVE2DPARAMINIT_M()["model_size"]);
        modelConfigItem.setModelX(Config::get_LIVE2DPARAMINIT_M()["model_x"]);
        modelConfigItem.setModelY(Config::get_LIVE2DPARAMINIT_M()["model_y"]);
        modelConfigItem.setMouseSpeed(Config::get_LIVE2DPARAMINIT_M()["mouse_speed"]);
        modelConfigItem.setEyeTime(Config::get_LIVE2DPARAMINIT_M()["eye_time"]);
        modelConfigItem.setEyeDeviation(Config::get_LIVE2DPARAMINIT_M()["eye_deviation"]);
        modelConfigItem.setEyeSpeed(Config::get_LIVE2DPARAMINIT_M()["eye_speed"]);
        modelConfigItem.setAudioAdd(Config::get_LIVE2DPARAMINIT_M()["audio_add"]);
        modelConfigItem.setAudioSmooth(Config::get_LIVE2DPARAMINIT_M()["audio_smooth"]);
        modelConfigItem.setModelDescription(Config::get_LIVE2DSTRING_M()["model_description"]);


        //model赋值
        modelConfigItem.setModelId(ui->listWidget_model->count());
        QString modelName=jsonPath;
        modelName.chop(QString(".model3.json").size());
        modelConfigItem.setFilePath(jsonPath);


        int index=modelName.size()-1;
        for (;index>=0;--index) {
            if(modelName[index]==QChar('\\')||modelName[index]==QChar('/')){
                index++;
                break;
            }
        }
        modelName=modelName.mid(index);
        modelConfigItem.setModelName(modelName);

        modV.push_back(modelConfigItem);
        Config::set_LIVE2DMODELCONFIG_V(modV);
        this->addListItem(modelConfigItem);

    });

    //删除模型
    connect(ui->pushButton_delete,&QPushButton::clicked,this,[=](){
        int id=ui->listWidget_model->currentRow();
        m_Live2dPassId=-1;
        if(id<0){return ;}
        ui->listWidget_model->clearSelection();
        qDebug()<<QString::number(id)<<" 号模型被删除，Name is "<<Config::get_LIVE2DMODELCONFIG_V()[id].getModelName();

        QVector<ModelConfigItem> modV=Config::get_LIVE2DMODELCONFIG_V();
        if(id>=modV.size()){return ;}
        modV.remove(id);
        //编号对齐
        for(int i=0;i<modV.size();++i){
            modV[i].setModelId(i);
        }
        Config::set_LIVE2DMODELCONFIG_V(modV);
        ui->listWidget_model->clear();

        for(int i=0;i<Config::get_LIVE2DMODELCONFIG_V().size();++i){
            addListItem(Config::get_LIVE2DMODELCONFIG_V().at(i));
        }

    });

    //修改模型
    connect(ui->pushButton_change,&QPushButton::clicked,this,[=](){
        if(SetLive2DDialogWidget::live2DIsOpen==false||SetLive2DDialogWidget::m_Live2dOpenId==-1){
            QMessageBox::warning(this,"Error occurred","请选择模型！");

            return ;
        }

        ChangeLive2DWidget widget;
        QString filePath=ui->lineEdit_way->text();
        int index=filePath.size()-1;
        for (;index>=0;--index) {
            if(filePath[index]==QChar('\\')||filePath[index]==QChar('/')){
                break;
            }
        }

        QString directoryPath = ui->lineEdit_way->text().mid(0,index);
        qDebug()<<"open file path"<<directoryPath;
        widget.refresh(directoryPath);
        connect(&widget,&ChangeLive2DWidget::sendhandle,[=](QString handleStr){
            emit sendModelHandle(handleStr);
        });

        widget.exec();
    });

    //打开文件位置
    connect(ui->pushButton_way,&QPushButton::clicked,this,[=](){

        QString filePath=ui->lineEdit_way->text();
        int index=filePath.size()-1;
        for (;index>=0;--index) {
            if(filePath[index]==QChar('\\')||filePath[index]==QChar('/')){
                break;
            }
        }

        QString directoryPath = ui->lineEdit_way->text().mid(0,index);
        qDebug()<<"open file path"<<directoryPath;

        // 构建目录的URL
        QUrl directoryUrl = QUrl::fromLocalFile(directoryPath);

        // 打开资源管理器并指定目录
        if (QDesktopServices::openUrl(directoryUrl)) {
            qDebug() << "File explorer opened successfully.";
        } else {
            qDebug() << "Failed to open file explorer.";
        }
    });

    //看向鼠标
    connect(ui->radioButton_look_enable_yes, &QRadioButton::clicked, [&](){sendConfigHandle("IsLookMouse",100);});
    connect(ui->radioButton_look_enable_no, &QRadioButton::clicked, [&](){sendConfigHandle("IsLookMouse",0);});
    //窗口置顶
    connect(ui->radioButton_top_enable_yes, &QRadioButton::clicked, [&](){sendWindowhandle("top");});
    connect(ui->radioButton_top_enable_no, &QRadioButton::clicked, [&](){sendWindowhandle("normal");});
    //模型缩放比例
    connect(ui->horizontalSlider_zoom_model_size,slider,this,[=](int value){sendConfigHandle("ScaleScaleProportion",value);updateLineEdit();});
    //X坐标
    connect(ui->horizontalSlider_zoom_model_X,slider,this,[=](int value){sendConfigHandle("X",value);updateLineEdit();});
    //Y坐标
    connect(ui->horizontalSlider_zoom_model_Y,slider,this,[=](int value){sendConfigHandle("Y",value);updateLineEdit();});
    //RX坐标
    connect(ui->horizontalSlider_zoom_model_RX,slider,this,[=](int value){sendConfigHandle("RX",value);updateLineEdit();});
    //RY坐标
    connect(ui->horizontalSlider_zoom_model_RY,slider,this,[=](int value){sendConfigHandle("RY",value);updateLineEdit();});
    //RZ坐标
    connect(ui->horizontalSlider_zoom_model_RZ,slider,this,[=](int value){sendConfigHandle("RZ",value);updateLineEdit();});

    //看向鼠标速度
    connect(ui->horizontalSlider_zoom_mouse_speed,slider,this,[=](int value){sendConfigHandle("Damping",value);updateLineEdit();});
    //平均眨眼周期
    connect(ui->horizontalSlider_zoom_eye_time,slider,this,[=](int value){sendConfigHandle("Mean",value);updateLineEdit();});
    //眨眼偏差时间
    connect(ui->horizontalSlider_zoom_eye_deviation,slider,this,[=](int value){sendConfigHandle("MaximumDeviation",value);updateLineEdit();});
    //眨眼速度快慢
    connect(ui->horizontalSlider_zoom_eye_speed,slider,this,[=](int value){sendConfigHandle("Timescale",value);updateLineEdit();});
    //音频增益效果
    connect(ui->horizontalSlider_zoom_audio_add,slider,this,[=](int value){sendConfigHandle("Gain",value);updateLineEdit();});
    //音频平滑效果
    connect(ui->horizontalSlider_zoom_audio_smooth,slider,this,[=](int value){sendConfigHandle("Smoothing",value);updateLineEdit();});


    //重置

    connect(ui->pushButton_zoom_model_size,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_model_size->setValue(Config::get_LIVE2DPARAMINIT_M()["model_size"]);
        sendConfigHandle("ScaleScaleProportion",ui->horizontalSlider_zoom_model_size->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_model_X,&QPushButton::clicked,[&](){ui->horizontalSlider_zoom_model_X->setValue(
        Config::get_LIVE2DPARAMINIT_M()["model_x"]);
        sendConfigHandle("X",ui->horizontalSlider_zoom_model_X->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_model_Y,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_model_Y->setValue(Config::get_LIVE2DPARAMINIT_M()["model_y"]);
        sendConfigHandle("Y",ui->horizontalSlider_zoom_model_Y->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_model_RX,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_model_RX->setValue(Config::get_LIVE2DPARAMINIT_M()["model_rx"]);
        sendConfigHandle("RX",ui->horizontalSlider_zoom_model_RX->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_model_RY,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_model_RY->setValue(Config::get_LIVE2DPARAMINIT_M()["model_ry"]);
        sendConfigHandle("RY",ui->horizontalSlider_zoom_model_RY->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_model_RZ,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_model_RZ->setValue(Config::get_LIVE2DPARAMINIT_M()["model_rz"]);
        sendConfigHandle("RZ",ui->horizontalSlider_zoom_model_RZ->value());
        updateLineEdit();
    });

    connect(ui->pushButton_zoom_mouse_speed,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_mouse_speed->setValue(Config::get_LIVE2DPARAMINIT_M()["mouse_speed"]);
        sendConfigHandle("Damping",ui->horizontalSlider_zoom_mouse_speed->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_eye_time,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_eye_time->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_time"]);
        sendConfigHandle("Mean",ui->horizontalSlider_zoom_eye_time->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_eye_deviation,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_eye_deviation->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_deviation"]);
        sendConfigHandle("MaximumDeviation",ui->horizontalSlider_zoom_eye_deviation->value());
        updateLineEdit();
    });

    connect(ui->pushButton_zoom_eye_speed,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_eye_speed->setValue(Config::get_LIVE2DPARAMINIT_M()["eye_speed"]);
        sendConfigHandle("Timescale",ui->horizontalSlider_zoom_eye_speed->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_audio_add,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_audio_add->setValue(Config::get_LIVE2DPARAMINIT_M()["audio_add"]);
        sendConfigHandle("Gain",ui->horizontalSlider_zoom_audio_add->value());
        updateLineEdit();
    });
    connect(ui->pushButton_zoom_audio_smooth,&QPushButton::clicked,[&](){
        ui->horizontalSlider_zoom_audio_smooth->setValue(Config::get_LIVE2DPARAMINIT_M()["audio_smooth"]);
        sendConfigHandle("Smoothing",ui->horizontalSlider_zoom_audio_smooth->value());
        updateLineEdit();
    });

    //保存配置
    connect(ui->pushButton_save,&QPushButton::clicked,[=](){
        if(SetLive2DDialogWidget::m_Live2dOpenId==-1){
            return;
        }

        //model初始化
        ModelConfigItem modelConfigItem=Config::get_LIVE2DMODELCONFIG_V().at(SetLive2DDialogWidget::m_Live2dOpenId);
        modelConfigItem.setModelId(SetLive2DDialogWidget::m_Live2dOpenId);
        modelConfigItem.setModelName(ui->lineEdit_name->text());
        modelConfigItem.setFilePath(ui->lineEdit_way->text());
        modelConfigItem.setLookEnable(bool(ui->radioButton_look_enable_yes->isChecked()?1:0));
        modelConfigItem.setTopEnable(bool(ui->radioButton_top_enable_yes->isChecked()?1:0));
        modelConfigItem.setModelSize(ui->horizontalSlider_zoom_model_size->value());
        modelConfigItem.setModelX(ui->horizontalSlider_zoom_model_X->value());
        modelConfigItem.setModelY(ui->horizontalSlider_zoom_model_Y->value());
        modelConfigItem.setMouseSpeed(ui->horizontalSlider_zoom_mouse_speed->value());
        modelConfigItem.setEyeTime(ui->horizontalSlider_zoom_eye_time->value());
        modelConfigItem.setEyeDeviation(ui->horizontalSlider_zoom_eye_deviation->value());
        modelConfigItem.setEyeSpeed(ui->horizontalSlider_zoom_eye_speed->value());
        modelConfigItem.setAudioAdd(ui->horizontalSlider_zoom_audio_add->value());
        modelConfigItem.setAudioSmooth(ui->horizontalSlider_zoom_audio_smooth->value());
        modelConfigItem.setModelDescription(ui->textEdit_explain->toPlainText());



        QVector<ModelConfigItem> modV=Config::get_LIVE2DMODELCONFIG_V();
        for (int i=0;i<modV.size();++i) {
            if(modV.at(i).getModelId()==SetLive2DDialogWidget::m_Live2dOpenId){
                modV[i]=modelConfigItem;
                break;
            }

        }
        Config::set_LIVE2DMODELCONFIG_V(modV);

        //更新ui
        qDebug()<<"更新ui id = "<<SetLive2DDialogWidget::m_Live2dOpenId;
        QListWidgetItem* listWidgetItem=ui->listWidget_model->item(SetLive2DDialogWidget::m_Live2dOpenId);
        (static_cast<Live2DListItemsWidget*>(ui->listWidget_model->itemWidget(listWidgetItem)))->setText(modelConfigItem.getModelName());

    });


}

void SetLive2DDialogWidget::addListItem(const ModelConfigItem &modItem)
{

    QListWidgetItem* item = new QListWidgetItem();
    Live2DListItemsWidget* widget=new Live2DListItemsWidget;
    widget->init(modItem);
    widget->resize(ui->listWidget_model->size().width(),widget->height());
    ui->listWidget_model->addItem(item);
    item->setSizeHint(widget->size());
    ui->listWidget_model->setItemWidget(item,widget);
    ui->listWidget_model->scrollToBottom();
    SendPass sendPass=&Live2DListItemsWidget::sendPass;
    connect(widget,sendPass,[=](ModelConfigItem passItem){
        qDebug()<<passItem.getModelId();
        qDebug()<<Config::get_LIVE2DMODELCONFIG_V().size();
        if(passItem.getModelId()>=Config::get_LIVE2DMODELCONFIG_V().size()){return ;}

        m_Live2dPassId=passItem.getModelId();
        Config::set_UNITY_STARTMODELPATH(Config::get_LIVE2DMODELCONFIG_V()[passItem.getModelId()].getFilePath());
        ui->listWidget_model->setCurrentItem(item);
        qDebug()<<"pass model id : "<<passItem.getModelId();

    });
}

void SetLive2DDialogWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    //可扩展窗口变化自适应

}

void SetLive2DDialogWidget::sendConfigHandle(const QString &str, int val)
{
    if(NetLive2D::getIsConnect()==false){return;}
    double value=double(val)/double(100);
    QString handleStr= QString("Config:%1,%2;").arg(str).arg(QString::number(value));
    emit sendModelHandle(handleStr);

}

void SetLive2DDialogWidget::sendWindowhandle(const QString &str)
{
    if(NetLive2D::getIsConnect()==false){return;}
    QString handleStr= QString("Window:%1;").arg(str);
    emit sendModelHandle(handleStr);
}


void SetLive2DDialogWidget::updateLineEdit()
{
     ui->lineEdit_zoom_model_size->setText(QString::number(ui->horizontalSlider_zoom_model_size->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_model_size->value()/10%10));
     ui->lineEdit_zoom_model_X->setText(QString::number(ui->horizontalSlider_zoom_model_X->value()/100)+"."+QString::number(qAbs(ui->horizontalSlider_zoom_model_X->value()/10%10)));
     ui->lineEdit_zoom_model_Y->setText(QString::number(ui->horizontalSlider_zoom_model_Y->value()/100)+"."+QString::number(qAbs(ui->horizontalSlider_zoom_model_Y->value()/10%10)));
     ui->lineEdit_zoom_model_RX->setText(QString::number(ui->horizontalSlider_zoom_model_RX->value()/100)+"."+QString::number(qAbs(ui->horizontalSlider_zoom_model_RX->value()/10%10)));
     ui->lineEdit_zoom_model_RY->setText(QString::number(ui->horizontalSlider_zoom_model_RY->value()/100)+"."+QString::number(qAbs(ui->horizontalSlider_zoom_model_RY->value()/10%10)));
     ui->lineEdit_zoom_model_RZ->setText(QString::number(ui->horizontalSlider_zoom_model_RZ->value()/100)+"."+QString::number(qAbs(ui->horizontalSlider_zoom_model_RZ->value()/10%10)));

     ui->lineEdit_zoom_mouse_speed->setText(QString::number(ui->horizontalSlider_zoom_mouse_speed->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_mouse_speed->value()/10%10));
     ui->lineEdit_zoom_eye_time->setText(QString::number(ui->horizontalSlider_zoom_eye_time->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_eye_time->value()/10%10));
     ui->lineEdit_zoom_eye_deviation->setText(QString::number(ui->horizontalSlider_zoom_eye_deviation->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_eye_deviation->value()/10%10));
     ui->lineEdit_zoom_eye_speed->setText(QString::number(ui->horizontalSlider_zoom_eye_speed->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_eye_speed->value()/10%10));
     ui->lineEdit_zoom_audio_add->setText(QString::number(ui->horizontalSlider_zoom_audio_add->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_audio_add->value()/10%10));
     ui->lineEdit_zoom_audio_smooth->setText(QString::number(ui->horizontalSlider_zoom_audio_smooth->value()/100)+"."+QString::number(ui->horizontalSlider_zoom_audio_smooth->value()/10%10));

}



void SetLive2DDialogWidget::updateForUi()
{
    if(SetLive2DDialogWidget::m_Live2dOpenId==-1){return;}
    ModelConfigItem passItem=Config::get_LIVE2DMODELCONFIG_V().at(SetLive2DDialogWidget::m_Live2dOpenId);

    ui->lineEdit_name->setText(passItem.getModelName());
    ui->lineEdit_way->setText(passItem.getFilePath());

    if(passItem.getLookEnable())ui->radioButton_look_enable_yes->setChecked(true);
    else ui->radioButton_look_enable_no->setChecked(true);
    if(passItem.getTopEnable())ui->radioButton_top_enable_yes->setChecked(true);
    else ui->radioButton_top_enable_no->setChecked(true);

    ui->horizontalSlider_zoom_model_size->setValue(passItem.getModelSize());
    ui->horizontalSlider_zoom_model_X->setValue(passItem.getModelX());
    ui->horizontalSlider_zoom_model_Y->setValue(passItem.getModelY());

    ui->horizontalSlider_zoom_mouse_speed->setValue(passItem.getMouseSpeed());
    ui->horizontalSlider_zoom_eye_time->setValue(passItem.getEyeTime());
    ui->horizontalSlider_zoom_eye_deviation->setValue(passItem.getEyeDeviation());
    ui->horizontalSlider_zoom_eye_speed->setValue(passItem.getEyeSpeed());
    ui->horizontalSlider_zoom_audio_add->setValue(passItem.getAudioAdd());
    ui->horizontalSlider_zoom_audio_smooth->setValue(passItem.getAudioSmooth());

    ui->textEdit_explain->setText(passItem.getModelDescription());


    this->updateLineEdit();

}

void SetLive2DDialogWidget::updateForUnity()
{

    if(SetLive2DDialogWidget::m_Live2dOpenId==-1){return;}
    if(NetLive2D::getIsConnect()==false){return;}



    QTimer::singleShot(5,this,[=](){
        if(ui->radioButton_look_enable_yes->isChecked())sendConfigHandle("IsLookMouse",100);
        else sendConfigHandle("IsLookMouse",0);
    });

    QTimer::singleShot(10,this,[=](){
        if(ui->radioButton_top_enable_yes->isChecked())sendWindowhandle("top");
        else sendWindowhandle("normal");
    });

    QTimer::singleShot(15,this,[=](){sendConfigHandle("ScaleScaleProportion",ui->horizontalSlider_zoom_model_size->value());});
    QTimer::singleShot(20,this,[=](){sendConfigHandle("X",ui->horizontalSlider_zoom_model_X->value());});
    QTimer::singleShot(25,this,[=](){sendConfigHandle("Y",ui->horizontalSlider_zoom_model_Y->value());});
    QTimer::singleShot(30,this,[=](){sendConfigHandle("Damping",ui->horizontalSlider_zoom_mouse_speed->value());});
    QTimer::singleShot(35,this,[=](){sendConfigHandle("Mean",ui->horizontalSlider_zoom_eye_time->value());});
    QTimer::singleShot(40,this,[=](){sendConfigHandle("MaximumDeviation",ui->horizontalSlider_zoom_eye_deviation->value());});
    QTimer::singleShot(45,this,[=](){sendConfigHandle("Timescale",ui->horizontalSlider_zoom_eye_speed->value());});
    QTimer::singleShot(50,this,[=](){sendConfigHandle("Gain",ui->horizontalSlider_zoom_audio_add->value());});
    QTimer::singleShot(55,this,[=](){sendConfigHandle("Smoothing",ui->horizontalSlider_zoom_audio_smooth->value());});


    //Item位置初始化
    QTimer::singleShot(70,this,[=](){emit sendModelHandle("InitItems:self;");});
    //Item渲染初始化
    QTimer::singleShot(75,this,[=](){emit sendModelHandle("InitItems:appoint;");});


}