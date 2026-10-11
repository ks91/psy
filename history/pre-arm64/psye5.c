/*
/  source file header;
/ 	name		= psye5.c : psy edit function source file 5;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/06/15;
/ 	contents	= tra_ltr	: transpose letters;
/ 			= tra_wrd	: transpose words;
/ 			= tra_lin	: transpose lines;
*/
 
/* source file id */
 
#define	PSYE5_C
 
/* include file */
 
#include	<stdio.h>
#include	"psy.h"
#include	"psyext.h"
#include	"psyfnc.h"
 
/* public declaration */
 
/* public functions */
 
int		tra_ltr();		/* transpose letters */
int		tra_wrd();		/* transpose words */
int		tra_lin();		/* transpose lines */
 
/*
/  specification;
/ 	name		= tra_ltr;
/ 	creation	= 1990/06/15;
/ 	function	= exchanges positions of two letters;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		tra_ltr()
{
	register	wchar_t		c;		/* exchange buffer */
	register	int		rp_cnt = rp;	/* repeat count */
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		c = fwd_ltrk(FALSE);
		if (c == '\n')
			bwd_ltrk(FALSE);
 
		if (sp_cm - cm_stack < 2)
		{
			show3();
			break;		/* cannot exchange letters */
		}
 
		c = getletr(cur_cm);
		putletr(cur_cm,getletr(*(sp_cm - 1)));
		putletr(*(sp_cm - 1),c);
 
		mod_buf(TRUE);
		show3();
		showwln();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= tra_wrd;
/ 	creation	= 1990/06/15;
/ 	function	= exchanges positions of two words;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		tra_wrd()
{
	register	cmcellpt_t	reg1_cm,reg2_cm;	/* pointer registers */
	cmcellpt_t			reg3_cm,reg4_cm;	/* pointer registers */
	cmcellpt_t	temp;		/* pointer register */
	cmcellpt_t	*spcm;		/* stack pointer */
	wchar_t		c;		/* a letter */
 
	int		rp_cnt = rp;	/* repeat count */ 
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		if (!getnext(reg2_cm = cur_cm))
			break;		/* end of text */
		spcm = sp_cm;
 
		/* skip separator */
 
		while ((cur_cm = getnext(push_cm(cur_cm))) && !inword(c = getletr(cur_cm)) && c != '\n')
			count_cm(c);
		reg1_cm = cur_cm = pop_cm();	/* beginning of a word */
		if (c == '\n')
			break;		/* end of line */
 
		/* skip current word */
 
		while ((cur_cm = getnext(push_cm(cur_cm))) && inword(c = getletr(cur_cm)))
			count_cm(c);
		reg3_cm = cur_cm = pop_cm();	/* end of a word */
 
		/* skip current separator */
 
		while (reg2_cm != _cm && (c = getletr(reg2_cm)) != '\n' && !inword(c))
			reg2_cm = *--spcm;
		if (reg2_cm == _cm || c == '\n')
			continue;		/* beginning of line */
		reg4_cm = reg2_cm;
 
		/* skip word */
 
		while (reg2_cm != _cm && inword(c = getletr(reg2_cm)))
			reg2_cm = *--spcm;
 
		/* transpose words */
 
		temp = getnext(reg3_cm);
		if (reg1_cm == reg4_cm)
		{
			putnext(reg3_cm,getnext(reg2_cm));
			putnext(reg2_cm,getnext(reg1_cm));
			putnext(reg1_cm,temp);
		}
		else
		{
			putnext(reg3_cm,getnext(reg4_cm));
			putnext(reg4_cm,temp);
			temp = getnext(reg1_cm);
			putnext(reg1_cm,getnext(reg2_cm));
			putnext(reg2_cm,temp);
		}
 
		clear_cm();		/* clear column stack */
		point.column = 1;
		reg1_cm = cur_ln -> addr;
 
		/* counts column */
 
		do	count_cm(getletr(reg1_cm = getnext(push_cm(reg1_cm))));
		while (reg1_cm != reg4_cm);
 
		cur_cm = reg1_cm;
		mod_buf(TRUE);
		show3();
		showwln();
	}
 
	return (OK);
}
 
/*
/  specification;
/ 	name		= tra_lin;
/ 	creation	= 1990/06/17;
/ 	function	= exchanges positions of two lines;
/ 	parameter (in)	= none;
/ 	parameter (out)	= none;
/ 	return		= OK	: (int) done;
*/
 
int		tra_lin()
{
	register	cmcellpt_t	reg1_cm,reg2_cm;	/* pointer registers */
	cmcellpt_t	reg3_cm;				/* pointer register */
	lncellpt_t			reg1_ln,reg2_ln;	/* pointer registers */
	int		rp_cnt = rp;	/* repeat count */ 
 
	clr_rp();		/* clear repeat count */
 
	for (; rp_cnt-- > 0;)
	{
		fwd_link(1,TRUE);
 
		if (sp_ln - ln_stack < 2)
			break;		/* cannot exchange lines */
 
		reg3_cm = getnext(reg1_cm = (reg1_ln = *(sp_ln - 2)) -> addr);
		putnext(reg1_cm,getnext(reg2_cm = (reg2_ln = *(sp_ln - 1)) -> addr));
		putnext(reg2_cm,reg3_cm);
 
		for (reg3_cm = reg2_cm; getnext(reg2_cm) != reg3_cm; reg2_cm = getnext(reg2_cm))
		{
			if (reg2_cm == bgnbk_cm || reg2_cm == endbk_cm)
				bgnbk_cm = endbk_cm = (cmcellpt_t)NULL;
		}
		putnext(reg2_cm,reg3_cm = cur_ln -> addr);
		for (; getnext(reg1_cm) != reg3_cm; reg1_cm = getnext(reg1_cm))
		{
			if (reg1_cm == bgnbk_cm || reg1_cm == endbk_cm)
				bgnbk_cm = endbk_cm = (cmcellpt_t)NULL;
		}
		putnext(reg1_cm,reg2_ln -> addr);
 
		mod_buf(TRUE);
	}
 
	show();
 
	return (OK);
}
 
/* end of psye5.c */
