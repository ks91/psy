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
	int arg = 1;
	setlocale(LC_CTYPE, "");
	if (MB_CUR_MAX == 1) setlocale(LC_CTYPE, "en_US.UTF-8");
	while (arg < argc && !strncmp(argv[arg], "--", 2)) {
		char *value;
		int *setting;
		if (!strcmp(argv[arg], "--")) { arg++; break; }
		if (!strcmp(argv[arg], "--help")) {
			printf("usage: psy [--read-encoding=auto|utf-8|shift-jis] "
			       "[--write-encoding=utf-8|shift-jis] [--] [file]\n");
			return 0;
		}
		if (!strncmp(argv[arg], "--read-encoding=", 16)) {
			value = argv[arg] + 16; setting = &psy_read_encoding;
		} else if (!strncmp(argv[arg], "--write-encoding=", 17)) {
			value = argv[arg] + 17; setting = &psy_encoding;
		} else {
			fprintf(stderr, "psy: unknown option: %s\n", argv[arg]); return 1;
		}
		if (!strcmp(value, "utf-8")) *setting = PSY_UTF8;
		else if (!strcmp(value, "shift-jis")) *setting = PSY_SJIS;
		else if (setting == &psy_read_encoding && !strcmp(value, "auto"))
			*setting = PSY_AUTO;
		else { fprintf(stderr, "psy: invalid encoding: %s\n", value); return 1; }
		arg++;
	}
	if (argc - arg > 1 || (arg < argc && strlen(argv[arg]) >= PATH_SIZE)) {
		fprintf(stderr, "psy: expected at most one file, with path shorter than %d bytes.\n", PATH_SIZE);
		return 1;
	}
	/* credit */
 
	fprintf(stderr,"\
Psy : Unicode / large-buffer edition (based on version 1.00c).\n\
Copyleft 1990,1991 by Kenji Psytoe. A public domain software.\n\n");
 
	/* initialization */
 
	initial();
 
	/* start editting */
 
	edit(argc - arg + 1, argv + arg - 1);
 
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
 
	/* Pools grow as needed; the config sizes are initial capacities. */
	if (!psy_init_memory() ||
	    !(edit_buf = calloc((size_t)buff_no, sizeof(bfcell_t))) ||
	    !(edit_wdw = calloc(MAX_WNDW_NO, sizeof(wdcell_t)))) {
		fprintf(stderr, "psy: insufficient memory.\n");
		exit(1);
	}

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
	char_t	cus_exp[4096];		/* custom expression */
	char_t	*dir;			/* custom file directory */
	int	err_cnt = 0;		/* error count */
	register	int	line;	/* line number */
	register	FILE	*fp;	/* file pointer */
	int i; /* to search for '\r' */
 
	/* open custom file */
 
	dir = (char_t *)getenv(CUS_ENV);
	if (snprintf((char *)cus_path, sizeof(cus_path), "%s%s",
	             dir ? (char *)dir : "", CUS_FILE) >= sizeof(cus_path)) {
		fprintf(stderr, "psy: configuration path is too long.\n");
		exit(1);
	}
	if (!(fp = fopen((char *)cus_path, "rb")))
	{
		fprintf(stderr,"psy: cannot open %s.",cus_path);
		exit(1);
	}
 
	fprintf(stderr,"Reading %s...\n",cus_path);
 
	fp = psy_config_utf8(fp);
	if (!fp) {
		fprintf(stderr, "psy: invalid configuration encoding.\n");
		exit(1);
	}
	/* read custom file */
 
	for (line = 1; fgets(cus_exp,sizeof(cus_exp),fp); line++) {
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
	if (psy_screen_small()) return;
 
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
		psy_mode_bar();
	else if (code == M_NAME)
		psy_mode_name(inf1);
	else if (code == M_INSMOD)
	{
		locate(10,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		psy_print_clipped((char *)mod_str[(inf1 != NULL) ? M_INSRT : M_OVWRT], 5);
	}
	else if (code == M_MODIFY)
	{
		locate(6,cur_wdw -> wd_ceil + cur_wdw -> wd_heig);
		putchar((inf1 != NULL) ? '*' : ' ');
	}
 
	/* error messages */
 
	else if (code == E_OPNFIL || code == E_CRTFIL)
		psy_statusf((const char *)err_str[code - E_MSG],inf1);
	else if (code >= E_LARGE && code <= E_CUSTOM)
		psy_statusf((const char *)"%s",err_str[code - E_MSG]);
 
	/* prompt messages */
 
	else if (code == P_CLEAR)
		psy_statusf("");
	else if (code == P_SAVE)
		psy_statusf((const char *)pro_str[P_SAVE - P_MSG],inf1);
	else if (code == P_ARG)
		psy_statusf((const char *)pro_str[P_ARG - P_MSG],*(int *)inf1);
	else if (code >= P_LINE && code <= P_TABLEN)
		psy_statusf((const char *)"%s",pro_str[code - P_MSG]);
 
	/* information messages */
 
	else if (code == I_POINT)
		psy_statusf((const char *)inf_str[I_POINT - I_MSG],((point_t *)inf1) -> line,((point_t *)inf1) -> column);
	else if (code == I_SIZE)
		psy_statusf((const char *)inf_str[I_SIZE - I_MSG],((size_inf_t *)inf1) -> txt_size,((size_inf_t *)inf1) -> ava_byte,((size_inf_t *)inf1) -> ava_line);
	else if (code == I_CAPACITY)
		psy_statusf(inf_str[I_CAPACITY - I_MSG] ?
		 (const char *)inf_str[I_CAPACITY - I_MSG] :
		 "自動拡張 | %ld文字 | 確保済み空き %ld文字/%ld行",
		 ((size_inf_t *)inf1) -> txt_size,
		 ((size_inf_t *)inf1) -> ava_byte,
		 ((size_inf_t *)inf1) -> ava_line);
	else if (code == I_INFO)
		psy_statusf((const char *)inf_str[I_INFO - I_MSG],((info_t *)inf1) -> name,((info_t *)inf1) -> size,inf_str[((info_t *)inf1) -> modified ? I_MODYES - I_MSG : I_MODNO - I_MSG],((info_t *)inf1) -> cnt);
 
	/* program state messages */
 
	else if (code == S_LOADIN || code == S_SAVING)
		psy_statusf((const char *)sta_str[code - S_MSG],inf1);
	else if (code >= S_WAIT && code <= S_RELMEM)
		psy_statusf((const char *)"%s",sta_str[code - S_MSG]);
 
	/* if mode line, restore character attribute */
 
	if (code < E_MSG)
		restore();
 
	/* set displaying message flag */
 
	else if (code >= E_MSG)
		displaying = (code == P_CLEAR ? clear : remaining);
	refresh();
}
 
/* end of psy.c */
