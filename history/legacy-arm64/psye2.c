/*
/  source file header;
/ 	name		= psye2.c : psy edit function source file 2;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/20;
/ 	contents	= fwd_ltr	: (int) forward letter;
/ 			= fwd_ltrk	: (wchar_t) forward letter kernel; 
/ 			= bwd_ltr	: (int) backward letter;
/ 			= bwd_ltrk	: (wchar_t) backward letter kernel;
/ 			= fwd_lin	: (int) forward line;
/ 			= fwd_link	: (int) forward line kernel;
/ 			= bwd_lin	: (int) backward line;
/ 			= bwd_link	: (int) backward line kernel;
/ 			= btm_lin	: (int) bottom of the line;
/ 			= top_lin	: (int) top of the line;
/ 			= fwd_wrd	: (int) forward word;
/ 			= bwd_wrd	: (int) backward word;
/ 			= fwd_wdw	: (int) forward window;
/ 			= bwd_wdw	: (int) backward window;
/ 			= btm_wdw	: (int) bottom of the window;
/ 			= top_wdw	: (int) top of the window;
/ 			= rht_wdw	: (int) right edge of the window;
/ 			= lft_wdw	: (int) left edge of the window;
/ 			= btm_txt	: (int) bottom of the text;
/ 			= top_txt	: (int) top of the text;
/ 			= inword	: (flag_t) checks if in word;
*/
 
/* source file id */
 
#define	PSYE2_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		fwd_ltr();	/* forward letter */
wchar_t		fwd_ltrk();	/* forward letter kernel */
int		bwd_ltr();	/* backward letter */
wchar_t		bwd_ltrk();	/* backward letter kernel */
int		fwd_lin();	/* forward line */
int		fwd_link();	/* forward line kernel */
int		bwd_lin();	/* backward line */
int		bwd_link();	/* backward line kernel */
int		btm_lin();	/* bottom of the line */
int		top_lin();	/* top of the line */
int		fwd_wrd();	/* forward word */
int		bwd_wrd();	/* backward word */
int		fwd_wdw();	/* forward window */
int		bwd_wdw();	/* backward window */
int		btm_wdw();	/* bottom of the window */
int		top_wdw();	/* top of the window */
int		rht_wdw();	/* right edge of the window */
int		lft_wdw();	/* left edge of the window */
int		btm_txt();	/* bottom of the text */
int		top_txt();	/* top of the text */
flag_t		inword();	/* checks if in word */
 
/*
/  specification;
/ 	name		= fwd_ltr;
/ 	creation	= 1990/05/20;
/ 	function	= moves one letter forward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		fwd_ltr()
{
	register	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		/* move one letter forward */
 
		if (!fwd_ltrk(FALSE))
			break;
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= fwd_ltrk;
/ 	creation	= 1990/06/05;
/ 	function	= moves one letter forward;
/ 	parameter (in)	= scrl	: (flag_t) scroll;
/ 	parameter (out)	= none;
/ 	return		= NULL		: cannot move;
/ 			= otherwise	: (wchar_t) acquired letter;
*/
 
wchar_t		fwd_ltrk(scrl)
register	flag_t		scrl;
{
	register	wchar_t		c;	/* a letter */
 
	/* cannot move if end of text */
 
	if (!(cur_cm = getnext(push_cm(cur_cm))))
	{
		cur_cm = pop_cm();
		return (0);
	}
 
	/* move one line forward if end of line */
 
	if ((c = getletr(cur_cm)) == '\n')
		fwd_link(1,scrl);
 
	/* move one letter forward */
 
	else
	{
		count_cm(c);
		if (scrl)
			adj_x();	/* adjust x_bas */
	}
 
	return (c);
}
 
/*
/  specification;
/ 	name		= bwd_ltr;
/ 	creation	= 1990/05/20;
/ 	function	= moves one letter backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		bwd_ltr()
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */ 
 
	for (; rp_cnt-- > 0; )
	{
		/* move one letter backward */
 
		if (!bwd_ltrk(FALSE))
			break;
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= bwd_ltrk;
/ 	creation	= 1990/06/05;
/ 	function	= moves one letter backward;
/ 	parameter (in)	= scrl	: (flag_t) scroll;
/ 	parameter (out)	= none;
/ 	return		= NULL		: cannot move;
/ 			= otherwise	: (wchar_t) acquired letter;
*/
 
wchar_t		bwd_ltrk(scrl)
register	flag_t		scrl;
{
	register	wchar_t		c;	/* a letter */
 
	/* cannot move if top of the text */
 
	if (cur_cm == _cm)
		return (0);
 
	/* move one line backward if top of the line */
 
	if ((c = getletr(cur_cm)) == '\n')
		bwd_link(MAXCOLM,scrl);
 
	/* move one letter backward */
 
	else
	{
		getback(c);
		if (scrl)
			adj_x();	/* adjust x_bas */
	}
 
	return (c);
}
 
/*
/  specification;
/ 	name		= fwd_lin;
/ 	creation	= 1990/05/20;
/ 	function	= moves one line forward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int)	succeeded;
*/
 
int		fwd_lin()
{
	register	int	rp_cnt = rp;	/* repeat count */
	point_t		target;			/* target point */ 
 
	clr_rp();		/* clear repeat count */ 
 
	if (rp_cnt <= 1)
	{
		/* move one line forward */
 
		fwd_link(point.column,FALSE);
		show3();
	}
	else
	{
		target.line = point.line + rp_cnt;
		target.column = point.column;
		view(&target);
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= fwd_link;
/ 	creation	= 1990/06/10;
/ 	function	= moves one line forward;
/ 	parameter (in)	= c_x	: (int) target column;
/ 			= scrl	: (flag_t) scroll;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		fwd_link(c_x,scrl)
register	int	c_x;
register	flag_t	scrl;
{
	/* cannot move if last line */
 
	if (!(cur_ln = push_ln(cur_ln) -> link))
	{
		cur_ln = pop_ln();
		return (ERROR);
	}
 
	/* move one line forward */
 
	++point.line;
	get_cm(c_x);
 
	if (scrl)
	{
		y_bas++;
		adj_x();	/* adjust x_bas */
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= bwd_lin;
/ 	creation	= 1990/05/20;
/ 	function	= moves one line backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		bwd_lin()
{
	register	int	rp_cnt = rp;	/* repeat count */
	point_t		target;			/* target point */
 
	clr_rp();		/* clear repeat count */ 
 
	if (rp_cnt <= 1)
	{
		/* move one letter backward */
 
		bwd_link(point.column,FALSE);
		show3();
	}
	else
	{
		target.line = point.line - rp_cnt;
		target.column = point.column;
		view(&target);
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= bwd_link;
/ 	creation	= 1990/06/10;
/ 	function	= moves one line backward;
/ 	parameter (in)	= c_x	: (int) target column;
/ 			= scrl	: (flag_t) scroll;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		bwd_link(c_x,scrl)
register	int	c_x;
register	flag_t	scrl;
{
	/* cannot move if first line */
 
	if (cur_ln == _ln)
		return (ERROR);
 
	/* move one line backward */
 
	cur_ln = pop_ln();
 
	--point.line;
	get_cm(c_x);
 
	if (scrl)
	{
		if (--y_bas < 1)
			y_bas = 1;
		adj_x();	/* adjust x_bas */ 
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= btm_lin;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the bottom of the line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		btm_lin()
{
	clr_rp();		/* clear repeat count */
 
	/* move to the bottom of the line */
 
	get_cm(MAXCOLM);
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= top_lin;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the top of the line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		top_lin()
{
	clr_rp();		/* clear repeat count */
 
	/* move to the top of the line */
 
	get_cm(1);
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= fwd_wrd;
/ 	creation	= 1990/05/20;
/ 	function	= moves one word forward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		fwd_wrd()
{
	register	wchar_t		c;	/* a letter */
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		/* if end of line, move one line forward */
 
		if (!getnext(cur_cm))
			break;
 
		if (getletr(getnext(cur_cm)) == '\n')
		{
			if (fwd_link(1,FALSE))
				break;
			show3();
			continue;
		}
 
		/* skip current word */
 
		while ((cur_cm = getnext(push_cm(cur_cm))) && inword(c = getletr(cur_cm)))
			count_cm(c);
		cur_cm = pop_cm();
 
		/* skip separator */
 
		while ((cur_cm = getnext(push_cm(cur_cm))) && !inword(c = getletr(cur_cm)) && c != '\n')
			count_cm(c);
		cur_cm = pop_cm();
 
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= bwd_wrd;
/ 	creation	= 1990/05/20;
/ 	function	= moves one word backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		bwd_wrd()
{
	register	wchar_t		c;	/* a letter */
	register	int	rp_cnt = rp;	/* repeat cuont */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		/* if top of the line, move one line backword */
 
		if (cur_cm == _cm || getletr(cur_cm) == '\n' && bwd_link(MAXCOLM,FALSE))
			break;
 
		/* skip current separator */
 
		while (cur_cm != _cm && (c = getletr(cur_cm)) != '\n' && !inword(c))
			getback(c);
 
		/* skip word */
 
		while (cur_cm != _cm && inword(c = getletr(cur_cm)))
			getback(c);
 
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= fwd_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves one virtual screen forward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		fwd_wdw()
{
	register	int	rp_cnt = rp;	/* repeat count */
	point_t		target;			/* target point */
 
	clr_rp();		/* clear repeat count */
	_prev.line =  point.line;
	_prev.column = point.column;
 
	target.line = point.line + height * rp_cnt;
	target.column = point.column;
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= bwd_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves one virtual screen backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		bwd_wdw()
{
	register	int	rp_cnt = rp;	/* repeat count */
	point_t		target;			/* target point */
 
	clr_rp();		/* clear repeat count */
	_prev.line = point.line;
	_prev.column = point.column;
 
	target.line = point.line - height * rp_cnt;
	target.column = point.column;
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= btm_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the bottom of the virtual screen;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		btm_wdw()
{
	clr_rp();		/* clear repeat count */
 
	/* get to the line */
 
	while ((point.line - y_bas) < (height - 1) && cur_ln -> link)
	{
		cur_ln = push_ln(cur_ln) -> link;
		++point.line;
	}
 
	/* get to the column */
 
	get_cm(point.column);
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= top_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the top of the virtual screen;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		top_wdw()
{
	clr_rp();		/* clear repeat count */
 
	/* get to the line */
 
	while (point.line - y_bas > 0)
	{
		cur_ln = pop_ln();
		--point.line;
	}
 
	/* get to the column */
 
	get_cm(point.column);
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= rht_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the right edge of the virtual screen;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		rht_wdw()
{
	clr_rp();		/* clear repeat count */
 
	get_cm(x_bas + (WIDTH - 2));
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= lft_wdw;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the left edge of the virtual screen;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		lft_wdw()
{
	clr_rp();		/* clear repeat count */
 
	get_cm(x_bas);
	show3();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= btm_txt;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the bottom of the text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		btm_txt()
{
	point_t		target;		/* target point */
 
	clr_rp();		/* clear repeat count */
	_prev.line = point.line;
	_prev.column = point.column;
 
	target.line = MAX_BUFF_LINE;
	target.column = MAXCOLM;
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= top_txt;
/ 	creation	= 1990/05/20;
/ 	function	= moves to the top of the text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		top_txt()
{
	point_t		target;		/* target point */
 
	clr_rp();		/* clear repeat count */
	_prev.line = point.line;
	_prev.column = point.column;
 
	target.line = target.column = 1;
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= inword;
/ 	creation	= 1990/05/20;
/ 	function	= checks if the specified letter is in a word;
/ 	parameter (in)	= c	: (wchar_t) letter;
/ 	parameter (out)	= none;
/ 	return		= TRUE	: (flag_t) the letter is in a word;
/ 			= FALSE	: (flag_t) the letter is not in a word;
*/
 
flag_t		inword(c)
register	wchar_t		c;
{
	return (isalpha(c) || isdigit(c) || c >= CONT_1 && c <= CHO_ON || c > SEP_END || c >= 0xa6 && c <= 0xdf);
}
 
/* end of psye2.c */
