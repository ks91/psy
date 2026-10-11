/*
/  source file header;
/ 	name		= psybas.c : psy basic edit function source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/01;
/ 	contents	= new_ln	: (lncellpt_t) allocate new line cell;
/ 			= new_cm	: (cmcellpt_t) allocate new column cell;
/ 			= insletr	: (int) insert a letter;
/ 			= delletr	: (wchar_t) delete a letter;
/ 			= release	: (void) release cells;
/ 			= new_buf	: (bfcell_t *) allocate new buffer cell;
/ 			= rel_buf	: (void) release buffer cell;
/ 			= mod_buf	: (void) set / reset modify flag;
/ 			= new_wdw	: (wdcell_t *) allocate new window cell;
/ 			= loa_wat	: (void) load window attribute;
/ 			= sav_wat	: (void) save window attribute;
/ 			= count_cm	: (void) count column forward;
/ 			= getback	: (void) count column backward;
/ 			= get_cm	: (void) get to the specified column;
/ 			= undefbk	: (flag_t) check if block is defined;
/ 			= find_bk	: (lncellpt_t *) find block in stack;
*/
 
/* source file id */
 
#define	PSYBAS_C
 
/* include file */
 
#include 	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
lncellpt_t	new_ln();	/* allocate new line cell */
cmcellpt_t	new_cm();	/* allocate new column cell */
int		insletr();	/* insert a letter */
wchar_t		delletr();	/* delete a letter */
void		release();	/* release cells */
bfcell_t	*new_buf();	/* allocate new buffer cell */
void		rel_buf();	/* release buffer cell */
void		mod_buf();	/* set / reset modify flag */
wdcell_t	*new_wdw();	/* allocate new window cell */
void		loa_wat();	/* load window attribute */
void		sav_wat();	/* save window attribute */
void		count_cm();	/* count column forward */
void		getback();	/* count column backward */
void		get_cm();	/* get to the specified column */
flag_t		undefbk();	/* check if block is defined */
lncellpt_t	*find_bk();	/* find block in stack */
 
/*
/  specification;
/ 	name		= new_ln;
/ 	creation	= 1990/05/01;
/ 	function	= allocates new line cell;
/ 	parameter (in)	= none;
/ 	parameter (out) = none;
/ 	return		= NULL		: no more free cells;
/ 			= otherwise	: (lncellpt_t) pointer to the allocated cell ;
*/
 
/*
/  specification;
/ 	name		= insletr;
/ 	creation	= 1990/05/01;
/ 	function	= inserts a letter;
/ 	parameter (in)	= c	: (wchar_t) letter;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded ;
/ 			= ERROR	: (int) failed;
*/
 
int		insletr(c)
wchar_t		c;
{
	register	cmcellpt_t	reg1_cm,reg2_cm;/* pointer register */
	lncellpt_t	reg1_ln,reg2_ln;		/* pointer register */
 
	mod_buf(TRUE);		/* text is taken as modified */
 
	reg2_cm = getnext(cur_cm);	/* save next letter */
 
	if (!putnext(cur_cm,reg1_cm = new_cm()))/* allocate new cell */
	{
		/* allocation failed */
		putnext(cur_cm,reg2_cm);	/* recovery */
		psy_msg(E_LARGE,NULL);
		return (ERROR);
	}
 
	/* connect the letter */
 
	putletr(reg1_cm,c);
	putnext(reg1_cm,reg2_cm);
 
	/* if new line, insert a line */
 
	if (c == '\n')
	{
		reg2_ln = cur_ln -> link;	/* save next line */
 
		if (!(cur_ln -> link = reg1_ln = new_ln()))	/* allocate new cell */
		{
			/* allocation failed */ 
			cur_ln -> link = reg2_ln;	/* recovery */
			psy_msg(E_LARGE,NULL);
			return (ERROR);
		}
 
		/* connect new line */
 
		reg1_ln -> addr = reg1_cm;
		reg1_ln -> link = reg2_ln;
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= delletr;
/ 	creation	= 1990/05/01;
/ 	function	= deletes a letter;
/ 	parameter (in)	= kill	: (kill_t) kill mode;
/ 	parameter (out)	= none;
/ 	return		= NULL		: end of text;
/ 			= otherwise	: deleted letter (wchar_t);
*/
 
wchar_t		delletr(kill)
kill_t		kill;
{
	wchar_t		c;			/* letter to delete */
	flag_t		flag = FALSE;		/* if beginning or ending of block */
	register	cmcellpt_t	reg2_cm;/* pointer register */
	register	lncellpt_t	reg2_ln;/* pointet register */
 
	/* get the letter to delete */
 
	if (!(reg2_cm = getnext(cur_cm)))
		return (0);	/* end of text */
 
	mod_buf(TRUE);		/* text is taken as modified */
 
	if (reg2_cm == bgnbk_cm || reg2_cm == endbk_cm)
		flag = TRUE;	/* beginning or ending of block */
 
	c = getletr(reg2_cm);
	putnext(cur_cm,getnext(reg2_cm));
 
	switch (kill)
	{
		case abandon :
			putletr(reg2_cm,'\0');		/* release one cell */
			break;
 
		case forward :
			putnext(reg2_cm,0);
			if (!bgnkl_cm)
				bgnkl_cm = endkl_cm = reg2_cm;
			else
				endkl_cm = putnext(endkl_cm,reg2_cm);
			break;
 
		case backward :
			if (!bgnkl_cm)
			{
				bgnkl_cm = endkl_cm = reg2_cm;
				putnext(reg2_cm,0);
			}
			else
			{
				putnext(reg2_cm,bgnkl_cm);
				bgnkl_cm = reg2_cm;
			}
			break;
 
		default :
			break;
	}
 
	/* if line, delete the line */
 
	if (c == '\n')
	{
		if ((reg2_ln = cur_ln -> link) == bgnbk_ln || reg2_ln == endbk_ln)
			flag = TRUE;	/* beginning or ending of block */
		reg2_ln -> addr = 0;	/* release one cell */
		cur_ln -> link = reg2_ln -> link;
	}
 
	/* if beginning or ending of block, release the block */
 
	if (flag)
	{
		bgnbk_cm = endbk_cm = 0;
		show();
	}
 
	return (c);
}
 
/*
/  specification;
/ 	name		= release;
/ 	creation	= 1990/05/01;
/ 	function	= releases cells;
/ 	parameter (in)	= ln		: (lncellpt_t) line cell to start;
/ 			= lnlast	: (lncellpt_t) line cell to finish;
/ 			= cm		: (cmcellpt_t) column cell to start;
/ 			= cmlast	: (cmcellpt_t) column cell to finish;
/ 			= kill		: (kill_t) kill mode;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		release(ln,lnlast,cm,cmlast,kill)
lncellpt_t	ln,lnlast;
register	cmcellpt_t	cm,cmlast;
kill_t		kill;
{
	cmcellpt_t	reg1_cm;	/* register */
 
	if (cm != bgnkl_cm)
		mod_buf(TRUE);		/* text is taken as modified */ 
 
	/* release kill if not abandon and not appending */
 
	if (kill != abandon)
		res_kil();	/* reset kill buffer */
	reg1_cm = bgnkl_cm;
 
	/* release line cells */
 
	if (ln != lnlast)
	{
		do
		{
			if (ln == bgnbk_ln || ln == endbk_ln)
				bgnbk_cm = endbk_cm = 0;	/* release block */
			ln -> addr = 0;
		}
		while ((ln = ln -> link) != lnlast);
	}
 
	/* release column cells */
 
	if (cm != cmlast)
	{
		do
		{
			if (cm == bgnbk_cm || cm == endbk_cm)
				bgnbk_cm = endbk_cm = 0;	/* release block */
			switch (kill)
			{
				case abandon :
					putletr(cm,'\0');
					break;
 
				case forward :
					if (!reg1_cm)
						reg1_cm = bgnkl_cm = endkl_cm = cm;
					else
						endkl_cm = putnext(endkl_cm,cm);
					break;
 
				case backward :
					if (!reg1_cm)
					{
						reg1_cm = bgnkl_cm = endkl_cm = cm;
						putnext(cm,0);
					}
					else
					{
						putnext(cm,getnext(reg1_cm));
						putnext(reg1_cm,cm);
						reg1_cm = cm;
					}
					break;
 
				default :
					break;
			}
		}
		while ((cm = getnext(cm)) != cmlast);
 
		if (kill == forward)
			putnext(endkl_cm,0);
	}
 
	apnd_kill = FALSE;
}
 
/*
/  specification;
/ 	name		= new_buf;
/ 	creation	= 1990/05/01;
/ 	function	= allocates new buffer cell;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= NULL		: insufficient memory;
/ 			= otherwise	: (bfcell_t *) pointer to new cell ;
*/
 
bfcell_t	*new_buf()
{
	register	bfcell_t	*p;	/* for searching */
 
	/* search array */
 
	for (p = edit_buf + buff_no - 1; p >= edit_buf; p--)
	{
		if (!(p -> bf_ln))	/* found free buffer cell */
		{
			strcpy(p -> bf_name,mod_str[M_NEW - M_MSG]);
			if (!(p -> bf_cm = new_cm()) || !(p -> bf_ln = new_ln()))
				return (NULL);	/* insufficient memory */
			p -> bf_ln -> addr = p -> bf_cm;
			p -> bf_ln -> link = NULL;
			putletr(p -> bf_cm,'\n');
			putnext(p -> bf_cm,0);
			p -> bf_cnt = 0;
			p -> bf_modified = FALSE;
			p -> bf_encoding = psy_encoding;
			p -> bf_load_failed = FALSE;
 
			return (p);
		}
	}
 
	return (NULL);	/* no more free buffer cells */
}
 
/*
/  specification;
/ 	name		= rel_buf;
/ 	creation	= 1990/05/01;
/ 	function	= releases buffer cell;
/ 	parameter (in)	= bf	: (bfcell_t *) buffer cell to release;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		rel_buf(bf)
register	bfcell_t	*bf;
{
	/* if not in use, release buffer cell */
 
	if (--bf -> bf_cnt <= 0 && !getnext(bf -> bf_cm))
	{
		putletr(bf -> bf_cm,'\0');	/* release first column cell */
		bf -> bf_ln -> addr = 0;	/* release first line cell */
		bf -> bf_cm = 0;
		bf -> bf_ln = NULL;
	}
}
 
/*
/  specification;
/ 	name		= mod_buf;
/ 	creation	= 1990/06/23;
/ 	function	= sets / resets modify flag;
/ 	parameter (in)	= set	: (flag_t) set modify flag;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		mod_buf(set)
flag_t		set;
{
	register	bfcell_t	*bf = cur_wdw -> bf_addr;	/* current buffer */
 
	if (set)
	{
		if (!(bf -> bf_modified))
		{
			bf -> bf_modified = TRUE;
			if (cur_wdw != edit_wdw)
				psy_msg(M_MODIFY,TRUE);
		}
	}
	else
	{
		bf -> bf_modified = FALSE;
		if (cur_wdw != edit_wdw)
			psy_msg(M_MODIFY,FALSE);
	}
}
 
/*
/  specification;
/ 	name		= new_wdw;
/ 	creation	= 1990/05/01;
/ 	function	= allocates new window cell;
/ 	parameter (in)	= new_buf_f	: (flag_t) if new buffer;
/ 	parameter (out)	= none;
/ 	return		= NULL		: no more free cells;
/ 			= otherwise	: (wdcell_t *) pointer to new cell ;
*/
 
wdcell_t	*new_wdw(new_buf_f)
flag_t		new_buf_f;
{
	register	wdcell_t	*p;	/* for searching */
 
	/* search array */
 
	for (p = edit_wdw + MAX_WNDW_NO - 1; p > edit_wdw; p--)
	{
		if (!(p -> bf_addr))		/* found free window cell */
		{
			p -> wd_link = NULL;
			p -> wd_ceil = ceiling + height / 2 + 1;
			p -> wd_heig = ceiling + height - p -> wd_ceil;
			p -> wd_tabl = tab_len;
			p -> wd_insm = ins_mode;
			if (new_buf_f)
			{
				if (!(p -> bf_addr = new_buf()))
					return (NULL);
				p -> wd_point.line = p -> wd_prev.line = p -> wd_mark.line = 1;
				p -> wd_point.column = p -> wd_prev.column = p -> wd_mark.column = 1;
				p -> wd_x_bas = p -> wd_y_bas = 1;
			}
			else
			{
				p -> bf_addr = cur_wdw -> bf_addr;
				p -> wd_point.line = point.line;
				p -> wd_prev.line = _prev.line;
				p -> wd_mark.line = _mark.line;
				p -> wd_point.column = point.column;
				p -> wd_prev.column = _prev.column;
				p -> wd_mark.column = _mark.column;
				p -> wd_x_bas = x_bas;
				p -> wd_y_bas = y_bas;
			}
 
			p -> bf_addr -> bf_cnt++;
			return (p);
		}
	}
 
	return (NULL);
}
 
/*
/  specification;
/ 	name		= loa_wat;
/ 	creation	= 1990/05/01;
/ 	function	= loads window attribute;
/ 	parameter (in)	= wdw	: (wdcell_t *) window to load;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		loa_wat(wdw)
register	wdcell_t	*wdw;
{
	ceiling = wdw -> wd_ceil;
	height = wdw -> wd_heig;
	tab_len = wdw -> wd_tabl;
	ins_mode = wdw -> wd_insm;
	x_bas = wdw -> wd_x_bas;
	y_bas = wdw -> wd_y_bas;
	point.line = wdw -> wd_point.line;
	_prev.line = wdw -> wd_prev.line;
	_mark.line = wdw -> wd_mark.line;
	point.column = wdw -> wd_point.column;
	_prev.column = wdw -> wd_prev.column;
	_mark.column = wdw -> wd_mark.column;
 
	_ln = wdw -> bf_addr -> bf_ln;
	_cm = wdw -> bf_addr -> bf_cm;
}
 
/*
/  specification;
/ 	name		= sav_wat;
/ 	creation	= 1990/05/01;
/ 	function	= saves window attribute;
/ 	parameter (in)	= wdw	: (wdcell_t *) window to save;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		sav_wat(wdw)
register	wdcell_t	*wdw;
{
	wdw -> wd_ceil = ceiling;
	wdw -> wd_heig = height;
	wdw -> wd_tabl = tab_len;
	wdw -> wd_insm = ins_mode;
	wdw -> wd_x_bas = x_bas;
	wdw -> wd_y_bas = y_bas;
	wdw -> wd_point.line = point.line;
	wdw -> wd_prev.line = _prev.line;
	wdw -> wd_mark.line = _mark.line;
	wdw -> wd_point.column = point.column;
	wdw -> wd_prev.column = _prev.column;
	wdw -> wd_mark.column = _mark.column;
}
 
/*
/  specification;
/ 	name		= count_cm;
/ 	creation	= 1990/05/02;
/ 	function	= counts column number;
/ 	parameter (in)	= c	: (wchar_t) current letter;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		count_cm(c)
register	wchar_t		c;
{
	/* count column number according to the character status */
 
	if (c == '\t')
		point.column = point.column + tab_len - (point.column - 1) % tab_len;
	else
		point.column += psy_char_width(c);
}
 
/*
/  specification;
/ 	name		= getback;
/ 	creation	= 1990/05/01;
/ 	function	= counts column number backward;
/ 	parameter (in)	= c	: (wchar_t) current letter;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		getback(c)
register	wchar_t		c;
{
	register	cmcellpt_t	reg1_cm;	/* pointer register */
 
	/* if tab, counts from the top of the line */
 
	if (c == '\t')
	{
		reg1_cm = cur_cm;
		cur_cm = cur_ln -> addr;
		point.column = 1;
 
		while ((cur_cm = getnext(cur_cm)) != reg1_cm)
			count_cm(getletr(cur_cm));
	}
	else		/* if not so, decrease column number */
	{
		point.column -= psy_char_width(c);
	}
 
	cur_cm = pop_cm();
}
 
/*
/  specification;
/ 	name		= get_cm;
/ 	creation	= 1990/05/01;
/ 	function	= gets to the specified column;
/ 	parameter (in)	= column	: (int) column number;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		get_cm(column)
register	int	column;
{
	wchar_t		c;	/* a letter */
 
	/* get to the top of the line */
 
	clear_cm();		/* clear column stack */
	point.column = 1;
	cur_cm = cur_ln -> addr;
 
	/* counts column */
 
	while (point.column < column && (cur_cm = getnext(push_cm(cur_cm))) && (c = getletr(cur_cm)) != '\n')
		count_cm(c);
 
	/* if end of text or end of line, get back */
 
	if (point.column < column)
		cur_cm = pop_cm();
}
 
/*
/  specification;
/ 	name		= undefbk;
/ 	creation	= 1990/05/01;
/ 	function	= checks if block is defined;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= TRUE	: (flag_t) undefined or within block;
/ 			= FALSE	: (flag_t) ready to operate block;
*/
 
flag_t		undefbk()
{
	register	cmcellpt_t	reg1_cm;	/* pointer register */
 
	/* check if defined */
 
	if (!bgnbk_cm || !endbk_cm || (reg1_cm = bgnbk_cm) == endbk_cm)	/* block is not defined */
		return (TRUE);
 
	/* check if current position is in the block */
 
	while ((reg1_cm = getnext(reg1_cm)) != endbk_cm)
	{
		if (!reg1_cm || reg1_cm == cur_cm)	/* within block or block is not defined properly */
			return (TRUE);
	}
 
	return (FALSE);
}
 
/*
/  specification;
/ 	name		= find_bk;
/ 	creation	= 1990/05/02;
/ 	function	= finds block in line stack;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= point in stack (lncellpt_t *);
*/
 
lncellpt_t	*find_bk()
{
	register	lncellpt_t	*tempspln;	/* for searching */
 
	for (tempspln = ln_stack + 1; tempspln != sp_ln; ++tempspln)
		if (*tempspln == bgnbk_ln)
			break;	/* block is in stack */
 
	return (tempspln);
}
 
/* end of psybas.c */
