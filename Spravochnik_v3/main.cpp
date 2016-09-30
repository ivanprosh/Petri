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

//общая очередь записей
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
//указатель на ключевую функцию для потоков
void (*algorithm)(int i,Note curNote);
//Строковый поток для записи в файл
QTextStream* out;
QFile* outputFile;
//Глобальные настроечные параметры
int N(0),M(0),PT(0),PA(1);

//текущее кол-во потоков в работе
int curThreadCount = 0;
//контейнер для хранения массива потоков
QVector<QThread*> vecThreads;
/*
//для блокировки одновременного доступа к крит. секции
QMutex PetriJump;
int countThreadOnCycle = 0;

*/

/*
//матрица входов/выходов сети Петри
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

//переход в сети Петри (аналог семафора в данной работе)
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
            // Движение можно сделать
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

    while(::count>0 && (vecDetails.at(i)->state != 2))//пока есть детали
    {
        //синхронизация в начале кванта
        semaphore_sync.acquire();
        syncmutex.lock();
        synchronize.wait(&syncmutex);
        curThreadCount++;
        countThreadOnCycle++;
        syncmutex.unlock();
        semaphore_sync.release();


        //пока не подойдет очередь согласно SJF для данного потока - ждем
        while(sortVecDetails.back()->id!=i);

        if(!sortVecDetails.empty()){
            // блокировка сетью Петри (взять фишку)
            move_petry(1);
            //извлекаем детали в порядке очереди
            sortVecDetails.pop_back();
            // Основной алгоритм обработки детали
            work(i,MaxT);
            //отдать фишку
            move_petry(0);
            //Имитируем длительность кванта
            if(vecDetails.at(i)->state != 2) QThread::currentThread()->msleep(MaxT);

            //считаем, что работа потока завершена на данном кванте, уменьшаем количество запущенных потоков
            syncmutex.lock();
            curThreadCount--;
            syncmutex.unlock();
        }
    }    
}
*/

//Чтение из файла
void rfile(const QString& name)
{
    QSettings settings(name, QSettings::IniFormat);
    //общие
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
    //отладочный вывод
    qDebug() << " PA="<< PA << " N="<< N << " M="<< M << " PT="<< PT;
    qDebug() << "*******************Finish read input file********************************";
#endif
}
/*
//Вывод результатов в файл
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
            // вывод станка и оставшегося времени
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
//функция основной работы для потока
void work(int thr_id,Note curNote)
{
    qDebug() << "Thread " << thr_id << " in work";
    if(curNote.family==Goal) {
        *out << "Family: "<< curNote.family <<" Number: " << curNote.ph_number <<" Address: " << curNote.address << "\n";
    }
    QThread::currentThread()->msleep(PT);
}

void ThreadPetriInit(){
    //Создаем потоки
    for(int i=0;i<M;i++)
    {
      PetriThread *thr = new PetriThread(algorithm,i);
      thr->start();
      vecThreads.push_back(thr);
    }

    CreatePetri(PetriThreadArr,M, queue);

}
void ThreadArrInit(){

    //Создаем потоки
    for(int i=0;i<M;i++)
    {
      SimpleThread *thr = new SimpleThread(algorithm,i);
      thr->start();
      vecThreads.push_back(thr);
    }

    //Семафор для синхронизации
    semaphore_sync.release(vecThreads.size());

    //даем возможность поработать другим потокам
    while((curThreadCount==0) || semaphore_sync.available()!=vecThreads.size()){
        QThread::currentThread()->msleep(0);
    }
    //сюда попадаем, когда работа закончена
    return;
}
void SystemThreadPoolInit()
{
    //WorkTask curtask(algorithm);
    //QThreadPool pool;

    QThreadPool::globalInstance()->setMaxThreadCount(M);

    Note curNote;

    int curIndex(0);
    while(!queue.isEmpty()){
        curNote = queue.dequeue();
        WorkTask* curtask = new WorkTask(algorithm,curNote,curIndex++);
        QThreadPool::globalInstance()->start(curtask);
    }

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
        //функция основной работы для потока
        algorithm = work;
        //инициализация генератора случ. чисел
        QTime midnight(0,0,0);
        qsrand(midnight.secsTo(QTime::currentTime()));
        //генерация очереди данных
        for(int i=0;i<N;i++){
            Note newNote;
            newNote.filling();
//            queue.enqueue(newNote);
        }
        //выбор случайной целевой фамилии из доступных
        Goal = AvailFamil.at(qrand()%AvailFamil.size());
        qDebug() << "Gaol Family: " << Goal;

        //таймер для оценки быстродействия
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
        }

        *out << "Time elapsed: " << runtime.elapsed();
        qDebug() << "Time elapsed: " << runtime.elapsed();
        /*


  while (::count>0)
  {
      int deadlock;
      for(int i=0;i<vecDetails.size();i++){
        if(vecDetails.at(i)->state!=2) sortVecDetails.push_back(vecDetails.at(i)); //вектор для очереди деталей
      }
      int sizeDet = sortVecDetails.size();

      if(algorithm==SJF_n){
        //сортируем по убыванию длительности CPU_burst
        qSort( sortVecDetails.begin(), sortVecDetails.end(),moreCPU_BurstThen);
      } else if (algorithm==SJF_p) {
        qSort( sortVecDetails.begin(), sortVecDetails.end(),morePriorThen);
      } else throw Error("Algorithm sorting is undefined for current method PA");

      //проверяем готовность станков
      for(int i=0;i<vecMachines.size();i++)
      {
          vecMachines.at(i)->ready |= vecMachines.at(i)->readyNextCycle;
          vecMachines.at(i)->readyNextCycle = 0;
      }

      //будим потоки

      while(semaphore_sync.available()>(vecDetails.size()-sizeDet));
      synchronize.wakeAll();

      //пока количество потоков не стало максимальным
      while(countThreadOnCycle!=sizeDet){
          QThread::currentThread()->msleep(0);
      }
      deadlock = 0;
      //пока потоки не завершились все
      while(curThreadCount>0) {
          QThread::currentThread()->msleep(0);
      }


      //вывод на экран
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



