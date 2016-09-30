#ifndef MYPETRI_H
#define MYPETRI_H

#include "Mod_Petri/zpetri.hxx"
#include "Mod_Petri/zpetri-env.hxx"
#include "mythread.h"
#include <QDebug>

using namespace z;
using namespace z::petri;

enum CurMethod {ThreadArr = 1,PetriThreadArr,SystemThreadPool,PetriThreadPool};

extern int PT;
extern void work(int thr_id,Note curNote);
// заполнение содержимого сети Петри
petrinet_type :: content_type content;


//состояние сети Петри
class gstate_type: public place_type
{
    int type;
public:
    enum{
        S_FREE, //семафор свободен
        S_BUSY, //семафор занят
        NUMBER
    };
    explicit gstate_type(int t):type(t)
    {
        Q_ASSERT(type>=0 && type < NUMBER);
    }
};
//Переход Сети Петри
class gaction_type: public transition_simple_type
{
    int type;
public:
    enum{
        KEEP, //семафор захватить
        RELEASE, //семафор освободить
        EXIT,
        NUMBER
    };
    explicit gaction_type(int t):type(t)
    {
        Q_ASSERT(type>=0 && type < NUMBER);
    }
    int get_type(){return type;}
};
//окружение сети Петри
class genv_type: public petrinet_type::environment_abstract_type
{
    //int meth;
public:
    int wait(const petrinet_type::enabledlist_type &enabled, const petrinet_type::markedlist_type &marked)
    {
        int choice(0);
        petrinet_type::enabledlist_type::const_iterator it;

        for (it = enabled.begin(); it != enabled.end(); ++it)
        {
            gaction_type *curaction = dynamic_cast<gaction_type *>(*it);
            if(curaction->get_type()==gaction_type::KEEP){
                synchronize.wakeOne();
                choice = std::distance(enabled.begin(), it);
            }
            if(curaction->get_type()==gaction_type::RELEASE){
                while(curThreadCount>0) {
                    QThread::currentThread()->sleep(0);
                }
                choice = std::distance(enabled.begin(), it);
            }

            if(curaction->get_type()==gaction_type::EXIT){
                if(queue.isEmpty()) return std::distance(enabled.begin(), it);
            }
        }

        return choice;
    }
    //genv_type(int curMeth):meth(curMeth){}
};


// длительные операции
template<class T>
class job_type: public threadenv_type::longjob_abstract_type
{
private:
    int id;
    T &m_queues;
    void run(void)
    {
        //qDebug() << "job begin Thr id " << id;
        if(!m_queues.isEmpty()) {
            work(id,m_queues.dequeue());
            ::Sleep(PT);
        }
        //qDebug() << "prepare end " << m_queues.Test.top();
    }
public:
    job_type(T &queues,int curid): m_queues(queues), id(curid) {}
};

template<class T>
void CreatePetri(int curMeth,int MaxCountThreads, T& data_query)
{
    //petrinet_type::environment_abstract_type* env;
    genv_type envArr;
    threadenv_type envPool;
    //сеть, моделирующая семафор
    if(curMeth==PetriThreadArr){
        //семафор свободен/занят
        gstate_type p_free(gstate_type::S_FREE),p_busy(gstate_type::S_BUSY);
        content.add_place(p_free);
        content.add_place(p_busy);
        //переходы
        gaction_type t_keep(gaction_type::KEEP),t_release(gaction_type::RELEASE),t_exit(gaction_type::EXIT);
        content.add_transition(t_keep);
        content.add_transition(t_release);
        content.add_transition(t_exit);
        //дуги
        content.add_arc(p_free,t_keep);
        content.add_arc(t_keep,p_busy);
        content.add_arc(p_busy,t_release);
        content.add_arc(t_release,p_free);
        content.add_arc(p_free,t_exit);
        content.add_token(p_free);       
    }
    if(curMeth==PetriThreadPool){
        QVector<threadenv_type ::transition_long_type*> vecJobs;
        QVector<place_type> places(MaxCountThreads);
        place_type start;
        //gaction_type finish(gaction_type::EXIT);
        place_type exit;

        content.add_place(start);
        content.add_place(exit);

        for(int i=0;i<MaxCountThreads;i++){
            //вектор потоков
            job_type<T> job(data_query,i);
            threadenv_type::transition_long_type* jobs = new threadenv_type::transition_long_type(job,envPool);
            vecJobs.push_back(jobs);
            //добавляем поток и состояние между его работой
            //place_type place;
            //places.push_back(place);
            content.add_place(places[i]);
            content.add_transition(*vecJobs.back());
            content.add_arc(start,*vecJobs.back());
            //фишка в состояние обратной связи для каждого потока (для исключения ситуации повторного выполнения)
            content.add_arc(places[i],*vecJobs.back());
            content.add_arc(*vecJobs.back(),places[i]);
            //после завершения поток кладет фишку в выходное состояние
            content.add_arc(*vecJobs.back(),exit);
            //добавляем переход на выход
            //content.add_arc(places.last(),finish);
            //кладем фишку в состояние обратной связи
            content.add_token(places[i]);
        }
        content.add_token(start,queue.size());
        //content.add_arc(finish,exit);

    }

    petrinet_type petrinet(content);
    if(curMeth==PetriThreadPool) petrinet.live(envPool);
    if(curMeth==PetriThreadArr) petrinet.live(envArr);
}

#endif // MYPETRI_H

