#include "c_arpa_data.h"
#include <math.h>
#define PI 3.141592654f
C_ARPA_data::C_ARPA_data()
{

}

C_ARPA_data::~C_ARPA_data()
{

}
void C_ARPA_data::SortTrack()
{
    /*for (unsigned short i=0;i<track_list.size()-1;i++)
    {
        for(unsigned short j=i+1;j<track_list.size();j++)
        {
            if(track_list[i].id>track_list[j].id)
            {
                ARPA_track temp= track_list[i];
                track_list[i] = track_list[j];
                track_list[j] = temp;
            }
        }
    }*/
}
void C_ARPA_data::processData(char* data, unsigned short len)
{
    for(short i=0;i<len;i++)
    {
        if(*data==0)(*data)++;
    }

    QString str(data);
    QStringList strList = str.split(",");
    if((*strList.begin()).contains("TTM"))
    {
        std::string tNum;
        tNum.append((*(strList.begin()+1)).toStdString());
        float tDistance = (*(strList.begin()+2)).toFloat()*CONST_NM;
        float tazi = (*(strList.begin()+3)).toFloat();// in degrees
        if(tazi<0)tazi += 360;
        float velocity = (*(strList.begin()+5)).toFloat();
        float course = (*(strList.begin()+6)).toFloat();

        addARPA(tNum,tDistance,tazi,course,velocity);
    }

}
void C_ARPA_data::addARPA(std::string id,float r,float a,float course,float velocity)
{
    ARPA_object_t newobj;
    newobj.centerA = a;
    newobj.centerR = r;//in km
    newobj.centerX = newobj.centerR*sinf(newobj.centerA);
    newobj.centerY = - newobj.centerR*cosf(newobj.centerA);
    newobj.course  =  course;//radian
    newobj.velocity = velocity*18;
    newobj.time = QDateTime::currentMSecsSinceEpoch();

    for(ushort i = 0;i<track_list.size();i++)
    {
        if(!id.compare(track_list[i].id))
        {
            //printf(id.data());
            track_list[i].addObject(&newobj);
            return;
        }
    }
    ARPA_track newtrack;
    newtrack.addObject(&newobj);
    newtrack.id.clear();
    newtrack.id.append(id);
    //printf(newtrack.id.data());return;
    track_list.push_back(newtrack);
    //SortTrack();
}
