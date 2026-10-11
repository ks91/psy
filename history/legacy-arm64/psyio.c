/* Updated version: 2001/10/14 to support multiple OS new-line code systems */
/*
/  source file header;
/ 	name		= psyio.c : psy I/O functions source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/01;
/ 	contents	= load		: (int) load file;
/ 			= save		: (int) save file;
/ 			= makebak	: (void) make back up path name;
/ 			= rea_min	: (int) read mini-buffer;
/ 			= rea_conf	: (flag_t) read confirmation;
/ 			= printf	: (int) printf;
*/
 
/* source file id */
 
#define	PSYIO_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		load();		/* load file */
int		save();		/* save file */
void		makebak();	/* make back up path name */
int		rea_min();	/* read mini-buffer */
flag_t		rea_conf();	/* read confirmation */
#ifndef	UNIX_STDIO
int		printf();	/* printf */
#endif
 
/*
/  specification;
/ 	name		= load;
/ 	creation	= 1990/05/01;
/ 	function	= loads file;
/ 	parameter (in)	= path	: (char_t *) load path;
/			= new	: (flag_t) whether to accept new file;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		load(path,new)
char_t		*path;
flag_t		new;
{
	register	cmcellpt_t	reg1_cm,reg2_cm;	/* pointer registers */
	lncellpt_t	reg1_ln,reg2_ln;			/* pointer registers */ 
	wchar_t		c;		/* a letter */
	int             x;              /* a letter buffer */
	FILE		*fp;		/* file pointer */
	int		ret = OK;	/* return code */
 
	/* open file */
 
	psy_msg(S_WAIT,NULL);
 
	if (!(fp = fopen(path,"r")))
	{
		psy_msg(new ? P_NEW : E_OPNFIL,path);		/* cannot open */
		return (ERROR);
	}
 
	/* load file */

	psy_msg(S_LOADIN,path);
 
	reg1_cm = cur_cm;
	reg2_cm = getnext(cur_cm);
	reg1_ln = cur_ln;
	reg2_ln = cur_ln -> link;
 
	while ((x = getwc(fp)) != EOF)
	{
		/* deals with different new-line code systems */
		if (x == '\r') {
			/* DOS/Windows */
			if ((x = getwc(fp)) != EOF && x == '\n') {
				
			/* Macintosh */
			} else {
				ungetwc(x, fp);
				x = '\n';
			}
		}

	        c = x;
		/* allocate new column cell */
 
		if (!putnext(reg1_cm,new_cm()))
		{
			/* allocation failed */
			ret = ERROR;
			break;
		}
 
		/* if new line, insert line */
		if (putletr(reg1_cm = getnext(reg1_cm),c) == '\n')
		{
			/* allocate new line cell */
 
			if (!(reg1_ln -> link = new_ln()))
			{
				/* allocation failed */
				ret = ERROR;
				break;
			}
 
			(reg1_ln = reg1_ln -> link) -> addr = reg1_cm;
		}
	}
 
	putnext(reg1_cm,reg2_cm);
	reg1_ln -> link = reg2_ln;
 
	/* close file */
 
	fclose(fp);
 
	psy_msg(ret ? E_LARGE : P_CLEAR,NULL);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= save;
/ 	creation	= 1990/05/01;
/ 	function	= saves file;
/ 	parameter (in)	= path	: (char_t *) save path;
/ 			= first	: (cmcellpt_t) letter;
/ 			= last	: (cmcellpt_t) last letter;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		save(path,first,last)
char_t		*path;
register	cmcellpt_t	first;
register	cmcellpt_t	last;
{
	FILE		*fp;		/* file pointer */
 
	/* create file */
 
	psy_msg(S_WAIT,NULL);
 
	if (!(fp = fopen(path,"w")))
	{
		psy_msg(E_CRTFIL,path);		/* cannot create */ 
		return (ERROR);
	}
 
	/* save file */
 
	psy_msg(S_SAVING,path);
 
#ifdef	JIS
	jis_cont = TRUE;
#endif
	while ((first = getnext(first)) != last)
		if (putwc(getletr(first),fp) == EOF)
		{
			psy_msg(E_CRTFIL,path);
			return (ERROR);
		}
#ifdef	JIS
	jis_cont = FALSE;
	if (jis_kanji)
	{
		jis_kanji = FALSE;
		putc(0x1b,fp);
		putc(0x28,fp);
		putc(0x4a,fp);
	}
#endif
 
	/* close file */
 
	if (fclose(fp))
	{
		psy_msg(E_CRTFIL,path);		/* cannot create */ 
		return (ERROR);
	}

	psy_msg(P_CLEAR,NULL);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= makebak;
/ 	creation	= 1990/05/01;
/ 	function	= makes back up path name;
/ 	parameter (in)	= editpath	: (char_t *) path to update;
/ 	parameter (out)	= subpath	: (char_t *) back up path;
/ 	return		= none (void);
*/
 
void		makebak(editpath,subpath)
register	char_t	*editpath,*subpath;
{
	char_t	*ext = NULL;	/* pointer to the extension */
 
	/* make back up path */
 
	while (*editpath)
	{
		/* get the beginning of the extension */
 
		switch (*editpath)
		{
			case	'.' :	/* extension separator */
				ext = subpath;
				break;
 
			case	'/' :	/* directory separator */
			case	'\\' :
				ext = NULL;
				break;
 
			default :
				break;
		}
 
		/* if kanji, step forward */
 
		if (iskanji(*editpath))
			*subpath++ = *editpath++;
		*subpath++ = *editpath++;
	}
 
	/* set an extension for the back up path */
 
	strcpy(ext ? ext : subpath,BACKUP);
}
 
/*
/  specification;
/ 	name		= rea_min;
/ 	creation	= 1990/05/02;
/ 	function	= reads mini-buffer;
/ 	parameter (in)	= code	: (int) information code;
/ 			= len	: (int) length of the string;
/ 	parameter (out)	= s	: (char_t *) string to store;
/ 	return		= OK	: (int) succeeded;
/ 			= ERROR	: (int) failed;
*/
 
int		rea_min(code,s,len)
int		code;
char_t		*s;
int		len;
{
	register	cmcellpt_t	reg1_cm;	/* pointer register */
	lncellpt_t			reg1_ln;		/* pointer register */
	register	wchar_t		c;		/* a letter */
	char_t		first_byte;	/* first byte of the letter */
	wdcell_t	*previous;	/* previous window */
	int		ret;		/* return code */
 
	/* check if already in mini-buffer */
 
	if (cur_wdw == edit_wdw)
		return (ERROR);		/* duplication */
 
	/* switch to mini-window */
 
	sav_wat(previous = cur_wdw);
	loa_wat(cur_wdw = edit_wdw);
	enstack();
 
	/* print prompt */
 
	psy_msg(code,NULL);
 
	/* edit mini-buffer */
 
	ret = edit(0, NULL);
 
	/* copy mini-buffer to string */
 
	if (ret != ERROR)
	{
		for (reg1_cm = cur_ln -> addr; (reg1_cm = getnext(reg1_cm)) && (c = getletr(reg1_cm)) != '\n' && --len > 0;)
		{
			if ((first_byte = c >> CHAR_BIT))
				*s++ = first_byte;
			*s++ = c & CHAR_MASK;
		}
 
		*s++ = '\0';
	}
 
	/* append a line */
 
	if (!(cur_ln -> link))
	{
		while ((reg1_cm = getnext(cur_cm)))
			cur_cm = reg1_cm;
		insletr('\n');
	}
 
	/* remove first line in the case of overflow */
 
	if (point.line > mini_his)
	{
		release(_ln -> link,reg1_ln = _ln -> link -> link,getnext(_cm),reg1_cm = getnext(_ln -> link -> addr),abandon);
		_ln -> link = reg1_ln;
		putnext(_cm,reg1_cm);
		point.line--;
		y_bas--;
	}
 
	x_bas = point.column = 1;
	while (cur_ln -> link)
	{
		cur_ln = cur_ln -> link;
		point.line++;
		y_bas++;
	}
 
	sav_wat(cur_wdw);
	shw_wdw(cur_wdw);
 
	/* restore window */
 
	loa_wat(cur_wdw = previous);
	enstack();
 
	psy_msg(P_CLEAR,NULL);
 
	return (ret != ERROR ? OK : ERROR);
}
 
/*
/  specification;
/ 	name		= rea_conf;
/ 	creation	= 1990/05/02;
/ 	function	= reads confirmation character;
/ 	parameter (in)	= code	: (int) information code;
/ 			= inf1	: (location_t) detail;
/ 	parameter (out)	= none;
/ 	return		= TRUE	: (flag_t) yes;
/ 			= FALSE	: (flag_t) no;
*/
 
flag_t		rea_conf(code,inf1)
int		code;
location_t	inf1;
{
	register	wchar_t	c;		/* confirmation character */
	register	flag_t	confirmed;	/* if confirmed */
 
	/* print prompt */
 
	psy_msg(code,inf1);
 
	/* read yes or no */
 
	for (; ;)
	{
		c = getwchar();
 
		if ((confirmed = equalc(c,conf_yes)) || equalc(c,conf_no))
			break;
	}
 
	psy_msg(P_CLEAR,NULL);
 
	return (confirmed);
}
 
#ifndef	UNIX_STDIO
/*
/  specification;
/ 	name		= printf;
/ 	creation	= 1990/06/24;
/ 	function	= printf with no buffering putchar;
/ 	parameter (in)	= format	: (char_t *) format;
/ 	parameter (out) = none;
/ 	return		= none (void);
*/
 
int		printf(format)
char		*format;
{
	register	char	c;		/* format character */
	register	char	*s;		/* string */
	int	*arg = (int *)(&format);	/* arguments */
 
	while ((c = *format++))
	{
		if (c == '%')
		{
			switch ((c = *format++))
			{
				case 'c'	:
					putchar(*++arg);
					break;
 
				case 'd'	:
					put_int(*++arg);
					break;
 
				case 's'	:
					for (s = (char *)*++arg; *s; )
						putchar(*s++);
					break;
 
				case 'l'	:
					put_long(*(arg + 1),*(arg + 2));
					arg += 2;
					format++;
					break;
 
				default		:
					putchar(c);
					break;
			}
		}
		else
			putchar(c);
	}
}
 
put_int(x)
register	int	x;
{
	if (x < 10)
		putchar(x + '0');
	else
	{
		put_int(x / 10);
		put_int(x % 10);
	}
}
 
put_long(x)
register	long	x;
{
	if (x < 10)
		putchar((int)x + '0');
	else
	{
		put_long(x / 10);
		put_long(x % 10);
	}
}
#endif
 
/* end of psyio.c */
