/**************************************************************************/
/* Ctr look at a selected member                                          */
/**************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>        /* Error numbers */
#include <sys/file.h>
#include <a.out.h>

#ifdef Status
#undef Status
#endif

#include <err/err.h>      /* Error handling */
#include <tgm/tgm.h>      /* Telegram definitions for application programs */
#include <tgv/tgv.h>
#include <TimLib.h>

#include <errno.h>
extern int errno;

#define SZE 128

/**************************************************************************/
/* Code section from here on                                              */
/**************************************************************************/

#include "DisplayLine.c"

/**************************************************************************/

char *defaultconfigpath = "/dsc/bin/tim/timtest.config";

char *configpath = NULL;
char localconfigpath[SZE];  /* After a CD */

/**************************************************************************/

static char path[SZE];

char *GetFile(char *name) {
FILE *gpath = NULL;
char txt[SZE];
int i, j;

   if (configpath) {
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = NULL;
      }
   }

   if (configpath == NULL) {
      configpath = "./timtest.config";
      gpath = fopen(configpath,"r");
      if (gpath == NULL) {
	 configpath = "/dsc/local/data/timtest.config";
	 gpath = fopen(configpath,"r");
	 if (gpath == NULL) {
	    configpath = defaultconfigpath;
	    gpath = fopen(configpath,"r");
	    if (gpath == NULL) {
	       configpath = NULL;
	       sprintf(path,"./%s",name);
	       return path;
	    }
	 }
      }
   }

   bzero((void *) path,SZE);

   while (1) {
      if (fgets(txt,SZE,gpath) == NULL) break;
      if (strncmp(name,txt,strlen(name)) == 0) {
	 for (i=strlen(name); i<strlen(txt); i++) {
	    if (txt[i] != ' ') break;
	 }
	 j= 0;
	 while ((txt[i] != ' ') && (txt[i] != 0)) {
	    path[j] = txt[i];
	    j++; i++;
	 }
	 strcat(path,name);
	 fclose(gpath);
	 return path;
      }
   }
   fclose(gpath);
   return NULL;
}

/*****************************************************************/

char *PadStr(char *str, int pad) {
int i;

   if (str) {
      for (i=strlen(str); i<pad; i++) str[i] = ' ';
      str[pad] = 0;
      return str;
   }
   return NULL;
}

/**************************************************************************/
/* Convert a TimLib time in milliseconds to a string routine.             */
/* Result is a pointer to a static string representing the time.          */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                         */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                         */

volatile char *TimeToStr(TimLibTime *t) {

static char tbuf[SZE];

char tmp[SZE];
char *yr, *ti, *md, *mn, *dy;
time_t second = t->Second;

   bzero((void *) tbuf, SZE);
   bzero((void *) tmp, SZE);

   if (t->Second) {
#ifdef __68k__
      ctime_r(&second, tmp, 128);
#else
      ctime_r(&second, tmp);
#endif
      tmp[3] = 0;
      dy = &(tmp[0]);
      tmp[7] = 0;
      mn = &(tmp[4]);
      tmp[10] = 0;
      md = &(tmp[8]);
      if (md[0] == ' ') md[0] = '0';
      tmp[19] = 0;
      ti = &(tmp[11]);
      tmp[24] = 0;
      yr = &(tmp[20]);
      sprintf (tbuf, "%s-%s/%s/%s %s"  , dy, md, mn, yr, ti);
      if (t->Nano) {
	  sprintf(&tbuf[strlen(tbuf)],".%03lu",t->Nano/1000000);
      }

   } else sprintf (tbuf, "--- Zero ---");

   
   if ((t->Machine != TgmMACHINE_NONE) && (t->CTrain > 0)) {
      strcat(tbuf," ");
      strcat(tbuf,TgmGetMachineName(t->Machine));
      strcat(tbuf," C:");
      sprintf(tmp,"%d",(int) t->CTrain);
      strcat(tbuf,tmp);
   }
   return (tbuf);
}

/**************************************************************************/
/* Convert a ptim to its name                                             */

typedef struct {
   unsigned long Eqp;
   char Name[32];
 } PtmNames;

PtmNames ptm_names[SZE];
static char ptm_name_txt[SZE];
int ptm_names_size = 0;

char *GetPtmName(unsigned long eqp) {

char *cp, *ep;
int i;
FILE *inp;

   if (ptm_names_size == 0) {
      GetFile("ltim.obnames");
      inp = fopen(path,"r");
      if (inp) {
	 while (fgets(ptm_name_txt,SZE,inp) != NULL) {
	    cp = ep = ptm_name_txt;
	    ptm_names[ptm_names_size].Eqp = strtoul(cp,&ep,0);
	    if (cp == ep) continue;
	    for (i=strlen(ep); i>=0; i--) {
	       if (ep[i] == '\n') {
		  ep[i] = 0;
		  break;
	       }
	    }
	    strcpy(ptm_names[ptm_names_size++].Name,++ep);
	    if (ptm_names_size >= SZE) break;
	 }
	 fclose(inp);
      }
   }

   for (i=0; i<ptm_names_size; i++) {
      if (ptm_names[i].Eqp == eqp) {
	 sprintf(ptm_name_txt,"%04d:%s",(int) eqp,ptm_names[i].Name);
	 return ptm_name_txt;
      }
   }

   sprintf(ptm_name_txt,"%04d",(int) eqp);
   return ptm_name_txt;
}

/**************************************************************************/
/* Status To String                                                       */

#define STATAE 5

static char *StatusOn[STATAE] = { "GmtOk", "PllOk", "SlfOk", "EnbOk", "BusOk"  };
static char *StatusOf[STATAE] = { "***GmtErr***","***PllErr***","***SlfErr***","***EnbErr***","***BusErr***" };

#define ST_STR_SZ 48
static char StsStr[ST_STR_SZ];

char *StatusToStr(TimLibStatus sts) {
int i;
unsigned long msk;

   bzero((void *) StsStr,ST_STR_SZ);
   for (i=0; i<STATAE; i++) {
      msk = 1 << i;
      if (msk & TimLibStatusBITS) {
	 if (msk & sts) strcat(StsStr,StatusOn[i]);
	 else           strcat(StsStr,StatusOf[i]);
	 strcat(StsStr,":");
      } else break;
   }
   return StsStr;
}

/**************************************************************************/
/* Hardware Interrupt to String                                           */

#define SOURCES 14
static char *HardNames[SOURCES] =
   {"CTIM","Ch1","Ch2","Ch3","Ch4","Ch5","Ch6","Ch7","Ch8",
    "Pll","Evnt","PPS","OneKHz","Match" };

#define HD_STR_SZ 80
static char HrdStr[HD_STR_SZ];

char *HardToStr(TimLibHardware hrd, int idx) {
int i;
unsigned long msk;

   bzero((void *) HrdStr,HD_STR_SZ);
   if (idx) {
      if ((int) hrd < SOURCES)
	 strcat(HrdStr,HardNames[(int) hrd]);
   } else {
      for (i=0; i<SOURCES; i++) {
	 msk = 1 << i;
	 if (msk & TimLibHardwareBITS) {
	    if (msk & hrd) {
	       strcat(HrdStr,HardNames[i]);
	       strcat(HrdStr,":");
	    }
	 } else break;
      }
   }
   return HrdStr;
}

/*****************************************************************/
/* Output Masks to String                                        */

#define OUT_MASKS  12
static char *OtmNames[OUT_MASKS] = {"Ctim","Ch1","Ch2","Ch3","Ch4","Ch5","Ch6","Ch7","Ch8",
				    "40Mh","ExCk1","ExCk2" };

#define OTM_STR_SZ 80
static char OtmStr[OTM_STR_SZ];

char *OtmToStr(TimLibOutput otm) {
int i;
unsigned long msk;

   bzero((void *) OtmStr,OTM_STR_SZ);
   if (otm == 0) {
      strcat(OtmStr,"NotSet");
      return OtmStr;
   }
   for (i=0; i<OUT_MASKS; i++) {
      msk = 1 << i;
      if (msk & TimLibOutputBITS) {
	 if (msk & otm) {
	    strcat(OtmStr,OtmNames[i]);
	    strcat(OtmStr,":");
	 }
      } else break;
   }
   return OtmStr;
}

/*****************************************************************/

#define CCV_FIELDS 13
#define POLARATIES 3

static char *CounterStart [TimLibSTARTS]  = {"Nor", "Ext1", "Ext2", "Chnd", "Self", "Remt", "Pps", "Chnd+Stop"};
static char *CounterMode  [TimLibMODES]   = {"Once", "Mult", "Brst", "Mult+Brst"};
static char *CounterClock [TimLibCLOCKS]  = {"1KHz", "10MHz", "40MHz", "Ext1", "Ext2", "Chnd" };
static char *Polarity     [POLARATIES]    = {"TTL","BAR","TTL"};
static char *Enable       [TimLibENABLES] = {"NoOut","Out","Bus","OutBus"};

static char CcvStr[SZE];

char * CcvToStr(int pln, TimLibCcvMask ccm, TimLibCcv *ccv) {

int i, w;
unsigned long msk;
TgmGroupDescriptor desc;
char tmp[SZE];
TgvName tname;
char *cp;

   bzero((void *) CcvStr,SZE);
   bzero((void *) tmp   ,SZE);

   sprintf(CcvStr,"%02d:",pln);

   for (i=0; i<CCV_FIELDS; i++) {
      msk = 1 << i;
      if (msk & TimLibCcvMaskBITS) {
	 if (msk & ccm) {
	    switch (msk) {
	       case TimLibCcvMaskENABLE:
		  if (ccv->Enable < TimLibENABLES)
		     sprintf(tmp,"%s ",Enable[ccv->Enable]);
		  else
		     sprintf(tmp,"En:?[%d] ",(int) ccv->Enable);
	       break;

	       case TimLibCcvMaskSTART:
		  if (ccv->Start < TimLibSTARTS)
		     sprintf(tmp,"St%s:",CounterStart[ccv->Start]);
		  else
		     sprintf(tmp,"St?[%d]:",(int) ccv->Start);
	       break;

	       case TimLibCcvMaskMODE:
		  if (ccv->Mode < TimLibMODES)
		     sprintf(tmp,"%s:",CounterMode[ccv->Mode]);
		  else
		     sprintf(tmp,"Md?[%d]:",(int) ccv->Mode);
	       break;

	       case TimLibCcvMaskCLOCK:
		  if (ccv->Clock < TimLibCLOCKS)
		     sprintf(tmp,"Ck:%s ",CounterClock[ccv->Clock]);
		  else
		     sprintf(tmp,"Ck?[%d] ",ccv->Clock);
	       break;

	       case TimLibCcvMaskPWIDTH:
		  w = ccv->PulsWidth * 25;
		  if      (w >= 1000000) { w = w/1000000; cp = "ms"; }
		  else if (w >= 1000   ) { w = w/1000;    cp = "us"; }
		  else                   {                cp = "ns"; }
		  sprintf(tmp,"%d[%d%s]",(int) ccv->PulsWidth, w, cp);
	       break;

	       case TimLibCcvMaskDELAY:
		  sprintf(tmp,"%d>",(int) ccv->Delay);
	       break;

	       case TimLibCcvMaskOMASK:
		  sprintf(tmp,"%s",OtmToStr(ccv->OutputMask));
	       break;

	       case TimLibCcvMaskPOLARITY:
		  if ((ccv->Polarity == TimLibPolarityTTL_BAR)
		  ||  (ccv->Polarity == TimLibPolarityTTL)
		  ||  (ccv->Polarity == 0))
		     sprintf(tmp,"%s ",Polarity[ccv->Polarity]);
		  else
		     sprintf(tmp,"Vp:?[%d] ",(int) ccv->Polarity);
	       break;

	       case TimLibCcvMaskCTIM:
		  if (TgvGetNameForMember(ccv->Ctim,&tname) == NULL) sprintf(tname,"BadCtim");
		  sprintf(tmp,"Ctm:%d-%s ",(int) ccv->Ctim,(char *) tname);
	       break;

	       case TimLibCcvMaskPAYLOAD:
		  if (ccv->Payload == 0) break;
		  sprintf(tmp,"Pld:0x%04X ",(int) ccv->Payload);
	       break;

	       case TimLibCcvMaskMACHINE:
		  if (ccv->GrNum == 0) break;
		  cp = TgmGetMachineName(ccv->Machine);
		  if (cp) sprintf(tmp,"%s.",TgmGetMachineName(ccv->Machine));
		  else    sprintf(tmp,"???[%d].",(int) ccv->Machine);
	       break;

	       case TimLibCcvMaskGRNUM:
		  if (ccv->GrNum == 0) break;
		  if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
		     sprintf(tmp,"%s",desc.Name);
		     if (desc.Type == TgmBIT_PATTERN) strcat(tmp,"&");
		     else                             strcat(tmp,"=");
		  } else sprintf(tmp,"???[%d]*",(int) ccv->GrNum);
	       break;

	       case TimLibCcvMaskGRVAL:
		  if (ccv->GrNum == 0) break;
		  if (TgmGetGroupDescriptor(ccv->Machine,ccv->GrNum,&desc) == TgmSUCCESS) {
		     if (desc.Type == TgmEXCLUSIVE) sprintf(tmp,"%s",(char *) TgmGetLineName(ccv->Machine,desc.Name,ccv->GrVal));
		     else                           sprintf(tmp,"%d",(int) ccv->GrVal);
		  } else sprintf(tmp,"%d",(int) ccv->GrVal);
	       break;

	       default:
	       break;
	    }
	    strcat(CcvStr,tmp);
	    bzero((void *) tmp,SZE);
	 }
      }
   }
   PadStr(CcvStr,SZE);
   return CcvStr;
}

/*****************************************************************/

static Boolean error_handler(class,text)
ErrClass class;
char *text; {

   return(True);
}

/*****************************************************************/

#define OLD_SPS_HEADER 0x21000000
#define ANCIENT_SPS_HEADER 0x2F000000

static char WaitStr[SZE];

char *WaitToStr(TimLibClass    cls,
		unsigned long  eqp,
		unsigned long  pln,
		TimLibHardware src,
		TimLibTime     *zro,
		TimLibTime     *trg,
		TimLibTime     *str,
		unsigned long  ctm,
		unsigned long  pld,
		unsigned long  mod,
		unsigned long  mis,
		unsigned long  qsz,
		TgmMachine     mch ) {

TgmBeamState bs;
TgmGroupDescriptor gd;
unsigned long us, bp, frm, hdr;
char *cp, tmp[SZE];
TgvName nam;

   bzero((void *) WaitStr, SZE);
   bzero((void *) tmp,     SZE);

   if (cls != TimLibClassHARDWARE) {
      if ((mch != -1) && (TgmGetGroupDescriptor(mch,1,&gd) == TgmSUCCESS)) {
	 if (TgmGetGroupValue(mch,TgmCURRENT,0,1,(int *) &us) == 0) us = 0;
	 cp = (char *) TgmGetLineName(mch,gd.Name,us);
	 if (cp) sprintf(tmp,"%02d:%s: ",(int) pln,cp);
	 else    sprintf(tmp,"%02d:"    ,(int) pln);
	 strcat(WaitStr,tmp);
      } else strcpy(gd.Name,"USER");
   }

   if      (cls == TimLibClassPTIM) sprintf(tmp,"Ptm:%s: "     ,GetPtmName(eqp));
   else if (cls == TimLibClassCTIM) sprintf(tmp,"Ctm:%d:%s: "  ,(int) eqp,(char *) TgvGetNameForMember(eqp,&nam));
   else                             sprintf(tmp,"Hrd:0x%X:%s: ",(int) eqp,HardToStr(eqp,0));
   strcat(WaitStr,tmp);

   if ((cls == TimLibClassPTIM) && (ctm)) {
      cp = (char *) TgvGetNameForMember(ctm,(TgvName *) &nam);
      if (cp) sprintf(tmp,"Ctm:%d:%s ",(int) ctm, cp);
      else    sprintf(tmp,"Ctm:%d "   ,(int) ctm);
      strcat(WaitStr,tmp);
   }

   sprintf(tmp,"Src:%s ",HardToStr(src,1));
   strcat(WaitStr,tmp);

   if (pld) {
      frm = TgvGetFrameForMember(eqp);
      hdr = frm & 0xFF000000;
      if ((hdr == OLD_SPS_HEADER) || (hdr == ANCIENT_SPS_HEADER)) {
	 sprintf(tmp,"Pld:0x%04X ",(int) pld);
      } else {
	 sprintf(tmp,"Pld:%d",(int) pld);
	 TgmDecodeTag(pld,&bs,(Cardinal *) &us,(Cardinal *) &bp);
	 if ((us>=gd.Minimum) && (us<=gd.Maximum)) {
	    cp = (char *) TgmGetLineName(mch,gd.Name,us);
	    if (cp) sprintf(tmp,"Pld:%02d.%s.%s ",(int) bp,gd.Name,cp);
	 }
      }
      strcat(WaitStr,tmp);
   }

   sprintf(tmp,"Out:%s",TimeToStr(zro));
   strcat(WaitStr,tmp);
   PadStr(WaitStr,SZE);
   return WaitStr;
}

/**************************************************************************/
/* Prompt and do commands in a loop                                       */
/**************************************************************************/

static char *DevNames[TimLibDEVICES] = { "...", "CTR", "CPS_TG8", "SPS_TG8", "NETWORK" };

int main(int argc,char *argv[]) {

TimLibError  err;
TimLibDevice dev;
TimLibStatus sts;
TimLibClass  cls;

TimLibClass    icls;  /* Class of interrupt */
unsigned long  ieqp;  /* PTIM CTIM or hardware mask */
unsigned long  ipln;  /* Ptim line number 1..n or 0 */
TimLibHardware isrc;  /* Hardware source of interrupt */
TimLibTime     izro;  /* Time of interrupt/output */
TimLibTime     itrg;  /* Time of counters load */
TimLibTime     istr;  /* Time of counters start */
unsigned long  ictm;  /* CTIM trigger equipment ID */
unsigned long  ipld;  /* Payload of trigger event */
unsigned long  imod;  /* Module that interrupted */
unsigned long  imis;  /* Number of missed interrupts */
unsigned long  iqsz;  /* Remaining interrupts on queue */
TgmMachine     imch;  /* Corresponding TgmMachine */

TimLibCcvMask  ccvm;  /* Valid fields in ccv */
TimLibCcv      ccv;   /* CCV fields */

unsigned long mod, pmod, chn, pdim, cnt, row, eqp, frm;
char *cp, *ep, eqn[SZE], tit[80];
int i, mch = -1;

   printf("%s: Compiled %s %s\n",argv[0],__DATE__,__TIME__);

   if (argc < 4) {
      printf("%s Args: <module> H/P/C <member> [<device>]\n",argv[0]);
      exit((int) TimLibErrorSUCCESS);
   }

   ErrSetHandler((ErrHandler) error_handler);

   dev = TimLibDevice_ANY;
   cp = argv[4];
   if (cp) {
      for (i=TimLibDevice_CTR; i<TimLibDEVICES; i++) {
	 if (strcmp(DevNames[i],cp) == 0) {
	    dev = (TimLibDevice) i;
	    break;
	 }
      }
   }

   err = TimLibInitialize(dev);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stderr,"%s: FatalError:%s\n",argv[0],TimLibErrorToString(err));
      exit((int) err);
   }

   cnt = TimLibGetInstalledModuleCount();
   cp = argv[1];
   mod = strtol(cp,&ep,0);
   if ((cp == ep) || (mod > cnt)) {
      fprintf(stderr,"%s: Ilegal module number:%s [0..%d]\n",argv[0],argv[1],(int) cnt);
      exit((int) TimLibErrorMODULE);
   }

   sts = TimLibGetStatus(mod,&dev);
   if (((sts & TimLibStatusGMT_OK ) == 0)
   ||  ((sts & TimLibStatusENABLED) == 0)) {
      fprintf(stderr,"%s: Bad module status:%s\n",argv[0],StatusToStr(sts));
      exit((int) TimLibErrorNOT_ENAB);
   }

   cp = argv[3];
   eqp = strtol(cp,&ep,0);
   if ((eqp == 0) || (cp == ep)) {
      fprintf(stderr,"%s: Bad equipment number:%s\n",argv[0],argv[3]);
      exit((int) TimLibErrorCONNECT);
   }

   if      ((argv[2][0] == 'P') || (argv[2][0] == 'p')) {
      cls = TimLibClassPTIM;
      sprintf(tit,"PTIM:");
   } else if ((argv[2][0] == 'C') || (argv[2][0] == 'c')) {
      cls = TimLibClassCTIM;
      sprintf(tit,"CTIM:%d:",(int) eqp);
   } else if ((argv[2][0] == 'H') || (argv[2][0] == 'h')) {
      cls = TimLibClassHARDWARE;
      sprintf(tit,"HARDWARE:0x%04X:",(int) eqp);
   } else {
      fprintf(stderr,"%s: Ilegal timing class:%s [P|C|H]\n",argv[0],argv[2]);
      exit((int) TimLibErrorCONNECT);
   }

   chn = 0;
   row = 0;

   if (cls == TimLibClassPTIM) {
      err = TimLibGetPtimObject(eqp,&pmod,&chn,&pdim);
      if (err != TimLibErrorSUCCESS) {
	 fprintf(stderr,"%s: FatalError:%s\n",argv[0],TimLibErrorToString(err));
	 exit((int) err);
      }
      if (mod == 0) mod = pmod;
      if (mod != pmod) {
	 err = TimLibErrorMODULE;
	 fprintf(stderr,"%s: WARNING:That PTIM is on module:%d\n",argv[0],(int) pmod);
	 mod = pmod;
      }
      sprintf(eqn,"%s[1..%02d] Mod:%d Ch:%d",
	      GetPtmName(eqp),
	      (int) pdim,
	      (int) mod,
	      (int) chn);
   }

   if (cls == TimLibClassCTIM) {
      err = TimLibGetCtimObject(eqp,&frm);
      if (err != TimLibErrorSUCCESS) {
	 fprintf(stderr,"%s: FatalError:%s\n",argv[0],TimLibErrorToString(err));
	 exit((int) err);
      }
   }

   if (cls == TimLibClassHARDWARE) {
      sprintf(eqn,"%s Mod:%d",
	      HardToStr((TimLibHardware) eqp,0),
	      (int) mod);
   }

   err = TimLibConnect(cls,eqp,mod);
   if (err != TimLibErrorSUCCESS) {
      fprintf(stderr,"%s: FatalError:%s\n",argv[0],TimLibErrorToString(err));
      exit((int) err);
   }

   strcat(tit,eqn);
   DisplayInit(tit,6,SZE);

   for (i=0; i<1000; i++) {

      err = TimLibWait(&icls,  /* Class of interrupt */
		       &ieqp,  /* PTIM CTIM or hardware mask */
		       &ipln,  /* Ptim line number 1..n or 0 */
		       &isrc,  /* Hardware source of interrupt */
		       &izro,  /* Time of interrupt/output */
		       &itrg,  /* Time of counters load */
		       &istr,  /* Time of counters start */
		       &ictm,  /* CTIM trigger equipment ID */
		       &ipld,  /* Payload of trigger event */
		       &imod,  /* Module that interrupted */
		       &imis,  /* Number of missed interrupts */
		       &iqsz,  /* Remaining interrupts on queue */
		       &imch); /* Corresponding TgmMachine */
      if (err != TimLibErrorSUCCESS) {
	 fprintf(stderr,"%s: Warning:%s\n",argv[0],TimLibErrorToString(err));
	 sleep(1000);
	 continue;
      }

      if ((mch == -1) && (imch != -1)) {
	 mch = imch;
	 TgmAttach(imch,TgmTELEGRAM | TgmGROUPS | TgmLINE_NAMES);
      }

      if (row > 5) row = 1;
      else         row++;

      if (icls == TimLibClassPTIM) {
	 err = TimLibGet(ieqp,ipln,0,0,&ccvm,&ccv);
	 if (err != TimLibErrorSUCCESS) {
	    fprintf(stderr,"%s: Warning:%s\n",argv[0],TimLibErrorToString(err));
	    continue;
	 }
	 DisplayLine(CcvToStr(ipln,ccvm,&ccv),row,0);
	 row++;
      }
      cp = WaitToStr( icls,   /* Class of interrupt            */
		      ieqp,   /* PTIM CTIM or hardware mask    */
		      ipln,   /* Ptim line number 1..n or 0    */
		      isrc,   /* Hardware source of interrupt  */
		     &izro,   /* Time of interrupt/output      */
		     &itrg,   /* Time of counters load         */
		     &istr,   /* Time of counters start        */
		      ictm,   /* CTIM trigger equipment ID     */
		      ipld,   /* Payload of trigger event      */
		      imod,   /* Module that interrupted       */
		      imis,   /* Number of missed interrupts   */
		      iqsz,   /* Remaining interrupts on queue */
		      imch);  /* Corresponding TgmMachine      */

      if (cp) DisplayLine(cp,row,1);
      else {
	 fprintf(stderr,"%s: Bad TGM configuration\n",argv[0]);
	 exit((int) TimLibErrorINIT);
      }
      if (cls == TimLibClassPTIM) row++;
   }
   exit((int) TimLibErrorSUCCESS);
}
