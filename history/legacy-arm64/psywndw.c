/*
/  source file header;
/ 	name		= psywndw.c : psy window control source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/03;
/ 	contents	= dsp_scr	: (int) display all windows;
/ 			= shw_wdw	: (void) show specified window;
/ 			= enstack	: (void) make stack for window;
/ 			= opn_wdw	: (int) open window;
/ 			= opn_shw	: (int) open shell window;
/ 			= cls_wdw	: (int) close window;
/ 			= sel_wdw	: (int) select window;
/ 			= slb_wdw	: (int) select window backward;
/ 			= enl_wdw	: (int) enlarge window;
/ 			= ens_wdw	: (int) ensmall window;
/ 			= adj_x		: (void) adjust left margin;
/ 			= get_shw	: (wdcell_t *) get shell window;
/ 			= opn_wdwk	: (wdcell_t *) open window kernel;
*/
 
/* source file id */
 
#define	PSYWNDW_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functoins */
 
int		dsp_scr();	/* display all windows */
void		shw_wdw();	/* show specified window */
void		enstack();	/* make stack for window */
int		opn_wdw();	/* open window */
int		opn_shw();	/* open shell window */
int		cls_wdw();	/* close window */
int		sel_wdw();	/* select window */
int		slb_wdw();	/* select window backward */
int		enl_wdw();	/* enlarge window */
int		ens_wdw();	/* ensmall window */
void		adj_x();	/* adjust left margin */
wdcell_t	*get_shw();	/* get shell window */
 
/* private declaration */
 
/* private functions */
 
static	wdcell_t	*opn_wdwk();	/* open window kernel */
 
/*
/  specification;
/ 	name		= dsp_scr;
/ 	creation	= 1990/05/03;
/ 	function	= displays all the windows;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		dsp_scr()
{
	register	wdcell_t	*p_wd;	/* pointer to window cell */
	register	wdcell_t	*previous;	/* previous window */
 
	clr_rp();		/* clear repeat count */
 
	/* save current window */
 
	sav_wat(previous = cur_wdw);
 
	/* display edit windows */
 
	for (p_wd = wdw_list; p_wd; p_wd = p_wd -> wd_link)
		shw_wdw(p_wd);
 
	/* display mini-window */
 
	shw_wdw(edit_wdw);
 
	/* restore current window */
 
	loa_wat(cur_wdw = previous);
	enstack();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= shw_wdw;
/ 	creation	= 1990/05/03;
/ 	function	= displays the specified window;
/ 	parameter (in)	= p_wd	: (wdcell_t *) pointer to the window cell;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		shw_wdw(p_wd)
register	wdcell_t	*p_wd;
{
	/* make the specified window current */
 
	loa_wat(cur_wdw = p_wd);
	enstack();
 
	/* display window */
 
	show();
 
	/* display mode line */
 
	if (p_wd != edit_wdw)	/* if not mini-window */
	{
		psy_msg(M_MODE,NULL);
		psy_msg(M_NAME,p_wd -> bf_addr -> bf_name);
		psy_msg(M_INSMOD,p_wd -> wd_insm);
		psy_msg(M_MODIFY,p_wd -> bf_addr -> bf_modified);
	}
}
 
/*
/  specification;
/ 	name		= enstack;
/ 	creation	= 1990/05/03;
/ 	function	= makes stack for current window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		enstack()
{
	register	int	c_y;			/* line counter */
	register	lncellpt_t	reg1_ln;	/* pointer register */
 
	/* clear stack */
 
	clear_ln();
	clear_cm();
 
	/* push till current line */
 
	for (c_y = 1, reg1_ln = _ln; c_y < point.line && reg1_ln -> link; c_y++, reg1_ln = push_ln(reg1_ln) -> link)
		; 
	*sp_ln = reg1_ln;
 
	/* adjust ceiling */
 
	if ((y_bas = c_y - (point.line - y_bas)) < 1)
		y_bas = 1;		/* line cannot be minus */
 
	if (c_y - y_bas >= height)
		y_bas = c_y - height + 1;
 
	/* set current line */
 
	point.line = c_y;
	cur_ln = reg1_ln;
 
	/* set column stack */
 
	get_cm(point.column);
 
	adj_x();	/* adjust left margin */
}
 
/*
/  specification;
/ 	name		= opn_wdw;
/ 	creation	= 1990/05/03;
/ 	function	= opens a window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		opn_wdw()
{
	clr_rp();
 
	if (!strcmp(cur_wdw -> bf_addr -> bf_name,mod_str[M_SHELL - M_MSG]) || !opn_wdwk(FALSE))
		return (OK);
	return (dsp_scr());
}
 
/*
/  specification;
/ 	name		= opn_shw;
/ 	creation	= 1990/06/14;
/ 	function	= opens a shell window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		opn_shw()
{
	register	wdcell_t	*wdw;	/* pointer to window cell */
 
	clr_rp();
 
	/* if there was already shell window, cannot open */
 
	if (get_shw())
		return (OK);
 
	if (!(wdw = opn_wdwk(TRUE)))
		return (OK);
	strcpy(wdw -> bf_addr -> bf_name,mod_str[M_SHELL - M_MSG]);
 
	return (dsp_scr());
}
 
/*
/  specification;
/ 	name		= cls_wdw;
/ 	creation	= 1990/05/04;
/ 	function	= closes the current window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		cls_wdw()
{
	register	wdcell_t	*wdw,*p_wdw;	/* pointers to window cells */
 
	clr_rp();		/* clear repeat count */
 
	if (cur_wdw == wdw_list && !(cur_wdw -> wd_link) || cur_wdw == edit_wdw || !strcmp(cur_wdw -> bf_addr -> bf_name,mod_str[M_NEW - M_MSG]) && getnext(_cm) && cur_wdw -> bf_addr -> bf_cnt <= 1)
		return (OK);		/* cannot close the only window or mini-window */
 
	/* search current window in window list */
 
	if (cur_wdw == wdw_list)
		p_wdw = NULL;
	else
		for (p_wdw = wdw_list; p_wdw -> wd_link != cur_wdw; p_wdw = p_wdw -> wd_link)
			;
 
	/* give window area to the next window */
 
	if (!(wdw = cur_wdw -> wd_link))
		wdw = p_wdw;
	else
		wdw -> wd_ceil = ceiling;
 
	wdw -> wd_heig += height + 1;
 
	/* reconnect the window list */
 
	if (!p_wdw)
		wdw_list = wdw_list -> wd_link;
	else
		p_wdw -> wd_link = cur_wdw -> wd_link;
 
	/* release text if shell window */
 
	if (!strcmp(cur_wdw -> bf_addr -> bf_name,mod_str[M_SHELL - M_MSG]))
	{
		psy_msg(S_RELMEM,NULL);
		release(_ln -> link,NULL,getnext(_cm),0,abandon);
 
		_ln -> link = NULL;
		putnext(_cm,0);
 
		psy_msg(P_CLEAR,NULL);
	}
 
	/* release buffer */
 
	rel_buf(cur_wdw -> bf_addr);
	cur_wdw -> bf_addr = NULL;
 
	/* change current window */
 
	if (!(cur_wdw = cur_wdw -> wd_link))
		cur_wdw = wdw_list;
 
	loa_wat(cur_wdw);
 
	return (dsp_scr());
}
 
/*
/  specification;
/ 	name		= sel_wdw;
/ 	creation	= 1990/05/04;
/ 	function	= selects window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int	sel_wdw()
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	if (cur_wdw == edit_wdw)
		return (OK);	/* cannot select from mini-window */
 
	/* save window */
 
	sav_wat(cur_wdw);
 
	/* select next window */
 
	for (; rp_cnt-- > 0;)
		if (!(cur_wdw = cur_wdw -> wd_link))
			cur_wdw = wdw_list;
 
	/* load window */
 
	loa_wat(cur_wdw);
 
	shw_wdw(cur_wdw);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= slb_wdw;
/ 	creation	= 1990/05/04;
/ 	function	= selects window backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		slb_wdw()
{
	register	wdcell_t	*p_wd;	/* for searching */
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	if (cur_wdw == edit_wdw)
		return (OK);	/* cannot select from mini-window */
 
	/* save window */
 
	sav_wat(cur_wdw);
 
	/* select previous window */
 
	for (; rp_cnt-- > 0;)
	{
		for (p_wd = wdw_list; p_wd -> wd_link && p_wd -> wd_link != cur_wdw; p_wd = p_wd -> wd_link)
			;
		cur_wdw = p_wd;
	}
 
	/* load window */
 
	loa_wat(cur_wdw);
 
	shw_wdw(cur_wdw);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= enl_wdw;
/ 	creation	= 1990/05/04;
/ 	function	= enlarges the current window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		enl_wdw()
{
	register	wdcell_t	*wdw;	/* window to get space from */
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		if (cur_wdw == wdw_list && !(cur_wdw -> wd_link) || cur_wdw == edit_wdw)
			break;	/* cannot enlarge the only window or mini-window */
 
		if (!(wdw = cur_wdw -> wd_link))
		{			/* enlarge the last window */
			for (wdw = wdw_list; wdw -> wd_link != cur_wdw; wdw = wdw -> wd_link)
				;
 
			/* wdw == previous window */
	
			if (wdw -> wd_heig > 1)
			{
				wdw -> wd_heig -= 1;
				ceiling -= 1;
				height += 1;
			}
		}
		else
		{
			/* wdw == next window */
 
			if (wdw -> wd_heig > 1)
			{
				wdw -> wd_heig -= 1;
				wdw -> wd_ceil += 1;
				height += 1;
			}
		}
	}
 
	return (dsp_scr());
}
 
/*
/  specification;
/ 	name		= ens_wdw;
/ 	creation	= 1990/05/04;
/ 	function	= ensmalls the current window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		ens_wdw()
{
	register	wdcell_t	*wdw;	/* window to give space to */
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		if (cur_wdw == wdw_list && !(cur_wdw -> wd_link) || cur_wdw == edit_wdw)
			break;		/* cannot ensmall the only window or mini-window */
 
		if (height > 1)
		{
			if (!(wdw = cur_wdw -> wd_link))
			{			/* ensmall the last window */
				for (wdw = wdw_list; wdw -> wd_link != cur_wdw; wdw = wdw -> wd_link)
					;
 
				/* wdw == previous window */
 
				wdw -> wd_heig += 1;
				ceiling += 1;
				height -= 1;
			}
			else
			{
				/* wdw == next window */		
 
				wdw -> wd_heig += 1;
				wdw -> wd_ceil -= 1;
				height -= 1;
			}		
		}
	}
 
	return (dsp_scr());
}
		
/*
/  specification;
/ 	name		= adj_x;
/ 	creation	= 1990/06/11;
/ 	function	= adjusts left margin;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		adj_x()
{
	register	int	column = point.column;	/* current column */
 
	while ((column - x_bas) < 0)
		x_bas -= (WIDTH / 2);
	while ((column - x_bas) >= (WIDTH - 1))
		x_bas += (WIDTH / 2); 
}
 
/*
/  specification;
/ 	name		= get_shw;
/ 	creation	= 1990/06/15;
/ 	function	= gets pointer to shell window;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= NULL		: shell window is not opened;
/ 			= otherwise	: (wdcell_t *) pointer to shell window;
*/
 
wdcell_t		*get_shw()
{
	register	wdcell_t	*wdw;
 
	for (wdw = wdw_list; wdw; wdw = wdw -> wd_link)
		if (!strcmp(wdw -> bf_addr -> bf_name,mod_str[M_SHELL - M_MSG]))
			break;
	return (wdw);
}
 
/*
/  specification;
/ 	name		= opn_wdwk;
/ 	creation	= 1990/06/15;
/ 	function	= opens window;
/ 	parameter (in)	= new_buf_f	: (flag_t) if new buffer;
/ 	parameter (out)	= none;
/ 	return		= NULL		: cannot open;
/ 			= otherwise	: (wdcell_t *) opened window;
*/
 
static	wdcell_t	*opn_wdwk(new_buf_f)
flag_t		new_buf_f;
{
	register	wdcell_t	*wdw;	/* pointer to window cell */
 
	/* if height is too short, cannot open */
 
	if (height < 3)
		return ((wdcell_t *)OK);
 
	/* save current window */
 
	sav_wat(cur_wdw);
 
	/* open a window */
 
	if (!(wdw = new_wdw(new_buf_f)))
	{			/* cannot allocate window cell */
		psy_msg(E_NOMEM,NULL);
		return (NULL);
	}
 
	/* set attribute */
 
	height = cur_wdw -> wd_heig - wdw -> wd_heig - 1;
	if (point.line - y_bas >= height)
		y_bas += height;
 
	/* link window list */
 
	wdw -> wd_link = cur_wdw -> wd_link;
	cur_wdw -> wd_link = wdw;
 
	return (wdw);
}
 
/* end of psywndw.c */
