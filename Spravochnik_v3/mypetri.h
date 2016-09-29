#ifndef MYPETRI_H
#define MYPETRI_H

#include "Mod_Petri/zpetri.hxx"
#include "Mod_Petri/zpetri-env.hxx"
#include "mythread.h"
#include <QDebug>

using namespace z;
using namespace z::petri;

enum CurMethod {ThreadArr = 1,PetriThreadArr,SystemThreadPool,PetriThreadPool};

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
public:
    int wait(const petrinet_type::enabledlist_type &enabled, const petrinet_type::markedlist_type &marked)
    {
        int choice(0);
        //
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
};


// длительные операции
template<class T>
class jobprepare_type: public threadenv_type::longjob_abstract_type
{
private:
    int id;
    T &m_queues;
    void run(void)
    {
        qDebug() << "prepare begin " << m_queues.Test.top();
        // ... длительные вычисления
        ::Sleep(200);
        qDebug() << "prepare end " << m_queues.Test.top();
    }
public:
    jobprepare_type(T &queues,int curid): m_queues(queues), id(curid) {}
};

template<class T>
void CreatePetri(int curMeth,int MaxCountThreads, T& data_query)
{
    //сеть, моделирующая семафор
    if(curMeth==PetriThreadArr){
        //
        gstate_type p_free(gstate_type::S_FREE),p_busy(gstate_type::S_BUSY);
        content.add_place(p_free);
        content.add_place(p_busy);
        //
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

    genv_type env;
    petrinet_type petrinet(content);
    petrinet.live(env);
    /*
    //~ srand(unsigned(time(NULL)));
    QVector threads;

    const int n = 3;

    // очереди данных
    queues_type queues;
    // ... наполнение входной очереди идентификаторов

    // выполняемые длительные работы
    for(){
        threads.jobprepare_type jprepare(queues,id);
    }

    threadenv_type env;
    // позиции
    place_type id, id1, id2, id3;
    place_type rules, state, control, result, channel;
    // переходы
    transition_simple_type split;
    threadenv_type::transition_long_type prepare(jprepare, env);

    // наполнение сети
    petrinet_type::content_type content;
    // позиции
    content.add_place(id);
    content.add_place(id1);
    content.add_place(id2);
    content.add_place(id3);
    content.add_place(rules);
    content.add_place(state);
    content.add_place(control);
    content.add_place(result);
    content.add_place(channel);
    // переходы
    content.add_transition(split);
    content.add_transition(prepare);
    content.add_transition(get);
    content.add_transition(process);
    content.add_transition(post);
    // дуги
    content.add_arc(id, split);
    content.add_arc(split, id1);
    content.add_arc(split, id2);
    content.add_arc(split, id3);
    content.add_arc(id1, get);
    content.add_arc(get, state);
    content.add_arc(state, process);
    content.add_arc(id2, prepare);
    content.add_arc(prepare, rules);
    content.add_arc(rules, process);
    content.add_arc(process, control);
    content.add_arc(id3, post);
    content.add_arc(control, post);
    content.add_arc(post, result);
    content.add_arc(channel, get);
    content.add_arc(get, channel);
    content.add_arc(channel, post);
    content.add_arc(post, channel);
    // разметка
    content.add_token(id, n);
    content.add_token(channel);

    // создание и запуск сети
    petrinet_type petrinet(content);
    petrinet.live(env);
    */

}

#endif // MYPETRI_H

