/* Updated version: 2001/10/14 for multiple OS new-line code systems */
/*
/  source file header;
/ 	name		= psywchar.c : psy wide character I/O source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/06/23;
/ 	contents	= getwchar	: (int) get wide character;
/ 			= putwchar	: (int)	put wide character;
/ 			= getwc		: (int) get wide character from file;
/ 			= ungetwc	: (int) unget wide character to file;
/ 			= putwc		: (int) put wide character to file;
*/
 
/* source file id */
 
#define	PSYWCHAR_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
#ifndef	UNIX_STDIO
int		getwchar();	/* get wide character */
int		putwchar();	/* put wide character */
#endif
int		getwc();	/* get wide character from file */
int		ungetwc();	/* unget wide character to file */
int		putwc();	/* put wide character to file */
 
#ifndef	UNIX_STDIO
/*
/  specification;
/ 	name		= getwchar;
/ 	creation	= 1990/06/23;
/ 	function	= gets wide character;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= EOF		: (int) end of stdin;
/ 			= otherwise	: (wchar_t) wide character;
*/
 
int		getwchar()
{
	register	int	c;
 
	c = getchar();
#ifdef	JIS
	while (c == 0x1b)
	{
		if ((c = getchar()) == 0x24)
		{
			if ((c = getchar()) == 0x42 || c == 0x40)
				jis_kanji = TRUE;
		}
		else if (c == 0x28)
		{
			if ((c = getchar()) == 0x4a || c == 0x42)
				jis_kanji = FALSE;
		}
		c = getchar();
	}
 
	return (jis_kanji ? (c << CHAR_BIT) | getchar() : c);
#else
	return (iskanji(c) ? (c << CHAR_BIT) | getchar() : c);
#endif
}
 
/*
/  specification;
/ 	name		= putwchar;
/ 	creation	= 1990/06/23;
/ 	function	= puts wide character;
/ 	parameter (in)	= c	: (int) wide character;
/ 	parameter (out)	= none;
/ 	return		= wide character (int);
*/
 
int		putwchar(c)
register	int	c;
{
#ifdef	JIS
	if (c & ~CHAR_MASK)
	{
		putchar(0x1b);
		putchar(0x24);
		putchar(0x42);
		putchar(c >> CHAR_BIT);
		putchar(c & CHAR_MASK);
		putchar(0x1b);
		putchar(0x28);
		putchar(0x4a);
	}
	else
		putchar(c);
#else
	if (c & ~CHAR_MASK)
		putchar(c >> CHAR_BIT);
	putchar(c & CHAR_MASK);
#endif
 
	return (c);
}
#endif
 
/*
/  specification;
/ 	name		= getwc;
/ 	creation	= 1990/06/23;
/ 	function	= gets wide character from file;
/ 	parameter (in)	= fp	: (FILE *) file pointer;
/ 	parameter (out)	= none;
/ 	return		= EOF		: (int) end of stdin;
/ 			= otherwise	: (wchar_t) wide character;
*/
 
int global_pushed_back = 0;

int		getwc(fp)
register	FILE	*fp;
{
	register	int	c;
 
	if (global_pushed_back) {
		c = global_pushed_back;
		global_pushed_back = 0;
		return (c);
	}

	c = getc(fp);
#ifdef	JIS
	while (c == 0x1b)
	{
		if ((c = getc(fp)) == 0x24)
		{
			if ((c = getc(fp)) == 0x42 || c == 0x40)
				jis_kanji = TRUE;
		}
		else if (c == 0x28)
		{
			if ((c = getc(fp)) == 0x4a || c == 0x42)
				jis_kanji = FALSE;
		}
		c = getc(fp);
	}
 
	return (jis_kanji ? (c << CHAR_BIT) | getc(fp) : c);
#else
	return (iskanji(c) ? (c << CHAR_BIT) | getc(fp) : c);
#endif
}
 
int
ungetwc(int x, FILE *fp)
{
	return (global_pushed_back = x);
}

/*
/  specification;
/ 	name		= putwc;
/ 	creation	= 1990/06/23;
/ 	function	= puts wide character to file;
/ 	parameter (in)	= c	: (int) wide character;
/ 			= fp	: (FILE *) file pointer;
/ 	parameter (out)	= none;
/ 	return		= wide character (int);
*/
 
int		putwc(c,fp)
register	int	c;
register	FILE	*fp;
{
#ifdef	JIS
	if (jis_cont)
	{
		if (c & ~CHAR_MASK)
		{
			if (!jis_kanji)
			{
				jis_kanji = TRUE;
				if (putc(0x1b,fp) == EOF)
					return (EOF);
				if (putc(0x24,fp) == EOF)
					return (EOF);
				if (putc(0x42,fp) == EOF)
					return (EOF);
			}
			if (putc(c >> CHAR_BIT,fp) == EOF)
				return (EOF);
			if (putc(c & CHAR_MASK,fp) == EOF)
				return (EOF);
		}
		else
		{
			if (jis_kanji)
			{
				jis_kanji = FALSE;
				if (putc(0x1b,fp) == EOF)
					return (EOF);
				if (putc(0x28,fp) == EOF)
					return (EOF);
				if (putc(0x4a,fp) == EOF)
					return (EOF);
			}
			if (putc(c,fp) == EOF)
				return (EOF);
		}
	}
	else
	{
		if (c & ~CHAR_MASK)
		{
			if (putc(0x1b,fp) == EOF)
				return (EOF);
			if (putc(0x24,fp) == EOF)
				return (EOF);
			if (putc(0x42,fp) == EOF)
				return (EOF);
			if (putc(c >> CHAR_BIT,fp) == EOF)
				return (EOF);
			if (putc(c & CHAR_MASK,fp) == EOF)
				return (EOF);
			if (putc(0x1b,fp) == EOF)
				return (EOF);
			if (putc(0x28,fp) == EOF)
				return (EOF);
			if (putc(0x4a,fp) == EOF)
				return (EOF);
		}
		else
			if (putc(c,fp) == EOF)
				return (EOF);
	}
#else
	if (c & ~CHAR_MASK)
		if (putc(c >> CHAR_BIT,fp) == EOF)
			return (EOF);
	if (putc(c & CHAR_MASK,fp) == EOF)
		return (EOF);
#endif
 
	return (c);
}
 
/* end of psywchar.c */
