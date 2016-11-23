#include "mainwindow.h"
#include "ui_mainwindow.h"

bool isScreenUp2Date = false,isDisplayHeading = false;
short scrCtX = 450;
short scrCtY = 384;
short dx=0,dy=0,dxMap=0,dyMap=0;
float scale = 20;
short curAzi = 0;
char range = 1;
float turningAngle = 0;
float heading = 0;
bool blink = true;
short           mouseX,mouseY;
QTimer          dataUpdate;
QTimer          warningBlink;
C_radar_data*   radarData;
C_ARPA_data*    arpaData;
Q_vnmap              vnmap;
QPixmap              *pMap=NULL;
bool isGridOn = false;
QFile logFile("log.txt");
QUdpSocket      *udpSocket;

void MainWindow::blinking()
{
    if(blink)blink=false;
    else blink = true;
    for(uint i=0;i<arpaData->track_list.size();i++)
    {

        if(arpaData->track_list[i].lives>0)arpaData->track_list[i].lives--;
    }

}
void MainWindow::ShowPos()
{
    mouseX=this->mapFromGlobal(QCursor::pos()).x();
    mouseY=this->mapFromGlobal(QCursor::pos()).y();
    isScreenUp2Date = false;
    //this->setMouseTracking(true);
    float azi,range;
    radarData->getPolar((mouseX - scrCtX)/scale,-(mouseY - scrCtY)/scale,&azi,&range);
    ui->label_range_2->setText( "RG:"+QString::number(range/1.852f,'g',4)+"NM");//+QString::number(azi,'g',4);
    azi = azi/PI_CHIA2*90.0f;
    if(azi<0)azi+=360.0f;
    ui->label_azi->setText( "AZ:"+QString::number(azi,'g',4)+"\260");
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    ShowPos();
    update();
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    pMap = new QPixmap(height(),height());
    radarData = new C_radar_data();
    arpaData = new C_ARPA_data();
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(config.m_config.arpaPort);
    setMouseTracking(true);
    on_toolButton_standby_clicked();
    //load map
    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(processFrame()));
    //vnmap.LoadBinFile(config.m_config.mapFilename.data());
    //printf("%f",config.m_config.m_long);
    resetScale();
    connect(&dataUpdate,SIGNAL(timeout()),this,SLOT(playbackRadarData()));
    connect(&warningBlink,SIGNAL(timeout()),this,SLOT(blinking()));
    warningBlink.start(1000);
    //arpaData->addARPA("sss",12,PI_CHIA2/2,PI_CHIA2,2);
    //
    if(!logFile.open(QIODevice::WriteOnly))return;
    QRect rec = QApplication::desktop()->screenGeometry(0);
    setFixedSize(1024, 768);
    if((rec.height()==768)&&(rec.width()==1024))
    {
        this->showFullScreen();
        this->setGeometry(QApplication::desktop()->screenGeometry(0));//show on first screen
    }
    else
    {

        rec = QApplication::desktop()->screenGeometry(1);
        if((rec.height()==768)&&(rec.width()==1024))
        {
            this->showFullScreen();
            this->setGeometry(QApplication::desktop()->screenGeometry(0));//show on first screen
        }

    }
    vnmap.setUp(config.m_config.m_lat,config.m_config.m_long+config.m_config.scale, 500,config.m_config.mapFilename.data());
    DrawMap();
    update();

}
void MainWindow::setDisplayHeading(float heading_deg)
{
    heading = heading_deg;

}
void MainWindow::resetScale()
{
    switch(range)
    {
    case -4:
        scale = 360/(0.125f * 1.852f);
        ui->toolButton_range->setText("0.125 NM");
        break;
    case -3:
        scale = 360/(0.25f * 1.852f);
        ui->toolButton_range->setText("0.25 NM");
        break;
    case -2:
        scale = 360/(0.5f * 1.852f);
        ui->toolButton_range->setText("0.5 NM");
        break;
    case -1:
        scale = 360/(0.75f * 1.852f);
        ui->toolButton_range->setText("0.75 NM");
        break;
    case 0:
        scale = 360/(0.75f * 1.852f);
        ui->toolButton_range->setText("0.75 NM");
        break;
    case 1:
        scale = 360/(1.5f * 1.852f);
        ui->toolButton_range->setText("1.5 NM");
        break;
    case 2:
        scale = 360/(3 * 1.852f);
        ui->toolButton_range->setText("3 NM");
        break;
    case 3:
        scale = 360/(6 * 1.852f);
        ui->toolButton_range->setText("6 NM");
        break;
    case 4:
        scale = 360/(12 * 1.852f);
        ui->toolButton_range->setText("12 NM");
        break;
    case 5:
        scale = 360/(24 * 1.852f);
        ui->toolButton_range->setText("24 NM");
        break;
    case 6:
        scale = 360/(48 * 1.852f);
        ui->toolButton_range->setText("48 NM");
        break;
    case 7:
        scale = 360/(96 * 1.852f);
        ui->toolButton_range->setText("96 NM");
        break;
    default:
        scale = 360/(6 * 1.852f);
        ui->toolButton_range->setText("6 NM");
        break;
    }

    DrawMap();
    radarData->resetData(true);

}
void MainWindow::drawTarget(QPainter *p)
{
    //draw radar  target:
    QPen penTarget(Qt::cyan);
    penTarget.setWidth(1);
    //QPen penARPATarget(Qt::yellow);
    //penARPATarget.setWidth(3);
    QPen penTargetBlue(Qt::darkBlue);
    penTargetBlue.setWidth(2);
    //penTargetBlue.setStyle(Qt::DashLine);
    QPen penTrack(Qt::darkRed);
    ui->label_warning->setText("NO WARNING");
    ui->label_warning->setStyleSheet("color:rgb(0, 128, 0); border: 1px solid green; font: 12pt bold \"MS Shell Dlg 2\";");

    for(uint i=0;i<arpaData->track_list.size();i++)
    {
        short x,y;
        if(! arpaData->track_list[i].lives)continue;
        float target_azi,target_rg;
        for(uint j=0;j<(arpaData->track_list[i].object_list.size());j++)
        {
            float tx =  arpaData->track_list[i].object_list[j].centerX*scale;
            float ty =  arpaData->track_list[i].object_list[j].centerY*scale;
            x = tx * cosf(turningAngle) - ty * sinf(turningAngle)+(scrCtX-dx);
            y = tx * sinf(turningAngle) + ty * cosf(turningAngle)+(scrCtY-dy);

            //printf("\n x:%d y:%d",x,y);
            p->setPen(penTrack);
            p->drawPoint(x,y);
        }
        target_azi = arpaData->track_list[i].object_list[arpaData->track_list.size()-1].centerA;
        target_rg = arpaData->track_list[i].object_list[arpaData->track_list.size()-1].centerR;
        float angle = abs((heading/360.0*PI_NHAN2)-target_azi)&&target_rg<7;
        //printf("\nangle:%f",angle);
        bool warning = (abs(angle)<0.3||(abs(angle)>6.28-0.3));
        QPolygon poly;
        QPoint point;
        if(warning)
        {
            if(blink)
            {
                penTarget.setColor(QColor(Qt::red));
                ui->label_warning->setText("COLLISION WARNING");
                ui->label_warning->setStyleSheet("color:rgb(255, 0, 0); border: 1px solid green; font: 12pt bold \"MS Shell Dlg 2\";");
            }
            else
            {
                penTarget.setColor(QColor(Qt::darkRed));
                ui->label_warning->setText("");
                ui->label_warning->setStyleSheet("color:red;");
            }

        }
        else
        {
            penTarget.setColor(QColor(Qt::cyan));
        }
        point.setX(x+20*sinf( arpaData->track_list[i].course+turningAngle));
        point.setY(y-20*cosf( arpaData->track_list[i].course+turningAngle));
        poly<<point;

        point.setX(x+8*sinf( arpaData->track_list[i].course+turningAngle));
        point.setY(y-8*cosf( arpaData->track_list[i].course+turningAngle));
        poly<<point;
        point.setX(x+8*sinf( arpaData->track_list[i].course+turningAngle+2.3562f));
        point.setY(y-8*cosf( arpaData->track_list[i].course+turningAngle+2.3562f));
//        poly<<point;
//        point.setX(x);
//        point.setY(y);
        poly<<point;
        point.setX(x+8*sinf( arpaData->track_list[i].course+turningAngle-2.3562f));
        point.setY(y-8*cosf( arpaData->track_list[i].course+turningAngle-2.3562f));
        poly<<point;
        point.setX(x+8*sinf( arpaData->track_list[i].course+turningAngle));
        point.setY(y-8*cosf( arpaData->track_list[i].course+turningAngle));
        poly<<point;
//        if( arpaData->track_list[i].selected)
//        {
//            char buf[50];
//            p.setPen(penyellow);
//            sprintf(buf, "%3d:%3.3fNM:%3.3f\xB0", arpaData->track_list[i].id, arpaData->track_list[i].centerR/DEFAULT_NM,  arpaData->track_list[i].centerA*57.2957795);
//            QString info = QString::fromAscii(buf);
//            p.drawText(10,infoPosy,150,20,0,info);
//            infoPosy+=20;
//            if( arpaData->track_list[i].id==curTargetId)
//            {
//                p.setPen(penyellow);
//                p.setBrush(Qt::red);
//            }
//                else
//            {
//                p.setPen(penTarget);
//                p.setBrush(Qt::red);
//            }

//        }else
//        {

//            p.setPen(pensubtaget);
//            p.setBrush(QColor(100,100,50,100));
//        }
        p->setPen(penTarget);
        p->drawPolygon(poly);
        if(ui->toolButton_zoomIn_12->isChecked())
        {
            QFont font;
            font.setBold(false);
            font.setPointSize(8);
            p->setFont(font);
            p->drawText(x-20,y-20,100,100,0, arpaData->track_list[i].id.data(),0);
        }
    }
}
bool isInit = false;
void MainWindow::processFrame()
{
        while (udpSocket->hasPendingDatagrams()) {
            unsigned short len = udpSocket->pendingDatagramSize();
            QByteArray buff;
            buff.resize(len);
            udpSocket->readDatagram(buff.data(), len);

               // ProcHR(&buff);
            QString str(buff.data());

            QStringList frameList = str.split(";");
            for(ushort j = 0;j<frameList.size();j++)
            {
                QStringList strList = frameList[j].split(",");

                    if((*(strList.begin())).contains(config.m_config.shipName.data()))
                    {

                        config.m_config.m_lat = (*(strList.begin()+1)).toFloat();
                        config.m_config.m_long = (*(strList.begin()+2)).toFloat();
                        isInit=true;
                        config.SaveToFile();
                        vnmap.setUp(config.m_config.m_lat,config.m_config.m_long+config.m_config.scale, 500,config.m_config.mapFilename.data());
                        DrawMap();
                        ui->label_heading->setText(QString::number((*(strList.begin()+4)).toFloat())+"\260");
                        setDisplayHeading((*(strList.begin()+4)).toFloat());
                        if(isDisplayHeading)
                        {
                            turningAngle = -heading/360*PI_NHAN2;
                            radarData->setTrueN(-heading);

                        }

                    }
                    else if((strList.size()>6)&(isInit))
                    {
                        float d_y = 110.574f*((*(strList.begin()+1)).toFloat() - config.m_config.m_lat);
                        if(!d_y) continue;
                        float d_x = ((*(strList.begin()+2)).toFloat() - config.m_config.m_long)*111.320*cosf(config.m_config.m_lat);
                        float azi = atanf(d_x/d_y);
                        if(d_y<0)azi+=3.141592654f;
                        if(azi<0)azi+=PI_NHAN2;
                        float range = sqrt(d_x*d_x+d_y*d_y);
                        //printf("azi:%fx:%fy:%f",azi,d_x,d_y);
                        arpaData->addARPA((*strList.begin()).right(((*strList.begin()).length()-1)).toStdString(),range,azi,(*(strList.begin()+4)).toFloat()/360.0f*PI_NHAN2,5);
                        //DrawMap();
                    }

             }
         }

}
void MainWindow::newConnection()
{
    // need to grab the socket
    QTcpSocket *socket = controlServer->nextPendingConnection();

    //printf("\nHello client\r\n");
    //socket->flush();
    //QHash <QString, QString>  htable;//!!!!!!!!!!
    socket->waitForBytesWritten(300);
    QByteArray array = socket->read(1000);
    QTextStream outStream(&logFile);
    outStream<<"\n --New Message in ascii :";
    outStream<<QString::fromAscii(array.data(),array.size());
    outStream<<"\n --New Message in stdstring :";
    outStream<<QString::fromWCharArray((wchar_t*)(array.data()),array.size());
    //
    //printf(array.data());
    socket->close();
}
void MainWindow::playbackRadarData()
{
    ShowPos();
    update();
    for(short i=0;i<30;i++)
    {
    curAzi++;
    //printf("ff");
    if(curAzi>=4096) curAzi-=4096;
    radarData->GetDataSimulator(pMap,curAzi);
    update();
    }
}
void MainWindow::paintEvent(QPaintEvent *event)
{
    isScreenUp2Date = true;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    /*p.drawPixmap(scrCtX-scrCtY,0,height(),height(),
                     *pMap,
                     dxMap,dyMap,height(),height());*/
    DrawSignal(&p);
    drawTarget(&p);
    DrawViewFrame(&p);


}
void MainWindow::DrawSignal(QPainter *p)
{
    float signsize = 1;
    QRectF signRect(RADAR_MAX_RESOLUTION-(scrCtX-dx)/signsize,RADAR_MAX_RESOLUTION-(scrCtY-dy)/signsize,width()/signsize,height()/signsize);
    QRectF screen(0,0,width(),height());
    p->drawImage(screen,*radarData->sgn_img,signRect,Qt::AutoColor);
    return;
}
void MainWindow::DrawMap()
{
    if(!pMap) return;
    dxMap = 0;
    dyMap = 0;
    QPainter p(pMap);
    pMap->fill(Qt::black);
    QPen pen(QColor(0,0,114,255));
    QColor color[5];
    color[0].setRgb(255,255,0,255);//land
    color[1].setRgb(0,0,0,255);//lake
    color[2].setRgb(255,0,0,255);//building
    color[3].setRgb(0,0,0,255);//river
    color[4].setRgb(255,255,0,255);//road
    short centerX = pMap->width()/2-dx;
    short centerY = pMap->height()/2-dy;
    p.setRenderHint(QPainter::Antialiasing, true);

    //-----draw provinces in polygons


    for(uint i = 0; i < N_LAYER; i++) {
        //printf("vnmap.layers[i].size()%d\n",vnmap.layers[i].size());
        if(i<3)
        {
            for(uint j = 0; j < vnmap.layers[i].size(); j++) {
                QPolygon poly;
                for(uint k = 0; k < vnmap.layers[i][j].size(); k++) { // Polygon
                    QPoint int_point;
                    float x,y;
                    vnmap.ConvDegToScr(&x,&y,&vnmap.layers[i][j][k].m_Long,&vnmap.layers[i][j][k].m_Lat);
                    int_point.setX((int)(x*scale)+centerX);
                    int_point.setY((int)(y*scale)+centerY);
                    poly<<int_point;
                }
                p.setBrush(color[i]);
                pen.setColor(color[i]);
                pen.setWidth(0);
                p.setPen(pen);
                p.drawPolygon(poly);
            }
        }else
        {
            pen.setColor(color[i]);
            if(i==3)pen.setWidth(3);else pen.setWidth(0);

            p.setPen(pen);
            for(uint j = 0; j < vnmap.layers[i].size(); j++) {

                QPoint old_point;

                for(uint k = 0; k < vnmap.layers[i][j].size(); k++) { // Polygon
                    QPoint int_point;
                    float x,y;
                    vnmap.ConvDegToScr(&x,&y,&vnmap.layers[i][j][k].m_Long,&vnmap.layers[i][j][k].m_Lat);
                    int_point.setX((int)(x*scale)+centerX);
                    int_point.setY((int)(y*scale)+centerY);
                    if(k)p.drawLine(old_point,int_point);
                    old_point=int_point;
                }
                //p.setBrush(color[i]);


            }
        }


    }
    for(uint i = 0; i < arpaData->track_list.size(); i++)
    {
        pen.setWidth(1);
        pen.setColor(color[0]);
        p.setPen(pen);
        p.drawEllipse(arpaData->track_list[i].centerX*scale+centerX-2,arpaData->track_list[i].centerY*scale+centerY-2,4,4);
    }
    //DrawGrid(&p,centerX,centerY);
    //draw text
    /*pen.setColor(QColor(150,130,110,150));
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    //QPen pen2(Qt::red);
    //pen2.setWidth(2);
    for(uint i = 0; i < vnmap.placeList.size(); i++) {
            QPoint int_point;
            float x,y;
            vnmap.ConvDegToScr(&x,&y,&vnmap.placeList[i].m_Long,&vnmap.placeList[i].m_Lat);
            int_point.setX((int)(x*scale)+centerX);
            int_point.setY((int)(y*scale)+centerY);
            //p.drawEllipse(int_point,2,2);
            p.drawText(int_point.x()+5,int_point.y(),QString::fromWCharArray(vnmap.placeList[i].text.c_str()));
            //printf("toa do hien tai lat %f long %f\n",m_textList[i].m_Lat,m_textList[i].m_Long);
    }*/

}
void MainWindow::DrawViewFrame(QPainter* p)
{
    short d = height()-50;
    QPen penBlack(QColor(0,0,0,255));
    short linewidth = 0.6*height();
    penBlack.setWidth(linewidth/10);
    p->setPen(penBlack);
    for (short i=linewidth/12;i<linewidth;i+=linewidth/6)
    {
        p->drawEllipse(-i/2+(scrCtX-scrCtY)+25,-i/2+25,d+i,d+i);
    }
    penBlack.setWidth(0);
    p->setPen(penBlack);
    p->setBrush(Qt::black);
    p->drawRect(scrCtX+scrCtY,0,width()-scrCtX-scrCtY,height());
    p->drawRect(0,0,scrCtX-scrCtY,height());
    p->setBrush(Qt::NoBrush);
    //draw grid

    QPen pengrid(QColor(64,192,64,255));

    if(isGridOn)
    {
        penBlack.setWidth(1);
        p->setPen(penBlack);
        p->drawEllipse(scrCtX-d/12,scrCtY-d/12,d/6,d/6);
        p->drawEllipse(scrCtX-2*d/12,scrCtY-2*d/12,2*d/6,2*d/6);
        p->drawEllipse(scrCtX-3*d/12,scrCtY-3*d/12,3*d/6,3*d/6);
        p->drawEllipse(scrCtX-4*d/12,scrCtY-4*d/12,4*d/6,4*d/6);
        p->drawEllipse(scrCtX-5*d/12,scrCtY-5*d/12,5*d/6,5*d/6);
        p->drawEllipse(scrCtX-6*d/12,scrCtY-6*d/12,6*d/6,6*d/6);

        //p->drawEllipse(scrCtX-scrCtY+25+d/3,25+d/3,d-d/3,d-+d/3);
    }//
    if(!isDisplayHeading)
    {
        //turningAngle = -heading/360*PI_NHAN2;
        //radarData->setTrueN(-heading);
        p->setPen(QPen(QColor(255,255,255,255)));
        p->drawLine(scrCtX,scrCtY,scrCtX+sinf(heading/360*PI_NHAN2)*d/2,scrCtY-cosf(heading/360*PI_NHAN2)*d/2);
    }

    pengrid.setWidth(2);
    p->setPen(pengrid);
    QFont font = p->font() ;
    font.setPointSize(6);
    p->setFont(font);
    //short theta;
    for(short theta=0;theta<360;theta+=5){
        QPoint point0,point1,point2;
        float tanA = tanf(theta/57.295779513f);
        float sinA = sinf(theta/57.295779513f);
        float cosA = cosf(theta/57.295779513f);
        float a = (1+1.0f/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        float b= 2.0f*(dy/tanA - dx);
        float c= dx*dx+dy*dy-d*d/4.0f;
        float delta = b*b-4.0f*a*c;
        if(delta<30.0f)continue;
        delta = sqrtf(delta);

        if(theta==0)
                {
                    point2.setX(scrCtX  - dx);
                    point2.setY(scrCtY - sqrt((d*d/4.0- dx*dx)));
                    point1.setX(point2.x());
                    point1.setY(point2.y()-3);
                    point0.setX(point2.x());
                    point0.setY(point2.y()-18);
                }
        else if (theta<180)
        {
            short rx = (-b + delta)/2.0f/a;
            short ry = -rx/tanA;
            if(abs(rx)<100&&abs(ry)<100)continue;
            point2.setX(scrCtX + rx -dx);
            point2.setY(scrCtY + ry-dy);
            point1.setX(point2.x()+3*sinA);
            point1.setY(point2.y()-3*cosA);
            point0.setX(point2.x()+18*sinA);
            point0.setY(point2.y()-18*cosA);
        }
        else if(theta==180)
                {

                    point2.setX(scrCtX  - dx);
                    point2.setY(scrCtY + sqrt((d*d/4.0- dx*dx)));
                    point1.setX(point2.x());
                    point1.setY(point2.y()+3);
                    point0.setX(point2.x());
                    point0.setY(point2.y()+18);
                }
        else
        {
            short rx;
            short ry;
            rx =  (-b - delta)/2.0f/a;
            ry = -rx/tanA;
            if(abs(rx)<100&&abs(ry)<100)continue;
            point2.setX(scrCtX + rx - dx);
            point2.setY(scrCtY + ry - dy);
            point1.setX(point2.x()+3*sinA);
            point1.setY(point2.y()-3*cosA);
            point0.setX(point2.x()+18*sinA);
            point0.setY(point2.y()-18*cosA);
        }

        p->drawLine(point1,point2);
        if(theta%10==0)p->drawText(point0.x()-25,point0.y()-10,50,20,
                   Qt::AlignHCenter|Qt::AlignVCenter,
                   QString::number(theta));

    }


}
MainWindow::~MainWindow()
{
    delete ui;
    logFile.close();
}
/*
void MainWindow::on_toolButton_clicked()
{
    if(scale<50)scale+=5;
    DrawMap();
}

void MainWindow::on_toolButton_2_clicked()
{
    if(scale>10)scale-=5;
    DrawMap();

}
*/
void MainWindow::on_toolButton_zoomIn_clicked()
{
    if(range>-4)
    {
        range--;
        resetScale();
    }


}

void MainWindow::on_toolButton_zoomOut_clicked()
{
    if(range<7)
    {
        range++;
        resetScale();
    }
}

void MainWindow::on_toolButton_zoomOut_2_toggled(bool checked)
{
    isGridOn = checked;
    if(checked)
    {
        ui->toolButton_zoomOut_2->setText("RINGS ON");
    }
    else
    {
        ui->toolButton_zoomOut_2->setText("RINGS OFF");
    }
    isScreenUp2Date = false;
}

void MainWindow::on_toolButton_zoomIn_2_toggled(bool checked)
{
    isDisplayHeading = checked;
    if(isDisplayHeading)
    {
        turningAngle = -heading/360*PI_NHAN2;
        radarData->setTrueN(-heading);
        ui->toolButton_zoomIn_2->setText("H UP");
    }else
    {
        turningAngle = 0;
        radarData->setTrueN(0);
        ui->toolButton_zoomIn_2->setText("N UP");
    }
}

void MainWindow::on_toolButton_standby_clicked()
{
    dataUpdate.stop();
    ui->toolButton_tx->show();
    ui->toolButton_gain->hide();
    ui->toolButton_rain->hide();
    ui->toolButton_sea->hide();
    ui->toolButton_standby->hide();
    radarData->resetData(true);
    update();
}

void MainWindow::on_toolButton_tx_clicked()
{
    dataUpdate.start(30);
    ui->toolButton_tx->hide();
    ui->toolButton_gain->show();
    ui->toolButton_rain->show();
    ui->toolButton_sea->show();
    ui->toolButton_standby->show();
    radarData->resetData(true);
    update();
}

void MainWindow::on_toolButton_zoomIn_4_clicked()
{
    vnmap.setUp(config.m_config.m_lat,config.m_config.m_long+config.m_config.scale, 500,config.m_config.mapFilename.data());
    DrawMap();
    update();
}

void MainWindow::on_toolButton_zoomIn_5_clicked()
{
    close();
}

void MainWindow::on_toolButton_zoomIn_3_clicked()
{

}

void MainWindow::on_toolButton_zoomIn_3_toggled(bool checked)
{

}

void MainWindow::on_toolButton_zoomIn_4_toggled(bool checked)
{

}
