#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
#include <QWaitCondition>
#include <QRunnable>
#include "note.h"

QMutex syncmutex;
QWaitCondition synchronize;

extern QSemaphore semaphore_sync;
extern int curThreadCount;

class SimpleThread : public QThread
{
    void (*func)(int i,Note curNote);
public:
    int id;
    SimpleThread(void (*fptr)(int i,Note curNote),int detid):func(fptr),id(detid){}
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

                //������ �������, ����������� � ������
                func(id,curNote);
            }
            semaphore_sync.release();
        }
    }
};
//����� ��� ������ ����� ������
class PetriThread : public QThread
{
    void (*func)(int i,Note curNote);
public:
    int id;
    PetriThread(void (*fptr)(int i,Note curNote),int detid):func(fptr),id(detid){}
    void run(){
        if(func){
            Note curNote;

            while(!queue.isEmpty()){
                syncmutex.lock();
                synchronize.wait(&syncmutex);
                curThreadCount++;
                syncmutex.unlock();

                if(!queue.isEmpty()){
                    curNote = queue.dequeue();
                }

                //������ �������, ����������� � ������
                func(id,curNote);

                syncmutex.lock();
                curThreadCount--;
                syncmutex.unlock();
            }
        }
    }
};
//����� ������ ������ �� ���� ��� ������� ����� ������
class WorkTask : public QRunnable
{
    Note RunNote;
    int id;
    void (*func)(int i,Note curNote);
public:
    WorkTask(void (*fptr)(int i,Note _Note),Note cNote, int curIndex ):func(fptr),RunNote(cNote),id(curIndex){}
    void run()
    {
        if(func){
            //������ �������, ����������� � ������
            func(id,RunNote);
        }
    }
};

#endif // MYTHREAD_H
