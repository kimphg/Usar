#define PI 3.141592654f
#include "C_radar_data.h"
#include <math.h>
#include <QtDebug>

//static period_t                curPeriod;
//static std::queue<period_t>    period_cache;
//static unsigned short cur_mark_index = 0;
//static unsigned short cur_object_index = 0;

C_radar_data::C_radar_data()
{
    sgn_img = new QImage(RADAR_MAX_RESOLUTION*2+1,RADAR_MAX_RESOLUTION*2+1,QImage::Format_ARGB32);
    thresh = 3;
    isProcessing = true;
    brightness = 10;
    setTrueN(0);
    resetData(true);
}
C_radar_data::~C_radar_data()
{
    delete sgn_img;
    //if(pFile)fclose(pFile);
//    if(pScr_map)
//    {
//        delete[] pScr_map;
//    }
}
void C_radar_data::setProcessing(bool onOff)
{
    if(onOff)
    {

        resetData(true);
        isProcessing = true;
    }
    else
    {
        isProcessing = false;
    }
}
#define RADAR_COMMAND_FEEDBACK 6
#define RADAR_RAW_DATA 14
void C_radar_data::addSimObject(short ox, short oy)
{

    /*float azi = atan(ox/oy);
    if(ox<0){
        if(oy<0)
        azi+=3.141592654f;
        else
        azi+=PI_NHAN2;
    }
    */


}
void C_radar_data::GetDataSimulator(QPixmap* image,unsigned short azi)
{
    float alpha = PI_NHAN2/MAX_AZIR*azi;
    //printf("%f\n",alpha);
    short x,y;
    short imgctx = image->width()/2-1;
    short imgcty = image->height()/2-1;
    QImage img = image->toImage();
    short lastazi=azi-1,nextazi=azi+1;
    if(lastazi<0)lastazi+=MAX_AZIR;
    if(nextazi>=MAX_AZIR)nextazi-=MAX_AZIR;
    for(unsigned short i=0;i<RADAR_MAX_RESOLUTION;i++)
    {
        x = sinf(alpha)*i;
        y = -cosf(alpha)*i;
        if(abs(x)>=imgctx||abs(y)>=imgcty)break;
        if(!(rand()%(10+i/100)))
        {
            if(((img.pixel(imgctx+x,imgcty+y)!=img.pixel(imgctx+x+1,imgcty+y))||(img.pixel(imgctx+x,imgcty+y)!=img.pixel(imgctx+x,imgcty+y+1))))
            {
                sgn_img->setPixel(signal_map.frame[lastazi].raw_map[i].x,signal_map.frame[lastazi].raw_map[i].y,0xffffff00);
                signal_map.frame[azi].raw_map[i].cfar  = 255;
                //printf("%d-%d\n",azi,y);
            }
            else
            {

                signal_map.frame[azi].raw_map[i].cfar =0;
                short l = 0;
                bool nearby = false;
                for(short k= azi-10;;k++)
                {
                    l++;
                    if(l>20)break;
                    if(k<0)k+=MAX_AZIR;
                    if(k>=MAX_AZIR)k-=MAX_AZIR;
                    if(signal_map.frame[k].raw_map[i].cfar)
                    {
                        nearby=true;
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[i].x,signal_map.frame[azi].raw_map[i].y,0xffffff00-0x00151500*abs(l-10));
                    }
                }
                if(!nearby)sgn_img->setPixel(signal_map.frame[lastazi].raw_map[i].x,signal_map.frame[lastazi].raw_map[i].y,0xff0000ff);
            }
        }
        else
        {

            signal_map.frame[azi].raw_map[i].cfar =0;
            short l = 0;
            bool nearby = false;
            for(short k= azi-10;;k++)
            {
                l++;
                if(l>20)break;
                if(k<0)k+=MAX_AZIR;
                if(k>=MAX_AZIR)k-=MAX_AZIR;
                if(signal_map.frame[k].raw_map[i].cfar)
                {
                    nearby=true;
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[i].x,signal_map.frame[azi].raw_map[i].y,0xffffff00-0x000f0f00*abs(l-10));
                }
            }
            if(!nearby)sgn_img->setPixel(signal_map.frame[lastazi].raw_map[i].x,signal_map.frame[lastazi].raw_map[i].y,0xff0000ff);
        }

    }
    sgn_img->setPixel(RADAR_MAX_RESOLUTION,RADAR_MAX_RESOLUTION,0xffff00ff);
}
void C_radar_data::GetDataHR(unsigned char* data,unsigned short dataLen)
{
    short azi = 0xfff & (data[4] << 8 | data[5]);
    overload = data[4]>>5;//mrHung
    temp = data[3]/4.0f;//
    tempType = data[2];
    for (short i=0;i<8;++i)
    {
        command_feedback[i] = data[i+RADAR_COMMAND_FEEDBACK];
    }
    if(curAzir == azi){return;}
    short lastazi=azi-1;//,nextazi=azi+1;
    if(lastazi<0)lastazi+=MAX_AZIR;
    //else if(nextazi>=MAX_AZIR) nextazi -= MAX_AZIR;
    if(lastazi!=curAzir)
    {
        azi = lastazi;
    }// !!! sua loi mat frame
    curAzir = azi;
    if(!((unsigned char)(curAzir<<2))){
        procTracks(curAzir);
    }
    //overload = data[4] >> 4;
    unsigned short r_pos=1;//procPix chay bat dau tu 10
    //printf("\nazi:%d",azi);
    //unsigned short nData=0;
    dataLen--;
    for(unsigned short i = RADAR_RAW_DATA; (r_pos < RADAR_MAX_RESOLUTION-1);)//r_pos bo qua gia tri ngoai cung
    {
        uint color=0;
        if(data[i]==0xff)
        {

            unsigned short compressed_end = r_pos + (data[i+1]);
            if(compressed_end>RADAR_MAX_RESOLUTION-1)compressed_end = RADAR_MAX_RESOLUTION-1;
            for(;r_pos<compressed_end;++r_pos)
            {
                signal_map.frame[azi].raw_map[r_pos].cfar  = 0;
                if(signal_map.frame[azi].raw_map[r_pos].terrain )
                {
                    signal_map.frame[azi].raw_map[r_pos].terrain -= (signal_map.frame[azi].raw_map[r_pos].terrain)>>2;//terrain*0.75
                    if(signal_map.frame[azi].raw_map[r_pos].terrain*brightness>0xff)
                    {
                        color = 0xff008090;

                    }
                    else
                    {
                         color = 0x00008090|((signal_map.frame[azi].raw_map[r_pos].terrain*brightness)<<24);
                    }
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y,color);
                    if(spreading)
                    {
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x+1,signal_map.frame[azi].raw_map[r_pos].y,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x-1,signal_map.frame[azi].raw_map[r_pos].y,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y+1,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y-1,color);
                    }
                }
            }
            ++i;
            ++i;
        }
        else
        {
            //signal_map.frame[azi].raw_map[r_pos].level=data[i];
            if(data[i])
            {
                signal_map.frame[azi].raw_map[r_pos].cfar  = data[i];
                if(data[i]*brightness>0xff)
                {
                    color = 0xff00ff00;
                }else
                {
                    color =0x0000ff00|((data[i]*brightness)<<24);
                }
                signal_map.frame[azi].raw_map[r_pos].terrain += (data[i]-signal_map.frame[azi].raw_map[r_pos].terrain)>>2;
                procPix(azi,r_pos);
                sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y,color);
                if(spreading)
                {
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x+1,signal_map.frame[azi].raw_map[r_pos].y,color);
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x-1,signal_map.frame[azi].raw_map[r_pos].y,color);
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y+1,color);
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y-1,color);
                }
            }
            else
            {
                signal_map.frame[azi].raw_map[r_pos].cfar     = 0;
                if(signal_map.frame[azi].raw_map[r_pos].terrain )
                {
                    signal_map.frame[azi].raw_map[r_pos].terrain -= (signal_map.frame[azi].raw_map[r_pos].terrain)>>2;//terrain*0.75
                    if(signal_map.frame[azi].raw_map[r_pos].terrain*brightness>0xff)
                    {
                        color = 0xff008090;
                    }
                    else
                    {
                         color = 0x00008090|((signal_map.frame[azi].raw_map[r_pos].terrain*brightness)<<24);
                    }
                    sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y,color);
                    if(spreading)
                    {
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x+1,signal_map.frame[azi].raw_map[r_pos].y,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x-1,signal_map.frame[azi].raw_map[r_pos].y,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y+1,color);
                        sgn_img->setPixel(signal_map.frame[azi].raw_map[r_pos].x,signal_map.frame[azi].raw_map[r_pos].y-1,color);
                    }
                }
            }
            r_pos++;
            i++;
        }

        if((i >= dataLen))break;
    }
    if(r_pos < RADAR_MAX_RESOLUTION-1)
    {
        for(;r_pos < RADAR_MAX_RESOLUTION-1;++r_pos)
        {
            signal_map.frame[azi].raw_map[r_pos].cfar  = 0;
            signal_map.frame[azi].raw_map[r_pos].terrain -= (signal_map.frame[azi].raw_map[r_pos].terrain)>>2;//terrain*0.75

        }

    }
//    if(isProcessing&&!isDataTooLarge)//ENVDEP
//    {
//        //printf("\n data:%d",nData);
//        if(nData>100)
//        isDataTooLarge = true;

//    }
    return ;
    /*
	short azi = 0xfff & (buff[ADDR_AZI_H] << 8 | buff[ADDR_AZI_L]);
    if(curAzir==azi) return GetData();
    curAzir = azi;
    if(curAzir==4095){
        curPeriodIndex++;
        procTracks();
    }
    for(short r = 1; r < 1023; r++)
    {
        short i = (r>>3);
        short j = (r&0x0007);
        if((buff[VIDEO_RAW_LENGTH+i]>>j & 0x1))
        {


            //signal_map.frame[azi].raw_map[r].level = signal_map.frame[azi].raw_map[r].level<<
            //if(signal_map.frame[azi].raw_map[r].level<80)
            signal_map.frame[azi].raw_map[r].cfar  = 1;
            signal_map.frame[azi].raw_map[r].level = buff[r];
            signal_map.frame[azi].raw_map[r].terrain = float(signal_map.frame[azi].raw_map[r].terrain*0.95 + 0.05);//255*0.125;

            procPix(azi,r);
        }
        else
        {
            signal_map.frame[azi].raw_map[r].cfar  = 0;
            signal_map.frame[azi].raw_map[r].level = buff[r];
            signal_map.frame[azi].raw_map[r].terrain = float(signal_map.frame[azi].raw_map[r].terrain*0.95);
            //signal_map.frame[azi].raw_map[r].level = 0;
        }

    }
	delete[] buff;
    return azi;*/

}
void C_radar_data::procMark(mark_t* pMark)
{
    float ctA = pMark->sumA/pMark->size/MAX_AZIR*PI_NHAN2+trueN;
    float ctR = pMark->sumR/pMark->size;
    if(ctA >= PI_NHAN2)ctA -= PI_NHAN2;
    if((pMark->size>3))//ENVDEP
    {
            object_t newobject;
            newobject.az   = ctA;
            //newobject.daz  = pMark->maxA-pMark->minA;
            //if(newobject.daz<0)newobject.daz+=PI_NHAN2;
            newobject.rg   = ctR;
            //_________convert center AR to XY_______________//
            polarToXY(&newobject.x,
                      &newobject.y,
                      ctA,
                      ctR);
            procObject(&newobject);
    }

}
void C_radar_data::procTracks(unsigned short curA)
{
    //process all marks
    short pr_curA = curA-1;
    if(pr_curA<0)pr_curA+=MAX_AZIR;
    for(unsigned short i = 0;i<mark_list.size();++i)
    {
        if(mark_list[i].size)
        {
            if((mark_list[i].maxA!=curA)&&(mark_list[i].maxA!=pr_curA))
            {
                procMark(&mark_list[i]);
                mark_list[i].size =0;
            }

        }
    }
    //proc_track
    float azi = (float)curA/MAX_AZIR*PI_NHAN2+trueN;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList[i].state)continue;
        float dA = azi-mTrackList[i].estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;
        if(mTrackList[i].isProcessed)
        {
            if(abs(dA)<0.35f)//20 deg
            {
                mTrackList[i].isProcessed = false;
            }
        }
        else
        {
            if(abs(dA)>0.35f)//20 deg
            {
                mTrackList[i].isProcessed = true;
                mTrackList[i].update();
            }
        }
    }


}
void C_radar_data::addTrack(float x,float y)
{
    //add new track
    object_t mObject;
    float azi = atanf(x/y);//tinh azi theo chuan bac thuan kim dong ho
    if(y<0)azi+=PI;
    if(azi<0)azi += PI_NHAN2;
    float range = sqrt(x*x+y*y);
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList[i].state)
        {
            mObject.x = x;
            mObject.y = y;
            mTrackList[i].init(azi,range);
            mTrackList[i].object_list.push_back(mObject);
            return;
        }
    }
    track_t newTrack;
    mObject.x = x;
    mObject.y = y;
    newTrack.init(azi,range);
    newTrack.object_list.push_back(mObject);
    mTrackList.push_back(newTrack);
}
void C_radar_data::procObject(object_t* pObject)
{
        for(unsigned short i=0;i<mTrackList.size();i++)
        {
            if(mTrackList[i].state&&(!mTrackList[i].isProcessed))
            {
                if(mTrackList[i].checkProb(pObject)){
                    mTrackList[i].suspect_list.push_back(*pObject);
                }
            }
        }
}
void C_radar_data::procPix(short proc_azi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{

    short pr_proc_azi = proc_azi-1;
    if(pr_proc_azi<0)pr_proc_azi+=MAX_AZIR;
    short markIndex =-1;
    if(signal_map.frame[pr_proc_azi].raw_map[range].cfar)
    {
         markIndex = signal_map.frame[pr_proc_azi].raw_map[range].markIndex;

    }else if(signal_map.frame[proc_azi].raw_map[range-1].cfar)
    {
         markIndex = signal_map.frame[proc_azi].raw_map[range-1].markIndex;

    }
    else if(signal_map.frame[pr_proc_azi].raw_map[range-1].cfar)
    {
         markIndex = signal_map.frame[pr_proc_azi].raw_map[range-1].markIndex;

    }
    else if(signal_map.frame[pr_proc_azi].raw_map[range+1].cfar)
    {
         markIndex = signal_map.frame[pr_proc_azi].raw_map[range+1].markIndex;
    }
    if(markIndex!=-1)// add to existing marker
    {
        signal_map.frame[proc_azi].raw_map[range].markIndex = markIndex;
        mark_list[markIndex].size++;
        if(proc_azi<mark_list[markIndex].minA){
            mark_list[markIndex].sumA    +=  proc_azi + MAX_AZIR;
            mark_list[markIndex].maxA    =  proc_azi + MAX_AZIR;
        }else
        {
            mark_list[markIndex].sumA    +=  proc_azi;
            mark_list[markIndex].maxA    =  proc_azi;
        }
        /*if(range<mark_list[markIndex].minR)
        {
            mark_list[markIndex].minR = range;
        }else if(range>mark_list[markIndex].maxR)
        {
            mark_list[markIndex].maxR = range;
        }*/
        mark_list[markIndex].sumR    +=  range;


    }
    else//_________new marker found_____________//
    {

        mark_t              new_mark;
        new_mark.maxA =  new_mark.minA  = proc_azi;
        //new_mark.minR = new_mark.maxR = range;
        new_mark.size =  1;
        new_mark.sumA =  proc_azi;
        new_mark.sumR =  range;
        bool listFull = true;

        for(unsigned short i = 0;i<mark_list.size();++i)
        {
                //  overwrite
                if(!mark_list[i].size)
                {
                    signal_map.frame[proc_azi].raw_map[range].markIndex =  i;
                    mark_list[i] = new_mark;
                    listFull = false;
                    break;
                }
        }
        if(listFull)
        {
            mark_list.push_back(new_mark);
            signal_map.frame[proc_azi].raw_map[range].markIndex = mark_list.size()-1;
            //printf("\nmark_list.size():%d",mark_list.size());
        }

        //}
        //else//fill the buffer
        //{
        //    cur_mark_index                                      = mark_list.size();
        //    signal_map.frame[proc_azi].raw_map[range].markIndex =  cur_mark_index;
        //    mark_list.push_back(m_mark);
        //}
        /*bool listFull = true;
        for(unsigned short i=0;i<mark_list.size();i++)
        {
            if(mark_list[i].isProcessed)
            {
               mark_list[i] = m_mark;
               listFull = false;
               break;
            }
        }
        if(listFull)
        {
            mark_list.push_back(m_mark);
            printf("\nmark_list.size():%d",mark_list.size());
        }*/
    }
            //______________________________________//




}
/*void C_radar_data::polarToSnXY(short *xsn, short *ysn, short azi, short range)
{
    *xsn = signal_map.frame[azi].raw_map[range].x;
    *ysn = signal_map.frame[azi].raw_map[range].y;
}
//static short ctX=0,ctY=0;
//static float dr = 0;
*/
void C_radar_data::polarToXY(float *x, float *y, float azi, float range)
{

    *x = ((sinf(azi)))*range;
    *y = ((cosf(azi)))*range;
}
void C_radar_data::raw_map_init()
{
//    if(pScr_map)
//    {
//        delete[] pScr_map;
//    }
//    pScr_map = new raw_point_t[max_X][max_Y];

    float theta=trueN;
    float dTheta = 2*PI/MAX_AZIR;
    //int azir,range;
    //float r = scale*MAX_RANGE_KM;
    //printf("\nr:%f",r);
    //dr = 1;//r/MAX_RANGE;
    //ctX = max_X/2;
    //ctY = max_Y/2;
    for(short azir = 0;azir< MAX_AZIR;azir++)
	{

		float cost = cosf(theta);
		float sint = sinf(theta);

        for(short range = 0;range<RADAR_MAX_RESOLUTION;range++)
		{
            signal_map.frame[azir].raw_map[range].x     =  short(sint*range)+RADAR_MAX_RESOLUTION;
            signal_map.frame[azir].raw_map[range].y     =  -short(cost*range)+RADAR_MAX_RESOLUTION;
		}
		theta+=dTheta;
	}
}
void C_radar_data::resetData(bool sprding)
{
    spreading = sprding;
    for(short azir = 0;azir< MAX_AZIR;azir++)
    {
        for(short range = 0;range<RADAR_MAX_RESOLUTION;range++)
        {
            signal_map.frame[azir].raw_map[range].cfar  = 0;

            //signal_map.frame[azir].raw_map[range].level  = 0;
            signal_map.frame[azir].raw_map[range].terrain = 0;//NGUONG_DIA_VAT/2;
        }
    }
    sgn_img->fill(QColor(0xff0000ff));
}

