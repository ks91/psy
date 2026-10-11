/*
/  source file header;
/ 	name		= psyview.c : psy screen control source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/02;
/ 	contents	= show		: (void) show window;
/ 			= showrest	: (void) show rest of the window;
/ 			= showln	: (void) show rest of the line;
/ 			= showwln	: (void) show whole line;
/ 			= view		: (void) show certain part of the buffer;
/ 			= show2		: (void) show after block operation;
/ 			= show3		: (void) show after scroll occured;
/ 			= stripe	: (void) show line from cartain column;
/ 			= screen	: (void) show window from certain line;
/ 			= within	: (flag_t) check if within block;
/			= sp		: (void) print space;
*/
 
/* source file id */
 
#define	PSYVIEW_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
void		show();		/* show window */
void		showrest();	/* show rest of the window */
void		showln();	/* show rest of the line */
void		view();		/* show certain part of the buffer */
void		show2();	/* show after block operation */
void		show3();	/* show after scroll occured */
 
/* private declaration */
 
/* private functions */
 
static	void	stripe();	/* show line from certain column */
static	void	screen();	/* show window from certain line */
static	flag_t	within();	/* check if within block */
static	void	sp();		/* print space */
 
/*
/  specification;
/ 	name		= show;
/ 	creation	= 1990/05/03;
/ 	function	= displays current window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		show()
{
	locate(0,ceiling);
 
	screen(0,point.line == 1 ? cur_ln : *(spln_base + y_bas));
}
 
/*
/  specification;
/ 	name		= showrest;
/ 	creation	= 1990/05/03;
/ 	function	= displays rest of the window;
/ 	parameter (in)	= none;
/ 	paramter (out)	= none;
/ 	return		= none (void);
*/
 
void		showrest()
{
	showln();
	screen(point.line - y_bas + 1,cur_ln -> link);
}
 
/*
/  specification;
/ 	name		= showln;
/ 	creation	= 1990/05/03;
/ 	function	= displays the rest of the line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none;
*/
 
void		showln()
{
	locate(loc_x(),loc_y());
	stripe(point.column - x_bas,getnext(cur_cm));
}
 
/*
/  specification;
/ 	name		= showwln;
/ 	creation	= 1990/06/17;
/ 	function	= displays the whole line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none;
*/
 
void		showwln()
{
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	register	int		off_x;		/* offset */
	wchar_t		c;				/* wide character */
 
	reg1_cm = cur_ln -> addr;
 
	/* get to the left margin */
 
	for (off_x = 0; off_x < x_bas;)
	{
		if (!(reg1_cm = getnext(reg1_cm)) || (c = getletr(reg1_cm)) == '\n')
			break;		/* end of text or end of line */
 
		/* count column */
 
		if (c == '\t')
			off_x = off_x + tab_len - off_x % tab_len;
		else
		{
			if (c & ~CHAR_MASK)
				++off_x;
			++off_x;
		}
	}
 
	locate(0,loc_y());
 
	if (off_x == x_bas && c & ~CHAR_MASK)
	{				/* line starts from the second byte of a kanji */
		putchar('%');
		stripe(1,getnext(reg1_cm));	/* display line */
	}
	else
		stripe(0,reg1_cm);		/* display line */
}
 
/*
/  specification;
/ 	name		= view;
/ 	creation	= 1990/05/03;
/ 	function	= displays certain part of the window;
/ 	parameter (in)	= pt	: (point_t *) point to show;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		view(pt)
register	point_t		*pt;
{
	if (pt -> line < 1)
		pt -> line  = 1;	/* line cannot be minus */
 
	if (point.line != pt -> line)
	{				/* then must move current position */
		if (point.line > pt -> line)
		{			/* the line is in stack */
			cur_ln = *(sp_ln = spln_base + pt -> line);
			if ((y_bas -= point.line - pt -> line) < 1)
				y_bas = 1;	/* line cannot be minus */
			point.line = pt -> line;
		}
		else
		{			/* must make stack */
			while (point.line <= pt -> line && cur_ln)
			{
				++point.line;
				++y_bas;
				cur_ln = push_ln(cur_ln) -> link;
			}
			cur_ln = pop_ln();
			--y_bas;
			--point.line;
		}
	}
 
	/* set column */
 
	get_cm(pt -> column);
	adj_x();	/* adjust x_bas */
 
	show();
}
 
/*
/  specification;
/ 	name		= show2;
/ 	creation	= 1990/05/03;
/ 	function	= displays window after block operation;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none;
*/
 
void		show2()		/* show after block operation */
{
	lncell_t	**tempspln;	/* block position in stack */
	int		offset;		/* line number */
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	register	lncellpt_t	reg1_ln;	/* pointer register */
 
	if ((tempspln = find_bk()) == sp_ln && cur_ln != endbk_ln)
		showrest();	/* the operation was made within the rest of the buffer */
	else
	{
		if (cur_ln == endbk_ln)
		{			/* current column changes */
			clear_cm();
			x_bas = point.column = 1;
			reg1_cm = cur_cm;
			cur_cm = (cur_ln = bgnbk_ln) -> addr;
 
			while (cur_cm != reg1_cm)
			{
				count_cm(getletr(cur_cm = getnext(push_cm(cur_cm))));
				adj_x();	/* adjust x_bas */
			}
		}
 
		if (tempspln != sp_ln && bgnbk_ln != endbk_ln)
		{				/* operation was made behind the current position */
			reg1_ln = *(sp_ln = tempspln);
 
			if (reg1_ln != cur_ln)
			{			/* make stack */
				++sp_ln;
				while ((reg1_ln = reg1_ln -> link) != cur_ln)
					*sp_ln++ = reg1_ln;
			}
 
			offset = sp_ln - spln_base;
			if ((y_bas -= point.line - offset) < 1)
				y_bas = 1;	/* line cannot be minus */
			point.line = offset;
		}
 
		show();
	}
}
 
/*
/  specification;
/ 	name		= show3;
/ 	creation	= 1990/05/03;
/ 	function	= displays window when scroll occured;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		show3()
{
	register	flag_t	flag = FALSE;	/* if scroll occured */
 
	if ((point.column - x_bas) >= (WIDTH - 1))
	{					/* current position is over right margin */
		while ((point.column - x_bas) >= (WIDTH - 1))
			x_bas += (WIDTH / 2);
		flag = TRUE;
	}
	else if ((point.column - x_bas) < 0)
	{					/* current position is over left margin */
		while ((point.column - x_bas) < 0)
			x_bas -= (WIDTH / 2);
		flag = TRUE;
	}
 
	if ((point.line - y_bas) >= height)
	{					/* current position is over the floor */
		++y_bas;
		flag = TRUE;
	}
	else if ((point.line - y_bas) < 0)
	{					/* current position is over the ceiling */
		--y_bas;
		flag = TRUE;
	}
 
	if (flag)
		show();
}
 
/*
/  specification;
/ 	name		= stripe;
/ 	creation	= 1990/05/03;
/ 	function	= displays line from the specified column;
/ 	parameter (in)	= off_x		: (int) column;
/ 			= reg1_cm	: (cmcellpt_t) column cell;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void		stripe(off_x,reg1_cm)
register	int		off_x;
register	cmcellpt_t	reg1_cm;
{
	int		temp;		/* distance to the next tab stop */
	flag_t		flag;		/* if within block */
	wchar_t		c;		/* a letter */
	char_t		first_byte;	/* first byte of the letter */
#ifdef  XWINDOW
	flag_t          empty_line = FALSE;
#endif
 
	/* check if within block */
 
	flag = within(reg1_cm);
 
	/* displays till the end of the line */
 
	for (; off_x < (WIDTH - 1); reg1_cm = getnext(reg1_cm))
	{
		if (!reg1_cm || (c = getletr(reg1_cm)) == '\n')
#ifdef  XWINDOW
		{
		        if (!off_x)
			  empty_line = TRUE;
			break;
		}
#else
			break;		/* end of text or end of line */
#endif
 
		if (c == '\t')
		{			/* horizontal tab */
			sp((temp = tab_len - off_x % tab_len) <= (WIDTH - 1) - off_x ? temp : (WIDTH - 1) - off_x);
			off_x += temp;
		}
		else
		{
			first_byte = c >> CHAR_BIT;
			if (++off_x >= (WIDTH - 1) && first_byte)
				putchar('%');	/* line ended with the first byte of a kanji */
			else
			{
#ifdef	JIS
				if (first_byte)
					++off_x;
				putwchar(c);
#else
				if (first_byte)
				{
					++off_x;
					putchar(first_byte);
				} 
				putchar(c & CHAR_MASK);
#endif
			}
		}
 
		if (reg1_cm == bgnbk_cm && endbk_cm)
		{				/* beginning of block */
			flag = TRUE;
			reverse();
		}
 
		if (reg1_cm == endbk_cm)
		{				/* ending of block */
			flag = FALSE;
			restore();
		}
	}
 
	if (reg1_cm && c != '\n' && getletr(reg1_cm) != '\n')
		putchar('+');		/* there's more */
	else
	{
#ifdef  XWINDOW
	        if (empty_line)
		{
		  putchar(' ');
		  off_x++;
		}
#endif
		cls_ln();
		if (off_x < (WIDTH - 1))
			fwd_cm((WIDTH - 1) - off_x);
		putchar('<');		/* line ended before right margin */
	}

	if (flag)
		restore();	/* restore character attribute */
}
 
/*
/  specification;
/ 	name		= screen;
/ 	creation	= 1990/05/03;
/ 	function	= displays window from the specified line;
/ 	parameter (in)	= offset	: (int) line;
/ 			= reg1_ln	: (lncellpt_t) line cell;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void		screen(off_y,reg1_ln)
int		off_y;
register	lncellpt_t	reg1_ln;
{
	register	int	off_x;	/* column */
	cmcellpt_t	reg1_cm;		/* pointer register */
	wchar_t		c;			/* a letter */
 
	/* displays till the end of the window */
 
	for (; reg1_ln && off_y < height; ++off_y,reg1_ln = reg1_ln -> link)
	{
		reg1_cm = reg1_ln -> addr;
 
		/* get to the left margin */
 
		for (off_x = 0; off_x < x_bas;)
		{
			if (!(reg1_cm = getnext(reg1_cm)) || (c = getletr(reg1_cm)) == '\n')
				break;		/* end of text or end of line */
 
			/* count column */
 
			if (c == '\t')
				off_x = off_x + tab_len - off_x % tab_len;
			else
			{
				if (c & ~CHAR_MASK)
					++off_x;
				++off_x;
			}
		}
 
		if (off_x == x_bas && c & ~CHAR_MASK)
		{				/* line starts from the second byte of a kanji */
			putchar('%');
			stripe(1,getnext(reg1_cm));	/* display line */
		}
		else
			stripe(0,reg1_cm);		/* display line */
	}
 
	/* if text ended before the window ends, indicate it */
 
	while (off_y++ < height)
	{
#ifdef	XWINDOW
		putchar(' ');
#endif
		cls_ln();
#ifdef	XWINDOW
		fwd_cm((WIDTH - 1) - 1);
#else
		fwd_cm((WIDTH - 1));
#endif
		putchar('.');
	}
}
 
/*
/  specification;
/ 	name		= within;
/ 	creation	= 1990/05/03;
/ 	function	= checks if the position to be displayed is within block;
/ 	parameter (in)	= reg1_cm	: (cmcellpt_t) the position;
/ 	parameter (out)	= none;
/ 	return		= TRUE	: (flag_t) within block;
/ 			= FALSE	: (flag_t) not within block;
*/
 
static	flag_t		within(reg1_cm)
register	cmcellpt_t	reg1_cm;
{
	register	cmcellpt_t	reg2_cm;	/* pointer register */
	flag_t		flag = FALSE;			/* if within */
 
	if (!bgnbk_cm || !endbk_cm || (reg2_cm = bgnbk_cm) == endbk_cm)	/* block not yet defined */
		return (FALSE);
 
	do
	{
		if (!(reg2_cm = getnext(reg2_cm)))	/* bad block */
		{
			bgnbk_cm = endbk_cm = 0;	/* release block */
			return (FALSE);
		}
 
		if (reg1_cm == reg2_cm)
			flag = TRUE;		/* within block */
	}
	while (reg2_cm != endbk_cm);
 
	if (flag)
		reverse();
 
	return (flag);
}
 
/*
/  specification;
/ 	name		= sp;
/ 	creation	= 1990/07/01;
/ 	function	= prints space;
/ 	parameter (in)	= x	: (int) length of space;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void	sp(x)
register	int	x;
{
	register	int	c = ' ';
 
	while (x-- > 0)
		putchar(c);
}
 
/* end of psyview.c */
