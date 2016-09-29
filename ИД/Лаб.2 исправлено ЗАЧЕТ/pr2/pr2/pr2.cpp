// pr2.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <Windows.h>
#include <math.h>
#include <ctime>
#include <conio.h>

typedef struct tagPoint
{
	float x, y;
	float r, a;
	int typ;

	int i;
} TPoint;

// Данные о точках
TPoint *points, *p_p;
int point_c,
	p_i = 10; // Какая точка центр для поиска максимума

int pa = 2, m=0, pt=10;
int n = 100;

bool terminate = false;
bool term_thr[10];

// Узнать количество ядер
int get_proc_num()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	m = sysinfo.dwNumberOfProcessors;
	printf( "Processor count = %d\n", m );

	return m;
}

float sqr( float v )
{
	return v*v;
}

// Читаем параметры из файла
void read_file ()
{
	FILE *in = fopen( "input.txt", "rt" );

	fscanf( in, "%d", &pa );
	fscanf( in, "%d%d%d", &n, &m, &pt );
	fscanf( in, "%d", &p_i );

	fclose(in);
}

// Создаем случайные точки
void generate_points()
{
	points = (TPoint*) malloc( n * sizeof(TPoint) );
	memset( points, 0, n * sizeof(TPoint) );

	p_p = &points[p_i];


	for( int i = 0; i < n; i++ )
	{
		points[i].typ = 1 << (rand() % 2);
		points[i].i = i;

		if (points[i].typ == 1)
		{
			points[i].x = (rand() % 1000 - 500) / 10.0f;
			points[i].y = (rand() % 1000 - 500) / 10.0f;
		}
		else
		{
			points[i].r = (rand() % 500) / 10.0f;
			points[i].a = (rand() % 3600) / 10.0f * M_PI / 180.0;
		}
	}
}

// Данные для каждой из цепочек
TPoint* max_p[10];
float max_v[10];

// Для APC цепочек
// bool terminate[10];

void reset_calc()
{
	memset( max_p, 0, sizeof(max_p) );
	memset( max_v, 0, sizeof(max_v) );

	terminate = false;
}

void calc_data( TPoint *p, int ind )
{
	if (p->typ == 1)
	{
		p->typ = 3;

		p->r = sqrt( p->x * p->x + p->y * p->y );
		p->a = atan2f( p->x, p->y );
	}
	else // if (p->typ == 2)
	{
		p->typ = 3;

		p->x = p->r * cos( p->a );
		p->y = p->r * sin( p->a );		
	}

	float l;

	l = sqrt( sqr(p->x - p_p->x) + sqr(p->y - p_p->y) );

	if (max_v[ ind ] < p->r)
	{
		max_v[ ind ] = p->r;
		max_p[ ind ] = p;
	}

	Sleep( pt );
}

// Вывод итогов
void display_data( int threads )
{
	// Собираем данные со всех цепочек и выводим общий итог в первой ячейке
	for( int i = 1; i < threads; i++ )
	{
		if (max_v[i] > max_v[0])
		{
			max_v[0] = max_v[i];
			max_p[0] = max_p[i];
		}
	}

	TPoint *p = max_p[0];

	printf( "  Max distance\n    from point %d (%f; %f)\n    is point %d (%f; %f)\n    Distance = %f\n", 
		p_p->i, p_p->x, p_p->y, p->i, p->x, p->y, max_v[0] );
}

// Время линейной обработки
int l_time;
clock_t start_t, end_t;

void start_timer()
{
  start_t = clock();
}

void end_timer( char *cap )
{
	end_t = clock ();

	clock_t delta = end_t - start_t;

	printf( "%s\n  Work time: %d msec\n", cap, delta );
}

// Массивы указывают каждой цепочке, какую часть обработать. 
// Последний элемент включается
int start_el[10], end_el[10];

DWORD __stdcall thr_simple( void *ptr )
{
	int thr_i = (int) ptr, i;

	for( i = start_el[thr_i]; i <= end_el[thr_i]; i++ )
		calc_data( &points[i], thr_i );
	
	// For APC
	term_thr[ thr_i ] = true;

	printf( "| Simple thread %d end\n", thr_i );

	return 0;
}

// Линейно обработать
void run_no_threads()
{
	reset_calc ();

	reset_calc();
	start_timer ();
	start_el[0] = 0;
	end_el[0] = n-1;
	
	thr_simple( (LPDWORD) 0 );

	end_timer ( "Simple line processing" );

	display_data( 1 );
}

// Запустить несколько простых цепочек
void run_simple()
{
	HANDLE threads[100];
  
	reset_calc();
	start_timer ();
	start_el[0] = 0;

	int di = n / m;
	for( int i = 0; i < m; i++ )
	{
		if (i == (m-1))
			end_el[i] = n-1;
		else
		{
			end_el[i] = start_el[i] + di;
			start_el[i+1] = end_el[i]+1;
		}

		threads[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) thr_simple,
			(LPDWORD) i, 0, NULL );
	}

	WaitForMultipleObjects( m, &threads[0], TRUE, INFINITE );
	end_timer( "Simple mulithreading" );
		
		
	display_data(1);
}

CRITICAL_SECTION crit_section_petry;

class TCritPetry
{
private:
	static int nodes[2]; 
	static int moves_out[2][2];
	static int moves_in[2][2];

	void move( int m );
public:
	void Enter ()
	{
		move(0);
	}

	void Leave()
	{
		move(1);
	}
};

int TCritPetry::nodes[2] = 
	{ 1, 0 };

int TCritPetry::moves_out[2][2] = 
{
	{ 1,0 },
	{ 0,1 }
};

int TCritPetry::moves_in[2][2] =
{
	{ 0,1 },
	{ 1,0 }
};

void TCritPetry::move( int m )
{
	bool fl = false;

	while (!fl)
	{
		EnterCriticalSection( &crit_section_petry );

		fl = true;
		for( int i = 0; i < 2; i++ )
			if (nodes[i] < moves_out[m][i])
				fl = false;

		// Перемещаемся
		if (fl)
		{
			for( int i = 0; i < 2; i++ )
				nodes[i] = nodes[i] - moves_out[m][i] + moves_in[m][i];
		}

		LeaveCriticalSection( &crit_section_petry );

		if (!fl) Sleep(1);
	}	
}

class TSemPetry
{
private:
	static int nodes[3]; 
	static int moves_out[2][3];
	static int moves_in[2][3];

	void move( int m );
public:
	void Finished ()
	{
		move(0);
	}
	void Finish ()
	{
		move(1);
	}

	static void init_sem_petry( int n );
};

int TSemPetry::nodes[3] = 
	{ 9,0,0 };

int TSemPetry::moves_out[2][3] =
{
	{ 1,0,0 },
	{ 0,9,0 },
};

int TSemPetry::moves_in[2][3] =
{
	{ 0,1,0 }, 
	{ 0,0,9 },
};

// Инициалиируем кратные вершины нужной кратностью
void TSemPetry::init_sem_petry( int n )
{
	int i,j;

	for( i = 0; i < 3; i++ )
	{
		if (nodes[i] == 9)
			nodes[i] = n;

		for( j = 0; j < 2; j++ )
		{
			if (moves_in[j][i] == 9)
				moves_in[j][i] = n;
			if (moves_out[j][i] == 9)
				moves_out[j][i] = n;
		}
	}
}

void TSemPetry::move( int m )
{
	bool fl = false;

	while (!fl)
	{
		EnterCriticalSection( &crit_section_petry );

		fl = true;
		for( int i = 0; i < 3; i++ )
			if (nodes[i] < moves_out[m][i])
				fl = false;

		// Перемещаемся
		if (fl)
		{
			for( int i = 0; i < 3; i++ )
				nodes[i] = nodes[i] - moves_out[m][i] + moves_in[m][i];
		}

		LeaveCriticalSection( &crit_section_petry );

		if (!fl) Sleep(1);
	}	
}

/* class TPetry
{
public:
	int petry_1;

	void move_12()
	{
		EnterCriticalSection( &crit_section_petry );

		petry_1 = 0;
		petry_2 = 1;

		LeaveCriticalSection( &crit_section_petry );
	}

	void move_21()
	{
		while (petry_1 == 0)
		{
			EnterCriticalSection( &crit_section_petry );

			if (petry_2 == 1)
			{
				petry_1 = 1;
				petry_2 = 0;
			}

			LeaveCriticalSection( &crit_section_petry );

			if (petry_1 == 0) Sleep(10);
		}
	}
}; // */

int last_el;

int next_element()
{
	if (last_el < n)
		return last_el++;
	return -1;
}

void __stdcall thr_petry( void *ptr )
{
	TCritPetry *crit = new TCritPetry();
	int thr_i = (int) ptr, i;

	while (true)
	{
		// printf( "  petry %d enter\n", thr_i );

		crit->Enter();

		i = next_element();

		// printf( "  petry %d leave\n", thr_i );
		crit->Leave ();

		if (i < 0)  break;

		calc_data( &points[i], thr_i );
	}

	// printf( "  petry %d finish\n", thr_i );

	TSemPetry *sem = new TSemPetry ();
	sem->Finished();
}

// Запустить несколько простых цепочек
void run_petry()
{
	HANDLE threads[100];
  
	reset_calc();

	start_timer ();
	for( int i = 0; i < m; i++ )
		threads[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) thr_simple,
			(LPDWORD) i, 0, NULL );

	/* TSemPetry *sem = new TSemPetry();
	sem->Finish (); // */

	WaitForMultipleObjects( m, &threads[0], TRUE, INFINITE );

	end_timer( "Petry" );

	display_data(1);
}

void __stdcall thr_idle( void *ptr )
{
	int thr_i = (int) ptr, i;

	while (!term_thr[ thr_i ])
		SleepEx( 100, TRUE );

	printf( "| Idle thread %d end\n", thr_i );
}


void run_apc()
{
	HANDLE threads[10];

	reset_calc();

	start_timer ();
	for( int i = 0; i < m; i++ )
		threads[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) thr_idle,
			(LPDWORD) i, 0, NULL );

	for( int i = 0; i < m; i++ )
		QueueUserAPC( (PAPCFUNC) thr_simple, threads[i], (ULONG_PTR) i );

	WaitForMultipleObjects( m, &threads[0], TRUE, INFINITE );

	end_timer( "APC" );
	
	display_data(1);
}

void run_apc_petry ()
{
	HANDLE threads[10];

	reset_calc();

	start_timer ();
	for( int i = 0; i < m; i++ )
		threads[i] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) thr_idle,
			(LPDWORD) i, 0, NULL );

	for( int i = 0; i < m; i++ )
		QueueUserAPC( (PAPCFUNC) thr_petry, threads[i], (ULONG_PTR) i );

	TSemPetry *sem = new TSemPetry();
	sem->Finish ();

	terminate = true;
	// WaitForMultipleObjects( m, &threads[0], TRUE, INFINITE );

	end_timer( "APC with petry" );

	display_data(1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	read_file();

	generate_points();
	InitializeCriticalSection( &crit_section_petry );

	if (m == 0)
		m = get_proc_num();

	// Поправить семафор для количества цепочек
	TSemPetry::init_sem_petry( m );

	run_no_threads();
	printf( "  Threads: %d\n", m );

	run_simple ();
	run_petry ();

	memset( term_thr, 0, sizeof(term_thr) );
	run_apc ();

	memset( term_thr, 0, sizeof(term_thr) );
	run_apc_petry ();

	printf( "End\n" );

	_getch ();

	return 0;
}

