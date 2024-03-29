/* ============================================================= */
/* Receive packets at Gran Sasso from CERN.                      */
/* Julian Lewis AB/CO/HT 23rd Feb 2006 Julian.Lewis@CERN.ch      */
/* For Email the Subject line should contain the string "nospam" */
/* ============================================================= */

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CNGS.h>

#define SinSIZE (sizeof(struct sockaddr_in))

static int ssock     = 0;
static int errors    = 0;

typedef enum {
   HELP,
   PORT,
   DEBUG,
   OPTIONS
 } Options;

char *options[OPTIONS] = {"help","port","debug"};

#define MNAMES 8
char *mnames[MNAMES] = {"CPS","PSB","LEI","ADE","SPS","LHC","SCT","FCT"};

#define BUCKETS 100
static unsigned long buckets[BUCKETS];

/* ============================================================= */

int OpenPort(unsigned short port) {

struct sockaddr_in sin;

int s;

   if (!ssock) {

      bzero((void *) &sin, SinSIZE);

      sin.sin_family      = AF_INET;
      sin.sin_port        = htons(port);
      sin.sin_addr.s_addr = htonl(INADDR_ANY);

      s = socket(AF_INET, SOCK_DGRAM, 0);
      if (s < 0) {
	 fprintf(stderr,"RecvCNGS:OpenPort:Error:Cant open AF_INET sockets\n");
	 perror("RecvCNGS:OpenPort:errno");
	 return CngsFAIL;
      }
      ssock = s;

      if (bind(s,(struct sockaddr *) &sin, SinSIZE) < 0) {
	 fprintf(stderr,"RecvCNGS:OpenPort:Error:Cant bind port:%d\n",port);
	 perror("RecvCNGS:OpenPort:errno");
	 return CngsFAIL;
      }
   }
   return ssock;
}

/* ============================================================= */

int RecvFromPort(char *source_ip, unsigned short source_port, CngsPacket *pkt) {

int cc;
struct sockaddr_in sin;
socklen_t from;

   if (ssock) {

      bzero((void *) &sin, SinSIZE);

      sin.sin_family      = AF_INET;
      sin.sin_port        = htons(source_port);

      from = SinSIZE;

      cc = recvfrom(ssock,
		    (char *) pkt,
		    sizeof(CngsPacket),
		    0,
		    (struct sockaddr *) &sin,
		    &from);
      if (cc < 0) {
	 fprintf(stderr,
		"RecvCNGS:RecvFromPort:Error:Cant recvfrom port:%d at:%s\n",
		 source_port,
		 source_ip);
	 perror("RecvCNGS:RecvFromPort:errno");
	 return CngsFAIL;
      }

      strcpy(source_ip,inet_ntoa(sin.sin_addr));
      if (strncmp(source_ip,CngsSOURCE_NETWORK,strlen(CngsSOURCE_NETWORK)) != 0) {
	 fprintf(stderr,
		"RecvCNGS:RecvFromPort:Packet rejected (Not CERN): port:%d from:%s\n",
		 source_port,
		 source_ip);
	 return CngsFAIL;
      }

      if (pkt->MagicNumber != CngsMAGIG_NUMBER) {
	 fprintf(stderr,
		"RecvCNGS:RecvFromPort:Packet rejected (Corrupt): port:%d from:%s\n",
		 source_port,
		 source_ip);
	 return CngsFAIL;
      }


      return CngsOK;
   }
   return CngsFAIL;
}

/* ============================================================= */
/* Convert a CNGS time in milliseconds to a string routine.      */
/* Result is a pointer to a static string representing the time  */
/*    the format is: Thu-18/Jan/2001 08:25:14.967                */
/*                   day-dd/mon/yyyy hh:mm:ss.ddd                */
/* ============================================================= */

char *TimeToStr(CngsTime *t) {

static char tbuf[128];

char tmp[128];
char *yr, *ti, *md, *mn, *dy;

   bzero((void *) tbuf, 128);
   bzero((void *) tmp, 128);

   if (t->Second) {
      ctime_r(&t->Second, tmp);
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
      	  sprintf(&tbuf[strlen(tbuf)],".%09lu",t->Nano);
      }

   } else sprintf (tbuf, "--- Zero ---");

   
   if ((t->Machine < MNAMES) && (t->CTrain > 0)) {
      strcat(tbuf," ");
      strcat(tbuf,mnames[t->Machine]);
      strcat(tbuf," C:");
      sprintf(tmp,"%d",(int) t->CTrain);
      strcat(tbuf,tmp);
   }
   return (tbuf);
}

/* ============================================================= */
/* Get the time of day from the system clock in milliseconds.    */
/* ============================================================= */

CngsTime *GetSystemClock(void) {

static CngsTime result;

struct timeval tv;
struct timezone tz;

   tz.tz_dsttime = 0;
   tz.tz_minuteswest = 0;
   gettimeofday (&tv, &tz);

   result.Second  = tv.tv_sec;
   result.Nano    = tv.tv_usec * 1000;
   result.CTrain  = 0;
   result.Machine = 4;


   return &result;
}

/* ============================================================= */
/* Get difference between times                                  */
/* ============================================================= */

int TimeDifference(CngsTime *t1, CngsTime *t2) {

int dfns, dfsc;

   /* t1 must be greater than t2 */

   if (t1->Second < t2->Second) return 0;
   if ((t1->Second == t2->Second)
   &&  (t1->Nano < t2->Nano))   return 0;

   dfns = t1->Nano   - t2->Nano;
   dfsc = t1->Second - t2->Second;

   return (dfsc*1000) + (dfns/1000000);
}

/* ============================================================= */
/* To be filled out by Gran Sasso people                         */
/* This routine is called each basic period (1.2S) while there   */
/* is an active GNGS cycle. Put whatever logic you want here.    */
/* I guess this means that the detecor must go out of the        */
/* calibration mode an take a real measurment.                   */
/* ============================================================= */

void CngsCycleUserRoutine(CngsPacket *pkt) {    /* Cycle is CNGS */

   printf("UserRoutine >>> CNGS Cycle\n");
}

/* ============================================================= */
/* Go back to calibration mode, its not a CNGS cycle             */

void OtherCycleUserRoutine(CngsPacket *pkt) {   /* Not CNGS */

   printf("UserRoutine >>> Not a CNGS cycle\n");
}

/* ============================================================= */

int main(int argc,char *argv[]) {
int i, fw, tt, df, deb;
unsigned long newsqn, oldsqn, missed;
char source[32], *cp, *ep;
unsigned short port;
CngsPacket pkt;
CngsTime *tod;
FILE *fp;

   newsqn = oldsqn = missed = 0;
   bzero((void *) &buckets, BUCKETS * sizeof(unsigned long));

   port = CngsPORT;

   for (i=1; i<argc; i++) {

      if (strcmp(argv[i],options[HELP]) == 0) {
	 printf("\nOptions are:\n\n");
	 for (i=0; i<OPTIONS; i++) {
	    printf("%s ",options[i]);
	    switch ((Options) i) {
	       case HELP:
		  printf("[print this help text]\n");
	       break;

	       case PORT:
		  printf("<UDP Port number to be used. Default:%d>\n",CngsPORT);
	       break;

	       case DEBUG:
		  printf("<Debug level: 0/None 1/Debug ON. Default:0>\n");
	       break;

	       default:
		  printf("For help type: RecvCNGS help\n");
	    }
	 }
	 printf("\n\n");
	 exit(0);
      }

      else if (strcmp(argv[i],options[PORT]) == 0) {
	 i++;
	 cp = argv[i];
	 if (cp) port = (unsigned short) strtoul(cp,&ep,0);
	 continue;
      }

      else if (strcmp(argv[i],options[DEBUG]) == 0) {
	 i++;
	 cp = argv[i];
	 deb = strtoul(cp,&ep,0);
	 continue;
      }

      else {
	 printf("No such option: %s\n",argv[i]);
	 printf("For help type: timtest %s\n",options[HELP]);
	 exit(1);
      }
   }

   fprintf(stderr,"RecvCNGS:Sartup Port:%d\n",port);

   if (OpenPort(port))  {

      fprintf(stderr,"RecvCNGS: Up and running OK\n");

      while (errors < CngsMAX_ERRORS) {

	 if (RecvFromPort(source, port, &pkt)) {

	    tod = GetSystemClock(); /* Get local arrival time from system clock */

	    printf("\n==CNGS======:==PACKET===\n");

	    printf("SequenceNumb:%d\n",(int) pkt.SequenceNumber);
	    printf("SourceIpAddr:%s\n",source);
	    printf("Arrival time:%s (Local  NTP)\n",TimeToStr(tod));
	    printf("Sender  time:%s (Sender NTP)\n",TimeToStr(&pkt.SendTime));
	    printf("BPeriod time:%s (CERN GPS)\n",  TimeToStr(&pkt.BasicPeriod));
	    printf("Extract time:%s (CERN GPS)\n",  TimeToStr(&pkt.Extraction));
	    printf("StCycle time:%s (CERN GPS)\n",  TimeToStr(&pkt.StartCycle));
	    printf("EnCycle time:%s (CERN GPS)\n",  TimeToStr(&pkt.EndCycle));
	    printf("SPSCycleName:%s\n",pkt.CycleName);

	    /* Deal with missed packets by checking packet sequence numbers */

	    if ((oldsqn == 0)
	    &&  (newsqn == 0)) newsqn = pkt.SequenceNumber; /* First time ? */

	    oldsqn = newsqn;
	    newsqn = pkt.SequenceNumber;

	    if (newsqn > oldsqn) {
	       df = newsqn - oldsqn - 1;    /* Difference -1 should be zero */
	       missed += df;
	       if (df) printf("PacketMissed:%d\n",df);
	    }

	    if (missed) printf("TotalMissed:%d\n",(int) missed);

	    /* Calculate some travel/arrival time statistics */

	    tt = TimeDifference(tod,&pkt.SendTime);
	    printf("TravelTimeMs:%dms\n",tt);
	    if ((tt >= 0) && (tt < BUCKETS)) buckets[tt]++;

	    if (newsqn % BUCKETS) {             /* Dump stats in a file for gnuplot */
	       fp = fopen("arrival_stats","w");
	       if (fp) {
		  for (i=0; i<BUCKETS; i++) {
		     fprintf(fp,"%03d %d\n",i+1,(int) buckets[i]);
		  }
		  fclose(fp);
	       }
	    }

	    /* Extraction forwarning packets sent once per extraction */

	    fw = TimeDifference(&pkt.Extraction,tod);   /* Posative forewarning ? */
	    if (fw) printf("FWExtraction:%dms\7\n",fw); /* Kicker will fire at time */

	    /* Call some user code routine for CNGS and other cycles */

	    if (strstr(pkt.CycleName,"CNGS")) CngsCycleUserRoutine(&pkt);
	    else                             OtherCycleUserRoutine(&pkt);

	    errors = 0; /* Contiguous errors */

	 } else { sleep(1); errors++; }
      }
   } else errors++;

   close(ssock);

   fprintf(stderr,"RecvCNGS: ABORTED, after ErrorCount:%d\n",errors);

   exit(1);
}
