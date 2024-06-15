/* colours.h */
#ifndef _COLOURS_H
#define _COLOURS_H

#include <stdio.h>

/* for use with colour setting function */

#define BLACK		0
#define BLUE		1
#define GREEN		2
#define CYAN		3
#define RED			4
#define MAGENTA		5
#define BROWN		6
#define WHITE		7
#define GREY		8
#define LT_BLUE		9
#define LT_GREEN	10
#define LT_CYAN		11
#define LT_RED		12
#define LT_MAGENTA	13
#define YELLOW		14
#define LT_WHITE	15

#define SET_INT_BGROUND		'D'
#define SET_BLNK_BGROUND	'E'
#define SET_FGROUND			'F'
#define SET_BGROUND			'G'
#define SET_REV_FGROUND		'H'
#define SET_REV_BGROUND		'I'
#define SET_GRA_FGROUND		'J'
#define SET_GRA_BGROUND		'K'
#define RESET_NORMAL		'L' /* not proper function letter */
#define RESET_REVERSE		'M' /* not proper function letter */
#define USE_NORMAL			'N' /* not proper function letter */
#define USE_REVERSE			'O' /* not proper function letter */

#define SET_SEQ				"\033[=%d%c\033[0m"
#define WRITE_SEQ			"\033[=%d%c\033[0m%s"
#define USE_NORMAL_SEQ		"\033[0m"
#define USE_REVERSE_SEQ		"\033[7m"
#define RESET_NORMAL_SEQ	"\033[=%dF\033[=%dG\033[0m"
#define RESET_REVERSE_SEQ	"\033[=%dH\033[=%dI\033[7m"

#ifndef _COLOUR_STATE_
#define _COLOUR_STATE_
struct colour_state {
	int fgrnd;
	int bgrnd;
	int rev_fgrnd;
	int rev_bgrnd;
	int gra_fgrnd;
	int gra_bgrnd;
};
#endif	/* _COLOUR_STATE */
void set_default_colours(void);
void init_colours(FILE *, int, int, int, int, int, int);
void restore_colours(FILE *);
void reset_colours(FILE *);
void write_colour_normal(FILE *, char *, int);
void set_colour(FILE *, char, int);
void push_colours(void);
void pop_colours(void);
void set_color(int, int);
void set_color_stream(FILE *, int, int);
void set_color_mode(int);
void _settextcolor(int);
void _setbkcolor(int);
void set_colour_state(int);
int get_colour_state(void);
int get_fground_colour(void);
int get_bground_colour(void);

#endif /* _COLOURS_H */
