#define INCL_DOS
#define INCL_KBD

#include <os2.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>


/*
 * Public domain from Mark Kimes
 * icc /G4 /O+ /Gs+ /W3 /Kb /B"/RUNFROMVDM" /B"/STACK:16384" /Rn reboot.c
 */

VOID ShowHelp (APIRET rc) {

  static RESULTCODES rt;
  static CHAR        object[32],runme[CCHMAXPATH];

  sprintf(runme,"CMD.EXE /C HELP.CMD SYS%04u",rc);
  runme[strlen(runme) + 1] = 0;
  runme[7] = 0;
  DosExecPgm(object,sizeof(object),EXEC_SYNC,(PVOID)runme,NULL,&rt,(PSZ)runme);
}


INT to_upper (INT key) {

  if(key >= 'a' && key <= 'z')
    return ((key) + 'A' - 'a');
  return key;
}


int main (int argc,char *argv[]) {

  HFILE  hf;
  APIRET rc;
  ULONG  action;
  CHAR   key = 'N';
  INT    x;
  BOOL   clerr = FALSE;

  for(x = 0;x < argc;x++) {
    switch(*argv[x]) {
      case '/':
      case '-':
        switch(to_upper(argv[x][1])) {
          case '?':
            printf("\n  Usage:  REBOOT [/y[es]]\n"
                   "\nReboots the system as if you'd pressed CTRL-ALT-DEL."
                   "\n Hector wuz here.\n");
            return 0;
          case 'Y':
            key = 'Y';
            break;
          default:
            printf("\n **\07Unknown command line switch '%s'\n",argv[x]);
            DosSleep(1000L);
            clerr = TRUE;
            break;
        }
        break;
    }
  }

  if(clerr)
    key = 'N';

  rc = DosOpen("DOS$", &hf, &action, 0L, FILE_NORMAL, FILE_OPEN,
               OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE |
               OPEN_FLAGS_FAIL_ON_ERROR, 0L);
  if(!rc){
    if(key != 'Y') {  /* if /y not on command line */

      KBDKEYINFO kbd;

      while(!KbdCharIn(&kbd,IO_NOWAIT,0)) { /* flush keyboard */
        if(!(kbd.fbStatus & 0x40))
          break;
      }
      printf("\n\07Rudely reboot system? (y/N) ");
      if(!KbdCharIn(&kbd,IO_WAIT,0)) {
        if(kbd.fbStatus & 0x40)
          key = to_upper(kbd.chChar);
      }
      printf("%s\n",(key == 'Y') ? "Y" : "N\n  **Aborted.");
    }
    if(key == 'Y') {  /* we got the green light */
      rc = DosShutdown(0L);
      if(!rc) {
        rc = DosDevIOCtl(hf,0xd5,0xab,NULL,0,NULL,NULL,0,NULL);
        if(rc)
          printf("\n **\07DosDevIOCtl 0xd5/0xab failed, can't reboot.\n");
      }
      else
        printf("\n **\07DosShutdown failed, won't reboot.\n");
    }
    DosClose(hf);
  }
  else
    printf("\n **\07DOS.SYS not installed, can't reboot.\n");
  if(rc)
    ShowHelp(rc);
  return rc;
}
