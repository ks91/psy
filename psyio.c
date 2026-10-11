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
#include <errno.h>
#include <sys/stat.h>
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
 
int load(char_t *path, flag_t new)
{
    FILE *fp;
    cmcellpt_t first = 0, last = 0, cell, next;
    lncellpt_t first_line = NULL, last_line = NULL, line, next_line;
    int c, encoding, previous_cr = 0, ret = ERROR;
    bfcell_t *buffer = cur_wdw->bf_addr;

    psy_msg(S_WAIT, NULL);
    fp = fopen((char *)path, "rb");
    if (!fp) {
        if (new && errno == ENOENT) {
            buffer->bf_load_failed = FALSE;
            psy_msg(P_NEW, path);
            return OK;
        }
        if (new) buffer->bf_load_failed = TRUE;
        psy_msg(E_OPNFIL, path);
        return ERROR;
    }
    encoding = psy_read_encoding == PSY_AUTO ? psy_detect_encoding(fp) : psy_read_encoding;
    if (encoding == PSY_INVALID) goto done;
    psy_msg(S_LOADIN, path);

    /* Build a detached list; failed reads must not leave a partial document. */
    while ((c = psy_read_char(fp, encoding)) != EOF) {
        if (c == PSY_INVALID || !c) goto done;
        if (previous_cr && c == '\n') { previous_cr = 0; continue; }
        previous_cr = c == '\r';
        if (previous_cr) c = '\n';
        cell = new_cm();
        if (!cell) goto done;
        putletr(cell, c);
        putnext(cell, 0);
        if (last) putnext(last, cell);
        else first = cell;
        last = cell;
        if (c == '\n') {
            line = new_ln();
            if (!line) goto done;
            line->addr = cell;
            line->link = NULL;
            if (last_line) last_line->link = line;
            else first_line = line;
            last_line = line;
        }
    }
    if (ferror(fp)) goto done;
    ret = OK;

done:
    if (fclose(fp)) ret = ERROR;
    if (ret == OK) {
        if (first) {
            putnext(last, getnext(cur_cm));
            putnext(cur_cm, first);
        }
        if (first_line) {
            last_line->link = cur_ln->link;
            cur_ln->link = first_line;
        }
        if (new) buffer->bf_load_failed = FALSE;
        psy_msg(P_CLEAR, NULL);
    } else {
        for (cell = first; cell; cell = next) {
            next = getnext(cell);
            putletr(cell, 0);
        }
        for (line = first_line; line; line = next_line) {
            next_line = line->link;
            line->addr = 0;
        }
        if (new) buffer->bf_load_failed = TRUE;
        psy_msg(E_OPNFIL, path);
    }
    return ret;
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
 
int psy_save_file(char_t *path, cmcellpt_t first, cmcellpt_t last,
                  int encoding, int backup)
{
    char temporary[PATH_SIZE + 32], backup_path[PATH_SIZE + 8];
    struct stat info;
    FILE *fp = NULL;
    int fd, exists, moved = 0;
    cmcellpt_t cell;
    size_t length = strlen((char *)path);

    if (length >= PATH_SIZE) goto failed;
    exists = stat((char *)path, &info) == 0;
    if (!exists && errno != ENOENT) goto failed;
    snprintf(temporary, sizeof(temporary), "%s.psy.XXXXXX", path);
    fd = mkstemp(temporary);
    if (fd < 0) goto failed;
    if (exists && fchmod(fd, info.st_mode & 0777)) { close(fd); goto remove_temp; }
    fp = fdopen(fd, "wb");
    if (!fp) { close(fd); goto remove_temp; }
    psy_msg(S_SAVING, path);
    for (cell = getnext(first); cell != last; cell = getnext(cell)) {
        if (!cell || psy_write_char(getletr(cell), fp, encoding) == EOF) {
            fclose(fp); goto remove_temp;
        }
    }
    /* Convert and flush fully before touching either original or backup. */
    if (fflush(fp) || fsync(fileno(fp))) { fclose(fp); goto remove_temp; }
    if (fclose(fp)) goto remove_temp;
    if (exists && backup) {
        makebak(path, (char_t *)backup_path);
        if (!strcmp((char *)path, backup_path))
            snprintf(backup_path, sizeof(backup_path), "%s.bak", path);
        if (rename((char *)path, backup_path)) goto remove_temp;
        moved = 1;
    }
    if (rename(temporary, (char *)path)) {
        if (moved) rename(backup_path, (char *)path);
        goto remove_temp;
    }
    psy_msg(P_CLEAR, NULL);
    return OK;

remove_temp:
    unlink(temporary);
failed:
    psy_msg(E_CRTFIL, path);
    return ERROR;
}

int save(char_t *path, cmcellpt_t first, cmcellpt_t last)
{
    if (cur_wdw->bf_addr->bf_load_failed) {
        psy_msg(E_CRTFIL, path);
        return ERROR;
    }
    return psy_save_file(path, first, last, cur_wdw->bf_addr->bf_encoding, FALSE);
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
	char_t bytes[4];
	int n;
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
		for (reg1_cm = cur_ln -> addr; (reg1_cm = getnext(reg1_cm)) && (c = getletr(reg1_cm)) != '\n';)
		{
			n = psy_utf8_encode(c, bytes);
			if (!n || n >= len) break;
			memcpy(s, bytes, n);
			s += n;
			len -= n;
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
