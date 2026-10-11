/*
/  source file header;
/ 	name		= psycus.c : psy customizer source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/06/03;
/ 	contents	= cus_psy	: (int) customizer command;
/ 			= cus_psyk	: (int) customizer command kernel;
/ 			= exe_cmd	: (int) execute command function;
/ 			= get_fnc	: (location_t) get function;
/ 			= cus_buf	: (int) customize buffer;
/ 			= cus_edit	: (int) customize edit parameters;
/ 			= cus_key	: (int) customize key board binding;
/ 			= cus_msg	: (int) customize message;
/ 			= bin_sch	: (location_t) binary search;
/ 			= get_exp	: (char *) get expression class;
/ 			= get_int	: (int) get integer value;
/ 			= get_flg	: (int) get boolean value;
/ 			= get_str	: (char *) get string value;
*/
 
/* source file id */
 
#define	PSYCUS_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		cus_psy();	/* customizer command */
int		cus_psyk();	/* customizer command kernel */
int		exe_cmd();	/* execute command function */
 
/* private declaration */
 
/* private functions */
 
static	location_t	get_fnc();	/* get function */
static	int		cus_edit();	/* customize edit parameters */
static	int		cus_buf();	/* customize buffer */
static	int		cus_key();	/* customize key board binding */
static	int		cus_msg();	/* customize message */
static	location_t	bin_sch();	/* binary search */
static	char_t		*get_exp();	/* get expression class */
static	int		get_int();	/* get integer value */
static	int		get_flg();	/* get boolean value */
static	char_t		*get_str();	/* get string value */
 
/*
/  specification;
/ 	name		= cus_psy;
/ 	creation	= 1990/06/03;
/ 	function	= customizes psy;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		cus_psy()
{
	char_t	cus_exp[WIDTH * 4 + 1];	/* customizer expression */
 
	clr_rp();			/* clear repeat count */
 
	if (rea_min(P_CUSTOM,cus_exp,WIDTH * 4 + 1) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	if (cus_psyk(cus_exp))
		psy_msg(E_CUSTOM,NULL);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= cus_psyk;
/ 	creation	= 1990/06/03;
/ 	function	= customizes psy;
/ 	parameter (in)	= s	: (char_t *) customizer expression;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		cus_psyk(s)
char_t		*s;
{
	/* customizer expression class table */
 
	static	struct	cusclass
	{
		char	*str;
		int	(*fnc)();
	} cusclass_a[] =
	{
		{"buf",cus_buf},
		{"edit",cus_edit},
		{"key",cus_key},
		{"msg",cus_msg}
	};
 
	register	char_t	*class;	/* customizer expression class */
	flag_t		cont;		/* expression continue flag */
	register	int	(*fnc)();	/* customizer class function */
 
	/* check if null line */
	while (isspace(*s))
		s++;
	if (!*s)
		return (OK);

	/* get expression class */
 
	if (!(class = get_exp(&cont,&s)) || !cont) {
	  fprintf(stderr, "get_exp() failed.\n");
		return (ERROR);
	}

	/* call customizer class function */

	return (!(fnc = (int (*)())bin_sch(class,cusclass_a,sizeof(cusclass_a))) ? ERROR : (*fnc)(s));
}
 
/*
/  specification;
/ 	name		= exe_cmd;
/ 	creation	= 1990/06/03;
/ 	function	= execute command function;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
*/
 
int		exe_cmd()
{
	char_t		com[PATH_SIZE + 1];	/* command name */
	register	int	(*fnc)();	/* command function */
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();	/* clear repeat count */
 
	if (rea_min(P_COMMAND,com,PATH_SIZE + 1) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	if (!(fnc = (int (*)())get_fnc(com)))
	{
		psy_msg(E_COMMAND,NULL);
		return (OK);
	}
	else
	{
		rp = rp_cnt;
		return ((*fnc)());
	}
}
 
/*
/  specification;
/ 	name		= get_fnc;
/ 	creation	= 1990/06/03;
/ 	function	= gets command function;
/ 	parameter (in)	= s	: (char_t *) command function string;
/ 	parameter (out)	= none;
/ 	return		= NULL		: error;
/ 			= otherwise	: (location_t) pointer to the function;
*/
 
static	location_t	get_fnc(s)
char_t	*s;
{
	/* function name table */
 
	static	struct	fncnam
	{
		char	*str;
		int	(*fnc)();
	}	func_a[] =
	{
		{"abo_txt",abo_txt},
		{"apd_kil",apd_kil},
		{"bgn_blk",bgn_blk},
		{"brk_min",brk_min},
		{"btm_lin",btm_lin},
		{"btm_txt",btm_txt},
		{"btm_wdw",btm_wdw},
		{"bwd_lin",bwd_lin},
		{"bwd_ltr",bwd_ltr},
		{"bwd_wdw",bwd_wdw},
		{"bwd_wrd",bwd_wrd},
		{"chg_ins",chg_ins},
		{"cls_wdw",cls_wdw},
		{"cnt_byt",cnt_byt},
		{"cnt_pnt",cnt_pnt},
		{"cpy_blk",cpy_blk},
		{"cus_psy",cus_psy},
		{"del_blk",del_blk},
		{"del_lin",del_lin},
		{"del_ltr",del_ltr},
		{"del_wrd",del_wrd},
		{"dlb_lin",dlb_lin},
		{"dlb_ltr",dlb_ltr},
		{"dlb_wrd",dlb_wrd},
		{"dsp_scr",dsp_scr},
		{"end_blk",end_blk},
		{"enl_wdw",enl_wdw},
		{"ens_wdw",ens_wdw},
		{"exe_cmd",exe_cmd},
		{"exe_ect",exe_ect},
		{"exe_emt",exe_emt},
		{"exe_met",exe_met},
		{"exe_shc",exe_shc},
		{"ext_psy",ext_psy},
		{"fwd_lin",fwd_lin},
		{"fwd_ltr",fwd_ltr},
		{"fwd_tab",fwd_tab},
		{"fwd_wdw",fwd_wdw},
		{"fwd_wrd",fwd_wrd},
		{"ins_lin",ins_lin},
		{"jmp_lin",jmp_lin},
		{"jmp_mrk",jmp_mrk},
		{"lft_wdw",lft_wdw},
		{"lst_txt",lst_txt},
		{"nam_exp",nam_exp},
		{"new_lin",new_lin},
		{"nop",nop},
		{"opn_shw",opn_shw},
		{"opn_wdw",opn_wdw},
		{"pre_lin",pre_lin},
		{"rea_txt",rea_txt},
		{"rel_blk",rel_blk},
		{"ren_txt",ren_txt},
		{"rht_wdw",rht_wdw},
		{"rpt_cmd",rpt_cmd},
		{"sat_exp",sat_exp},
		{"sav_txt",sav_txt},
		{"scb_exp",scb_exp},
		{"sch_exp",sch_exp},
		{"sel_wdw",sel_wdw},
		{"set_mrk",set_mrk},
		{"set_tab",set_tab},
		{"slb_wdw",slb_wdw},
		{"stb_exp",stb_exp},
		{"ter_psy",ter_psy},
		{"top_lin",top_lin},
		{"top_txt",top_txt},
		{"top_wdw",top_wdw},
		{"tra_exp",tra_exp},
		{"tra_lin",tra_lin},
		{"tra_ltr",tra_ltr},
		{"tra_wrd",tra_wrd},
		{"vis_txt",vis_txt},
		{"wri_blk",wri_blk},
		{"ynk_kil",ynk_kil}
	};
 
	return ((location_t)bin_sch(s,func_a,sizeof(func_a)));
}
 
/*
/  specification;
/ 	name		= cus_buf;
/ 	creation	= 1990/06/09;
/ 	function	= scans buffer customizer expression;
/ 	parameter (in)	= s	: (char_t *) customizer expression;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
static	int	cus_buf(s)
char_t	*s;
{
	flag_t		cont;			/* expression continue flag */
	register	char_t	*bufclass;	/* expression class */
	register	ret = OK;		/* return code */
 
	/* get expression class */
 
	if (!(bufclass = get_exp(&cont,&s)) || cont)
		return (ERROR);

	/* get detail */
 
	if (!strcmp(bufclass,"size"))
	{
		if (buff_size)
			ret = ERROR;
		else
		{
			ret = get_int(&buff_size,s);
			if (buff_size > MAX_BUFF_SIZE)
				ret = ERROR;
		}
	}
	else if (!strcmp(bufclass,"mult"))
	{
		if (buff_no)
			ret = ERROR;
		else
		{
			ret = get_int(&buff_no,s);
			if (buff_no > MAX_BUFF_NO)
				ret = ERROR;
		}
	}
	else if (!strcmp(bufclass,"line"))
	{
		if (buff_line)
			ret = ERROR;
		else
		{
			ret = get_int(&buff_line,s);
			if (buff_line > MAX_BUFF_LINE)
				ret = ERROR;
		}
	}
	else if (!strcmp(bufclass,"mini"))
	{
		if (mini_his)
			ret = ERROR;
		else
		{
			ret = get_int(&mini_his,s);
			if (mini_his > MAX_MINI_HIS)
				ret = ERROR;
		}
	}
	else {
		ret = ERROR;
	}
	return (ret);
}
 
/*
/  specification;
/ 	name		= cus_edit;
/ 	creation	= 1990/06/20;
/ 	function	= scans edit parameter customizer expression;
/ 	parameter (in)	= s	: (char_t *) customizer expression;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
static	int	cus_edit(s)
char_t	*s;
{
	flag_t		cont;			/* expression continue flag */
	char_t		*file;			/* file name */
	register	char_t	*editclass;	/* expression class */
	register	ret = OK;		/* return code */
 
	/* get expression class */
 
	if (!(editclass = get_exp(&cont,&s)) || cont)
		return (ERROR);
 
	/* get detail */
 
	if (!strcmp(editclass,"d_ins_mode"))
		ret = get_flg(&d_ins_mode,s);
	else if (!strcmp(editclass,"d_tab_len"))
	{
		ret = get_int(&d_tab_len,s);
		if (d_tab_len > MAX_TAB_LENGTH)
			ret = ERROR;
	}
	else if (!strcmp(editclass,"rp_unit"))
		ret = get_int(&rp_unit,s);
	else if (!strcmp(editclass,"stdout_file"))
	{
		if (!(file = get_str(s)))
			ret = ERROR;
		else
		{
			if (stdout_file)
				free(stdout_file);
			stdout_file = file;
		}
	}
	else
		ret = ERROR;
 
	return (ret);
}
 
/*
/  specification;
/ 	name		= cus_key;
/ 	creation	= 1990/06/03;
/ 	function	= scans key binding customizer expression;
/ 	parameter (in)	= s	: (char_t *) customizer expression;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
static	int	cus_key(s)
char_t	*s;
{
	/* key board customizer expression table */
 
	static	struct	cuskey
	{
		char	*str;
		int	(**keytab)();
	} cuskey_a[] =
	{
		{"ctrl",ctrl},
		{"e_ctrl",e_ctrl},
		{"e_meta",e_meta},
		{"meta",meta}
	};
 
	register	char_t	*keyclass;	/* expression class */
	register	char_t	*func;		/* command function */
	flag_t	cont;		/* expression continue flag */
	location_t	*keytab;	/* key board binding table */
	location_t	fnc;		/* pointer to function */
 
	/* get expression class */
 
	if (!(keyclass = get_exp(&cont,&s)) || !cont)
		return (ERROR);
 
	/* get keyclass table */
 
	if (!(keytab = (location_t *)bin_sch(keyclass,cuskey_a,sizeof(cuskey_a))))
		return (ERROR);
 
	/* get key */
 
	if (!(keyclass = get_exp(&cont,&s)) || cont || strlen(keyclass) != 1 || !islower(*keyclass))
		return (ERROR);
 
	/* get function */
 
	if (!(func = get_str(s)) || !(fnc = get_fnc(func)))
		return (ERROR);
 
	keytab[*keyclass - 'a'] = fnc;
	return (OK);
}
 
/*
/  specification;
/ 	name		= cus_msg;
/ 	creation	= 1990/06/03;
/ 	function	= scans message customizer expression;
/ 	parameter (in)	= s	: (char_t *) customizer expression;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
static	int	cus_msg(s)
char_t	*s;
{
	/* message customizer expression table */
 
	struct	cusmsg
	{
		char	*str;
		char_t	**msg;
	};
 
	static	struct	cusmsg	error_a[] =
	{
		{"block",&err_str[E_BLOCK - E_MSG]},
		{"busy",&err_str[E_BUSY - E_MSG]},
		{"command",&err_str[E_COMMAND - E_MSG]},
		{"crtfil",&err_str[E_CRTFIL - E_MSG]},
		{"custom",&err_str[E_CUSTOM - E_MSG]},
		{"large",&err_str[E_LARGE - E_MSG]},
		{"nomem",&err_str[E_NOMEM - E_MSG]},
		{"nomore",&err_str[E_NOMORE - E_MSG]},
		{"opnfil",&err_str[E_OPNFIL - E_MSG]},
		{"regexp",&err_str[E_REGEXP - E_MSG]},
		{"tablen",&err_str[E_TABLEN - E_MSG]}
	};
 
	static	struct	cusmsg	info_a[] =
	{
		{"info",&inf_str[I_INFO - I_MSG]},		
		{"modno",&inf_str[I_MODNO - I_MSG]},
		{"modyes",&inf_str[I_MODYES - I_MSG]},
		{"point",&inf_str[I_POINT - I_MSG]},
		{"size",&inf_str[I_SIZE - I_MSG]}
	};
 
	static	struct	cusmsg	mode_a[] =
	{
		{"insrt",&mod_str[M_INSRT - M_MSG]},
		{"new",&mod_str[M_NEW - M_MSG]},
		{"ovwrt",&mod_str[M_OVWRT - M_MSG]},
		{"shell",&mod_str[M_SHELL - M_MSG]}
	};
 
	static	struct	cusmsg	prompt_a[] =
	{
		{"abort",&pro_str[P_ABORT - P_MSG]},
		{"arg",&pro_str[P_ARG - P_MSG]},
		{"command",&pro_str[P_COMMAND - P_MSG]},
		{"custom",&pro_str[P_CUSTOM - P_MSG]},
		{"line",&pro_str[P_LINE - P_MSG]},
		{"name",&pro_str[P_NAME - P_MSG]},
		{"new",&pro_str[P_NEW - P_MSG]},
		{"ovwrt",&pro_str[P_OVWRT - P_MSG]},
		{"read",&pro_str[P_READ - P_MSG]},
		{"renam",&pro_str[P_RENAM - P_MSG]},
		{"save",&pro_str[P_SAVE - P_MSG]},
		{"search",&pro_str[P_SEARCH - P_MSG]},
		{"shell",&pro_str[P_SHELL - P_MSG]},
		{"tablen",&pro_str[P_TABLEN - P_MSG]},
		{"write",&pro_str[P_WRITE - P_MSG]}
	};
 
	static	struct	cusmsg	state_a[] =
	{
		{"loading",&sta_str[S_LOADIN - S_MSG]},
		{"relmem",&sta_str[S_RELMEM - S_MSG]},
		{"saving",&sta_str[S_SAVING - S_MSG]},
		{"sching",&sta_str[S_SCHING - S_MSG]},
		{"wait",&sta_str[S_WAIT - S_MSG]}
	};
 
	static	struct	cuscon
	{
		char	*str;
		regular_t	**_reg;
	}	conf_a[] =
	{
		{"no",&conf_no},
		{"yes",&conf_yes}
	};
 
	register	char_t	*msgclass;	/* message class string */
	register	char_t	**msgstr;	/* pointer to message string */
	regular_t	**_reg;			/* pointer to regular expression */
	flag_t		cont;			/* expression continue flag */
	char_t		*msg;			/* pointer to message */
 
	/* get expression class */
 
	if (!(msgclass = get_exp(&cont,&s)))
		return (ERROR);
 
	/* get detail */
 
	if (!strcmp(msgclass,"confirm"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(_reg = (regular_t **)bin_sch(msgclass,conf_a,sizeof(conf_a))))
			return (ERROR);
		if (!(msg = get_str(s)))
			return (ERROR);
		if (!(*_reg = parse(msg)))
			return (ERROR);
		return (OK);
	}
	else if (!strcmp(msgclass,"error"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(msgstr = (char_t **)bin_sch(msgclass,error_a,sizeof(error_a))))
			return (ERROR);
	}
	else if (!strcmp(msgclass,"info"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(msgstr = (char_t **)bin_sch(msgclass,info_a,sizeof(info_a))))
			return (ERROR);
	}
	else if (!strcmp(msgclass,"mode"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(msgstr = (char_t **)bin_sch(msgclass,mode_a,sizeof(mode_a))))
			return (ERROR);
	}
	else if (!strcmp(msgclass,"prompt"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(msgstr = (char_t **)bin_sch(msgclass,prompt_a,sizeof(prompt_a))))
			return (ERROR);
	}
	else if (!strcmp(msgclass,"state"))
	{
		if (!cont || !(msgclass = get_exp(&cont,&s)) || cont)
			return (ERROR);
		if (!(msgstr = (char_t **)bin_sch(msgclass,state_a,sizeof(state_a))))
			return (ERROR);
	}
	else if (!strcmp(msgclass,"beep"))
	{
		if (cont)
			return (ERROR);
		return (get_flg(&attention,s));
	}
	else
		return (ERROR);
 
	/* get message string */
 
	if (!(msg = get_str(s)))
		return (ERROR);
 
	*msgstr = msg;
	return (OK);
}
 
/*
/  specification;
/ 	name		= bin_sch;
/ 	creation	= 1990/06/03;
/ 	function	= searches table by binary method;
/ 	parameter (in)	= s	: (char_t *) string to search;
/ 			= arry	: (cusbin_t []) array;
/ 			= size	: (int) size of array;
/ 	parameter (out)	= none;
/ 	return		= NULL		: cannot find;
/ 			= otherwise	: (location_t) a pointer to an information;
*/
 
static	location_t	bin_sch(s,arry,size)
char_t		*s;
cusbin_t	arry[];
int		size;
{
	register	int	index;	/* index */
	register	int	min = 0;			/* minimun */
	int		max = size / sizeof(cusbin_t) - 1;	/* maximum */
	int		result;		/* result of strcmp */

	while ((index = (max + min) / 2),min <= max)
	{

		if (!(result = strcmp(s,arry[index].str)))
			return (arry[index].inf);	/* found */

		if (result > 0)
			min = index + 1;
		else
			max = index - 1;

	}

	return (NULL);		/* not found */
}
 
/*
/  specification;
/ 	name		= get_exp;
/ 	creation	= 1990/06/03;
/ 	function	= gets next customizer expression;
/ 	parameter (in)	= s	: (char_t **) start point;
/ 	parameter (out)	= cont	: (flag_t *) if more;
/ 			= s	: (char_t **) stop point;
/ 	return		= NULL		: error in expression;
/ 			= otherwise	: (char_t *) expression;
*/
 
static	char_t	*get_exp(cont,s)
flag_t		*cont;
register	char_t		**s;
{
	register	char_t	*start;
	char_t	*stop;
 
	while (isspace(**s))	/* skip space */
		(*s)++;
	for (start = *s; !isspace(**s) && **s != '.' && **s != '='; (*s)++)
		if (!**s)
			return (NULL);
	stop = *s;
	while (isspace(**s))	/* skip space */
		(*s)++;
	if (**s == '.')
		*cont = TRUE;
	else if (**s == '=')
		*cont = FALSE;
	else
		return (NULL);
	*stop = '\0';
	(*s)++;
	return (start);
}
 
/*
/  specification;
/ 	name		= get_int;
/ 	creation	= 1990/06/03;
/ 	function	= gets integer value;
/ 	parameter (in)	= s	: (char_t *) start point;
/ 	parameter (out)	= value	: (long *) value;
/ 	return		= OK	: succeeded;
/ 			= ERROR	: failed;
*/
 
static	int	get_int(value,s)
long	*value;
char_t	*s;
{
	char_t	*number;	/* digit string */
 
	while (isspace(*s))	/* skip space */
		s++;
 
	for (number = s; !isspace(*s) && *s != ';'; s++)
		if (!*s)
			return (ERROR);

	*s = '\0';
	*value = atol(number);

	return (OK);
}
 
/*
/  specification;
/ 	name		= get_flg;
/ 	creation	= 1990/06/03;
/ 	function	= gets boolean value;
/ 	parameter (in)	= s	: (char *) start point;
/ 	parameter (out)	= value	: (flag_t *) value;
/ 	return		= OK	: succeeded;
/ 			= ERROR	: failed;
*/
 
static	int	get_flg(value,s)
flag_t	*value;
char_t	*s;
{
	char_t	*flag;		/* boolean string */
 
	while (isspace(*s))	/* skip space */
		s++;
 
	for (flag = s; !isspace(*s) && *s != ';'; s++)
		if (!*s)
			return (ERROR);

	*s = '\0';
	if (!strcmp(flag,"yes"))
		*value = TRUE;
	else if (!strcmp(flag,"no"))
		*value = FALSE;
	else
		return (ERROR);
	return (OK);
}
 
/*
/  specification;
/ 	name		= get_str;
/ 	creation	= 1990/06/03;
/ 	function	= gets string value;
/ 	parameter (in)	= s	: (char_t *) start point;
/ 	parameter (out)	= none;
/ 	return		= NULL		: error;
/ 			= otherwise	: (char_t *) string value;
*/
 
static	char_t	*get_str(s)
register	char_t	*s;
{
	char_t	msg[WIDTH + 1];		/* message string */
	register	char_t	*msg_p = msg;	/* pointer to message */
 
	while (isspace(*s))	/* skip space */
		s++;
	if (*s != '"')
	{
		while (*s != ';')
			if (!(*msg_p++ = *s++))
				return (NULL);
	}
	else
	{
		while (*++s != '"')
			switch (*s)
			{
				case '\0' :
					return (NULL);
 
				case '\\' :
					s++;
				default :
					if (iskanji(*s))
						*msg_p++ = *s++;
					*msg_p++ = *s;
					break;
			}
	}
	*msg_p = '\0';
 
	if (!(msg_p = (char_t *)malloc(strlen(msg) + 1)))
		return (NULL);
	return ((char_t *)strcpy(msg_p,msg));
}
 
/* end of psycus.c */
