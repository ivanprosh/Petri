#include <QSettings>
#include <QCoreApplication>
#include <QSemaphore>
#include <QFile>
#include <QTime>
#include <QStringList>
#include <QQueue>
#include <QThreadPool>

#include "mythread.h"
#include "mypetri.h"
#include "Mod_Petri/zpetri.hxx"
#include "Mod_Petri/zpetri-env.hxx"
#include "note.h"

using namespace std;

//����� ������� �������
QQueue<Note> queue;


class Error
{
public:
    QString descr;
    Error(QString str):descr(str){}
};

QStringList AvailFamil;
QString Goal;

int Note::count = 0;

QSemaphore semaphore_sync;
//��������� �� �������� ������� ��� �������
void (*algorithm)(int i,Note curNote);
//��������� ����� ��� ������ � ����
QTextStream* out;
QFile* outputFile;
//���������� ����������� ���������
int N(0),M(0),PT(0),PA(1);

//������� ���-�� ������� � ������
int curThreadCount = 0;
//��������� ��� �������� ������� �������
QVector<QThread*> vecThreads;


//������ �� �����
void rfile(const QString& name)
{
    QSettings settings(name, QSettings::IniFormat);
    //�����
    settings.beginGroup("common");
    const QStringList childKeys = settings.childKeys();
    if(childKeys.empty()) throw Error("Something wrong with input file");

    PA = settings.value("PA").toInt();
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

//������� �������� ������ ��� ������
void work(int thr_id,Note curNote)
{
    qDebug() << "Thread " << thr_id << " in work";
    if(curNote.family==Goal) {
        *out << "Family: "<< curNote.family <<" Number: " << curNote.ph_number <<" Address: " << curNote.address << "\n";
    }
    QThread::currentThread()->msleep(PT);
}

void ThreadPetriInit(){
    //������� ������
    for(int i=0;i<M;i++)
    {
      PetriThread *thr = new PetriThread(algorithm,i);
      thr->start();
      vecThreads.push_back(thr);
    }

    CreatePetri(PetriThreadArr,M, queue);

}
void PetriThreadPoolInit()
{
    CreatePetri(PetriThreadPool,M, queue);
    return;
}
void ThreadArrInit(){

    //������� ������
    for(int i=0;i<M;i++)
    {
      SimpleThread *thr = new SimpleThread(algorithm,i);
      thr->start();
      vecThreads.push_back(thr);
    }

    //������� ��� �������������
    semaphore_sync.release(vecThreads.size());

    //���� ����������� ���������� ������ �������
    while((curThreadCount==0) || semaphore_sync.available()!=vecThreads.size()){
        QThread::currentThread()->msleep(0);
    }
    //���� ��������, ����� ������ ���������
    return;
}
void SystemThreadPoolInit()
{
    QThreadPool::globalInstance()->setMaxThreadCount(M);
    Note curNote;
    int curIndex(0);

    while(!queue.isEmpty()){
        curNote = queue.dequeue();
        WorkTask* curtask = new WorkTask(algorithm,curNote,curIndex++);
        QThreadPool::globalInstance()->start(curtask);
    }
    QThreadPool::globalInstance()->waitForDone();
    return;
}

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
        //������� �������� ������ ��� ������
        algorithm = work;
        //������������� ���������� ����. �����
        QTime midnight(0,0,0);
        qsrand(midnight.secsTo(QTime::currentTime()));
        //��������� ������� ������
        for(int i=0;i<N;i++){
            Note newNote;
            newNote.filling();
//            queue.enqueue(newNote);
        }
        //����� ��������� ������� ������� �� ���������
        Goal = AvailFamil.at(qrand()%AvailFamil.size());
        qDebug() << "Gaol Family: " << Goal;

        //������ ��� ������ ��������������
        QTime runtime;
        runtime.start();

        switch(PA){
        case ThreadArr:
            *out << " Method: Simple Array of Threads - System Semaphore\n";
            ThreadArrInit();
            break;
        case PetriThreadArr:
             *out << " Method: Simple Array of Threads - Custom Petri Net Modeling Semaphore\n";
            ThreadPetriInit();
            break;
        case SystemThreadPool:
             *out << " Method: System ThreadPool \n";
            SystemThreadPoolInit();
            break;
        case PetriThreadPool:
             *out << " Method: Petri ThreadPool \n";
            PetriThreadPoolInit();
            break;
        }

        *out << "Time elapsed: " << runtime.elapsed();
        qDebug() << "Time elapsed: " << runtime.elapsed();

  }
  catch(Error err){
      qDebug() << "Error: "<< err.descr;
      outputFile->close();
  }

  outputFile->close();
  return app.exec();
}



