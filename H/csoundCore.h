#ifndef CSOUNDCORE_H
#define CSOUNDCORE_H

#ifdef __cplusplus
extern "C" {
#endif

  /*
    csoundCore.h:

    Copyright (C) 1991-2003 Barry Vercoe, John ffitch

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
  */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#ifdef HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#include "sysdep.h"
#include "cwindow.h"
#include "opcode.h"
#include "fft.h"
#include "version.h"
#include <sndfile.h>
#include "csound.h"

#define OK        (0)
#define NOTOK     (-1)

/* IV - Feb 19 2005: value to pass to longjmp() to return with success */
/* (e.g. after --help or running an utility) */
#define CSOUND_EXITJMP_SUCCESS  256

#define INSTR     1
#define ENDIN     2
#define OPCODE    3
#define ENDOP     4
#define LABEL     5
#define SETBEG    6
#define STRSET    6
#define PSET      7
#define SETEND    8
  /* #define VSET      6 */
  /* #define GVSET     7 */
  /* #define VDIM      8 */
  /* #define VPROGS    9 */
  /* #define DVPROGS   10 */
  /* #define PGMINIT   17 */
  /* #define DPGMINIT  18 */

#define MAXINSNO   (200)
#define PMAX       (1000)
#define VARGMAX    (1001)
#define TOKMAX  50L             /* Should be 50 but bust */
  /* IV - Oct 24 2002: max number of input/output args for user defined opcodes */
#define OPCODENUMOUTS   24

#define ORTXT      h.optext->t
#define INCOUNT    ORTXT.inlist->count
#define OUTCOUNT   ORTXT.outlist->count
#define INOCOUNT   ORTXT.inoffs->count
#define OUTOCOUNT  ORTXT.outoffs->count
#define XINCODE    ORTXT.xincod
#  define XINARG1  (p->XINCODE & 2)
#  define XINARG2  (p->XINCODE & 1)
#  define XINARG3  (p->XINCODE & 4)
#  define XINARG4  (p->XINCODE & 8)
#define XOUTCODE   ORTXT.xoutcod        /* IV - Sep 1 2002 */
#define STRARG     ORTXT.strargs[0]
#define STRARG2    ORTXT.strargs[1]
#define STRARG3    ORTXT.strargs[2]
#define STRARG4    ORTXT.strargs[3]

#define MAXLEN     0x1000000L
#define FMAXLEN    ((MYFLT)(MAXLEN))
#define PHMASK     0x0FFFFFFL
#define PFRAC(x)   ((MYFLT)((x) & ftp->lomask) * ftp->lodiv)
#define MAXPOS     0x7FFFFFFFL

#define BYTREVS(n) ((n>>8  & 0xFF) | (n<<8 & 0xFF00))
#define BYTREVL(n) ((n>>24 & 0xFF) | (n>>8 & 0xFF00L) | \
                    (n<<8 & 0xFF0000L) | (n<<24 & 0xFF000000L))

#define OCTRES     8192
#define CPSOCTL(n) ((MYFLT)(1<<((int)(n)>>13))*csound->cpsocfrc[(int)(n)&8191])

#define LOBITS     10
#define LOFACT     1024
  /* LOSCAL is 1/LOFACT as MYFLT */
#define LOSCAL     FL(0.0009765625)

#define LOMASK     1023

#define SSTRCOD    3945467
#define SSTRSIZ    200
#define ALLCHNLS   0x7fff
#define DFLT_SR    FL(44100.0)
#define DFLT_KR    FL(4410.0)
#define DFLT_KSMPS 10
#define DFLT_NCHNLS 1
#define MAXCHNLS   256

#define MAXNAME 128

#define DFLT_DBFS (FL(32767.0))

  void dbfs_init(MYFLT dbfs);

  typedef struct {
    int     odebug, initonly;
    int     sfread, sfwrite, sfheader, filetyp;
    int     inbufsamps, outbufsamps;
    int     informat, outformat;
    int     insampsiz, sfsampsize;
    int     displays, graphsoff, postscript, msglevel;
    int     Beatmode, cmdTempo, oMaxLag;
    int     usingcscore, Linein, Midiin, FMidiin;
    int     OrcEvts;        /* - for triginstr (re Aug 1999) */
    int     RTevents, ksensing;
    int     ringbell, termifend, stdoutfd;
    int     rewrt_hdr, heartbeat, gen01defer;
    long    sr_override, kr_override;
    long    instxtcount, optxtsize;
    long    poolcount, gblfixed, gblacount;
    long    argoffsize, strargsize, filnamsize;
    char    *argoffspace, *strargspace, *filnamspace;
    char    *infilename, *outfilename, *playscore;
    char    *Linename, *Midiname, *FMidiname;
    char    *Midioutname;   /* jjk 09252000 - MIDI output device, -Q option */
    int     expr_opt;       /* IV - Jan 27 2005: for --expression-opt */
  } OPARMS;

#define  ONEPT          1.02197486             /* A440 tuning factor */
#define  LOG10D20       0.11512925             /* for db to ampfac   */
#define  DV32768        FL(0.000030517578125)

  typedef struct polish {
    char    opcod[12];
    int     incount;
    char    *arg[4];     /* Was [4][12] */
  } POLISH;

  typedef struct arglst {
    int     count;
    char    *arg[1];
  } ARGLST;

  typedef struct argoffs {
    int     count;
    int     indx[1];
  } ARGOFFS;

  /* Storage for parsed orchestra code, for each opcode in an INSTRTXT. */
  typedef struct text {
    int     linenum;        /* Line num in orch file (currently buggy!)  */
    int     opnum;          /* Opcode index in opcodlst[] */
    char    *opcod;         /* Pointer to opcode name in global pool */
    char    *strargs[4];    /* (Unquoted) array of file names if opcode uses */
    ARGLST  *inlist;        /* Input args (pointer to item in name list) */
    ARGLST  *outlist;
    ARGOFFS *inoffs;        /* Input args (index into list of values) */
    ARGOFFS *outoffs;
    int     xincod;         /* Rate switch for multi-rate opcode functions */
    int     xoutcod;        /* output rate switch (IV - Sep 1 2002) */
    char    intype;         /* Type of first input argument (g,k,a,w etc) */
    char    pftype;         /* Type of output argument (k, a etc) */
  } TEXT;

  /* This struct is filled out by otran() at orch parse time.
     It is used as a template for instrument events. */
  typedef struct instr {
    struct op * nxtop;              /* Linked list of instr opcodes */
    TEXT    t;                      /* Text of instrument (same in nxtop) */
    int     pmax, vmax, pextrab;    /* Arg count, size of data for all
                                       opcodes in instr */
    int     mdepends;               /* Opcode type (i/k/a) */
    int     lclkcnt, lcldcnt;       /* Storage reqs for this instr */
    int     lclwcnt, lclacnt;
    int     lclpcnt;
    int     lclfixed, optxtcount;
    short   muted;
    long    localen;
    long    opdstot;                /* Total size of opds structs in instr */
    long    *inslist;               /* Only used in parsing (?) */
    MYFLT   *psetdata;              /* Used for pset opcode */
    struct insds * instance;        /* Chain of allocated instances of
                                       this instrument */
    struct insds * lst_instance, *act_instance;     /* IV - Oct 26 2002 */
    struct instr * nxtinstxt;       /* Next instrument in orch (num order) */
    int     active;                 /* To count activations for control */
    int     maxalloc;
    MYFLT   cpuload;                /* % load this instrumemnt makes */
    struct opcodinfo *opcode_info;  /* IV - Nov 10 2002 */
    char    *insname;               /* instrument name */
  } INSTRTXT;


  /* A chain of TEXT structs. Note that this is identical with the first two
     members of struct INSTRTEXT, and is so typecast at various points in code. */
  typedef struct op {
    struct op * nxtop;
    TEXT    t;
  } OPTXT;

  typedef struct fdch {
    struct fdch * nxtchp;
    void   *fd;             /* Should be SNDFILE * */
    int    fdc;
  } FDCH;

  typedef struct auxch {
    struct auxch * nxtchp;
    long   size;
    void   *auxp, *endp;    /* was char* */
  } AUXCH;

  typedef struct monblk {
    short  pch;
    struct monblk *prv;
  } MONPCH;

  typedef struct {
    int     notnum[4];
  } DPEXCL;

  typedef struct {
    DPEXCL dpexcl[8];
    int    exclset[75];     /* for keys 25-99 */
  } DPARM;

  typedef struct dklst {
    struct dklst *nxtlst;
    long   pgmno;
    MYFLT  keylst[1];       /* cnt + keynos */
  } DKLST;

#define DKBAS  25

  typedef struct mchnblk {
    short  pgmno;           /* most recently received program change */
    short  insno;           /* instrument number assigned to this channel */
    short  RegParNo;
    short  mono;
    MONPCH *monobas;
    MONPCH *monocur;
    struct insds *kinsptr[128]; /* list of active notes (NULL: not active) */
    MYFLT  polyaft[128];    /* polyphonic pressure indexed by note number */
    MYFLT  ctl_val[136];    /* ... with GS vib_rate, stored in c128-c135 */
    short  pgm2ins[128];    /* program change to instr number (<=0: ignore) */
    short  ksuscnt;         /* number of held (sustaining) notes */
    short  sustaining;      /* current state of sustain pedal (0: off) */
    MYFLT  aftouch;         /* channel pressure (0-127) */
    MYFLT  pchbend;         /* pitch bend (-1 to 1) */
    MYFLT  pbensens;        /* pitch bend sensitivity in semitones */
    DKLST  *klists;         /* chain of dpgm keylists */
    DPARM  *dparms;         /* drumset params     */
    int    dpmsb;
    int    dplsb;
    int    datenabl;
  } MCHNBLK;

  /*
   *       Forward declaration.
   */
  struct ENVIRON_;

  /* This struct holds the info for a concrete instrument event
     instance in performance. */
  typedef struct insds {
    struct opds * nxti;             /* Chain of init-time opcodes */
    struct opds * nxtp;             /* Chain of performance-time opcodes */
    struct insds * nxtinstance;     /* Next allocated instance */
    struct insds * prvinstance;     /* Previous allocated instance */
    struct insds * nxtact;          /* Next in list of active instruments */
    struct insds * prvact;  /* Previous in list of active instruments */
    struct insds * nxtoff;  /* Next instrument to terminate */
    FDCH    fdch;           /* Chain of files used by opcodes in this instr */
    AUXCH   auxch;          /* Extra memory used by opcodes in this instr */
    MCHNBLK *m_chnbp;       /* MIDI note info block if event started
                               from MIDI */
    int     xtratim;        /* Extra release time requested with
                               xtratim opcode */
    short   insno;          /* Instrument number */
    short   m_sust;         /* non-zero for sustaining MIDI note */
    unsigned char m_pitch;  /* MIDI pitch, for simple access */
    unsigned char m_veloc;  /* ...ditto velocity */
    char    relesing;       /* Flag to indicate we are releasing,
                               test with release opcode */
    char    actflg;         /* Set if instr instance is active (perfing) */
    double  offbet;         /* Time to turn off event, in score beats */
    double  offtim;         /* Time to turn off event, in seconds (negative on
                               indef/tie) */
    struct insds * nxtolap; /* ptr to next overlapping MIDI voice */
    void   *pylocal;        /* Python namespace for just this instance. */
    struct ENVIRON_ *csound;/* ptr to Csound engine and API for externals */
    void    *opcod_iobufs;  /* IV - Sep 8 2002: user opcode I/O buffers */
    void    *opcod_deact, *subins_deact;    /* IV - Oct 24 2002 */
    MYFLT   p0;             /* Copy of required p-field values for
                               quick access */
    MYFLT   p1;
    MYFLT   p2;
    MYFLT   p3;
  } INSDS;

  typedef int    (*SUBR)(void *, void *);

  /* This struct holds the info for one opcode in a concrete
     instrument instance in performance. */
  typedef struct opds {
    struct opds * nxti;     /* Next opcode in init-time chain */
    struct opds * nxtp;     /* Next opcode in perf-time chain */
    SUBR    iopadr;         /* Initialization (i-time) function pointer */
    SUBR    opadr;          /* Perf-time (k- or a-rate) function pointer */
    /**
     * Deinitialization function pointer;
     * if not null, called during cleanup on each opcode instance;
     * useful for deallocating memory or other resources managed by the opcode.
     */
    SUBR    dopadr;         /* Deinitialization function pointer */
    OPTXT   *optext;        /* Orch file template part for this opcode */
    INSDS   *insdshead;     /* Owner instrument instance data structure */
  } OPDS;

  typedef struct lblblk {
    OPDS    h;
    OPDS    *prvi;
    OPDS    *prvp;
  } LBLBLK;

  typedef struct oentry {
    char    *opname;
    unsigned short  dsblksiz;
    unsigned short  thread;
    char    *outypes;
    char    *intypes;
    SUBR    iopadr;
    SUBR    kopadr;
    SUBR    aopadr;
    /**
     * Deinitialization function pointer;
     * if not null, called during cleanup on each opcode instance;
     * useful for deallocating memory or other resources managed by the opcode.
     */
    SUBR    dopadr;
    void    *useropinfo;    /* IV - Oct 12 2002: user opcode parameters */
    int     prvnum;         /* IV - Oct 31 2002 */
  } OENTRY;

  typedef struct {
    char *word;
    void (*fn)(void);
  } NGFENS;

  typedef struct {
    MYFLT   *begp, *curp, *endp, feedback[6];
    long    scount;
  } OCTDAT;

#define MAXOCTS 8

  typedef struct {
    long    npts, nocts, nsamps;
    MYFLT   lofrq, hifrq, looct, srate;
    OCTDAT  octdata[MAXOCTS];
    AUXCH   auxch;
  } DOWNDAT;

  typedef struct {
    long    ktimstamp, ktimprd;
    long    npts, nfreqs, dbout;
    DOWNDAT *downsrcp;
    AUXCH   auxch;
  } SPECDAT;

#define AIFF_MAXCHAN 8
  typedef struct {
    MYFLT   natcps;
    MYFLT   gainfac;
    short   loopmode1;
    short   loopmode2;
    long    begin1, end1;
    long    begin2, end2;
    MYFLT   fmaxamps[AIFF_MAXCHAN+1];
  } AIFFDAT;

  typedef struct {
    MYFLT gen01;
    MYFLT ifilno;
    MYFLT iskptim;
    MYFLT iformat;
    MYFLT channel;
    MYFLT sample_rate;
    char  strarg[SSTRSIZ];
  } GEN01ARGS;

  typedef struct {
    long    flen;
    long    lenmask;
    long    lobits;
    long    lomask;
    MYFLT   lodiv;
    MYFLT   cvtbas, cpscvt;
    short   loopmode1;
    short   loopmode2;
    long    begin1, end1;   /* all these in ..  */
    long    begin2, end2;
    long    soundend, flenfrms; /* .. sample frames */
    long    nchanls;
    long    fno;
    GEN01ARGS gen01args;
    MYFLT   ftable[1];
  } FUNC;

  typedef struct MEMFIL {
    char    filename[256];  /* Made larger RWD */
    char    *beginp;
    char    *endp;
    long    length;
    struct MEMFIL  *next;
  } MEMFIL;

  /* This struct holds the data for one score event. */
  typedef struct event {
    char    *strarg;        /* Original argument list string of event */
    char    opcod;          /* Event type */
    short   pcnt;           /* Number of p-fields */
    MYFLT   p2orig;         /* Event start time */
    MYFLT   p3orig;         /* Length */
    MYFLT   p[PMAX+1];      /* All p-fields for this event */
  } EVTBLK;

  typedef struct eventnode {
    struct eventnode  *nxt;
    unsigned long     start_kcnt;
    EVTBLK            evt;
  } EVTNODE;

  typedef struct {
    EVTBLK  e;
    double  tpdlen;
    int     fno, guardreq, fterrcnt;
    long    flen, flenp1, lenmask;
  } FGDATA;

  typedef struct {
    OPDS    h;
    MYFLT   *ktempo, *istartempo;
    MYFLT   prvtempo;
  } TEMPO;

  typedef struct opcodinfo {              /* IV - Oct 24 2002 */
    long    instno;
    char    *name, *intypes, *outtypes;
    short   inchns, outchns, perf_incnt, perf_outcnt;
    short   *in_ndx_list, *out_ndx_list;
    INSTRTXT *ip;
    struct  opcodinfo *prv;
  } OPCODINFO;

  typedef void (*RSET)(struct ENVIRON_*);
  typedef struct resetter {
    RSET            fn;
    struct resetter *next;
  } RESETTER;

#define MAXCHAN 16      /* 16 MIDI channels; only one port for now */

#include "sort.h"
#include "midiops2.h"
  typedef void    (*GEN)(FUNC *, struct ENVIRON_ *);

  /* sensevents() state */
  typedef struct {
    double  prvbt, curbt, nxtbt;  /* previous, current, and next score beat */
    double  curp2, nxtim;         /* current and next score time (seconds)  */
    double  timeOffs, beatOffs;   /* start time of current section          */
    double  curTime, curTime_inc; /* cur. time in seconds, inc. per kperiod */
    double  curBeat, curBeat_inc; /* current time in beats, inc per kperiod */
    double  beatTime;             /* beat time = 60 / tempo                 */
    int     cyclesRemaining;      /* number of k-periods to kperf() before  */
                                  /*   next score event                     */
    EVTBLK  evt;                  /* current score event                    */
  } sensEvents_t;

  /* MIDI globals */

#define MBUFSIZ         (4096)
#define MIDIINBUFMAX    (1024)
#define MIDIINBUFMSK    (MIDIINBUFMAX-1)

  typedef union {
    unsigned long dwData;
    unsigned char bData[4];
  } MIDIMESSAGE;

  typedef struct {
    short  type;
    short  chan;
    short  dat1;
    short  dat2;
  } MEVENT;

  typedef struct midiglobals {
    MEVENT  *Midevtblk;
    int     sexp;
    int     MIDIoutDONE;
    int     MIDIINbufIndex;
    MIDIMESSAGE MIDIINbuffer2[MIDIINBUFMAX];
    int     (*MidiInOpenCallback)(void*, void**, const char*);
    int     (*MidiReadCallback)(void*, void*, unsigned char*, int);
    int     (*MidiInCloseCallback)(void*, void*);
    int     (*MidiOutOpenCallback)(void*, void**, const char*);
    int     (*MidiWriteCallback)(void*, void*, unsigned char*, int);
    int     (*MidiOutCloseCallback)(void*, void*);
    char    *(*MidiErrorStringCallback)(int);
    void    *midiInUserData;
    void    *midiOutUserData;
    void    *midiFileData;
    int     rawControllerMode;
    char    muteTrackList[256];
    unsigned char mbuf[MBUFSIZ];
    unsigned char *bufp, *endatp;
    short   datreq, datcnt;
  } MGLOBAL;

  typedef struct token {
        char    *str;
        short   prec;
  } TOKEN;


  typedef struct ENVIRON_
  {
    int (*GetVersion)(void);
    int (*GetAPIVersion)(void);
    void *(*GetHostData)(void *csound);
    void (*SetHostData)(void *csound, void *hostData);
    int (*Perform)(void *csound, int argc, char **argv);
    int (*Compile)(void *csound, int argc, char **argv);
    int (*PerformKsmps)(void *csound);
    int (*PerformBuffer)(void *csound);
    void (*Cleanup)(void *csound);
    void (*Reset)(void *csound);
    MYFLT (*GetSr)(void *csound);
    MYFLT (*GetKr)(void *csound);
    int (*GetKsmps)(void *csound);
    int (*GetNchnls)(void *csound);
    int (*GetSampleFormat)(void *csound);
    int (*GetSampleSize)(void *csound);
    long (*GetInputBufferSize)(void *csound);
    long (*GetOutputBufferSize)(void *csound);
    void *(*GetInputBuffer)(void *csound);
    void *(*GetOutputBuffer)(void *csound);
    MYFLT *(*GetSpin)(void *csound);
    MYFLT *(*GetSpout)(void *csound);
    MYFLT (*GetScoreTime)(void *csound);
    MYFLT (*GetProgress)(void *csound);
    MYFLT (*GetProfile)(void *csound);
    MYFLT (*GetCpuUsage)(void *csound);
    int (*IsScorePending)(void *csound);
    void (*SetScorePending)(void *csound, int pending);
    MYFLT (*GetScoreOffsetSeconds)(void *csound);
    void (*SetScoreOffsetSeconds)(void *csound, MYFLT offset);
    void (*RewindScore)(void *csound);
    void (*Message)(void *csound, const char *format, ...);
    void (*MessageV)(void *csound, const char *format, va_list args);
    void (*ThrowMessage)(void *csound, const char *format, ...);
    void (*ThrowMessageV)(void *csound, const char *format, va_list args);
    void (*SetMessageCallback)(void *csound,
                               void (*csoundMessageCallback)(void *hostData,
                                                             const char *format,
                                                             va_list valist));
    void (*SetThrowMessageCallback)(void *csound,
                     void (*throwMessageCallback)(void *hostData,
                                                  const char *format,
                                                  va_list valist));
    int (*GetMessageLevel)(void *csound);
    void (*SetMessageLevel)(void *csound, int messageLevel);
    void (*InputMessage)(void *csound, const char *message__);
    void (*KeyPress)(void *csound, char c__);
    void (*SetInputValueCallback)(void *csound,
                                  void (*inputValueCalback)(void *hostData,
                                                            char *channelName,
                                                            MYFLT *value));
    void (*SetOutputValueCallback)(void *csound,
                   void (*outputValueCalback)(void *hostData,
                                                              char *channelName,
                                                              MYFLT value));
    int (*ScoreEvent)(void *csound, char type, MYFLT *pFields, long numFields);
    void (*SetExternalMidiInOpenCallback)(void *csound,
                                          int (*func)(void*, void**,
                                                      const char*));
    void (*SetExternalMidiReadCallback)(void *csound,
                                        int (*func)(void*, void*,
                                                    unsigned char*, int));
    void (*SetExternalMidiInCloseCallback)(void *csound,
                                           int (*func)(void*, void*));
    void (*SetExternalMidiOutOpenCallback)(void *csound,
                                              int (*func)(void*, void**,
                                                          const char*));
    void (*SetExternalMidiWriteCallback)(void *csound,
                                         int (*func)(void*, void*,
                                                     unsigned char*, int));
    void (*SetExternalMidiOutCloseCallback)(void *csound,
                                            int (*func)(void*, void*));
    void (*SetExternalMidiErrorStringCallback)(void *csound,
                                               char *(*func)(int));
    void (*SetIsGraphable)(void *csound, int isGraphable);
    void (*SetMakeGraphCallback)(void *csound,
                                 void (*makeGraphCallback)(void *hostData,
                                                           WINDAT *p,
                                                           char *name));
    void (*SetDrawGraphCallback)(void *csound,
                                 void (*drawGraphCallback)(void *hostData,
                                                           WINDAT *p));
    void (*SetKillGraphCallback)(void *csound,
                                 void (*killGraphCallback)(void *hostData,
                                                           WINDAT *p));
    void (*SetExitGraphCallback)(void *csound,
                                 int (*exitGraphCallback)(void *hostData));
    opcodelist *(*NewOpcodeList)(void);
    void (*DisposeOpcodeList)(opcodelist *opcodelist_);
    int (*AppendOpcode)(void *csound, char *opname, int dsblksiz,
                        int thread, char *outypes, char *intypes,
                        int (*iopadr)(void*, void*),
                        int (*kopadr)(void*, void*),
                        int (*aopadr)(void*, void*),
                        int (*dopadr)(void*, void*));
    int (*LoadExternal)(void *csound, const char *libraryPath);
    int (*LoadExternals)(void *csound);
    void *(*OpenLibrary)(const char *libraryPath);
    void *(*CloseLibrary)(void *library);
    void *(*GetLibrarySymbol)(void *library, const char *procedureName);
    void (*SetYieldCallback)(void *csound,
                             int (*yieldCallback)(void *hostData));
    char *(*GetEnv)(void *csound, const char *name);
    char *(*FindInputFile)(void *csound,
                           const char *filename, const char *envList);
    char *(*FindOutputFile)(void *csound,
                            const char *filename, const char *envList);
    void (*SetPlayopenCallback)(void *csound,
                                int (*playopen__)(void *csound,
                                                  csRtAudioParams *parm));
    void (*SetRtplayCallback)(void *csound,
                              void (*rtplay__)(void *csound, void *outBuf,
                                               int nbytes));
    void (*SetRecopenCallback)(void *csound,
                               int (*recopen__)(void *csound,
                                                csRtAudioParams *parm));
    void (*SetRtrecordCallback)(void *csound,
                                int (*rtrecord__)(void *csound, void *inBuf,
                                                  int nbytes));
    void (*SetRtcloseCallback)(void *csound, void (*rtclose__)(void *csound));
    void (*AuxAlloc)(void *csound, long nbytes, AUXCH *auxchp);
    FUNC *(*FTFind)(void *csound, MYFLT *argp);
    FUNC *(*FTFindP)(void *csound_, MYFLT *argp);
    FUNC *(*FTnp2Find)(void *csound_, MYFLT *argp);
    MYFLT *(*GetTable)(void *csound_, int tableNum, int *tableLength);
    void *(*Malloc)(void *csound, size_t nbytes);
    void *(*Calloc)(void *csound, size_t nbytes);
    void *(*ReAlloc)(void *csound, void *oldp, size_t nbytes);
    void (*Free)(void *csound, void *ptr);
    void (*Die)(void *csound, const char *msg, ...);
    int (*InitError)(void *csound, const char *msg, ...);
    int (*PerfError)(void *csound, const char *msg, ...);
    void (*Warning)(void *csound, const char *msg, ...);
    void (*DebugMsg)(void *csound, const char *msg, ...);
    /* Internal functions that are needed */
    void (*dispset)(WINDAT *, MYFLT *, long, char *, int, char *);
    void (*display)(WINDAT *);
    MYFLT (*intpow_)(MYFLT, long);
    char *(*unquote_)(char *);
    MEMFIL *(*ldmemfile_)(void*, const char*);
    void (*err_printf_)(char *, ...);
    FUNC *(*hfgens_)(struct ENVIRON_*, EVTBLK *);
    int (*IsPowerOfTwo_)(long);
    void (*FFT2torlpacked_)(complex *, long, MYFLT, complex *);
    void (*FFT2realpacked_)(complex *, long, complex *);
    void (*cxmult_)(complex *,complex *,long);
    int (*getopnum_)(char *s);
    long (*strarg2insno_)(struct ENVIRON_ *csound, MYFLT *p, char *s);
    long (*strarg2opcno_)(struct ENVIRON_ *csound, MYFLT *p, char *s,
                                                   int force_opcode);
    INSDS *(*instance_)(int insno);
    void (*rewriteheader_)(SNDFILE *ofd, int verbose);
    void (*writeheader)(int ofd, char *ofname);
    void (*Printf)(const char *format, ...);
    int (*PerformKsmpsAbsolute_)(void *csound);
    int (*GetDebug)(void *csound);
    void (*SetDebug)(void *csound, int d);
    int (*TableLength)(void *csound, int table);
    MYFLT (*TableGet)(void *csound, int table, int index);
    void (*TableSet)(void *csound, int table, int index, MYFLT value);
    void *(*CreateThread)(void *csound, int (*threadRoutine)(void *userdata),
                          void *userdata);
    int (*JoinThread)(void *csound, void *thread);
    void *(*CreateThreadLock)(void *csound);
    void (*WaitThreadLock)(void *csound, void *lock, size_t milliseconds);
    void (*NotifyThreadLock)(void *csound, void *lock);
    void (*DestroyThreadLock)(void *csound, void *lock);
    void (*SetFLTKThreadLocking)(void *csound, int isLocking);
    int (*GetFLTKThreadLocking)(void *csound);
    /* IV - Jan 27 2005: new functions */
    void (*timers_struct_init)(RTCLOCK*);
    double (*timers_get_real_time)(RTCLOCK*);
    double (*timers_get_CPU_time)(RTCLOCK*);
    unsigned long (*timers_random_seed)(void);
    char *(*LocalizeString)(const char*);
    int (*CreateGlobalVariable)(void *csound, const char *name, size_t nbytes);
    void *(*QueryGlobalVariable)(void *csound, const char *name);
    void *(*QueryGlobalVariableNoCheck)(void *csound, const char *name);
    int (*DestroyGlobalVariable)(void *csound, const char *name);
    int (*CreateConfigurationVariable)(void *csound, const char *name,
                                       void *p, int type, int flags,
                                       void *min, void *max,
                                       const char *shortDesc,
                                       const char *longDesc);
    int (*SetConfigurationVariable)(void *csound, const char *name, void *value);
    int (*ParseConfigurationVariable)(void *csound, const char *name,
                                      const char *value);
    csCfgVariable_t *(*QueryConfigurationVariable)(void *csound,
                                                   const char *name);
    csCfgVariable_t **(*ListConfigurationVariables)(void *csound);
    int (*DeleteConfigurationVariable)(void *csound, const char *name);
    char *(*CfgErrorCodeToString)(int errcode);
    int (*GetSizeOfMYFLT)(void);
    void **(*GetRtRecordUserData)(void *csound);
    void **(*GetRtPlayUserData)(void *csound);
    MYFLT (*GetInverseComplexFFTScale)(void *csound, int FFTsize);
    MYFLT (*GetInverseRealFFTScale)(void *csound, int FFTsize);
    void (*ComplexFFT)(void *csound, MYFLT *buf, int FFTsize);
    void (*InverseComplexFFT)(void *csound, MYFLT *buf, int FFTsize);
    void (*RealFFT)(void *csound, MYFLT *buf, int FFTsize);
    void (*InverseRealFFT)(void *csound, MYFLT *buf, int FFTsize);
    void (*RealFFTMult)(void *csound, MYFLT *outbuf, MYFLT *buf1, MYFLT *buf2,
                        int FFTsize, MYFLT scaleFac);
    /* real-time audio callbacks */
    int (*playopen_callback)(void *csound, csRtAudioParams *parm);
    void (*rtplay_callback)(void *csound, void *outBuf, int nbytes);
    int (*recopen_callback)(void *csound, csRtAudioParams *parm);
    int (*rtrecord_callback)(void *csound, void *inBuf, int nbytes);
    void (*rtclose_callback)(void *csound);
    void (*InputValueCallback_)(void *csound, char *channelName, MYFLT *value);
    void (*OutputValueCallback_)(void *csound, char *channelName, MYFLT value);
    /* End of internals */
    OPDS          *ids, *pds;       /* used by init and perf loops */
    int           ksmps, nchnls;
    MYFLT         esr, ekr;
    int           global_ksmps;
    MYFLT         global_ensmps, global_ekr, global_onedkr;
    MYFLT         global_hfkprd, global_kicvt;
    MYFLT         cpu_power_busy;
    long          global_kcounter;
    char          *orchname_, *scorename_, *xfilename_;
    MYFLT         e0dbfs;
    /* oload.h */
    RESETTER      *reset_list_;
    short         nlabels;
    short         ngotos;
    int           strsmax_;
    char          **strsets_;
    int           peakchunks_;
    MYFLT         *zkstart_;
    MYFLT         *zastart_;
    long          zklast_;
    long          zalast_;
    long          kcounter_;
    EVTBLK        *currevent_;
    MYFLT         onedkr_;
    MYFLT         onedsr_;
    MYFLT         kicvt_;
    MYFLT         sicvt_;
    MYFLT         *spin_;
    MYFLT         *spout_;
    int           nspin_;
    int           nspout_;
    int           spoutactive_;
    int           keep_tmp_;
    int           dither_output_;
    OENTRY        *opcodlst_;
    void          *opcode_list;     /* IV - Oct 31 2002 */
    OENTRY        *oplstend_;
    long          holdrand_;
    int           maxinsno_;
    int           maxopcno_;        /* IV - Oct 24 2002 */
    INSDS         *curip_;
    EVTBLK        *Linevtblk_;
    long          nrecs;
    FILE*         Linepipe;
    int           Linefd_;
    MYFLT         *ls_table_;
    MYFLT         curr_func_sr_;
    char          *retfilnam_;
    INSTRTXT      **instrtxtp_;
#define ERRSIZ (200)
    char          errmsg_[ERRSIZ];   /* sprintf space for compiling msgs */
    FILE*         scfp_;
    FILE*         oscfp_;
    MYFLT         maxamp_[MAXCHNLS];
    MYFLT         smaxamp_[MAXCHNLS];
    MYFLT         omaxamp_[MAXCHNLS];
    MYFLT         *maxampend_;
    unsigned long maxpos_[MAXCHNLS], smaxpos_[MAXCHNLS], omaxpos_[MAXCHNLS];
    int           reinitflag;
    int           tieflag;
    FILE*         scorein_;
    FILE*         scoreout_;
    MYFLT         ensmps_, hfkprd_;
    MYFLT         *pool_;
    int           *argoffspace;
    INSDS         *frstoff;
    jmp_buf       exitjmp;
    SRTBLK        *frstbp;
    int           sectcnt;
    MCHNBLK       *m_chnbp[MAXCHAN];
    MYFLT         *cpsocfrc;
    int           inerrcnt_, synterrcnt_, perferrcnt_;
    char          strmsg_[100];
    INSTRTXT      instxtanchor_;
    INSDS         actanchor_;
    long          rngcnt[MAXCHNLS];
    short         rngflg, multichan;
    EVTNODE       *OrcTrigEvts;             /* List of events to be started */
    EVTNODE       *freeEvtNodes;
    char          name_full[256];           /* Remember name used */
    int           Mforcdecs, Mxtroffs, MTrkend;
    MYFLT         tran_sr_,tran_kr_,tran_ksmps_;
    MYFLT         tran_0dbfs_;
    int           tran_nchnls_;
    MYFLT         tpidsr_, pidsr_, mpidsr_, mtpdsr_;
    OPARMS        *oparms;
    void          *hostdata;
    OPCODINFO     *opcodeInfo;      /* IV - Oct 20 2002 */
    void          *instrumentNames;
    void          *strsav_str;
    void          *strsav_space;
    MYFLT         dbfs_to_float;
    unsigned int  rtin_dev;
    char *        rtin_devs;
    unsigned int  rtout_dev;
    char *        rtout_devs;
    int           displop4;
    void          *file_opened_;
    int           file_max_;
    int           file_num_;
    int           nchanik;
    MYFLT*        chanik;
    int           nchania;
    MYFLT*        chania;
    int           nchanok;
    MYFLT*        chanok;
    int           nchanoa;
    MYFLT*        chanoa;
    FGDATA        ff;
    FUNC**        flist;
    int           maxfnum;
    GEN           *gensub;
    int           genmax;
    int           ftldno;
    int           doFLTKThreadLocking;
    void          **namedGlobals;         /* IV - Jan 28 2005 */
    int           namedGlobalsCurrLimit;
    int           namedGlobalsMaxLimit;
    void          **cfgVariableDB;
    sensEvents_t  sensEvents_state;
    void          *rtRecord_userdata;
    void          *rtPlay_userdata;
    void          *memalloc_db;
    MGLOBAL       *midiGlobals;
    void          *envVarDB;
    int           evt_poll_cnt;
    int           evt_poll_maxcnt;
    MEMFIL        *memfiles;
    MEMFIL        *rwd_memfiles;
    int           FFT_max_size;
    void          *FFT_table_1;
    void          *FFT_table_2;
    /* statics from twarp.c should be TSEG* */
    void          *tseg, *tpsave, *tplim;
    long          fout_kreset;
    /* Statics from express.c */
    long          polmax;
    long          toklen;
    char          *tokenstring;
    POLISH        *polish;
    TOKEN         *token;
    TOKEN         *tokend;
    TOKEN         *tokens;
    TOKEN         **tokenlist;
    int           toklength;
    int           acount, kcount, icount, Bcount, bcount;
    char          *stringend;
    TOKEN         **revp, **pushp, **argp, **endlist;
    char          *assign_outarg;
    int           argcnt_offs, opcode_is_assign, assign_type;
  } ENVIRON;

#include "text.h"

#if defined(mac_classic) || defined(SYMANTEC)
# define __cdecl
#endif

#if (!defined(__BEOS__) || defined(__MWERKS__)) && !(defined(__CYGWIN__) || defined(WIN32))
#  define __cdecl
#endif

#include "prototyp.h"
  extern void err_printf(char *, ...);

#ifdef MSVC /* VL MSVC fix */
  __declspec(dllexport) extern void csoundPrintf(const char *, ...);
#else
  extern void csoundPrintf(const char *, ...);
#endif

#ifdef WIN32
#define tmpnam mytmpnam
  char *mytmpnam(char *);
#endif

#ifndef PI
#define PI      (3.14159265358979323846)
#endif
#define TWOPI   (6.28318530717958647692)
#define PI_F    ((MYFLT) PI)
#define TWOPI_F ((MYFLT) TWOPI)

#define WARNMSG 04

/*
 * Move the C++ guards to enclose the entire file,
 * in order to enable C++ to #include this file.
 */

#ifdef __cplusplus
};
#endif

#endif  /*      CSOUNDCORE_H */

