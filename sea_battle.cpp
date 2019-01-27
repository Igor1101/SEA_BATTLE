/*********************************************
 *			SEA BATTLE GAME 0.0.0
 *
 * *******************************************/
/*
 * "New BSD License" or "Modified BSD License" 
 * Copyright 2019 (c) Igor Muravyov
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided that 
 * the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder 
 * nor the names of its contributors may be used to endorse or promote products derived from this software 
 * without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//#define DEBUG_BATTLE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define AR_SIZE 10
#define CLEAR(X) memset(X, 0, sizeof X)
#define COPYING "Igor Muravyov (c) 2019"
#define INPUT_SIZE 10

/***************Platform dependent macro ***********************/
/* IO */
#ifdef __unix__
/* main func linker name */
#define SEA_BATTLE_MAIN main
#include <ctime>
#include <cassert>

/* printf-like */
#define PRINT printf
/* put + go to newline */
#ifndef PUTSTR
#define PUTSTR puts
#endif

/* get one str from input */
#define GETS fgets(cli_line, sizeof cli_line, stdin)
#elif defined (__ba__)
/* main func linker name */
#define SEA_BATTLE_MAIN sea_battle_main
#include <jendefs.h>
#include <stdio_dbg.h>
#include <uart_stream.h>
#include <AppHardwareApi.h>

/* printf-like */
#define PRINT pr_cont
/* put + go to newline */
#ifndef PUTSTR
#define PUTSTR(X) PUTS(X); NEWLINE
#endif

/* get one str from input */
#define GETS  strncpy(cli_line, input_buffer, sizeof cli_line)
#endif				/* platform */

/**************************************************************/
/********************** MAY-BE undefined data types ***********/
#ifdef DEBUG_BATTLE
#define __DEBUG__ PRINT
#else
#define __DEBUG__(...)
#endif

#ifndef UNUSED
#define UNUSED
#endif

#ifndef NEWLINE
#define NEWLINE PUTSTR("")
#endif
typedef char* string;
typedef bool state_t;
#ifndef PRIVATE
#define PRIVATE static
#endif

#ifndef GETS
#define GETS
#endif

typedef enum {
	SEA = 0,
	SEA_VERIFIED,
	SHIP_PART,
	SHIP_PART_DESTROYED,
} sea_attr;


/**************************************************************/

PRIVATE void platform_init(void);

/******************** CLASSES **************************/
class Battle {
	private:
	   	sea_attr area[AR_SIZE][AR_SIZE];
		//float probability_area[AR_SIZE][AR_SIZE];
	   	unsigned user_times;
	public:
		bool is_all_killed(void);
		bool is_ship_killed(unsigned X, unsigned Y, unsigned X_busy, unsigned Y_busy);
		void print_area(void);
		void private_print_area(void);
		bool is_near(unsigned X, unsigned Y);
		bool is_same(unsigned X, unsigned Y);
		bool is_near_destr(unsigned X, unsigned Y);
		int set_ship(unsigned X, unsigned Y, bool is_vert, unsigned size);
		void set_ships(void);
		int explode(unsigned X, unsigned Y);
		void reset(void);
		unsigned get_tryes(void);
		Battle();
};

#define CLI_WORDS 4
#define SIZE_OF_CMD INPUT_SIZE
#define CLI_IS(X) !strcmp(cli_word[0], X)
#define X_CONVERTED(X) strtol(X, NULL, 10) - 1
#define Y_CONVERTED(Y) Y - 'a'
class Cli {
	private:
		char cli_word[CLI_WORDS][SIZE_OF_CMD];
		Battle* battle ;
		string find_notspace(string str);
		string find_space(string str);
		void process(void);
		void get_words(void);
	public:
		char cli_line[INPUT_SIZE];
		void init(void);
		void start(void);
		void run_cmd(void);
		Cli(Battle *b);
};

/********************************* RANDOM NUMBER GENERATOR (PLATFORM DEPENDENT) *************/

namespace RNG {
	void init(void)
	{
#ifdef __unix__
		time_t UNIX_time;
		srand((unsigned)time(&UNIX_time));
#elif defined (__ba__)
		vAHI_StartRandomNumberGenerator(E_AHI_RND_CONTINUOUS, E_AHI_INTS_DISABLED);
#endif
	}

	int get(int from, int to)
	{
#ifdef __unix__
		return (from + rand() % (to + 1));
#elif defined (__ba__)
		while(bAHI_RndNumPoll() == FALSE);
		return (from + u16AHI_ReadRandomNumber() % (to + 1));
#else
		return to - from;
#endif
	}

	float get( float from, float to )
	{
	    float scale;
#ifdef __unix__
		scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
#elif defined (__ba__)
		scale = u16AHI_ReadRandomNumber() / (float) (-1); /* [0, 1.0] */
#endif
		return from + scale * ( to - from );      /* [from, to] */
	}
};

/********************************************************************************************/

/******************** DATA **************************/


/* GLOBAL VARIABLES */
#ifdef __ba__
PRIVATE UNUSED char input_buffer[INPUT_SIZE];
#endif
Battle b1 = Battle();
Cli cli_main = Cli(&b1);
/********************/
/******************** *******************************/

/******************** EXPORTED FUNCTIONS ************/

/****************************************************/

/********************************* SEA Battle CLI *******************************************/


string Cli::find_notspace(string str)
{
	if (str == NULL)
		return NULL;
	while (*(str++) == ' ') ;
	return str - 1;
}

string Cli::find_space(string str)
{
	if (str == NULL)
		return NULL;
	while (*(str++) != ' ') {
		if (*str == '\0') {
			return str;
		}
	}
	return str - 1;
}

void Cli::get_words(void)
{
	memset(cli_word, 0, sizeof cli_word);
	int w_num = 0;
	string w_n_start = find_notspace(cli_line);
	string w_n_1_st;
	for (w_num = 0; w_num < CLI_WORDS; w_num++) {
		w_n_1_st = find_notspace(find_space(w_n_start));
		if (find_space(w_n_start) != NULL) {
			*find_space(w_n_start) = '\0';
			strcpy(cli_word[w_num], w_n_start);
		}
		w_n_start = w_n_1_st;
	}
}

void Cli::init(void)
{
	CLEAR(cli_line);
	CLEAR(cli_word);
	PRINT(">>> ");
}

void Cli::process(void)
{
	if (CLI_IS("kill")) {
		unsigned X = X_CONVERTED(cli_word[1]);		
	    unsigned Y = Y_CONVERTED(cli_word[2][0]);
		if(!(X<AR_SIZE) || !(Y<AR_SIZE)) {
			PUTSTR("Wrong direction");
			return;
		}
		int result = battle->explode(X, Y);
#ifdef DEBUG_BATTLE
		battle->print_area();
#else
		battle->private_print_area();
#endif
		PRINT("This is your %d try\n", battle->get_tryes());
		if( result!= EXIT_SUCCESS) {
			PUTSTR("Seems you missed");
			return;
		}
		if(battle->is_ship_killed(X, Y, 100, 100) == true) {
			PUTSTR("KILLED!");
			if(battle->is_all_killed()) {
			    PRINT("YOU WON WITH SCORE %d", battle->get_tryes());
			}
		}
		else {
			PUTSTR("WOUNDED!");
		}

	}
}

void Cli::start(void)
{
	while (1) {
		init();
		GETS;
		get_words();
		process();
	}
}

void Cli::run_cmd(void)
{
    GETS;
    get_words();
    process();
}

Cli::Cli(Battle *b)
{
	battle = b;
	init();
}

/********************************************************************************************/

/********************************* SEA Battle GAME LOGIC ************************************/

void Battle::reset(void)
{
	CLEAR(area);
	//CLEAR(probability_area);
	RNG::init();
}

void Battle::print_area(void)
{
	PUTSTR("\n\r  \t a  b  c  d  e  f  g  h  i  j");
	int i;
	int j;
	for (i = 0; i < AR_SIZE; i++) {
		PRINT("\n\r%d\t", i + 1);
		for (j = 0; j < AR_SIZE; j++) {
			switch (area[i][j]) {
			case SEA:
				PRINT(" ~ ");
				break;
			case SEA_VERIFIED:
				PRINT(" ~*");
				break;
			case SHIP_PART:
				PRINT("[ ]");
				break;
			case SHIP_PART_DESTROYED:
				PRINT("[X]");
				break;
			default:
				PRINT(" ? ");
			}
		}
	}
	NEWLINE;
}

void Battle::private_print_area(void)
{
	PUTSTR("\n\r  \t a  b  c  d  e  f  g  h  i  j");
	int i;
	int j;
	for (i = 0; i < AR_SIZE; i++) {
		PRINT("\n\r%d\t", i + 1);
		for (j = 0; j < AR_SIZE; j++) {
			switch (area[i][j]) {
			case SEA:
				PRINT(" ~ ");
				break;
			case SEA_VERIFIED:
				PRINT(" ~*");
				break;
			case SHIP_PART:
				PRINT(" ~ ");
				break;
			case SHIP_PART_DESTROYED:
				PRINT("[X]");
				break;
			default:
				PRINT(" ? ");
			}
		}
	}
	NEWLINE;
}


bool Battle::is_ship_killed(unsigned X, unsigned Y, unsigned X_busy, unsigned Y_busy)
{
	__DEBUG__("X %d Y %d Xb %d Yb %d", X, Y, X_busy, Y_busy);
	if(area[X][Y] != SHIP_PART_DESTROYED) 
		return false;
	if(is_near(X, Y)) {
		/* something near is alive */
		return false;
	}
	__DEBUG__("main part is not alive");
	if ((X+1) != X_busy && Y != Y_busy && (X < AR_SIZE) && area[X + 1][Y] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X+1, Y, X, Y)) {
			return false;
		}
	}
	if ((X-1) != X_busy && Y != Y_busy && (X > 0) && area[X - 1][Y] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X-1, Y, X, Y)) {
			return false;
		}
	}
	/*
	if (X != X_busy && Y != Y_busy && area[X][Y] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X, Y, X, Y)) {
			return false;
		}
	}*/
	if (X - 1 != X_busy && Y + 1 != Y_busy && (X > 0) && (Y < AR_SIZE) && area[X - 1][Y + 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X-1, Y+1, X, Y)) {
			return false;
		}
	}
	if (X + 1 != X_busy && Y + 1 != Y_busy && (X < AR_SIZE) && (Y < AR_SIZE) && area[X + 1][Y + 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X+1, Y+1, X, Y)) {
			return false;
		}
	}
	if (X != X_busy && Y + 1 != Y_busy && (Y < AR_SIZE) && area[X][Y + 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X, Y+1, X, Y)) {
			return false;
		}
	}
	if (X - 1 != X_busy && Y - 1 != Y_busy && (X > 0) && (Y > 0) && area[X - 1][Y - 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X-1, Y-1, X, Y)) {
			return false;
		}
	}
	if (X + 1 != X_busy && Y - 1 != Y_busy && (Y > 0) && area[X + 1][Y - 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X+1, Y-1, X, Y)) {
			return false;
		}
	}
	if (X != X_busy && Y - 1 != Y_busy && (Y > 0) && area[X][Y - 1] == SHIP_PART_DESTROYED) {
		if(!is_ship_killed(X, Y-1, X, Y)) {
			return false;
		}
	}
	return true;

}
bool Battle::is_same(unsigned X, unsigned Y)
{
	if(is_near_destr(X, Y) || is_near(X, Y))
		return true;
	return false;
}

bool Battle::is_near_destr(unsigned X, unsigned Y)
{
	if ((X < AR_SIZE) && area[X + 1][Y] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((X > 0) && area[X - 1][Y] == SHIP_PART_DESTROYED) {
		return true;
	}
	if (area[X][Y] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((X > 0) && (Y < AR_SIZE) && area[X - 1][Y + 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((X < AR_SIZE) && (Y < AR_SIZE) && area[X + 1][Y + 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((Y < AR_SIZE) && area[X][Y + 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((X > 0) && (Y > 0) && area[X - 1][Y - 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((Y > 0) && area[X + 1][Y - 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	if ((Y > 0) && area[X][Y - 1] == SHIP_PART_DESTROYED) {
		return true;
	}
	return false;
}

bool Battle::is_near(unsigned X, unsigned Y)
{
	if ((X < AR_SIZE) && area[X + 1][Y] == SHIP_PART) {
		return true;
	}
	if ((X > 0) && area[X - 1][Y] == SHIP_PART) {
		return true;
	}
	if (area[X][Y] == SHIP_PART) {
		return true;
	}
	if ((X > 0) && (Y < AR_SIZE) && area[X - 1][Y + 1] == SHIP_PART) {
		return true;
	}
	if ((X < AR_SIZE) && (Y < AR_SIZE) && area[X + 1][Y + 1] == SHIP_PART) {
		return true;
	}
	if ((Y < AR_SIZE) && area[X][Y + 1] == SHIP_PART) {
		return true;
	}
	if ((X > 0) && (Y > 0) && area[X - 1][Y - 1] == SHIP_PART) {
		return true;
	}
	if ((Y > 0) && area[X + 1][Y - 1] == SHIP_PART) {
		return true;
	}
	if ((Y > 0) && area[X][Y - 1] == SHIP_PART) {
		return true;
	}
	return false;
}

int Battle::set_ship(unsigned X, unsigned Y, bool is_vert, unsigned size)
{
	assert(size <= 4);
	assert(X < AR_SIZE);
	assert(Y < AR_SIZE);
	unsigned X_ver = X;
	unsigned Y_ver = Y;
	unsigned i;
	if (is_vert) {
		for (i = 0; i < size; i++) {
			if (is_near(X_ver, Y_ver)) {
				return EXIT_FAILURE;
			}
			Y_ver--;
		}
	} else {
		for (i = 0; i < size; i++) {
			if (is_near(X_ver, Y_ver)) {
				return EXIT_FAILURE;
			}
			X_ver--;
		}
	}
    __DEBUG__("X=%d, Y=%d, vert=%d, size=%d\n", X, Y, is_vert, size);
	if (is_vert) {
		for (i = 0; i < size; i++) {
			area[X][Y--] = SHIP_PART;
		}
	} else {
		for (i = 0; i < size; i++) {
			area[X--][Y] = SHIP_PART;
		}
	}
	return EXIT_SUCCESS;

}

void Battle::set_ships(void)
{
    __DEBUG__("setting ships");
	/* one 4 */
	while (set_ship(RNG::get(3, AR_SIZE - 4),
			RNG::get(3, AR_SIZE - 4),
			RNG::get(0, 1), 4) == EXIT_FAILURE) ;
    __DEBUG__("set 4 - layer");
	/* two 3 */
	while (set_ship(RNG::get(2, AR_SIZE - 3),
			RNG::get(2, AR_SIZE - 3),
			RNG::get(0, 1), 3) == EXIT_FAILURE) ;
	while (set_ship(RNG::get(2, AR_SIZE - 3),
			RNG::get(2, AR_SIZE - 3),
			RNG::get(0, 1), 3) == EXIT_FAILURE) ;
    __DEBUG__("set 3 - layer");
	/* three 2 */
	while (set_ship(RNG::get(1, AR_SIZE - 2),
			RNG::get(1, AR_SIZE - 2),
			RNG::get(0, 1), 2) == EXIT_FAILURE) ;
	while (set_ship(RNG::get(1, AR_SIZE - 2),
			RNG::get(1, AR_SIZE - 2),
			RNG::get(0, 1), 2) == EXIT_FAILURE) ;
	while (set_ship(RNG::get(1, AR_SIZE - 2),
			RNG::get(1, AR_SIZE - 2),
			RNG::get(0, 1), 2) == EXIT_FAILURE) ;
    __DEBUG__("set 2 - layer");
	/* four 1 */
	while (set_ship(RNG::get(0, AR_SIZE - 1),
			RNG::get(0, AR_SIZE - 1),
			RNG::get(0, 1), 1) == EXIT_FAILURE) ;

	while (set_ship(RNG::get(0, AR_SIZE - 1),
			RNG::get(0, AR_SIZE - 1),
			RNG::get(0, 1), 1) == EXIT_FAILURE) ;

	while (set_ship(RNG::get(0, AR_SIZE - 1),
			RNG::get(0, AR_SIZE - 1),
			RNG::get(0, 1), 1) == EXIT_FAILURE) ;
	while (set_ship(RNG::get(0, AR_SIZE - 1),
			RNG::get(0, AR_SIZE - 1),
			RNG::get(0, 1), 1) == EXIT_FAILURE) ;
    __DEBUG__("set 1 - layer");
}

int Battle::explode(unsigned X, unsigned Y)
{
	assert(X < AR_SIZE);
	assert(Y < AR_SIZE);
	user_times++;
	if (area[X][Y] == SEA) {
		area[X][Y] = SEA_VERIFIED;
		return EXIT_FAILURE;
	} else if (area[X][Y] == SHIP_PART) {
		area[X][Y] = SHIP_PART_DESTROYED;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

unsigned Battle::get_tryes(void)
{
    return user_times;
}

Battle::Battle(void)
{
    user_times = 0;
}

bool Battle::is_all_killed(void)
{
    int i;
    int j;
    for(i=0; i<AR_SIZE; i++) {
        for(j=0; j<AR_SIZE; j++) {
            if(area[i][j] == SHIP_PART) {
                return false;
            }
        }
    }
    return true;
}

/********************************************************************************************/



extern "C" {
    int SEA_BATTLE_MAIN(void);
}
int SEA_BATTLE_MAIN(void)
{
    /* platform dependent initialization */
    platform_init();
    /*************************************/
	PUTSTR("generating ships via TRUE RNG... ");
	b1.reset();
	PUTSTR(COPYING);
	PUTSTR("you are playing SEA Battle game... ");
	b1.set_ships();
#ifdef __unix__
	cli_main.start();
#endif
	return EXIT_SUCCESS;
}

#ifdef __ba__
void getc_callback(char ch)
{
    static unsigned i_pointer = 0;
    if(i_pointer >= sizeof input_buffer) {
        PUTSTR("WARN: out of bounds");
        CLEAR(input_buffer);
        i_pointer = 0;
        return;
    }
    if(ch == '\r') {
        Cli cli = Cli(&b1);
        NEWLINE;
        /* set flag */
        input_buffer[i_pointer] = '\0';
        i_pointer = 0;
        cli.run_cmd();
        return;
    }
    PRINT("%c", ch);
    input_buffer[i_pointer++] = ch;
}
#endif /* BEYOND ARCHITECTURE */

PRIVATE void platform_init(void)
{
    /* stop console input*/
#ifdef __ba__
    uart1_callback = getc_callback;
#endif /* BA */
}
/********************************************************************************************/
