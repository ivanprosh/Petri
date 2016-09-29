#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <ctime>
#include <vector>
#include <iostream>

// ��� �������� � ��������� �����
const char* input_name = "../input.txt";
const char* output_name = "../output.txt";

int pa, m, pt, n;

HANDLE semaphore = 0;

// ������� ����
char *name_for;

// ��, ��� ����
int weight;

// ������
typedef struct tagBagaj
{
    char *fio, *name;
    int weight, cnt;
} TBagaj;

// ��������� ��������� �� �����
void read_params()
{
    FILE *f = fopen( input_name, "rt" );

    fscanf( f, "%d", &pa );
    fscanf( f, "%d%d%d", &n, &m, &pt );

    fclose(f);
}

// ������� ���������. ���� 0 - �� �� ���������� �����������
void check_proc_count()
{
    if (m == 0)
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo( &sysinfo );

        m = sysinfo.dwNumberOfProcessors;
        printf( "Pnumber (m)==0. Getting hardware process number: %d\n", m );
    }
}

// ������
TBagaj *data;

char* fio_list[] = { "ivanov", "petrov", "sidorov" };
char* name_list[] = { "chemodan", "sumka", "jivotnoe" };

// ������� ������ ��������� �������
void create_random_bagaj()
{
    data = (TBagaj*) malloc( n * sizeof(TBagaj) );

    memset( data, 0, n * sizeof(TBagaj) );

    name_for = name_list[0];
    for( int i = 0; i < n; i++ )
    {
        data[i].cnt = rand() % 10 + 1;
        data[i].weight = rand() % 10 + 5;
        data[i].fio = fio_list[ rand() % 3 ];
        data[i].name = name_list[ rand() % 3 ];
    }
}

// ��� ������ � ���������
void Lock()
{
    WaitForSingleObject( semaphore, INFINITE );
}

void Unlock ()
{
    ReleaseSemaphore( semaphore, 1, NULL );
}

// ������������ ������
void Obrabotat( TBagaj *d )
{
    if (strcmp( name_for, d->name ) == 0)
    {
        weight += d->cnt * d->weight;
    }

    Sleep( pt );
}

// ���������� �������� ����������
void ShowRes()
{
    printf( "Total weight for %s is %d\n", name_for, weight );
}

// ����� �������� ���������
int l_time, t_time;

// ������� ����������
void linear_process()
{
    clock_t st, en;

    st = clock();

    weight = 0;
    for( int i = 0; i < n; i++ )
        Obrabotat( &data[i] );

    en = clock();

    l_time = en-st;

    printf( "Linear time: %d\n", l_time );
    printf( "  Weight: %d\n", weight );

    ShowRes();
}

// ����� ��������������
int last_free = 0;

// �������� ��������� (� ������������ �� ���������)
// �������� �������������
int get_last ( )
{
    if (last_free >= n)
        return -1;
    else
        return last_free++;
}

// ������ ����������, ���� ���� ���������
DWORD __stdcall proc_thr( void *ptr )
{
    int pdata = (int) ptr;
    int i;
    bool fl;

    for( fl = true; fl; )
    {
        Lock();

        i = get_last ();
        if (i < 0)
            fl = false;

        Unlock ();

        if (fl)
            Obrabotat( &data[i] );
    }

    return 0;
}

// ��������� ��������� ������� �������
void thr_process()
{
    clock_t st;
    HANDLE thr[100];
    DWORD i;

    st = clock();
    weight = 0;

    for(  i = 0; i < m; i++ )
    {
        thr[i] = CreateThread( NULL, 0,
            (LPTHREAD_START_ROUTINE) proc_thr,
            (LPDWORD) i, 0, NULL );
    }

    WaitForMultipleObjects( m, &thr[0], TRUE, INFINITE );

    t_time = clock() - st;
    printf( "Multithreaded time: %d\n", t_time );

    ShowRes();
}

// �����. �� ������ ������� ���������� 3 ����� - 2 �� ������ ���� � ������� � 1 �� �����
// TPLink links[30];


int pnodes[20] =
{
  1,0, 0,0,0,0,0,0
};

int petry_out[40][40] =
{
  { 1,0, 0,0,0,0,0,0 }, // ���� ������
  { 0,1, 0,0,0,0,0,0 },
  { 1,0, 1,0,0,0,0,0 }, // ����������� ����� ����������� ������
  { 0,0, 0,1,0,0,0,0 }, // ���������

};

int petry_in[40][40] =
{
  { 0,1, 0,0,0,0,0,0 }, // ���� ������
  { 1,0, 0,0,0,0,0,0 },
  { 1,0, 0,0,0,0,0,0 }, // (�������� � ���-�� ������ �������)
  { 0,0, 1,0,0,0,0,0 }, // ���������

};

const int finish_move = 2;

int p_lock_node, p_priv_nodes, p_finish_nodes, p_end_node;
CRITICAL_SECTION crit_sect;


void move_petry( int t )
{
    bool fl = false;
    int i;

    while (!fl)
    {
        EnterCriticalSection( &crit_sect );

        fl = true;
        for( i = 0; (i < 20) & fl; i++ )
        {
            if (pnodes[i] < petry_in[t][i])
                fl = false;
        }

        if (fl)
        { // �������� ����� �������
            for( i = 0; (i < 20) & fl; i++ )
            {
                pnodes[i] -= petry_in[t][i];
                pnodes[i] += petry_out[t][i];
            }
//            std::cout << "Thread is " << GetCurrentThreadId() << " Current move " << t << std::endl;
//            for( i = 0; (i < 15) & fl; i++ )
//            {
//                std::cout << pnodes[i] << ' ';
//            }
//            std::cout << std::endl;
        }

        LeaveCriticalSection( &crit_sect );

        if (!fl) Sleep(10);
    }
}


void init_petry( int end_size )
{
    petry_in[2][finish_move] = end_size;
    pnodes[0] = m;
    petry_out[2][0] = 0;
    petry_out[2][2] = 0;
    petry_in[3][2] = 0;

    for( int i = 0; i < n; i++ )
    {
        int t = i*2 + 3;

        petry_in [ t   ][0] = 1;
        petry_out[ t   ][3+i] = 1;
        petry_in [ t+1 ][3+i] = 1;
        petry_out [ t+1 ][0] = 1;
        petry_out [ t+1 ][2] = 1;

    }

//    std::cout << "petry in: " << std::endl;
//    for (int i = 0; i < 20; i++)
//    {
//        for(int j = 0; j < 20; j++)
//        {
//           std::cout << petry_in[i][j] << ' ';
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "petry out: " << std::endl;
//    for (int i = 0; i < 20; i++)
//    {
//        for(int j = 0; j < 20; j++)
//        {
//           std::cout << petry_out[i][j] << ' ';
//        }
//        std::cout << std::endl;
//    }

}


DWORD __stdcall proc_petry( void *ptr )
{
    int pdata = (int) ptr;
    int i = 1;
     //

    while( i > 0)
    {
        if(!i) return 0;
        // ������� �����
        move_petry( 1 );

        i = get_last();
        //std::cout << " finished count is:" << i;

        // ������ �����
        move_petry( 0 );

        if (i >= 0) {
            Obrabotat( &data[i]  );
            // ������ ����� ������
            move_petry( 2 );
        }
    }

    return 0;
}
void close_thr(HANDLE* thr,DWORD* id){

    for( int i = 0; i < m; i++ )
    {
        // �������� ��������� ��� ���������� ������
        PostThreadMessage(id[i], WM_QUIT, 0, 0);

        // ���� ��������� ����� ��� �����������, �� ����� ����� �������� ��������� ��� ������
        WaitForSingleObject(thr[i], INFINITE);
        // ������ ����� ����� ����� ������� ���������-������ �� ����
        CloseHandle(thr[i]);
    }

}
void petry_process ()
{
    clock_t st;
    HANDLE thr[100];
    DWORD i;
    DWORD id[100];

    st = clock();
    weight = 0;
    petry_in[3][2] = n;

    for(  i = 0; i < m; i++ )
    {
        thr[i] = CreateThread( NULL, 0,
            (LPTHREAD_START_ROUTINE) proc_petry,
            (LPDWORD) i, 0, &id[i] );
    }

    move_petry( finish_move + 1 );

    t_time = clock() - st;

    printf( "Petry time: %d\n", t_time );

    ShowRes();

    close_thr(thr,id);
}

bool done = false;

void proc_apc( void *ptr )
{
    while (!done)
        SleepEx( 500, TRUE );
}

void thr_apc()
{
    clock_t st;
    HANDLE h;
    std::vector<HANDLE> *thr = new std::vector<HANDLE>();
    DWORD i;

    st = clock();
    weight = 0;

    // ������� ������ �������
    for(  i = 0; i < m; i++ )
    {
        h = CreateThread( NULL, 0,
            (LPTHREAD_START_ROUTINE) proc_apc,
            (LPDWORD) i, 0, NULL );

        thr->push_back( h );
    }

    // ��������� � ������ �� ��� APC
    for( i = 0; i < m; i++ )
    {
        h = thr->at( i  );

        QueueUserAPC( (PAPCFUNC) proc_thr, h, (ULONG_PTR) i );
    }

    int t;

    // ����, ���� ��� ����������
    while (last_free < n)
        Sleep(20);

    // ��������� �������� ������� � ����������
    done = true;

    t_time = clock () - st;

    printf( "APC time %d\n", t_time );

    ShowRes();
}

void pool_petry_thr( void *ptr )
{
    int i = (int) ptr;

    // ������� �����
    move_petry( i*2 + 3 );

    Obrabotat( &data[i]  );

    // ������ �����
    move_petry( i*2 + 3+1 );
}

void thr_pool_petry()
{
    clock_t st;
    HANDLE thr[100];
    //std::vector<HANDLE> *thr = new std::vector<HANDLE>();
    DWORD i;
    DWORD id[100];

    init_petry ( n );

    st = clock();

    for( i = 0; i < n; i++ )
    {
       thr[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) pool_petry_thr, (void*) i, NULL, &id[i] );
    }

    // ����, ���� ��� ���������� (���� time ����� ����������� � 0 � �����)
    move_petry( finish_move );

    t_time = clock() - st;

    printf( "APC Petry time: %d\n", t_time );

    // ������� ���������� ���������
    ShowRes();

    close_thr(thr,id);
} // */

int _tmain(int argc, _TCHAR* argv[])
{
    // read_params(); // ��������������� ��� ������������� ���������� �� ���������

    InitializeCriticalSection( &crit_sect );

    read_params ();

    // ��������� �������� ���������� ���������
    check_proc_count ();

    create_random_bagaj ();

    linear_process ();

    weight = 0;

    semaphore = CreateSemaphore(NULL, 1,1, NULL);

    if (pa == 1)
        thr_process ();
    else if (pa == 2)
        petry_process ();
    else if (pa == 3)
        thr_apc ();
    else if (pa == 4)
        thr_pool_petry (); // */

    FILE *f_out = fopen( output_name, "wt" );

    fprintf( f_out, "Linear time: %d\n", l_time );
    fprintf( f_out, "Multithreaded time: %d", t_time );

    fprintf( f_out, "Total weight of %s is %d\n", name_for, weight );

    fclose(f_out);

    printf( "Press any key to quit\n" );
    getch ();

    return 0;
}

