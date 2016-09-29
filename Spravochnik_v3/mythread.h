#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
#include <QWaitCondition>
#include "note.h"

QMutex syncmutex;
QWaitCondition synchronize;

extern QSemaphore semaphore_sync;
extern int curThreadCount;

class MyThread : public QThread
{
    void (*func)(int i,Note curNote);
public:
    int id;
    MyThread(void (*fptr)(int i,Note curNote),int detid):func(fptr),id(detid){}
    void run(){
        if(func){
            semaphore_sync.acquire();
            syncmutex.lock();
            //synchronize.wait(&syncmutex);
            curThreadCount++;
            syncmutex.unlock();

            Note curNote;

            while(!queue.isEmpty()){

                syncmutex.lock();
                if(!queue.isEmpty()){
                    curNote = queue.dequeue();
                }
                syncmutex.unlock();

                //запуск функции, привязанной к потоку
                func(id,curNote);
            }
            semaphore_sync.release();
        }
    }
};

#endif // MYTHREAD_H
