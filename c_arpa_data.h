#ifndef C_ARPA_DATA_H
#define C_ARPA_DATA_H
#include <vector>
#include <QDateTime>
#include <QStringList>
#ifndef CONST_NM
#define CONST_NM 1.825f
#endif
typedef struct  {
    float               centerA,centerR;//
    float               centerX,centerY;//in km
    float               course ;
    float               velocity;
    qint64    time;
}ARPA_object_t;

typedef std::vector<ARPA_object_t> ArpaobjectList;
class ARPA_track
{
public:
    ARPA_track(){}
    ArpaobjectList object_list;
    void addObject(ARPA_object_t*newobj)
    {
        object_list.pop_back();
        object_list.push_back(*newobj);
        centerX =   newobj->centerX;
        centerY =   newobj->centerY;
        centerA =   newobj->centerA;
        centerR =   newobj->centerR;
        course = newobj->course ;
        velocity =  newobj->velocity;
        lives = 40;
    }

    float          centerX,centerY;
    float          centerA,centerR;
    float          course ;
    float          velocity;
    std::string           id;
    bool            selected;
    unsigned short            lives;
};
typedef std::vector<ARPA_track> ArpatrackList;
class C_ARPA_data
{
public:
    C_ARPA_data();
    ~C_ARPA_data();
    void SortTrack();
    void processData(char *data, unsigned short len);
    ArpatrackList track_list;
    void addARPA(std::string id, float r, float a, float course, float velocity);
};
#endif // C_ARPA_DATA_H
