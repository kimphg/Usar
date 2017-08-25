#ifndef C_RAW_DATA_H
#define C_RAW_DATA_H

#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE

#define HR2048//define radar type
#ifdef HR2048
#define STATE_STABLE                6
#define MIN_TERRAIN                 10
#define DELTA_T                     5.0f// 5 sec./vong quet
#define PI_NHAN2                    6.2831853072f
#define PI_CHIA2                    1.5707963268f
#define PI                          3.141592654f
#define MAX_OBJECTS                 40
#define MAX_TRACK_LEN               400
#define K_PROB_MISS                 1.3f
#define MES_GAIN                    0.2f
#define MAX_AZIR                    4096
#define MAX_TARGET_V                0.8f//km/period
#define RADAR_MAX_RESOLUTION        1024
#define THREE_SIGMA                 9// 2048/126km
#define SIGNAL_SCALE_1 0.096//0.094523077
#define SIGNAL_SCALE_2 0.064//0.063015385
#define SIGNAL_SCALE_3 0.032//0.031507692
#endif
#include <vector>
#include <math.h>
#include <QImage>
#include <QPixmap>
#ifdef _WIN32
#include <armadilloWin32/armadillo>
#endif
using namespace arma;
//#include <list>
using namespace std;
typedef struct {
    short x,y;
    //unsigned char level;
    unsigned char cfar;
    unsigned char terrain;
    short markIndex;
}raw_point_t;

typedef struct {
    raw_point_t raw_map[RADAR_MAX_RESOLUTION];
}frame_t;

typedef struct  {
    frame_t frame[MAX_AZIR];
} signal_map_t;

typedef struct  {
    short maxA,minA;
    double sumA,sumR;
    //unsigned short maxR,minR;
    //float sumTer;
    double size;
    //bool isProcessed;
} mark_t;
typedef struct  {
    float          az ,rg;
    short          size;
    float          x,y;
    float          p;
}object_t;
typedef std::vector<mark_t> markList;
typedef std::vector<object_t> objectList;

//______________________________________//
class track_t {
public:
    track_t()
    {

    }
    fmat q1,q2,h,p,x;
    objectList suspect_list,object_list;
    float deltaAzi;
    float estX ,estY;
    float estA, estR;
    float course, velocity;
    char state;
    bool isProcessed;
    bool isUpdated,confirmed;
    bool isMoving;//,confirmed;
    bool isManeuvering;
    void init(float mesA,float mesR)
    {
        q1<<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  4 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  4 <<endr;

        q2<<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  12 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  12 <<endr;

        h <<  1 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  1 <<  0 <<  0 <<endr;

        p << 100 <<  0 <<  0 <<  0 <<endr
          <<  0 << 100 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  200 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  200 <<endr;//ENVDEP

        x <<  0 <<endr
          <<  0 <<endr
          <<  0 <<endr
          <<  0 <<endr;
        //
        isManeuvering = false;
       isProcessed= false;
       isUpdated = false;
       estA = mesA;
       estR = mesR;
       deltaAzi = 0;
       velocity = 0;
       state = 3;
       if(object_list.size())object_list.clear();
       if(suspect_list.size())suspect_list.clear();
       isMoving = false;
       confirmed = false;
       predict();
    }
    void update()
    {
        float mesA;
        float mesR;

        float pmax = 0;
        short k=-1;
        for(unsigned short i=0;i<suspect_list.size();i++)
        {
            if(pmax<suspect_list[i].p)
            {
                k=i;
                pmax=suspect_list[i].p;
            }
        }
        if(k>=0)
        {
            mesA = suspect_list[k].az;
            mesR = suspect_list[k].rg;
            object_list.push_back(suspect_list[k]);
            isUpdated = true;// !!!
            suspect_list.clear();
        }
        else
        {
            isUpdated = false;
        }
        if(object_list.size()>2)
        {
            confirmed = true;
        }
        //
        //______________________________________//
        //Bayesian EKF tracking algorithm written by Phung Kim Phuong
        //______________________________________//

        if(isUpdated)
        {
            if(state<10)state++;
            //measurement :
            float cc = mesR*cosf(mesA-estA)-estR;
            float dd = mesR*tanf(mesA-estA);
            fmat z;
            z<<cc<<endr<<dd<<endr;
            //z.print("\nZ:\n");return;//!!!
            //measurement error matrix r:
            //rr = estR*estR*0.0003// azi error for HR2D is about 1 deg -> (2*pi/360)^2=0.0003
            //range error is about 1.5-> 1.5^2=2
            fmat r;
            //r<<(object_list[object_list.size()-1].drg+1)*(object_list[object_list.size()-1].drg+1)<<0<<endr
            // <<0<<(object_list[object_list.size()-1].daz+1)*(object_list[object_list.size()-1].daz+1)/100.0<<endr;//NIM!!!
            r<<2<<0<<endr
             <<0<<estR*estR*0.0002<<endr;//NIM!!!
//            printf("object_list[object_list.size()-1].daz:%f",object_list[object_list.size()-1].daz);
//            r.print("\nr:------------\n");
            //Kalman gain:
            fmat k;
            k = p*h.t()*inv(h*p*h.t() + r);
            //k.ones();
            //k.print("\nk:--------\n");
            //correct estimation:
            //x.print("\nXP:");
            x = x+k*(z-h*x);
            //x.print("\nX:");
            deltaAzi = atanf(x.at(3,0)/estR);
            estA += deltaAzi;
            estR += x.at(2,0);//delta range = x.at(3,1);
            object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            //correct error covariance:
            p = p - k*h*p;
            if(confirmed)predict();
        }else//miss:
        {
            if(state)state--;
            deltaAzi = atan(x.at(3,0)/estR);
            estA += deltaAzi;
            estR += x.at(2,0);
            object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            if(confirmed)predict();
        }
        if(confirmed)
        {
            if(object_list.size()>MAX_TRACK_LEN)// lich su qui dao toi da
            {
                objectList new_ojb_list;
                new_ojb_list.swap(object_list);
                for(unsigned short i = 20; i < new_ojb_list.size() ; i++)//cut off 20 first elements
                {
                    object_list.push_back(new_ojb_list[i]);
                }
            }
            float dx,dy;
            if(object_list.size()>12)
            {
                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-12].x;
                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-12].y;
                float dist = dx*dx+dy*dy;
                velocity += 0.5f*(sqrtf(dist)/11.0f/(DELTA_T)-velocity);
            }
            else if(object_list.size()>6)
            {
                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-6].x;
                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-6].y;
                float dist = dx*dx+dy*dy;
                velocity  += 0.5f*(sqrtf(dist)/5.0f/(DELTA_T)-velocity);//= sqrtf(dist)/5.0f/(DELTA_T);
            }
            else if(object_list.size()>3)
            {
                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-2].x;
                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-2].y;
                float dist = dx*dx+dy*dy;
                velocity  += 0.5f*(sqrtf(dist)/(DELTA_T)-velocity);//= sqrtf(dist)/DELTA_T;
            }

            course = atanf(dx/dy);
            if(dy<0)course+=PI;
            if(course<0)course+=PI_NHAN2;
            if(velocity>0.05f)//pixel/s
            {
                isMoving = true;
            }
            else
            {
                isMoving = false;
            }
        }
    }
    void predict()
    {
        float aa = cos(deltaAzi);
        float bb = sin(deltaAzi);//NIM
        isManeuvering = (deltaAzi>0.001);
        //printf("\n delta azi:%f",deltaAzi);
        fmat a;// jacobian matrix
        a <<  0 <<  0 <<  aa<<  bb<<endr
          <<  0 <<  0 << -bb<<  aa<<endr
          <<  0 <<  0 <<  aa<<  bb<<endr
          <<  0 <<  0 << -bb<<  aa<<endr;
        x = a*x;
        //predict error covariance:
        if(isManeuvering)p = a*p*a.t()+q2;
        else p = a*p*a.t()+q1;
    }
    bool checkProb(object_t* object)
    {
        float dA = object->az - estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;//----------------
        float dR = object->rg - estR;
        dA*=dA;
        dR*=dR;
        if(confirmed&&(state>STATE_STABLE))
        {
            if(dR>=9 || dA>=0.0007f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
            object->p = (1.0f-dR/9.0f)*(1.0f-dA/0.0007f);
        }else
        {
            if(dR>=27 || dA>=0.0021f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
            object->p = (1.0f-dR/27.0f)*(1.0f-dA/0.0021f);
        }
        return true;
    }

};
typedef std::vector<track_t> trackList;
//______________________________________//
class C_radar_data {
public:

    C_radar_data();
    ~C_radar_data();
    trackList               mTrackList;
    markList                mark_list;
    signal_map_t            signal_map;
    unsigned char           thresh,overload,brightness;
    int                  noiseLevel;
    bool spreading;
    float                   temp;
    float                   trueN;
    unsigned char           tempType;
    QImage                *sgn_img;
    void getPolar(float x,float y,float *azi,float *range)
    {
        *azi = atanf(x/y);//tinh azi theo chuan bac thuan kim dong ho
        if(y<0)*azi+=PI;
        if(azi<0)*azi += PI_NHAN2;
        *range = sqrt(x*x+y*y);
    }
    //______________________________________//
    void GetDataHR(unsigned char *data, unsigned short dataLen);
    void addSimObject(short ox, short oy);
    void GetDataSimulator(QPixmap* image,unsigned short azi);
    void        raw_map_init();
    void        addTrack(float x, float y);
    void        setTrueN(float trueN_deg){

        while(trueN_deg<0)trueN_deg+=360;
        while(trueN_deg>=360)trueN_deg-=360;
        trueN=(trueN_deg/360.0f*PI_NHAN2);
        raw_map_init();
    }
    void        resetData(bool sprding);
    void        setProcessing(bool onOff);
    //bool        getDataOverload(){if(isDataTooLarge) {isDataTooLarge =false;return true;} else return false;}
    bool        checkFeedback(unsigned char* command)
    {
        for (short i=0;i<8;i++)
        {if(command[i]!=command_feedback[i])return false;}
        memset(&command_feedback[0],0,8);
        return true;
    }
private:
    unsigned char command_feedback[8];
    //bool        isDataTooLarge;
    void        polarToXY(float *x, float *y, float azi, float range);
    bool        isProcessing;
    short       curAzir;
    void        procPix(short proc_azi,short range);
    void        procTracks(unsigned short curA);
    void        procMark(mark_t* pMark);
    void        procObject(object_t* pObject);

    //void status_start();
    //FILE *pFile;
};

//extern C_radar_data radarData;

#endif
