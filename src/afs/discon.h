

/*
** These are defines for the different type of disconnected
** operations stored in the log.
*/

#ifndef  _DISCONNH
#define _DISCONNH

/* these are define for integrity checking */
#define CHECK_SLOTS	(long) 0x1
#define CHECK_LRUQ	(long) 0x2
#define CHECK_FREEVS	(long) 0x4

/* values for the dflags in the vcache */
#define VC_DIRTY	0x01
#define KEEP_VC		0x04
#define DBAD_VC		0x08		/* This is a know bad vcache */
/* this flags is used by GetVSlot to mark when a vcache was read from
** the disk.  
*/
#define READ_DISK	0x10

/* Flags for dflags in the fcache */
#define	KEEP_DC		0x01

/* Flags in the afs_VindexFlags */

#define VC_FREE		0x01 
#define HAS_CCORE	0x02
/* 0x04 is used by KEEP_VC */
#define VC_DATAMOD	0x08
#define VC_FLAG		0x10
#define VC_CLEARAXS	0x20
#define VC_HAVECB	0x40

/* magic number for data files */

#define AFS_DHMAGIC         0x7635fac1

/* these are the file name extensions for various errors */
#define STORE_EXT	".store"
#define RENAME_EXT	".ren"
#define CREATE_EXT	".creat"
#define MKDIR_EXT	".mkdir"
#define LINK_EXT	".link"
#define SYMLINK_EXT	".slink"
#define ORPH_EXT	".orph"


enum discon_modes {
	CONNECTED,
	DISCONNECTED,
	FETCH_ONLY,
	PARTIALLY_CONNECTED
};

typedef enum discon_modes discon_modes_t;

#define	IS_DISCONNECTED(state)	(state == DISCONNECTED)
#define	IS_FETCHONLY(state)	(state == FETCH_ONLY)
#define	IS_PARTIAL(state)	(state == PARTIALLY_CONNECTED)
#define	IS_CONNECTED(state)	(state == CONNECTED)
#define LOG_OPERATIONS(state)	((state == DISCONNECTED) ||  \
		(state == FETCH_ONLY) || (state == PARTIALLY_CONNECTED))
#define USE_OPTIMISTIC(state)	((state == DISCONNECTED) ||  \
		(state == FETCH_ONLY))
	

/* These are the different levels of error logging */
#define	DISCON_ERR	0
#define	DISCON_NOTICE	1
#define	DISCON_INFO	2
#define	DISCON_DEBUG	3

/* pioctl flags */
#define AFS_DIS_RECON		0	/* normal reconnect */
#define AFS_DIS_DISCON		1	/* disconnect */
#define AFS_DIS_PANIC		2	/* die, monster devil, die */
#define AFS_DIS_RECONTOSS	3	/* reconnect now! */
#define AFS_DIS_QUERY		4	/* query disconnected state */
#define AFS_DIS_FETCHONLY	5	/* disconnect, fetch-only mode */
#define AFS_DIS_PARTIAL		6	/* partially connected mode */
#define AFS_DIS_DISCONTOSS	7	/* disconnect without discarding callbacks */


/* these are items defined to fhe PSetDOps */

enum dis_setopt_op {
	GET_BACKUP_LOG_NAME,
	SET_USERLOG_LEVEL,
	SET_FILELOG_LEVEL,
	SET_LOGFILE,
	UPDATE_FLAGS,
	PING_SERVER,
	GET_LOG_NAME,
	GIVEUP_CBS,
	PRINT_INFO,
	VERIFY_VCACHE,
	VERIFY_DCACHE,
};

#define MAX_NAME  255

typedef enum dis_setopt_op dis_setopt_op_t;

typedef struct dis_setop_info {
	dis_setopt_op_t	op;
	char 	data[MAX_NAME];
} dis_setop_info_t;


#ifdef KERNEL


#define CELL_DIRTY   0x01
#define REALLY_BIG 1024

struct save_cell {
    long cell;				/* unique id assigned by venus */
    char cellName[MAX_NAME];		/* char string name of cell */
    short cellHosts[MAXHOSTS];		/* volume *location* hosts for this cell */
    short lcell;			/* Associated linked cell */
    short states;			/* state flags */
    long fsport;			/* file server port */
    long vlport;			/* volume server port */
    short cellIndex;                    /* relative index number per cell */
    short dindex;			/* disconnected index */
};

#define SERV_DIRTY 0x01

struct save_server {
    unsigned int cell; 		/* cell in which this host resides */
    long host;			/* in network byte order, except subsys */
    long portal;		/* in network byte order */
    unsigned int random;	/* server priority, used for randomizing requests */
    char isDown;		/* result of decision if server is down. */
    char vcbCount;		/* count of vcbs */
    short dindex;		/* disconnected index */
};


#define VOL_DIRTY 0x01

struct save_volume {
    long cell;                  /* the cell in which the volume resides */
    long volume;                /* This volume's ID number. */
    char name[MAX_NAME];        /* This volume's name, or 0 if unknown */
    short serverHost[MAXHOSTS];    /* servers serving this volume */
    struct VenusFid dotdot;     /* dir to access as .. */
    struct VenusFid mtpoint;    /* The mount point for this volume. */
    long rootVnode, rootUnique;	/* Volume's root fid */
    long roVol;
    long backVol;
    long rwVol;                 /* For r/o vols, original read/write volume. */
    long accessTime;            /* last time we used it */
    long copyDate;              /* copyDate field, for tracking vol releases */
    char states;                /* snuck here for alignment reasons */
    short dindex;
};


#define LLIST_SIZE  1024


#ifndef GET_WRITE
#define GET_WRITE   0x1
#define GET_READ    0x2
#define GET_SHARED  0x4

#define REL_WRITE   0x10
#define REL_READ    0x20
#define REL_SHARED  0x40

#define S_TO_W	    0x100
#define W_TO_S	    0x200
#define W_TO_R	    0x400
#define S_TO_R      0x800
#endif

struct llist {
    struct afs_lock *lk;
    short	operation;
    short	who;
    struct llist * next;
    struct llist *prev;
};


/* These are definition for the translation flags fields */

#define KNOWNBAD	0x20
#define SYMLINK		0x40
struct name_trans {
    struct VenusFid pfid;
    int ntrans_idx;
    int oname_idx;
    int nname_idx;
    int next_nt_idx;
    char *old_name;
    char *new_name;
    struct name_trans *next;
};

struct translations {
    struct VenusFid oldfid;
    struct VenusFid newfid;
    u_long flags;
    hyper validDV;
    int	trans_idx;
    long callback;
    long cbExpires;
    int	nl_idx;
    struct translations *next;
    struct name_trans *name_list;
};



/* 
 * this struct is used to help speed up finding the number of callbacks for
 * each server
 */

struct serv_cbcount {
    long  server;
    long  count;
    struct serv_cbcount *next;
};

/* Stuff for the link name persistence */

#define MAP_ENTS	100
#define	NAME_UNIT	255
#define	NAME_DIRTY	0x1

/* header for the name backing file */
typedef struct	map_header {
    long	magic;		/* magic number */
    int		num_ents;	/* number of names stored */
    char	flags;		/* flags for in core copy */
} map_header_t;

/*
 * this struct holds all the information pertaining to a certain
 * backing store used to keep persistance information
 */
typedef struct backing_store {
	long		bs_inode;	/* inode of the backing file */
	struct osi_dev	bs_dev;		/* device of backing file */
	char	 	*bs_name;	/* name of backing file */
	map_header_t	*bs_header;	/* pointer to the header */
	char 		*bs_map;	/* pointer to the header */
	struct afs_lock	bs_lock;	/* lock to synchronize access */
	struct osi_file	*tfile;		/* non-null if kept open */
} backing_store_t;

#endif KERNEL

/* CacheItems file has a header of type struct afs_fheader (keep aligned properly) */
struct afs_dheader {
    long magic;
    long firstCSize;
    long otherCSize;
    long spare;
    long current_op;
    discon_modes_t mode;
};

#ifdef KERNEL

#define have_shared_lock(lock) \
	((((lock)->excl_locked==SHARED_LOCK) &&  \
	   ((lock)->proc == osi_curpid())) ?  1 :  0)

#define have_write_lock(lock) \
	((((lock)->excl_locked==WRITE_LOCK) &&  \
		((lock)->proc == osi_curpid())) ? 1 :  0)

extern struct llist *llist;
extern struct llist *cur_llist;

/* these are function declarations so I can compile with -Wall in gcc,
 * not really needed, but help make clean compiles.
 */

extern int strlen();
#ifndef AFS_NETBSD_ENV
extern void strcpy();
#endif	/* AFS_NETBSD_ENV */
extern void bcopy();
extern int dir_Delete();
extern int dir_FindBlobs();
extern int dir_Lookup();
extern int dir_Create();
extern int find_file_name();
extern int afs_create();
extern int afs_PutDCache();

#endif KERNEL

#endif  _DISCONNH


#ifdef DISCONN
/*
This copyright pertains to modifications set off by the compile
option DISCONN
****************************************************************************
*Copyright 1993 Regents of The University of Michigan - All Rights Reserved*
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appears in all copies and that  *
* both that copyright notice and this permission notice appear in          *
* supporting documentation, and that the name of The University of Michigan*
* not be used in advertising or publicity pertaining to distribution of    *
* the software without specific, written prior permission. This software   *
* is supplied as is without expressed or implied warranties of any kind.   *
*                                                                          *
*            Center for Information Technology Integration                 *
*                  Information Technology Division                         *
*                     The University of Michigan                           *
*                       535 W. William Street                              *
*                        Ann Arbor, Michigan                               *
*                            313-763-4403                                  *
*                        info@citi.umich.edu                               *
*                                                                          *
****************************************************************************
*/
#endif

