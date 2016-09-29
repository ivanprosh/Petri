#ifndef NOTE_H
#define NOTE_H
#include <QDebug>

class Note;
//общая очередь записей
extern QQueue<Note> queue;
extern QStringList AvailFamil;

//функция перевода int в char
char Ch(int s) { return s+'a'; }

class Note{
private:
    const int id;
public:
    static int count;
    //Data
    QString family,address;
    int ph_number;

    void filling(){
        //набор данных случайным образом
        family = QString(Ch(qrand()%26)) + QString(Ch(qrand()%26)) + "nova";
        address = "St.Lenina " +  QString::number(qrand()%100) + " kv." + QString::number(qrand()%100);
        ph_number = 10000*(qrand()%9)+1000*(qrand()%9)+100*(qrand()%9)+10*(qrand()%9)+qrand()%9;
        //добавим в перечень фамилий
        AvailFamil << family;
        qDebug() << family << "," << address << "," << ph_number;

        queue.enqueue(*this);
    }

    Note():id(++count){
        qDebug() << "New id is " << id;
    }
    Note& operator=(Note& rhv){
       // this->id = rhv.id;
        this->family = rhv.family;
        this->address = rhv.address;
        this->ph_number = rhv.ph_number;
        return *this;
    }
};

#endif // NOTE_H

