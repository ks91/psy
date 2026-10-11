/*
/  source file header;
/ 	name		= psyext.h : psy extern declaration include file;
/ 	version		= 1.00;
/ 	designer	= Kenji Psytoe;
/ 	date		= 1990/04/30;
/ 	contents	= global variables
/ 				line cells
/ 				stack
/ 				stack pointers
/ 				long heap access
/ 				current cell pointers
/ 				current top of the buffer
/ 				current block pointers
/ 				current kill buffer pointers
/ 				append kill buffer flag
/ 				current screen information
/ 				free cell pointers
/ 				line and column counters
/ 				current edges
/ 				insert / over-write mode flag
/ 				tab length
/ 				repeating command
/ 				stdout file
/ 				mark position
/ 				buffer
/ 				window
/ 				keyboard-function table
/ 				messages;
*/
 
#ifdef	PSY_C
#	define	PUBLIC
#else
#	define	PUBLIC	extern
#endif
 
/* global variables */
 
/* line cells */
 
PUBLIC	lncellpt_t	cellbase,celltop;	/* cell base / top pointer */
 
/* stack */
 
PUBLIC	lncellpt_t	*ln_stack;		/* line stack base */
PUBLIC	cmcellpt_t	*cm_stack;		/* column stack base */
 
PUBLIC	lncellpt_t	*spln_base;		/* stack base pointer */
 
/* stack pointers */
 
PUBLIC	lncellpt_t	*sp_ln;			/* line stack pointer */
PUBLIC	cmcellpt_t	*sp_cm;			/* column stack pointer */
 
/* long heap access */
 
#ifdef	PTR_32BIT
PUBLIC	cmcell_t	*heap;			/* long heap access */
#else
PUBLIC	unsigned	heap[MAX_BUFF_UNIT];	/* long heap access */
#endif
 
/* current cell pointers */
 
PUBLIC	lncellpt_t	cur_ln;		/* current line cell pointer */
PUBLIC	cmcellpt_t	cur_cm;		/* current column cell pointer */
 
/* current top of the buffer */
 
PUBLIC	lncellpt_t	_ln;		/* first line cell */
PUBLIC	cmcellpt_t	_cm;		/* first column cell */
 
/* current block pointers */
 
#ifdef	PSY_C
PUBLIC	lncellpt_t	bgnbk_ln = NULL,endbk_ln = NULL;	/* block lines */
PUBLIC	cmcellpt_t	bgnbk_cm = 0,endbk_cm = 0;	/* block columns */
#else
PUBLIC	lncellpt_t	bgnbk_ln,endbk_ln;	/* block lines */
PUBLIC	cmcellpt_t	bgnbk_cm,endbk_cm;	/* block columns */
#endif
 
/* current kill buffer pointers */
 
#ifdef	PSY_C
PUBLIC	cmcellpt_t	bgnkl_cm = 0,endkl_cm = 0;	/* kill columns */
#else
PUBLIC	cmcellpt_t	bgnkl_cm,endkl_cm;	/* kill columns */
#endif
 
/* append kill buffer flag */
 
#ifdef	PSY_C
PUBLIC	flag_t		apnd_kill = FALSE;
#else
PUBLIC	flag_t		apnd_kill;
#endif
 
/* current screen information */
 
PUBLIC	int		ceiling,height;		/* first line and length */
 
/* free cell pointers */
 
PUBLIC	lncellpt_t	ln_free;		/* free line cell pointer */
PUBLIC	cmcellpt_t	cm_free;		/* free column cell pointer */
 
/* line and column counters */
 
PUBLIC	point_t		point;			/* current point */
 
/* current edges */
 
PUBLIC	int		x_bas,y_bas;		/* current edge */
 
/* insert / over-write mode flag */
 
PUBLIC	flag_t		ins_mode;		/* insert mode */
PUBLIC	flag_t		d_ins_mode;		/* default insert mode */
 
/* tab length */
 
PUBLIC	int		tab_len;		/* tab length */
PUBLIC	long		d_tab_len;		/* default tab length */
 
/* repeating command */ 
 
#ifdef	PSY_C
PUBLIC	int		rp = 1;			/* repeat counter */
#else
PUBLIC	int		rp;			/* repeat counter */
#endif
PUBLIC	long		rp_unit;		/* repeat unit */
 
/* stdout file */
 
#ifdef	PSY_C
PUBLIC	char_t		*stdout_file = NULL;	/* stdout file */
#else
PUBLIC	char_t		*stdout_file;		/* stdout file */
#endif
 
/* mark position */
 
PUBLIC	point_t		_mark,_prev;		/* mark position */
 
/* buffer */
 
PUBLIC	bfcell_t	*edit_buf;		/* buffer cell area */
#ifdef	PSY_C
PUBLIC	long			buff_size = 0;		/* buffer size */
PUBLIC	long			buff_no = 0;		/* multiple buffer number */
PUBLIC	long			buff_line = 0;		/* buffer line */
PUBLIC	long			mini_his = 0;		/* mini buffer history */
#else
PUBLIC	long			buff_size;		/* buffer size */
PUBLIC	long			buff_no;		/* multiple buffer number */
PUBLIC	long			buff_line;		/* buffer line */
PUBLIC	long			mini_his;		/* mini buffer history */
#endif
 
/* window */
 
PUBLIC  wdcell_t	*edit_wdw;		/* window cell area */
PUBLIC	wdcell_t	*wdw_list;		/* pointer to window list */
PUBLIC	wdcell_t	*cur_wdw;		/* current window */
 
/* keyboard-function table */ 
 
PUBLIC	int		(*ctrl['z' - 'a' + 1])();	/* with control key */
PUBLIC	int		(*meta['z' - 'a' + 1])();	/* with meta key */
PUBLIC	int		(*e_ctrl['z' - 'a' + 1])();	/* with extended control key */
PUBLIC	int		(*e_meta['z' - 'a' + 1])();	/* with extended meta key */
 
/* messages */
 
PUBLIC	flag_t	attention;			/* if beep when error */
#ifdef	PSY_C
PUBLIC	dsp_t	displaying = clear;		/* message status */
#else
PUBLIC	dsp_t	displaying;			/* message status */
#endif
PUBLIC	char_t	*mod_str[M_SHELL - M_MSG + 1];	/* Mode line strings */
PUBLIC	char_t	*err_str[E_CUSTOM - E_MSG + 1];	/* error message strings */
PUBLIC	char_t	*pro_str[P_TABLEN - P_MSG + 1];	/* prompt message strings */
PUBLIC	char_t	*inf_str[I_CAPACITY - I_MSG + 1];	/* information message strings */
PUBLIC	char_t	*sta_str[S_RELMEM - S_MSG + 1];	/* state message strings */
PUBLIC	regular_t	*conf_yes;		/* yes */
PUBLIC	regular_t	*conf_no;		/* no */
 
/* initial file */
 
#ifdef	PSY_C
PUBLIC	char_t		*initial_file = NULL;	/* initial file */
#else
PUBLIC	char_t		*initial_file;		/* initial file */
#endif
 
/* jis kanji I/O */
 
#ifdef	JIS
#	ifdef	PSY_C
PUBLIC	flag_t		jis_kanji = FALSE;	/* jis kanji I/O */
PUBLIC	flag_t		jis_cont = FALSE;	/* jis kanji continue */
#	else
PUBLIC	flag_t		jis_kanji;		/* jis kanji I/O */
PUBLIC	flag_t		jis_cont;		/* jis kanji continue */
#	endif
#endif
 
/* end of psyext . h */
