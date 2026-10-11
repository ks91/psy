/*
/  source file header;
/ 	name		= psyfnc.h : psy function declaration include file;
/ 	version		= 1.00a;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/04/30;
/ 	contents	= functions in psy.c;
/ 			= functions in psycus.c;
/ 			= functions in psywchar.c;
/ 			= functions in psybas.c;
/ 			= functions in psyasm.s	(on MS-DOS);
/ 			= functions in psyio.c;
/ 			= functions in psyview.c;
/ 			= functions in psywndw.c;
/ 			= functions in psyreg.c;
/ 			= functions in psye1.c;
/ 			= functions in psye2.c;
/ 			= functions in psye3.c;
/ 			= functions in psye4.c;
/ 			= functions in psye5.c;
*/
 
/* functions in psy.c */
 
#ifndef	PSY_C
extern	int		main(int argc, char **argv);		/* main function */
extern	void		psy_msg();		/* message-output */
#endif

/* functions in psycus.c */
 
#ifndef	PSYCUS_C
extern	int		cus_psy();	/* customizer command */
extern	int		cus_psyk();	/* customizer command kernel */
extern	int		exe_cmd();	/* execute command function */
#endif
 
/* functions in psywchar.c */
 
#ifndef	PSYWCHAR_C
#	ifndef	UNIX_STDIO
extern	int		getwchar();	/* get wide character */
extern	int		putwchar();	/* put wide character */
#	endif
extern	int		getwc();	/* get wide character from file */
extern	int		putwc();	/* put wide character to file */
#endif
 
/* functions in psybas.c */
 
#ifndef PSYBAS_C
extern	lncellpt_t	new_ln();	/* allocate new line cell */
extern	cmcellpt_t	new_cm();	/* allocate new column cell */
extern	int		insletr();	/* insert a letter */
extern	wchar_t		delletr();	/* delete a letter */
extern	void		release();	/* release cells */
extern	bfcell_t	*new_buf();	/* allocate new buffer cell */
extern	void		rel_buf();	/* release buffer cell */
extern	void		mod_buf();	/* set / reset modify flag */
extern	wdcell_t	*new_wdw();	/* allocate new window cell */
extern	void		loa_wat();	/* load window attribute */
extern	void		sav_wat();	/* save window attribute */
extern	void		count_cm();	/* count column forward */
extern	void		getback();	/* count column backward */
extern	void		get_cm();	/* get to the specified column */
extern	flag_t		undefbk();	/* check if block is defined */
extern	lncellpt_t	*find_bk();	/* find block in stack */
#endif
 
/* functions in psyasm.s */
 
#ifndef	UNIX_STDIO
#	ifndef	PSYASM_S
/* extern	int		getchar();	/* getchar */
/* extern	int		putchar();	/* putchar */
#		ifndef	PTR_32BIT
extern	wchar_t		getletr();	/* get letter from column cell */
extern	wchar_t		putletr();	/* put letter to column cell */
extern	cmcellpt_t	getnext();	/* get next from column cell */
extern	cmcellpt_t	putnext();	/* put next to column cell */
#		endif
#	endif
#endif
 
/* functions in psyio.c */
 
#ifndef	PSYIO_C
extern	int		load();		/* load file */
extern	int		save();		/* save file */
extern	void		makebak();	/* make back up path name */
extern	int		rea_min();	/* read mini-buffer */
extern	flag_t		rea_conf();	/* read confirmation */
#	ifndef	UNIX_STDIO
extern	int		printf();	/* printf */
#	endif
#endif
 
/* functions in psyview.c */
 
#ifndef	PSYVIEW_C
extern	void		show();		/* show window */
extern	void		showrest();	/* show rest of the window */
extern	void		showln();	/* show rest of the line */
extern	void		showwln();	/* show the whole line */
extern	void		view();		/* show certain part of the buffer */
extern	void		show2();	/* show after block operation */
extern	void		show3();	/* show after scroll occured */
#endif
 
/* functions in psywndw.c */
 
#ifndef	PSYWNDW_C
extern	int		dsp_scr();	/* display all windows */
extern	void		shw_wdw();	/* show specified window */
extern	void		enstack();	/* make stack for window */
extern	int		opn_wdw();	/* open window */
extern	int		opn_shw();	/* open shell window */
extern	int		cls_wdw();	/* close window */
extern	int		sel_wdw();	/* select window */
extern	int		slb_wdw();	/* select window backward */
extern	int		enl_wdw();	/* enlarge window */
extern	int		ens_wdw();	/* ensmall window */
extern	void		adj_x();	/* adjust left margin */
extern	wdcell_t	*get_shw();	/* get shell window */
#endif
 
/* functions in psyreg.c */
 
#ifndef	PSYREG_C
extern	int		nam_exp();	/* name expression */
extern	int		sch_exp();	/* search expression */
extern	int		scb_exp();	/* search expression backward */
extern	int		tra_exp();	/* transliterate expression */
extern	int		sat_exp();	/* search and transliterate expression */
extern	int		stb_exp();	/* search and transliterate expression backward */
extern	regular_t	*parse();	/* regular expression parser */
extern	void		free_reg();	/* free regular expression */
extern	flag_t		equalc();	/* pattern matching kernel */
#endif
 
/* functions in psye1.c */
 
#ifndef	PSYE1_C
extern	int		edit(int argc, char **argv);		/* edit top level function */
extern	int		rpt_cmd();	/* set repeat count */
extern	int		exe_met();	/* execute meta key */
extern	int		exe_ect();	/* execute extended control key */
extern	int		exe_emt();	/* execute extended meta key */
extern	int		sav_txt();	/* save text */
extern	int		abo_txt();	/* abort text */
extern	int		ter_psy();	/* terminator */
extern	int		ext_psy();	/* exit from psy */
extern	int		exe_shc();	/* execute shell command */
extern	int		nop();		/* no operation */
#endif
 
/* functions in psye2.c */
 
#ifndef	PSYE2_C
extern	int		fwd_ltr();	/* forward letter */
extern	int		bwd_ltr();	/* backward letter */
extern	int		fwd_lin();	/* forward line */
extern	int		bwd_lin();	/* backward line */
extern	int		btm_lin();	/* bottom of the line */
extern	int		top_lin();	/* top of the line */
extern	int		fwd_wrd();	/* forward word */
extern	int		bwd_wrd();	/* backward word */
extern	int		fwd_wdw();	/* forward window */
extern	int		bwd_wdw();	/* backward window */
extern	int		btm_wdw();	/* bottom of the window */
extern	int		top_wdw();	/* top of the window */
extern	int		rht_wdw();	/* right edge of the window */
extern	int		lft_wdw();	/* left edge of the window */
extern	int		btm_txt();	/* bottom of the text */
extern	int		top_txt();	/* top of the text */
extern	flag_t		inword();	/* checks if in word */
#endif
 
/* functions in psye3.c */
 
#ifndef	PSYE3_C
extern	int		new_lin();	/* new line */
extern	int		fwd_tab();	/* forward tab */
extern	int		chg_ins();	/* change insert / over-write mode */
extern	int		set_tab();	/* set tab stop */
extern	int		del_ltr();	/* delete letter */
extern	int		dlb_ltr();	/* delete left letter */
extern	int		del_lin();	/* delete line */
extern	int		dlb_lin();	/* delete line backward */
extern	int		del_wrd();	/* delete word */
extern	int		dlb_wrd();	/* delete left word */
extern	int		ins_lin();	/* insert line */
extern	int		set_mrk();	/* set mark */
extern	int		jmp_mrk();	/* jump to mark */
extern	int		pre_lin();	/* previous line */
extern	int		jmp_lin();	/* jump to specified line */
extern	int		brk_min();	/* break mini-buffer */
extern	int		cnt_pnt();	/* count cursor point */
extern	int		cnt_byt();	/* count buffer bytes */
extern	int		vis_txt();	/* visit text */
extern	int		rea_txt();	/* read text */
extern	int		ren_txt();	/* rename text */
extern	int		lst_txt();	/* list text */
#endif
 
/* functions in psye4.c */
 
#ifndef	PSYE4_C
extern	int		bgn_blk();	/* begin block */
extern	int		end_blk();	/* end block */
extern	int		rel_blk();	/* release block */
extern	int		del_blk();	/* delete block */
extern	int		cpy_blk();	/* copy block */
extern	int		wri_blk();	/* write block to file */
extern	int		apd_kil();	/* append kill buffer by next deletion */
extern	int		ynk_kil();	/* yank kill buffer */
#endif
 
/* functions in psye5.c */
 
#ifndef	PSYE5_C
extern	int		tra_ltr();	/* transpose letters */
extern	int		tra_wrd();	/* transpose words */
extern	int		tra_lin();	/* transpose lines */
#endif
 
/* Shared editing kernels. */
extern wchar_t fwd_ltrk(flag_t scrl);
extern wchar_t bwd_ltrk(flag_t scrl);
extern int fwd_link(int c_x, flag_t scrl);
extern int bwd_link(int c_x, flag_t scrl);
extern void del_blkk(void);
extern void res_kil(void);
extern int ungetwc(int c, FILE *fp);

/* end of psyfnc.h */
