#ifndef DB2CURSES_H
  #define DB2CURSES_H


/* use curses to display application-level snapshot */ 
#include <stdlib.h>
#include <ncurses.h>                                    
/* to handle terminal resize */  
#include <signal.h>              
#include <unistd.h>              
#include <sys/param.h>           
#include <termios.h>             
#ifndef TIOCGWINSZ               
  #include <sys/ioctl.h>           
#endif                           
#include "db2topasutil.h"
#include "db2list.h"
#include "db2snap.h"

/* defines how the output is formatted on the curses screen */
#define HEADER 2                                              
#define COL_WIDTH 9                                           

#define HANDLE 0                                              
#define PROG_NM HANDLE                                              
#define CLIENT_NM  HANDLE                                             
#define NODE HANDLE
#define UTIL_ID HANDLE
#define TBSPACE_NODE HANDLE
#define TABNAME HANDLE
#define BPR_TM HANDLE

/* added 1 to show client or coord pid */
#define STATUS_CHANGE (1+HANDLE+COL_WIDTH)                                       
#define UTIL_STARTTM STATUS_CHANGE
#define TBSPACE STATUS_CHANGE
#define BPW_TM STATUS_CHANGE
#define SS_EXEC_TM STATUS_CHANGE

#define AUTHID  (STATUS_CHANGE+COL_WIDTH+8)                                            
#define CLIENTID  AUTHID                                            
#define UTIL_TYPE AUTHID
#define TBSPACE_TYPE AUTHID
#define DIOR_TM AUTHID
#define SS_STATUS CLIENTID

#define STATUS (AUTHID+COL_WIDTH)                                              
#define UTIL_STATUS STATUS
#define TBSPACE_STATUS STATUS
#define DIOW_TM STATUS

#define CPU (STATUS+COL_WIDTH+1)                                         
#define UTIL_PROG CPU
#define TBSPACE_USED CPU
#define TAB_TYPE CPU 
#define BPLIR CPU

#define RR (CPU + 6)                                                
#define UTIL_DBNAME RR
#define TBSPACE_TOTAL RR
#define TAB_DAT_SZ RR
#define BPLDR RR

#define RW (RR + COL_WIDTH - 2)                                                 
#define UTIL_NODE RW
#define TBSPACE_FREE RW
#define TAB_IX_SZ (RW + 2)
#define BPDW RW

#define BP  (RW + COL_WIDTH -2 )                                                
#define BPLR BP
#define BPPR (BP + 7)
#define MIN_DISPLAY_WIDTH BP
#define MEMUSG BP
#define NAGENTS BP
#define UTIL_DESC BP
#define TBSPACE_READS (BP +1)
#define TAB_READS (BP+4)
#define BPPDR BPLR
#define BPPIR BPPR
#define SS_RR BPLR
#define SS_RW BPPR


#define STMT_SIUD (BP+COL_WIDTH+4)                                               
#define STMT_SIUD_WIDTH 12

#define STMT_TEXT (STMT_SIUD+STMT_SIUD_WIDTH)

#define TQR  (BP+COL_WIDTH+4)                                               
#define TBSPACE_WRITES  (TBSPACE_READS+COL_WIDTH)                                               
#define TAB_WRITES (TQR)
#define BPLTR TQR

#define TQW  (TQR+COL_WIDTH-1)                                              
#define BPIW TQW

#define SIUD (TQW+COL_WIDTH)                                               
#define SIUD_WIDTH 15
#define DIOR SIUD
#define DIOW (DIOR + COL_WIDTH)

#define LOG (SIUD+SIUD_WIDTH+1)                                              
#define RB  LOG                                              

#define MAX_DISPLAY_WIDTH (LOG + COL_WIDTH)

/* functions to resize output when the terminal size changed */  
static void handler(int sig);                                    
static void resize_scrn(int sig);                                
                                                                 
/* Initialize a curses screen                                    
     with n number of rows                                       
     and  m number of columns                                    
   Return the pointer to the curses screen                       
 */                                                              
WINDOW *init_scr();                                              
                                                                 
/* refresh and display the curses screen with  snapshots
*/
int refresh_screen(WINDOW *scrn, Header *header, DB2List *list);

/* read keyboard input and take action */
void read_screen_input(int keyboard, WINDOW *scrn, Header *header, DB2List *list);

#endif
