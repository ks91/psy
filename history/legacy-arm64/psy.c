/*
/  source file header;
/ 	name		= psy.c : psy main source file;
/ 	version		= 1.00b;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1991/05/27;
/ 	contents	= main		: (void) main function;
/ 			= initial	: (void) initializer;
/			= custom	: (void) customizer;
/			= terminate	: (void) terminator;
/ 			= psy_msg	: (void) message-output;
*/
 
/* source file id */
 
#define	PSY_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		main(int argc, char **argv);		/* main function */
void		psy_msg();	/* output psy-messages */
 
/* private declaration */
 
/* private functions */
 
static	void	initial();	/* initializer */
static	void	custom();	/* customizer */
static	void	terminate();	/* terminator */
 
/*
/  specification;
/ 	name		= main;
/ 	creation	= 1990/04/30;
/ 	function	= controls top level function;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
int main(int argc, char **argv)
{
	/* credit */
 
	fprintf(stderr,"\
Psy : INU-project new standard screen editor version 1.00c.\n\
Copyleft 1990,1991 by Kenji Psytoe. A public domain software.\n\n");
 
	/* initialization */
 
	initial();
 
	/* start editting */
 
	edit(argc,argv);
 
	/* termination = clear screen */
 
	terminate();
	exit(0);
 
#ifndef	HI_UX
	fprintf(stderr,"\
 This is a dummy string to prevent the string area from being destroyed       \
by the environmental variables' saving area.                                  \
 This is only needed when this program is running on MS-DOS,                  \
well that's a hell of an operating system, isn't it?                          \
 Or is it Let's C compiler?                                                   \
 Let's see.                                                                   \n");
#endif

	return (0);
}
 
/*
/  specification;
/ 	name		= initial;
/ 	creation	= 1990/04/30;
/ 	function	= initializes psy environment;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void	initial()		/* initializer */
{
	int	i;				/* loop counter */
	register	unsigned	x;
	register	char		*p;
 
#ifndef	PTR_32BIT
#	ifndef	TURBO_CPP
	for (x = 65535L; !(p = malloc(x)); x--)
		;
	fprintf(stderr,"Arena:%04x-%04x\n",p,x);
	free(p);
#	endif
#endif
 
	/* call customizer */
 
	custom();
 
	/* memory allocation */
 
	if (!(cellbase = (lncellpt_t)calloc((int)buff_line,sizeof(lncell_t))))	/* area for line cells */
	{
		fprintf(stderr,"psy : insufficient memory. lncell.\n");
		exit(1);
	}
	celltop = cellbase + buff_line;		/* top of cells */
	if (!(ln_stack = (lncellpt_t *)calloc((int)buff_line,sizeof(lncellpt_t))))	/* line stack */
	{
		fprintf(stderr,"psy : insufficient memory. ln_stack.\n");
		exit(1);
	}
	if (!(cm_stack = (cmcellpt_t *)calloc(MAXCOLM,sizeof(cmcellpt_t))))	/* column stack */
	{
		fprintf(stderr,"psy : insufficient memory. cm_stack.\n");
		exit(1);
	}
	if (!(edit_buf = (bfcell_t *)calloc((int)buff_no,sizeof(bfcell_t))))		/* buffer control block */
	{
		fprintf(stderr,"psy : insufficient memory. bfcell.\n");
		exit(1);
	}
	if (!(edit_wdw = (wdcell_t *)calloc(MAX_WNDW_NO,sizeof(wdcell_t))))	/* window control block */
	{
		fprintf(stderr,"psy : insufficient memory. wdcell.\n");
		exit(1);
	}
 
	/* long heap allocation */
 
#ifdef	PTR_32BIT
	if (!(heap = (cmcell_t *)calloc((unsigned)buff_size,sizeof(cmcell_t))))	/* buffer */
	{
		fprintf(stderr,"psy : insufficient memory. heap.\n");
		exit(1);
	}
#else	
#	ifdef	TURBO_CPP
	{
		char	_cs	*cs;
		char _ds	*ds;

		setblkh(cs,ds - cs + 0x1000);
	}
#	else
	setblkh(csreg(),dsreg() - csreg() + 0x1000);	/* ozendate for alloch */
#	endif

	for (i = 0; i <= (unsigned)buff_size / BUFF_UNIT_SIZE; i++)
		if ((heap[i] = alloch(0x1000)) <= 8)		/* insufficient memory */
		{
			fprintf(stderr,"psy : insufficient memory. heap[%d]. code = %d.\n",i,heap[i]);
			exit(1);
		}
		else					/* else 64k bytes allocated */
			_zero(1,heap[i],0xffff);	/* initialize cells */
#endif
 
	/* initialize values */
 
	spln_base = ln_stack - 1;	/* set stack base */
 
	ln_free = cellbase;	/* free line cell pointer */
	cm_free = 1;		/* free column cell pointer */
 
	/* get mini buffer and window ready */ 
 
	edit_wdw[0].wd_ceil = BOTTOM - 1;
	edit_wdw[0].wd_heig = 1;
	edit_wdw[0].wd_tabl = d_tab_len;
	edit_wdw[0].wd_insm = d_ins_mode;
	edit_wdw[0].wd_x_bas = edit_wdw[0].wd_y_bas = 1;
	edit_wdw[0].wd_link = NULL;
	edit_wdw[0].bf_addr = new_buf();
	edit_wdw[0].bf_addr -> bf_cnt++;
	edit_wdw[0].wd_point.line = edit_wdw[0].wd_prev.line = edit_wdw[0].wd_mark.line = 1;
	edit_wdw[0].wd_point.column = edit_wdw[0].wd_prev.column = edit_wdw[0].wd_mark.column = 1;
 
	/* make window list */
 
	edit_wdw[1].wd_ceil = 0;
	edit_wdw[1].wd_heig = BOTTOM - 2;
	edit_wdw[1].wd_tabl = d_tab_len;
	edit_wdw[1].wd_insm = d_ins_mode;
	edit_wdw[1].wd_x_bas = edit_wdw[1].wd_y_bas = 1;
	edit_wdw[1].wd_link = NULL;
	edit_wdw[1].bf_addr = new_buf();
	edit_wdw[1].bf_addr -> bf_cnt++;
	edit_wdw[1].wd_point.line = edit_wdw[1].wd_prev.line = edit_wdw[1].wd_mark.line = 1;
	edit_wdw[1].wd_point.column = edit_wdw[1].wd_prev.column = edit_wdw[1].wd_mark.column = 1;
 
	loa_wat(cur_wdw = wdw_list = edit_wdw + 1);
 
	ini_scr();
	cls();
	dsp_scr();
}
 
/*
/  specification;
/ 	name		= custom;
/ 	creation	= 1990/06/02;
/ 	function	= customizes psy;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void	custom()
{
	char_t	cus_path[PATH_SIZE + 1];	/* custom file path */
	char_t	cus_exp[WIDTH * 4 + 1];		/* custom expression */
	char_t	*dir;			/* custom file directory */
	int	err_cnt = 0;		/* error count */
	register	int	line;	/* line number */
	register	FILE	*fp;	/* file pointer */
	int i; /* to search for '\r' */
 
	/* open custom file */
 
	if (!(dir = (char_t *)getenv(CUS_ENV)))
		cus_path[0] = '\0';
	else
		strcpy(cus_path,dir);
 
#ifndef	TURBO_CPP
	if (!(fp = fopen(strcat(cus_path,CUS_FILE),"r")))
#else
	strcat(cus_path,CUS_FILE);
	if (!(fp = fopen(cus_path,"r")))
#endif
	{
		fprintf(stderr,"psy: cannot open %s.",cus_path);
		exit(1);
	}
 
	fprintf(stderr,"Reading %s...\n",cus_path);
 
	/* read custom file */
 
	for (line = 1; fgets(cus_exp,WIDTH * 4 + 1,fp); line++) {
		if (cus_exp[0] != '/' && cus_psyk(cus_exp) == ERROR)
		{
			fprintf(stderr,"psy: error in %s, line %d.\n",CUS_FILE,line);
			err_cnt++;
		}
	}

	/* close custom file */
 
	fclose(fp);
	if (err_cnt)
		exit(1);
}
 
/*
/  specification;
/ 	name		= terminate;
/ 	creation	= 1990/07/01;
/ 	function	= terminates psy;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void	terminate()
{
	cls();
	ter_scr();
}
 
/*
/  specification;
/ 	name		= psy_msg;
/ 	creation	= 1990/04/30;
/ 	function	= output psy-message;
/ 	parameter (in)	= code		: (int) message code;
/ 			= inf1		: (location_t) added information;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		psy_msg(code,inf1)
register	int		code;
register	location_t	inf1;
{
	/* preparation */
 
	if (code >= E_MSG)
	{
		locate(0,BOTTOM);
		cls_ln();
		if (code < P_CLEAR && attention)	/* if error, ring a bell */
			putchar(BELL);
	}
	else				/* if mode line, reverse */
		reverse();
 
	/* output message according to code */
 
	/* mode line */
 
	if (code == M_MODE)
	{
		locate(0,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		printf("\
Psy:=[=]=[=====]=[==============================================================");
	}
	else if (code == M_NAME)
	{
		locate(18,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		printf("\
==============================================================");
		locate(18,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		printf("%s]",inf1);
	}
	else if (code == M_INSMOD)
	{
		locate(10,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		printf("%s",mod_str[(inf1 != NULL) ? M_INSRT : M_OVWRT]);
	}
	else if (code == M_MODIFY)
	{
		locate(6,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		putchar((inf1 != NULL) ? '*' : ' ');
	}
 
	/* error messages */
 
	else if (code == E_OPNFIL || code == E_CRTFIL)
		printf(err_str[code - E_MSG],inf1);
	else if (code >= E_LARGE && code <= E_CUSTOM)
		printf("%s",err_str[code - E_MSG]);
 
	/* prompt messages */
 
	else if (code == P_CLEAR)
		;
	else if (code == P_SAVE)
		printf(pro_str[P_SAVE - P_MSG],inf1);
	else if (code == P_ARG)
		printf(pro_str[P_ARG - P_MSG],*(int *)inf1);
	else if (code >= P_LINE && code <= P_TABLEN)
		printf("%s",pro_str[code - P_MSG]);
 
	/* information messages */
 
	else if (code == I_POINT)
		printf(inf_str[I_POINT - I_MSG],((point_t *)inf1) -> line,((point_t *)inf1) -> column);
	else if (code == I_SIZE)
		printf(inf_str[I_SIZE - I_MSG],((size_inf_t *)inf1) -> txt_size,((size_inf_t *)inf1) -> ava_byte,((size_inf_t *)inf1) -> ava_line);
	else if (code == I_INFO)
		printf(inf_str[I_INFO - I_MSG],((info_t *)inf1) -> name,((info_t *)inf1) -> size,inf_str[((info_t *)inf1) -> modified ? I_MODYES - I_MSG : I_MODNO - I_MSG],((info_t *)inf1) -> cnt);
 
	/* program state messages */
 
	else if (code == S_LOADIN || code == S_SAVING)
		printf(sta_str[code - S_MSG],inf1);
	else if (code >= S_WAIT && code <= S_RELMEM)
		printf("%s",sta_str[code - S_MSG]);
 
	/* if mode line, restore character attribute */
 
	if (code < E_MSG)
		restore();
 
	/* set displaying message flag */
 
	else if (code >= E_MSG)
		displaying = (code == P_CLEAR ? clear : remaining);
	refresh();
}
 
/* end of psy.c */
