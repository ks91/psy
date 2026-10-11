/*
/  source file header;
/ 	name		= psyreg.c : psy regular expression control source file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/05/04;
/ 	contents	= private variables
/ 				regular expression pointers
/ 				beginning of line
/ 				transliterated string;
/ 			= nam_exp	: (int) name expression;
/ 			= sch_exp	: (int) search expression;
/ 			= scb_exp	: (int) search expression backward;
/ 			= tra_exp	: (int) transliterate expression;
/ 			= sat_exp	: (int) search and transliterate expression;
/ 			= stb_exp	: (int) search and transliterate expression backward;
/ 			= parse		: (regular_t *) regular expression parser;
/ 			= free_reg	: (void) free regular expression;
/ 			= equalc	: (flag_t) pattern matching kernel;
/ 			= sch_expk	: (int)	search expression kernel;
/ 			= sat_expk	: (int) search and transliterate expression kernel;
/ 			= tr_str	: (void) transliterate strings;
/ 			= replaceblk	: (void) replaces block;
/ 			= parse2	: (regular_t *) bracket paser;
/ 			= append	: (void) connect regular expression nodes;
/ 			= esc		: (wchar_t) escape sequence parser;
/ 			= equal		: (flag_t) search and transliterate strings;
/ 			= equalqq	: (flag_t) search and transliterate strings core;
/ 			= getvalue	: (wchar_t) get transliterated value;
*/
 
/* source file id */
 
#define	PSYREG_C
 
/* include file */
 
#include	<stdio.h>
#include	<ctype.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* private macro definition */
 
#define	ishex(c)	(isdigit(c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F')
 
/* public declaration */
 
/* public functions */
 
int		nam_exp();		/* name expression */
int		sch_exp();		/* search expression */
int		scb_exp();		/* search expression backward */
int		tra_exp();		/* transliterate expression */
int		sat_exp();		/* search and transliterate expression */
int		stb_exp();		/* search and transliterate expression backward */
regular_t	*parse();		/* regular expression parser */
void		free_reg();		/* free regular expression */
flag_t		equalc();		/* pattern matching kernel */
 
/* private declaration */
 
/* private variables */
 
static	regular_t	*s1 = NULL,*s2 = NULL;	/* regular expression pointer */
static	flag_t		bol = FALSE;		/* beginning of line */
static	wchar_t		*wstring;
static size_t wstring_capacity;	/* translated string */
 
/* private functions */
 
static	int		sch_expk();	/* search expression kernel */
static	int		sat_expk();	/* search and transliterate expression kernel */
static	void		tr_str();	/* transliterate strings */
static	void		replaceblk();	/* replaces block */
static	regular_t	*parse2();	/* bracket parser */
static	void		append();	/* connect regular expression nodes */
static	wchar_t		esc();		/* escape sequence parser */
static	flag_t		equal();	/* search and transliterate strings */
static	flag_t		equalqq();	/* search and transliterate strings core */
static	wchar_t		getvalue();	/* get transliterated value */
 
/*
/  specification;
/ 	name		= nam_exp;
/ 	creation	= 1990/06/04;
/ 	function	= specifies regular expression;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		nam_exp()
{
	char_t	sch_exp[REGEXP_SIZE + 1];	/* regular expression string */
	register	char_t	*rep_exp;	/* expression to replace with */
	register	char_t	*eol = sch_exp;		/* end of line pointer */
 
	clr_rp();		/* clear repeat count */
 
	/* read expressions */
 
	if (rea_min(P_SEARCH,sch_exp,REGEXP_SIZE + 1) == ERROR)
		return (OK);	/* break from mini-buffer */
 
	/* free regular expression */
 
	free_reg(s1);
	free_reg(s2);
	s1 = s2 = NULL;
 
	/* get expression to replace with */
 
	for (rep_exp = sch_exp; *rep_exp; rep_exp++)
	{
		if (*rep_exp == '/')
		{
			*rep_exp++ = '\0';
			break;
		}
		eol = rep_exp;
		if (*rep_exp == '\\' && *(rep_exp + 1))
			rep_exp++;

	}
 
	/* sets bol and eol */
 
	bol = sch_exp[0] == '^';
	if (*eol == '$')
	{
		*eol = '\n';
		if (*rep_exp)
			strcat(rep_exp,"\n");
		else
			rep_exp = (char_t *)"\n";
	}
 
	/* parse regular expressions */
 
	if (!(s1 = parse(bol ? &sch_exp[1] : sch_exp)))
	{
		psy_msg(E_REGEXP,NULL);
		return (OK);
	}
	if (*rep_exp && !(s2 = parse(rep_exp)))
		psy_msg(E_REGEXP,NULL);
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= sch_exp;
/ 	creation	= 1990/06/04;
/ 	function	= searches regular expression;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		sch_exp()
{
	return (sch_expk(TRUE));
}
 
/*
/  specification;
/ 	name		= scb_exp;
/ 	creation	= 1990/06/04;
/ 	function	= searches regular expression backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		scb_exp()
{
	return (sch_expk(FALSE));
}
 
/*
/  specification;
/ 	name		= tra_exp;
/ 	creation	= 1990/06/04;
/ 	function	= transliterates regular expression;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		tra_exp()
{
	clr_rp();		/* clear repeat count */
 
	if (!s1)
	{		/* expression not specified */
		psy_msg(E_REGEXP,NULL);
		return (OK);
	}
 
	if (undefbk())	/* bad block */
		return (OK);
	replaceblk();
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= sat_exp;
/ 	creation	= 1990/06/04;
/ 	function	= searches and transliterates regular expression;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		sat_exp()
{
	return (sat_expk(TRUE));
}
 
/*
/  specification;
/ 	name		= stb_exp;
/ 	creation	= 1990/06/04;
/ 	function	= searches and transliterates regular expression backward;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		stb_exp()
{
	return (sat_expk(FALSE));
}
 
/*
/  specification;
/ 	name		= parse;
/ 	creation	= 1990/05/13;
/ 	function	= parses regular expression;
/ 	parameter (in)	= s	: (char_t *) regular expression;
/ 	parameter (out)	= none;
/ 	return		= NULL		: insufficient memory;
/ 			= otherwise	: (regular_t *) pointer to regular expression tree;
*/
 
regular_t	*parse(s)
char_t	*s;
{
	regular_t	*first = NULL;			/* pointer to the first node */
	register	regular_t	*p,*dummy;	/* pointers to regular expression nodes */
 
	/* parse till the end of the expression */
 
	while (*s)
	{
		if (!(p = (regular_t *)malloc(sizeof(regular_t))))
		{
			free_reg(first);
			return (NULL);		/* cannot allocate regular expression node */
		}
 
		switch (*s)
		{
			case	'.' :		/* period */
				p -> id = period;
				break;
 
			case	'[' :		/* bracket */
				p -> id = bracket;
				if  ((p -> data.leg.negate = *++s == '^'))
					++s;
				if (!(p -> data.leg.nest = parse2(&s)))
				{
					free(p);
					free_reg(first);
					return(NULL);
				}
				break;
 
			default :		/* usual letter */
				p -> id = natural;
				p -> data.letter = esc(&s);
				break;
		}
 
		p -> link = NULL;
 
		if (*++s == '*')	/* closure */
		{
			dummy = p;
			if (!(p = (regular_t *)malloc(sizeof(regular_t))))
				return (NULL);	/* cannot allocate regular expression node */
			p -> id = closure;
			p -> data.leg.nest = dummy;
			p -> link = NULL;
			++s;
		}
 
		append(&first,p);
	}
 
	return (first);
}
 
/*
/  specification;
/ 	name		= free_reg;
/ 	creation	= 1990/05/13;
/ 	function	= releases regular expression nodes;
/ 	parameter (in)	= s	: (regular_t *) pointer to regular expression;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
void		free_reg(s)
regular_t	*s;
{
	if (!s)
		return;	/* end of expression */
 
	switch (s -> id)
	{
		case	closure :	/* closure */
		case	bracket	:	/* bracket */
			free_reg(s -> data.leg.nest);
			break;
 
		default :
			break;
	}
 
	free_reg(s -> link);	/* do it recursively */
	free(s);
}
 
/*
/  specification;
/ 	name		= equalc;
/ 	creation	= 1990/06/23;
/ 	function	= matches a letter with regular expression;
/ 	parameter (in)	= c	: (wchar_t) a letter;
/ 			= p	: (regular_t *) regular expression;
/ 	parameter (out)	= none;
/ 	return		= TRUE	: (flag_t) succeeded in matching;
/ 			= FALSE	: (flag_t) failed in matching; 
*/
 
flag_t		equalc(c,p)
wchar_t		c;
regular_t	*p;
{
	flag_t		clsr;
	wchar_t		tr;
 
	return (equalqq(c,&tr,p,NULL,&clsr));
}
 
/*
/  specification;
/ 	name		= sch_expk;
/ 	creation	= 1990/06/04;
/ 	function	= searches regular expression;
/ 	parameter (in)	= fwd	: (flag_t) search forward;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
static	int	sch_expk(fwd)
flag_t		fwd;
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
	_prev.line = point.line;
	_prev.column = point.column;
 
	for (; rp_cnt-- > 0;)
	{
		if (!s1)
		{		/* expression not specified */
			psy_msg(E_REGEXP,NULL);
			break;
		}
 
		psy_msg(S_SCHING,NULL);
		tr_str(s1,s2,fwd);
		psy_msg(P_CLEAR,NULL);
		show();
		if (!bgnbk_cm)		/* no more expressions */
		{
			psy_msg(E_NOMORE,NULL);
			break;
		}
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= sat_expk;
/ 	creation	= 1990/06/04;
/ 	function	= searches and transliterates regular expression;
/ 	parameter (in)	= fwd	: (flag_t) search forward;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
static	int	sat_expk(fwd)
flag_t		fwd;
{
	register	int	rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
	_prev.line = point.line;
	_prev.column = point.column;
 
	for (; rp_cnt-- > 0;)
	{
		if (!s1)
		{		/* expression not specified */
			psy_msg(E_REGEXP,NULL);
			break;
		}
 
		psy_msg(S_SCHING,NULL);
		tr_str(s1,s2,fwd);
		psy_msg(P_CLEAR,NULL);
		if (!bgnbk_cm)		/* no more expressions */
		{
			show();
			psy_msg(E_NOMORE,NULL);
			break;
		}
 
		replaceblk();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= tr_str;
/ 	creation	= 1990/05/13;
/ 	function	= transliterates strings;
/ 	parameter (in)	= s1	: (regular_t *) source expression;
/ 			= s2	: (regular_t *) destination expression;
/ 			= fwd	: (flag_t) search forward;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void		tr_str(s1,s2,fwd)
register	regular_t	*s1,*s2;
flag_t		fwd;
{
	/* A closure can translate an entire document, not just REGEXP_SIZE letters. */
	size_t needed = (size_t)buff_size + REGEXP_SIZE + 1;
	if (needed > wstring_capacity) {
		wchar_t *grown = realloc(wstring, needed * sizeof(*wstring));
		if (!grown) {
			bgnbk_cm = endbk_cm = 0;
			psy_msg(E_NOMEM, NULL);
			return;
		}
		wstring = grown;
		wstring_capacity = needed;
	}
	/* find and replace */
 
	if (bol)
	{		/* if beginning of line, */
		if (cur_cm == cur_ln -> addr || (fwd ? !fwd_link(1,TRUE) : !bwd_link(1,TRUE)))
			do
			{
				if (equal(s1,s2,wstring))
					break;
			}
			while (fwd ? !fwd_link(1,TRUE) : !bwd_link(1,TRUE));
	}
	else
	{
		do
		{
			if (equal(s1,s2,wstring))
				break;
		}
		while (fwd ? fwd_ltrk(TRUE) : bwd_ltrk(TRUE));
	}
 
	*sp_ln = cur_ln;	/* for display */
}
 
/*
/  specification;
/ 	name		= replaceblk;
/ 	creation	= 1990/05/13;
/ 	function	= replaces block;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= none (void);
*/
 
static	void		replaceblk()
{
	register	cmcellpt_t	reg1_cm,reg2_cm;	/* pointer registers */
	lncellpt_t	reg1_ln,reg2_ln;			/* pointer registers */
	int		c_y;		/* line number */
	wchar_t		*p;		/* pointer to wstring */
	flag_t		err = FALSE;	/* if error */
 
	reg1_cm = bgnbk_cm;
	reg1_ln = bgnbk_ln;
 
	del_blkk();	/* delete block */
 
	bgnbk_cm = reg1_cm;
	bgnbk_ln = reg1_ln;
	reg2_cm = getnext(reg1_cm);
	reg2_ln = reg1_ln -> link;
 
	/* till the end of the wstring */
 
	for (p = wstring; *p; p++)
	{
		/* allocate new column cell */
 
		if (!putnext(reg1_cm,new_cm()))
		{
			err = TRUE;
			break;
		}
		if (putletr(reg1_cm = getnext(reg1_cm),*p) == '\n')
		{
			/* allocate new line cell */
 
			if (!(reg1_ln -> link = new_ln()))
			{
				err = TRUE;
				break;
			}
			(reg1_ln = reg1_ln -> link) -> addr = reg1_cm;
		}
	}
 
	endbk_cm = reg1_cm;
	endbk_ln = reg1_ln;
	putnext(reg1_cm,reg2_cm);
	reg1_ln -> link = reg2_ln;
	if (err)
		psy_msg(E_LARGE,NULL);
 
	clear_ln();
	clear_cm();
 
	for (c_y = 1, reg1_ln = _ln; reg1_ln != endbk_ln; reg1_ln = push_ln(reg1_ln) -> link,c_y++)
		;
	*sp_ln = reg1_ln;
	if ((y_bas = c_y - (point.line - y_bas)) < 1)
		y_bas = 1;
	point.line = c_y;
	
	for (point.column = 1,cur_cm = (cur_ln = reg1_ln) -> addr; cur_cm != endbk_cm; cur_cm = getnext(push_cm(cur_cm)),count_cm(getletr(cur_cm)))
		;
	adj_x();
	show();
}
 
/*
/  specification;
/ 	name		= parse2;
/ 	creation	= 1990/05/13;
/ 	function	= parses inside bracket;
/ 	parameter (in)	= s	: (char_t **) string inside bracket;
/ 	parameter (out)	= s	: (char_t **) string after bracket;
/ 	return		= NULL		: insufficient memory or unclosed bracket;
/ 			= otherwise	: (regular_t *) pointer to regular expression tree;
*/
 
static	regular_t	*parse2(s)
char_t	**s;
{
	regular_t	*first = NULL;		/* pointer to the first node */
	register	regular_t	*p;	/* pointer to regular expression node */
	register	wchar_t		temp;	/* a letter */
 
	/* parse till the end of the bracket */
 
	while (**s != ']')
	{
		if (!**s || !(p = (regular_t *)malloc(sizeof(regular_t))))
		{
			free_reg(first);
			return (NULL);	/* unclosed bracket or cannot allocate regular expression node */
		}
 
		temp = esc(s);		/* get the letter */
 
		if (*++*s == '-')	/* subrange */
		{
			++*s;
			p -> id = subrange;
			p -> data.between.from = temp;
			p -> data.between.to = esc(s);
			++*s;
		}
		else			/* usual letter */
		{
			p -> id = natural;
			p -> data.letter = temp;
		}
 
		p -> link = NULL;
		append(&first,p);
	}
 
	return (first);
}
 
/*
/  specification;
/ 	name		= append;
/ 	creation	= 1990/05/13;
/ 	function	= appends regular expression nodes;
/ 	parameter (in)	= first	: (regular_t **) top of the tree;
/ 			= p	: (regular_t *) the last node of the tree;
/ 	parameter (out)	= first	: (regular_t **) top of the tree;
/ 	return		= none (void);
*/
 
static	void	append(first,p)	
regular_t	**first,*p;
{
	register	regular_t	*dummy;	/* pointer to regular expression node */
 
	if (!*first)	/* if there's no tree, make one */
		*first = p;
	else
	{		/* put the specified node to the end of the tree */
		for (dummy = *first; dummy -> link; dummy = dummy -> link)
			;
		dummy -> link = p;
	}
}
 
/*
/  specification;
/ 	name		= esc;
/ 	creation	= 1990/05/13;
/ 	function	= parses escape sequece;
/ 	parameter (in)	= s	: (char_t **) escape sequence;
/ 	parameter (out)	= s	: (char_t **) string after escape sequence;
/ 	return		= character code	: (wchar_t) parsed letter;
*/
 
static	wchar_t		esc(s)	
char_t	**s;
{
	char_t		first_byte;	/* first byte of a letter */
	wchar_t		c;		/* returning letter */
	register	int	i;	/* loop counter */
	register	int	len;	/* length of the hex string */
 
	/* if starts from '\\', escape sequence */
 
	if (**s == '\\')
	{
		len = 2;	/* default length */
 
		switch (*++*s)
		{
			case	'\0' :
				return (*--*s);	/* end of string */
 
			case	'n' :		/* end of line */
				return ('\n');
 
			case	'k' :		/* shift-JIS kanji code */
				len = 4;	/* kanji code length */
				++*s;
 
			default :
				c = '\0';
				for (i = 0; i < len; ++*s,i++)
				{
					register	char_t	temp;
 
					if (!(temp = **s))
						return (*--*s);	/* end of string */
					if (!ishex(temp))
						goto label;
 
					c = c * 16 + (temp <= '9' ? temp - '0' : tolower(temp) - 'a' + 10);
					/* The code above depends on ASCII character set */ 
				}
				--*s;
				return (c);
		}
	}
 
label:
	c = psy_next_char(s);
	--*s; /* callers advance past the last byte */
	return c;
}
 
/*
/  specification;
/ 	name		= equal;
/ 	creation	= 1990/05/13;
/ 	function	= searches and transliterates strings;
/ 	parameter (in)	= s1	: (register_t *) source expression;
/ 			= s2	: (register_t *) destination expression;
/ 	parameter (out)	= new	: (wchar_t *) transliterated string;
/ 	return		= TRUE	: (flag_t) found;
/ 			= FALSE	: (flag_t) not found;
*/
 
static	int	equal(s1,s2,new)
regular_t	*s1,*s2;
wchar_t		*new;
{
	lncellpt_t	*tempsp_ln;
	cmcellpt_t	*tempsp_cm;
	int		tmp_x,tmp_y;
	point_t		tmp_point;
	wchar_t		c;
	wchar_t		tr;
	flag_t		out_clsr;
	register	lncellpt_t	reg1_ln;
	register	cmcellpt_t	reg1_cm;
 
	/* save current location */
 
	reg1_cm = cur_cm;
	reg1_ln = cur_ln;
 
	tempsp_ln = sp_ln;
	tempsp_cm = sp_cm;
 
	tmp_point.line = point.line;
	tmp_point.column = point.column;
	tmp_x = x_bas;
	tmp_y = y_bas;
 
	/* pattern matching */
 
	while (s1)
	{
		for (; s2 && s2 -> id == natural; s2 = s2 -> link)
			*new++ = s2 -> data.letter;
 
		if (!(c = fwd_ltrk(TRUE)) || !equalqq(c,&tr,s1,s2,&out_clsr))
		{
			/* restore current location */
 
			y_bas = tmp_y;
			x_bas = tmp_x;
			point.line = tmp_point.line;
			point.column = tmp_point.column;
 
			sp_cm = tempsp_cm;
			sp_ln = tempsp_ln;
 
			cur_ln = reg1_ln;
			cur_cm = reg1_cm;
 
			/* release block */
 
			bgnbk_cm = endbk_cm = 0;
 
			return (FALSE);
		}
 
		if (s1 -> id != closure || out_clsr)
		{
			if (s1 -> id != natural && s2)
				s2 = s2 -> link;
			s1 = s1 -> link;
 
			if (out_clsr)
			{
				bwd_ltrk(TRUE);
				if (s2)
					new--;
			}
		}
		if (tr)
			*new++ = tr;
	}
 
	/* if destination expression is longer */
 
	while (s2)
	{
		if (s2 -> id == natural)
			*new++ = s2 -> data.letter;
		s2 = s2 -> link;
	}
 
	*new = '\0';
 
	/* make block */
 
	bgnbk_cm = reg1_cm;
	endbk_cm = cur_cm;
	bgnbk_ln = reg1_ln;
	endbk_ln = cur_ln;
 
	return (TRUE);
}
 
/*
/  specification;
/ 	name		= equalqq;
/ 	creation	= 1990/05/13;
/ 	function	= matches pattern;
/ 	parameter (in)	= c	: (wchar_t) a letter;
/ 			= s1	: (regular_t *) source expression;
/ 			= s2	: (regular_t *) destination expression;
/ 	parameter (out)	= tr	: (wchar_t *) transliterated letter;
/ 			= clsr	: (flag_t *) if out of closure;
/ 	return		= TRUE	: succeeded in pattern matching;
/ 			= FALSE	: failed in pattern matching;
*/
 
static	flag_t	equalqq(c,tr,s1,s2,clsr)
wchar_t		c;
wchar_t		*tr;
register	regular_t	*s1;
register	regular_t	*s2;
flag_t		*clsr;
{
	flag_t		ret = FALSE;	/* return value */
	flag_t		negate;		/* negation */
	regular_t	*p;		/* pointer to regular expression */
 
	*clsr = FALSE;
	*tr = 0;
 
	/* pattern matching */ 
 
	switch (s1 -> id)
	{
		case	period :	/* period */
			ret = TRUE;
			*tr = getvalue(c,s1,s2);
			break;	/* anything's OK */
 
		case	closure :	/* closure */
			s1 = s1 -> data.leg.nest;
			p = (s2 && s2 -> id == closure ? s2 -> data.leg.nest : s2);
			if (equalqq(c,tr,s1,p,clsr))
				return (TRUE);
			else
				*clsr = ret = TRUE;	/* out of closure */
			break;
 
		case	bracket :
			p = (s2 && s2 -> id == bracket ? s2 -> data.leg.nest : s2);
			negate = s1 -> data.leg.negate;
			for (s1 = s1 -> data.leg.nest; s1; s1 = s1 -> link)
			{
				if (equalqq(c,tr,s1,p,clsr))
					break;
				if (p && s2 -> id == bracket)
					p = p -> link;
			}
			if (!s1 && negate)
			{
				ret = TRUE;
				*tr = getvalue(c,NULL,p);
			}
			else if (s1 && !negate)
				ret = TRUE;
			break;
 
		case	subrange :	/* subrange */
			if (c >= s1 -> data.between.from && c <= s1 -> data.between.to)
			{
				ret = TRUE;
				*tr = getvalue(c,s1,s2);
			}
			break;
 
		case	natural :	/* natural */
			if (c == s1 -> data.letter)
			{
				ret = TRUE;
				*tr = (s2 && s2 -> id == natural ? getvalue(c,s1,s2) : '\0');
			}
			break;
 
		default :
			break;
	}
 
	return (ret);
}
 
/*
/  specification;
/ 	name		= getvalue;
/ 	creation	= 1990/05/13;
/ 	function	= gets transliterated value;
/ 	parameter (in)	= c	: (wchar_t) actual letter;
/ 			= s1	: (regular_t *) source expression node;
/ 			= s2	: (regular_t *) destination expression node;
/ 	return		= NULL		: no value;
/ 			= otherwise	: (wchar_t) acquired value;
*/
 
static	wchar_t		getvalue(c,s1,s2)
wchar_t		c;
register	regular_t	*s1,*s2;
{
	if (!s2)
		return ('\0');
 
	switch (s2 -> id)
	{
		case	subrange :	/* subrange */
			if (s1 -> id != subrange)
				return ('\0');
			return (c - s1 -> data.between.from + s2 -> data.between.from);
 
		case	period :	/* period */
			return (c);
 
		case	natural	:
			return (s2 -> data.letter);
 
		default :
			return ('\0');
	}
}
 
/* end of psyreg.c */
