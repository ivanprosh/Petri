#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QSemaphore>
#include <QFile>
#include <QTime>
#include <QStringList>
#include "mythread.h"
#include "Mod_Petri/zpetri.hxx"
#include "Mod_Petri/zpetri-env.hxx"

using namespace std;

class Error
{
public:
    QString descr;
    Error(QString str):descr(str){}
};

QStringList AvailFamil;

//������� �������� int � char
char Ch(int s) { return s+'a'; }

class Note{
private:
    const int id;
public:
    static int count;
    //Data
    QString family,address;
    int ph_number;

    Note():id(++count){
        qDebug() << "New id is " << id;

        //����� ������ ��������� �������
        family = QString(Ch(qrand()%26)) + QString(Ch(qrand()%26)) + "nova";
        address = "St.Lenina " +  QString::number(qrand()%100) + " kv." + QString::number(qrand()%100);
        ph_number = 10000*(qrand()%9)+1000*(qrand()%9)+100*(qrand()%9)+10*(qrand()%9)+qrand()%9;
        //������� � �������� �������
        AvailFamil << family;
        qDebug() << family << "," << address << "," << ph_number;
    }
};
int Note::count = 0;

QSemaphore semaphore_sync;
//��������� �� �������� ������� ��� �������
void (*algorithm)(int i);
//��������� ����� ��� ������ � ����
QTextStream* out;
QFile* outputFile;
//���������� ����������� ���������
int N(0),M(0),PT(0);
//������ ������������
QString PA;
/*
//����� ���-�� �������
int count = 0;
//������� ���-�� ������� � ������
int curThreadCount = 0;
//��� ���������� �������������� ������� � ����. ������
QMutex PetriJump;
int countThreadOnCycle = 0;

*/

/*
//������� ������/������� ���� �����
int pnodes[2] =
{
  1,0
};

int petry_out[2][2] =
{
  { 1,0 },
  { 0,1 }
};

int petry_in[2][2] =
{
  { 0,1 },
  { 1,0 }
};

//������� � ���� ����� (������ �������� � ������ ������)
void move_petry( int t )
{
    bool fl = false;
    int i;

    while (!fl)
    {

        fl = true;
        for( i = 0; i < 2; i++ )
        {
            if (pnodes[i] < petry_in[t][i])
            {fl = false;break;}
        }

        if (fl)
        {
            PetriJump.lock();
            // �������� ����� �������
            for( i = 0; i < 2; i++ )
            {
                pnodes[i] -= petry_in[t][i];
                pnodes[i] += petry_out[t][i];
            }

//#ifdef QT_DEBUG
            for( i = 0; i < 2; i++ )
            {
                //qDebug() << pnodes[i] << ' ';
            }
//#endif
            PetriJump.unlock();
        }

        //if (!fl) QThread::currentThread()->msleep(100);
    }
}
*/
/*
void SJF_n(int i)
{
    QVector<TDetail*>* debug = &sortVecDetails;
    int* temp = &countThreadOnCycle;

    while(::count>0 && (vecDetails.at(i)->state != 2))//���� ���� ������
    {
        //������������� � ������ ������
        semaphore_sync.acquire();
        syncmutex.lock();
        synchronize.wait(&syncmutex);
        curThreadCount++;
        countThreadOnCycle++;
        syncmutex.unlock();
        semaphore_sync.release();


        //���� �� �������� ������� �������� SJF ��� ������� ������ - ����
        while(sortVecDetails.back()->id!=i);

        if(!sortVecDetails.empty()){
            // ���������� ����� ����� (����� �����)
            move_petry(1);
            //��������� ������ � ������� �������
            sortVecDetails.pop_back();
            // �������� �������� ��������� ������
            work(i,MaxT);
            //������ �����
            move_petry(0);
            //��������� ������������ ������
            if(vecDetails.at(i)->state != 2) QThread::currentThread()->msleep(MaxT);

            //�������, ��� ������ ������ ��������� �� ������ ������, ��������� ���������� ���������� �������
            syncmutex.lock();
            curThreadCount--;
            syncmutex.unlock();
        }
    }    
}
*/

//������ �� �����
void rfile(const QString& name)
{
    QSettings settings(name, QSettings::IniFormat);
    //�����
    settings.beginGroup("common");
    const QStringList childKeys = settings.childKeys();
    if(childKeys.empty()) throw Error("Something wrong with input file");

    PA = settings.value("PA").toString();
    N = settings.value("N").toInt();
    M = settings.value("M").toInt();
    PT = settings.value("PT").toInt();

    if(M==0) M=QThread::idealThreadCount();

    settings.endGroup();  

#ifdef QT_DEBUG
    //���������� �����
    qDebug() << " PA="<< PA << " N="<< N << " M="<< M << " PT="<< PT;
    qDebug() << "*******************Finish read input file********************************";
#endif
}
/*
//����� ����������� � ����
void print(QTextStream* out)
{
    //syncmutex.lock();

    *out << qSetFieldWidth(4) << kvant++ << "- ";

    for(int i = 0; i < vecDetails.size(); i++ )
    {
        if (vecDetails.at(i)->state==0)
            *out << "   w";
        else if (vecDetails.at(i)->state==2)
            *out << "    ";
        else
        {
            int cur_mach = vecDetails.at(i)->cur_mach;
            // ����� ������ � ����������� �������
            *out << qSetFieldWidth(4) << QString::number(vecDetails.at(i)->curtime) + Ch(vecDetails.at(i)->stanok_i[cur_mach]-1);
        }
    }

    *out << " # " ;
    for(int i = 0; i < vecMachines.size(); i++ )
        *out << " " << vecMachines[i]->Details.size();

    *out << " # " << ::count << endl;

    //syncmutex.unlock();
}
*/
int main(int argc, char** argv)
{
  QCoreApplication  app(argc, argv);

  outputFile = new QFile("output.txt");
  out = new QTextStream(outputFile);

  if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text ))
      qDebug() << "Cann't open output file!";
  else qDebug() << "open output file!";

  try{
        rfile("input.ini");

        //������������� ���������� ����. �����
        QTime midnight(0,0,0);
        qsrand(midnight.secsTo(QTime::currentTime()));

        Note n1,n2,n3;
        /*
        //������� ������
        for(int i=0;i<vecDetails.size();i++)
        {
          MyThread *thr = new MyThread(algorithm,vecDetails.at(i)->id);
          thr->start();
          vecThreads.push_back(thr);
        }

  //������� ��� �������������
  semaphore_sync.release(vecDetails.size());

  while (::count>0)
  {
      int deadlock;
      for(int i=0;i<vecDetails.size();i++){
        if(vecDetails.at(i)->state!=2) sortVecDetails.push_back(vecDetails.at(i)); //������ ��� ������� �������
      }
      int sizeDet = sortVecDetails.size();

      if(algorithm==SJF_n){
        //��������� �� �������� ������������ CPU_burst
        qSort( sortVecDetails.begin(), sortVecDetails.end(),moreCPU_BurstThen);
      } else if (algorithm==SJF_p) {
        qSort( sortVecDetails.begin(), sortVecDetails.end(),morePriorThen);
      } else throw Error("Algorithm sorting is undefined for current method PA");

      //��������� ���������� �������
      for(int i=0;i<vecMachines.size();i++)
      {
          vecMachines.at(i)->ready |= vecMachines.at(i)->readyNextCycle;
          vecMachines.at(i)->readyNextCycle = 0;
      }

      //����� ������

      while(semaphore_sync.available()>(vecDetails.size()-sizeDet));
      synchronize.wakeAll();

      //���� ���������� ������� �� ����� ������������
      while(countThreadOnCycle!=sizeDet){
          QThread::currentThread()->msleep(0);
      }
      deadlock = 0;
      //���� ������ �� ����������� ���
      while(curThreadCount>0) {
          QThread::currentThread()->msleep(0);
      }


      //����� �� �����
      print(out);

      countThreadOnCycle = 0;
  }

  *out << "Finish!";
    */
  }
  catch(Error err){
      qDebug() << "Error: "<< err.descr;
      outputFile->close();
  }

  outputFile->close();
  return app.exec();
}



