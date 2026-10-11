/*
/  source file header;
/ 	name		= psye3.c : psy edit function source file 3;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/20;
/ 	contents	= new_lin	: (int) new line;
/ 			= fwd_tab	: (int) forward tab;
/ 			= chg_ins	: (int) change insert / over-write mode;
/ 			= set_tab	: (int) set tab stop;
/ 			= del_ltr	: (int) delete letter;
/ 			= dlb_ltr	: (int) delete left letter;
/ 			= del_lin	: (int) delete line;
/ 			= dlb_lin	: (int) delete line backward;
/ 			= del_wrd	: (int) delete word;
/ 			= dlb_wrd	: (int) delete left word;
/ 			= ins_lin	: (int) insert line;
/ 			= set_mrk	: (int) set mark;
/ 			= jmp_mrk	: (int) jump to mark;
/ 			= pre_lin	: (int) previous line;
/ 			= jmp_lin	: (int) jump to specified line;
/ 			= brk_min	: (int) break mini-buffer;
/ 			= cnt_pnt	: (int) count cursor point;
/ 			= cnt_byt	: (int) count buffer bytes;
/ 			= vis_txt	: (int) visit text;
/ 			= rea_txt	: (int) read text;
/ 			= ren_txt	: (int) rename text;
/ 			= lst_txt	: (int) list text;
/ 			= del_link	: (int) delete line kernel;
*/
 
/* source file id */
 
#define	PSYE3_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		new_lin();	/* new line */
int		fwd_tab();	/* forward tab */
int		chg_ins();	/* change insert / over-write mode */
int		set_tab();	/* set tab stop */
int		del_ltr();	/* delete letter */
int		dlb_ltr();	/* delete left letter */
int		del_lin();	/* delete line */
int		dlb_lin();	/* delete line backward */
int		del_wrd();	/* delete word */
int		dlb_wrd();	/* delete left word */
int		ins_lin();	/* insert line */
int		set_mrk();	/* set mark */
int		jmp_mrk();	/* jump to mark */
int		pre_lin();	/* previous line */
int		jmp_lin();	/* jump to specified line */
int		brk_min();	/* break mini-buffer */
int		cnt_pnt();	/* count cursor point */
int		cnt_byt();	/* count buffer bytes */
int		vis_txt();	/* visit text */
int		rea_txt();	/* read text */
int		ren_txt();	/* rename text */
int		lst_txt();	/* list text */
 
/* private declaration */
 
/* private functions */
 
static	int	del_link();	/* delete line kernel */
 
/*
/  specification;
/ 	name		= new_lin;
/ 	creation	= 1990/05/20;
/ 	function	= inserts new line / moves to the next line / exits mini-buffer;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK		: (int) succeeded;
/ 			= MINI_END	: (int) end of mini-buffer;
*/
 
int		new_lin()
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	/* if mini-window, exits mini-buffer */
 
	if (cur_wdw == edit_wdw)
		return (MINI_END);
 
	for (; rp_cnt-- > 0;)
	{
		if (!(cur_ln = push_ln(cur_ln) -> link) || ins_mode)
		{	/* inserts line */
			if (!ins_mode)
			{
				while ((cur_cm = getnext(push_cm(cur_cm))))
					count_cm(getletr(cur_cm));
				cur_cm = pop_cm();
			}
 
			cur_ln = pop_ln();
			if (insletr('\n') == ERROR)
				return (OK);
			showrest();
 
			fwd_link(1,FALSE);
		}
		else	/* top of the next line */
		{
			get_cm(x_bas);
			point.line++;
		}
 
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= fwd_tab;
/ 	creation	= 1990/05/20;
/ 	function	= inserts tab / moves to the next tab stop;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		fwd_tab()
{
	register	int	tempc;		/* next tab stop */
	register	int	rp_cnt = rp;	/* repeat count */
	wchar_t		c;	/* a letter */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		tempc = point.column + tab_len - (point.column - 1) % tab_len;
 
		if (ins_mode || !getnext(cur_cm) || getletr(getnext(cur_cm)) == '\n')
		{			/* inserts tab */
			if (insletr('\t') == ERROR)
				break;
 
			showln();
			cur_cm = getnext(push_cm(cur_cm));
			point.column = tempc;
		}
		else
		{			/* moves to the next tab stop */
			while (point.column < tempc)
			{
				if (!(cur_cm = getnext(push_cm(cur_cm))) || (c = getletr(cur_cm)) == '\n')
				{	/* must insert tab */
					c = '\t';
					cur_cm = pop_cm();
 
					if (insletr(c) == ERROR)
						return (OK);
 
					cur_cm = getnext(push_cm(cur_cm));
				}
 
				count_cm(c);
			}
		}
 
		show3();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= chg_ins;
/ 	creation	= 1990/05/20;
/ 	function	= alters insert / overwrite mode;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		chg_ins()
{
	clr_rp();		/* clear repeat count */
 
	ins_mode = !ins_mode;
 
	/* if not mini-buffer, display the mode */
 
	if (cur_wdw != edit_wdw)
		psy_msg(M_INSMOD,ins_mode);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= set_tab;
/ 	creation	= 1990/05/20;
/ 	function	= sets tab length;
/ 	parameter (in)	= none; 
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		set_tab()
{
	char_t		buf[8];
	char_t *digit = buf;
	int code;		/* buffer for the string */
	register	int	new_tab;	/* new tab length */
 
	clr_rp();		/* clear repeat count */
 
	if (rea_min(P_TABLEN,buf,sizeof(buf)) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	code = psy_next_char(&digit);
	new_tab = code >= ZERO ? code - ZERO : code - '0';
	if (new_tab <= MAX_TAB_LENGTH && new_tab > 0)
	{
		tab_len = new_tab;
		show();			/* column number not guaranteed */
	}
	else
		psy_msg(E_TABLEN,NULL);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= del_ltr;
/ 	creation	= 1990/05/20;
/ 	function	= deletes the letter;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		del_ltr()
{
	register	int	rp_cnt = rp;	/* repeat count */
	register	flag_t	rp_flg = rp > 1;/* repeat flag */
 
	clr_rp();		/* clear repeat count */
	if (rp_flg)
		res_kil();	/* clear kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		switch (delletr(rp_flg ? forward : abandon))
		{
			case	0 :	/* end of text */
				return (OK);
 
			case	'\n' : 		/* end of line */
				showrest();
				break;
 
			default :
				showln();
				break;
		}
	}
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= dlb_ltr;
/ 	creation	= 1990/05/20;
/ 	function	= deletes the letter backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		dlb_ltr()
{
	register	int	rp_cnt = rp;	/* repeat count */
	register	flag_t	rp_flg = rp > 1;/* repeat flag */
 
	clr_rp();		/* clear repeat count */
	if (rp_flg)
		res_kil();	/* clear kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		/* cannot delete if top of the text */
 
		if (cur_cm == _cm)
			break;
 
		bwd_ltr();
		switch (delletr(rp_flg ? backward : abandon))
		{
			case	0 :	/* end of text */
				return (OK);
 
			case	'\n' : 		/* end of line */
				showrest();
				break;
 
			default :
				showln();
				break;
		}
	}
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= del_lin;
/ 	creation	= 1990/05/20;
/ 	function	= deletes the line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		del_lin()
{
	register	cmcellpt_t	reg2_cm;	/* pointer register */
	register	int		rp_cnt = rp;	/* repeat count */
	flag_t		rp_flg = rp > 1;		/* repeat flag */
 
	clr_rp();		/* clear repeat count */
	res_kil();		/* reset kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		if ((reg2_cm = getnext(cur_cm))) 
		{
			if (getletr(reg2_cm) == '\n')
			{
				delletr(forward);
				if (!rp_flg)
					showrest();
			}
			else
			{
				while ((reg2_cm = getnext(cur_cm)) && getletr(reg2_cm) != '\n')
					delletr(forward);
				if (rp_flg)
					delletr(forward);
				else
					showln();
			}
		}		
		else
			break;
	}
 
	if (rp_flg)
		showrest();
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= dlb_lin;
/ 	creation	= 1990/06/17;
/ 	function	= deletes the line backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		dlb_lin()
{
	register	wchar_t		c;		/* a letter */
	register	int		rp_cnt = rp;	/* repeat count */
	flag_t		rp_flg = rp > 1;		/* repeat flag */
 
	clr_rp();		/* clear repeat count */
	res_kil();		/* reset kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		if ((c = bwd_ltrk(TRUE)))
		{
			if (c == '\n')
			{
				delletr(backward);
				if (!rp_flg)
					show();
			}
			else
			{
				do	delletr(backward);
				while ((c = bwd_ltrk(rp_flg ? TRUE : FALSE)) && c != '\n');
				if (rp_flg && c)
					delletr(backward);
				else
				{
					if (c)
						fwd_ltr();
					showln();
				}
			}
		}		
		else
			break;
	}
 
	if (rp_flg)
		show();
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= del_wrd;
/ 	creation	= 1990/05/20;
/ 	function	= deletes the word;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		del_wrd()
{
	register	wchar_t		c;		/* a letter */
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	int		rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
	res_kil();		/* reset kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		/* if end of line, delete line */
 
		if (getletr(getnext(cur_cm)) == '\n')
		{
			delletr(forward);
			showrest();
			continue;
		}
 
		/* deletes separator */
 
		while ((reg1_cm = getnext(cur_cm)) && !inword(c = getletr(reg1_cm)) && c != '\n')
			delletr(forward);
 
		/* deletes word */
 
		while ((reg1_cm = getnext(cur_cm)) && inword(getletr(reg1_cm)))
			delletr(forward);
 
		showln();
	}
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= dlb_wrd;
/ 	creation	= 1990/05/20;
/ 	function	= deletes the word backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		dlb_wrd()
{
	register	int	rp_cnt = rp;	/* repeat count */
	register	wchar_t	c;		/* a letter */
 
	clr_rp();		/* clear repeat count */
	res_kil();		/* reset kill buffer */
 
	for (; rp_cnt-- > 0;)
	{
		/* cannot delete if top of the text */
 
		if (cur_cm == _cm)
			break;
 
		/* if end of line, delete line */
 
		if (getletr(cur_cm) == '\n')
		{
			bwd_ltr();
			delletr(backward);
			showrest();
			continue;
		}
 
		/* deletes separator */
 
		while (cur_cm != _cm && !inword(c = getletr(cur_cm)) && c != '\n')
		{
			bwd_ltr();
			delletr(backward);
		}
 
		/* deletes word */
 
		while (cur_cm != _cm && inword(getletr(cur_cm)))
		{
			bwd_ltr();
			delletr(backward);
		}
 
		showln();
	}
 
	apnd_kill = FALSE;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= ins_lin;
/ 	creation	= 1990/05/20;
/ 	function	= inserts new line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		ins_lin()
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		if (insletr('\n') == ERROR)
			break;
	}
 
	showrest();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= set_mrk;
/ 	creation	= 1990/05/20;
/ 	function	= sets mark;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		set_mrk()
{
	clr_rp();		/* clear repeat count */
 
	_mark.line = point.line;
	_mark.column = point.column;
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= jmp_mrk;
/ 	creation	= 1990/05/20;
/ 	function	= jumps to mark;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		jmp_mrk()
{
	clr_rp();		/* clear repeat count */
 
	_prev.line = point.line;
	_prev.column = point.column;
	view(&_mark);
 
	return (OK);
}
 
/*
/  specification;
/ 	name	 	= pre_lin;
/ 	creation	= 1990/05/20;
/ 	function	= moves back to the previous line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		pre_lin()
{
	point_t		target;	/* target point */
 
	clr_rp();		/* clear repeat count */
 
	target.line = _prev.line;
	target.column = _prev.column;
	_prev.line = point.line;
	_prev.column = point.column;
 
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= jmp_lin;
/ 	creation	= 1990/05/20;
/ 	function	= jumps to the specified line;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		jmp_lin()
{
	char_t		buf[6];	/* buffer for the string */
	point_t		target;	/* target point */
 
	clr_rp();		/* clear repeat count */
 
	if (rea_min(P_LINE,buf,6) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	_prev.line = point.line;
	_prev.column = point.column;
	target.line = atoi(buf);
	target.column = 1;
	view(&target);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= brk_min;
/ 	creation	= 1990/05/20;
/ 	function	= breaks from mini-buffer;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) not in mini-buffer;
/ 			= ERROR	: (int) break;
*/
 
int		brk_min()
{
	clr_rp();		/* clear repeat count */
 
	return (cur_wdw == edit_wdw ? ERROR : OK);
}
 
/*
/  specification;
/ 	name		= cnt_pnt;
/ 	creation	= 1990/05/20;
/ 	function	= counts current point;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		cnt_pnt()
{
	clr_rp();		/* clear repeat count */
 
	psy_msg(I_POINT,&point);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= cnt_byt;
/ 	creation	= 1990/05/20;
/ 	function	= counts the size of the text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		cnt_byt()
{
	register	unsigned	x;		/* counter */
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	size_inf_t		inf;				/* size information */
	
	for (x = 0, reg1_cm = _cm; (reg1_cm = getnext(reg1_cm)); ++x)
		;
	inf.txt_size = x;
	for (x = 0, reg1_cm = 1; reg1_cm && reg1_cm <= (unsigned)buff_size;
	 reg1_cm++)
	{
		if (!getletr(reg1_cm))
			x++;
	}
	inf.ava_byte = x;
	inf.ava_line = psy_free_lines();
 
	psy_msg(I_CAPACITY,&inf);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= vis_txt;
/ 	creation	= 1990/05/20;
/ 	function	= visits text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		vis_txt()
{
	char_t		path[PATH_SIZE + 1];	/* path name */
	register	flag_t	found = FALSE;		/* if text is found */
	register	bfcell_t	*p;		/* pointer to buffer cell */
 
	clr_rp();		/* clear repeat count */
 
	if ((!strcmp(cur_wdw -> bf_addr -> bf_name,mod_str[M_NEW - M_MSG]) || !strcmp(cur_wdw -> bf_addr -> bf_name,mod_str[M_SHELL - M_MSG])) && getnext(_cm) && cur_wdw -> bf_addr -> bf_cnt <= 1)
		return (OK);
 
	/* read path name */
 
	if (initial_file)
		strcpy(path,initial_file);
	else if (rea_min(P_NAME,path,PATH_SIZE + 1) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	/* search path name */
 
	for (p = edit_buf + buff_no - 1; p > edit_buf; --p)
	{
		if (p -> bf_ln)
		{
			if (!strcmp(path,p -> bf_name))
			{	/* link current window with existing buffer */
				rel_buf(cur_wdw -> bf_addr);
				cur_wdw -> bf_addr = p;
				p -> bf_cnt++;
				found = TRUE;
				break;
			}
		}
	}
 
	if (!found)
	{
		if (!(p = new_buf()))
		{
			psy_msg(E_NOMEM,NULL);
			return (OK);
		}
 
		/* link current window with new buffer */
 
		rel_buf(cur_wdw -> bf_addr);
		cur_wdw -> bf_addr = p;
		p -> bf_cnt++;
	}
 
	cur_wdw -> wd_x_bas = cur_wdw -> wd_y_bas = 1;
	cur_wdw -> wd_point.line = cur_wdw -> wd_mark.line = cur_wdw -> wd_prev.line = 1;
	cur_wdw -> wd_point.column = cur_wdw -> wd_mark.column = cur_wdw -> wd_prev.column = 1;
	loa_wat(cur_wdw);
	cur_ln = _ln;
	cur_cm = _cm;
 
	if (!found)
	{
		load(path,TRUE);	/* load text */
		strcpy(cur_wdw -> bf_addr -> bf_name,path);
	}
 
	shw_wdw(cur_wdw);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= rea_txt;
/ 	creation	= 1990/05/20;
/ 	function	= reads text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		rea_txt()
{
	char_t		path[PATH_SIZE + 1];	/* path name */
 
	clr_rp();		/* clear repeat count */
 
	/* read path name */
 
	if (rea_min(P_READ,path,PATH_SIZE + 1) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	/* load text */
 
	if (load(path,FALSE) == ERROR)
		return (OK);
 
	showrest();
	mod_buf(TRUE);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= ren_txt;
/ 	creation	= 1990/05/20;
/ 	function	= renames text;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		ren_txt()
{
	char_t		path[PATH_SIZE + 1];	/* path name */
 
	clr_rp();		/* clear repeat count */
 
	/* read path name */
 
	if (rea_min(P_RENAM,path,PATH_SIZE + 1) == ERROR)
		return (OK);
 
	/* rename text */
 
	strcpy(cur_wdw -> bf_addr -> bf_name,path);
 
	psy_msg(M_NAME,path);
	mod_buf(TRUE);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= lst_txt;
/ 	creation	= 1990/05/20;
/ 	function	= lists text names;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		lst_txt()
{
	bfcell_t	*p;	/* pointer to buffer cell */
	info_t		inf;	/* text informatoin */
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	register	unsigned		x;		/* buffer size */ 
 
	clr_rp();		/* clear repeat count */
 
	for (p = edit_buf + buff_no - 1; p >= edit_buf; --p)
	{
		if (p -> bf_ln && p != edit_wdw -> bf_addr)
		{
			inf.name = p -> bf_name;
			for (x = 0, reg1_cm = p -> bf_cm; (reg1_cm = getnext(reg1_cm)); ++x)
				;
			inf.size = x;
			inf.modified = p -> bf_modified;
			inf.cnt = p -> bf_cnt;
			if (!rea_conf(I_INFO,&inf))
				break;
		}
	}
 
	return (OK);
}
 
/* end of psye3.c */
