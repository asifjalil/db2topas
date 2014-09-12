#include <unistd.h>                                                                
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlutil.h>
#include <db2ApiDf.h>
#include <sqlenv.h>
#include <pthread.h>
#include <signal.h>

#include "db2topasutil.h"
#include "db2list.h"
#include "db2snap.h"
#include "db2curses.h"

/*********************************************************************************************/
/* global variable for signal handling                                                       */
/*********************************************************************************************/
/* 
  0 means no signal has been raised
  1 means resize  curses window 
  2 means abort
*/
int sig_action = 0;
/*********************************************************************************************/
/* private helper functions                                                                  */
/*********************************************************************************************/
static void handler(int sig);
int  parse_cmdline(int argc, char *argv[], Header*header);
int  show_help(void);

int  main(int argc, char *argv[]) 
{
  char                   mesg[1024];
  int                    keyboard, c, rc;

  pthread_t              take_db2_snapshot_thrd_id;
  pthread_attr_t         thrd_attr;
  /* struct pointer to pass args to take_db2_snapshot thread */ 
  TakeDb2SnapshotArgs    * snapshot_thrd_arg = NULL ; 
  WINDOW                 * scrn = NULL; /* curses window */
  Header                 * header = NULL; /* struct to hold cmdline options and keyboard inputs */
  DB2List                list; /* double-linked list to hold parsed snapshot output        */


  /* install signal handlers */
  signal(SIGINT,handler); 
  signal(SIGABRT,handler);
  signal(SIGTERM,handler);
/*  signal(SIGSEGV,handler); */
  signal(SIGWINCH, handler);


  if (sig_action == 2) {
    keyboard = 107;
    sig_action = 0;
  }
  strncpy(mesg, basename(argv[0]), 255);

  /*********************************************************************************************/
  /* initialize header
     which holds input from the cmdline */
  /*********************************************************************************************/
  header = init_header();
  if ( header == NULL)  {
    fprintf(stderr, "header error\n");
    ERR_MSG("header error");
  }
  /*********************************************************************************************/
  /* check command line args                                                                   */
  /*********************************************************************************************/
  rc = parse_cmdline(argc, argv, header);
  if (rc != 0 )  {
    free_header(header);
    ERR_MSG("parse_cmdline error");
  }
  /*********************************************************************************************/
  /* attach to a local or remote instance                                                      */
  /*********************************************************************************************/
  rc = InstanceAttach(header->instance, header->user_id , header->pswd);
  if (rc != 0)   {
    free_header(header);
    sprintf(mesg,"Failed to attach to instance %s", header->instance);
    fprintf(stderr, "Failed to attach to instance %s\n", header->instance);
    ERR_MSG(mesg);
  }
  sprintf(mesg, "Started %s\nMonitoring instance %s using id %s from host %s\n"
      , header->myname, header->instance, header->user_id, header->hostname);    
  INFO_MSG(mesg);                                                              
  /*********************************************************************************************/
  /* Init list that will hold snapshot output */
  /*********************************************************************************************/
  db2list_init(&list, NULL, NULL, 0);
  /*********************************************************************************************/
  /* start curses mode                                                                         */
  /*********************************************************************************************/
  scrn = init_scr();
  if (scrn == NULL) {
    /* release memory before exiting */
    free_header(header);
    InstanceDetach(header->instance);
    fprintf(stderr, "Failed to start curses mode\n");
    ERR_MSG("Failed to start curses mode");
  }

  printw("Initializing db2topas ...\n");
  refresh();

  

  /*********************************************************************************************/
  /* init args for take_db2_snapshot thread */
  /*********************************************************************************************/
  snapshot_thrd_arg = (TakeDb2SnapshotArgs *) malloc(sizeof(TakeDb2SnapshotArgs)) ;
  if (snapshot_thrd_arg == NULL) {
    /* release memory before exiting */
    free_header(header);
    InstanceDetach(header->instance);
    /* end curses mode */
    delwin(scrn);
    endwin();
    fprintf(stderr, "Failed to initialize thread argument\n");
    ERR_MSG ("Failed to initialize thread argument");
  }
  snapshot_thrd_arg->header = header; 
  snapshot_thrd_arg->buffer_ptr = NULL ; /* buffer to hold monitor stream */
  snapshot_thrd_arg->buffer_sz = 0; /* estimated buffer size for snapshot output stream */
  /*********************************************************************************************/
  /* setup thread attribute                                                                    */
  /*********************************************************************************************/
  pthread_attr_init(&thrd_attr);
  pthread_attr_setdetachstate(&thrd_attr, PTHREAD_CREATE_JOINABLE);
  /*********************************************************************************************/
  /* Big Loop                                                                                  */
  /* Loop until user presses 'q' to exit                                                       */
  /*********************************************************************************************/
  while (keyboard != 107 && (keyboard = getch()) != 113) {
    if (sig_action == 2) {
      keyboard = 107;
      sig_action = 0;
    } else if (sig_action == 1) {
      sig_action = 0;
      delwin(scrn);
      endwin();
      scrn = init_scr();
      if (scrn == NULL) {
        WARN_MSG("Failed to resize curses window");
        break;
      }
    }
    /* there is a input from the keyboard */
    if (keyboard != ERR) {
      read_screen_input(keyboard, scrn, header, &list);
    }

    if (header->reinit_DB2SnapReq == TRUE) {
      rc = init_snapRequest( 
            &(snapshot_thrd_arg->db2SnapReq)
          , &(snapshot_thrd_arg->buffer_ptr)
          , &(snapshot_thrd_arg->buffer_sz)
          , (peek_snapReq(header))->type
          , (peek_snapReq(header))->node
          , (peek_snapReq(header))->dbname
          , (peek_snapReq(header))->agent_id
          , &list);
      if (rc != 0) {
        WARN_MSG("Couldn't init snapshot request");
        break;
      }
      /* take db2 snapshot */
      rc = pthread_create(&take_db2_snapshot_thrd_id, &thrd_attr       
        , take_db2_snapshot, (void *) snapshot_thrd_arg);             
                                                                   
      if (rc != 0) {                                                   
        WARN_MSG("Failed to start take_db2_snapshot thread");           
        break;
      } else {
        rc = pthread_join(take_db2_snapshot_thrd_id, &(snapshot_thrd_arg->status));
        if ( rc != 0 || (long) (snapshot_thrd_arg->status) != (long) 0 ) {
          sprintf(mesg, "pthread_join error rc: %d, status: %ld ", rc, (long) (snapshot_thrd_arg->status) );
          WARN_MSG(mesg);
          break;
        }
      }              
      header->reinit_DB2SnapReq = FALSE;
    }
    rc = parse_monitor_stream( 
          header
        , &list
        , snapshot_thrd_arg->buffer_ptr) ;
    if (rc != 0) {
      sprintf(mesg, "Couldn't parse monitor stream rc:%d", rc);
      WARN_MSG(mesg);
      break;
    }

    if ((peek_snapReq(header))->type == agent_id_cmdline) {
      rc = pop_snapReq(header);
      if (rc != 0) {
        WARN_MSG("pop_snapReq error!");
        break;
      }  
      /* reinit snapshot request since we are poping a request */ 
      header->reinit_DB2SnapReq = TRUE;                              
      continue;
    }

    /* take db2 snapshot */
    rc = pthread_create(&take_db2_snapshot_thrd_id, &thrd_attr       
      , take_db2_snapshot, (void *) snapshot_thrd_arg);             
                                                                   
    if (rc != 0) {                                                   
      WARN_MSG("Failed to start take_db2_snapshot thread");           
      break;
    } 

    /* sort list before printing */
    sort_list(header, &list);

    /* display the snapshot data and then sleep */
    refresh_screen(scrn, header, &list);
    sleep(header->interval);
    rc = pthread_join(take_db2_snapshot_thrd_id, &(snapshot_thrd_arg->status));
    if ( rc != 0 || (long) (snapshot_thrd_arg->status) != (long) 0 ) {
      sprintf(mesg, "pthread_join error rc: %d, status: %ld ", rc, (long) (snapshot_thrd_arg->status) );
      WARN_MSG(mesg);
      break;
    }
    /*********************************************************************************************/
  } /* end of big while loop */
  /*********************************************************************************************/

  /*********************************************************************************************/
  /* cleanup thread variables */
  /*********************************************************************************************/
  pthread_attr_destroy(&thrd_attr);
  /*********************************************************************************************/
  /* end curses mode                                                                           */
  /* clean up  before exiting                                                                  */
  /*********************************************************************************************/
  if (scrn != NULL) {
    delwin(scrn);
    endwin();
  }
  InstanceDetach(header->instance);
  /* release memory before exiting */
  free_snapshot_memory((snapshot_thrd_arg->db2SnapReq).pioRequestData
    , snapshot_thrd_arg->buffer_ptr);
  free_header(header);
  free(snapshot_thrd_arg); 
  db2list_destroy(&list);

  if (keyboard == 113)
    printf("\n\nExiting...\n");
  else
    printf("\n\nAborted...\n");

  sprintf(mesg, "Stopping %s", header->myname);
  INFO_MSG(mesg);

  /*pthread_exit((void *) rc);*/
  pthread_exit(0);

} /*end of Main */


/*********************************************************************************************/
/* check program input                                                                       */
/*********************************************************************************************/
int  parse_cmdline(int argc, char *argv[], Header *header) 
{
  char buffer[1024];
  int  c, rc = 0;
  SnapReq snapReq = *(peek_snapReq(header));
  while ((c = getopt(argc, argv, ":d:ghi:n:u:UP:r:")) != -1) {
    switch (c) {
    case 'h':
      show_help();
      return 1;
    case 'r':
      header->interval = atoi(optarg);
      break;
    case 'n':
      snapReq.node = atoi(optarg);
      break;
    case 'u':
      strncpy(header->user_id, optarg, USERID_SZ + 1);
      fprintf(stdout, "Enter password for %s: ", header->user_id);
      fgets(header->pswd, PSWD_SZ + 1, stdin); 
      chomp(header->pswd);
      break;
    case 'P':                                                
      strcpy(snapReq.client_nm, "");                         
      snapReq.agent_id = 0;                                  
      strncpy(buffer, optarg, USERID_SZ);                    
      strncpy(snapReq.prog_nm, strtrim(buffer), USERID_SZ);  
      header->col1_opt = prog_nm;                            
      break;                                                 
    case 'U':
      snapReq.type = utils;
      break;
    case 'i':
      strncpy(header->instance, optarg, SQL_INSTNAME_SZ + 1);
      break;
    case 'd':
      strncpy(snapReq.dbname, optarg, SQL_DBNAME_SZ + 1);
      break;
    case 'g':
      snapReq.node = -2;
      break;
    case ':':
      fprintf(stderr,                                         
          "Option -%c requires an operand\n", optopt);
      return 2;
    case '?':
      fprintf(stderr,                                       
          "Unrecognized option: -%c\n", optopt);
      return 2;
    }
  }/* end of command line arg check */


  strncpy(header->myname, strtrim(argv[0]), 10);
  strcat (header->myname, " 1.10");
  if (argc > 1 && optind < argc ) {
    strncpy(snapReq.auth_id, strtrim(argv[optind]), USERID_SZ);
    if (isdigit(snapReq.auth_id[0])) {
      snapReq.type = agent_id;
      snapReq.agent_id = atoi(argv[optind]);
      memset(snapReq.auth_id, '\0', USERID_SZ + 1);
    }
  }

  
  rc = push_snapReq(header, &snapReq);
  if ( rc < 0)
    return rc;
  else
    return 0;
}


/*********************************************************************************************/
/* show how to start the program                                                             */
/*********************************************************************************************/
int  show_help(void) 
{
  char  *usage =                                                             
  "db2topas: Display application snapshot using curses                  \n"
  "                                                                     \n"
  "Usage:                                                               \n"
  "                                                                     \n"
  " db2topas [[-i <instance> [-u <id>]] [-d <dbname>] [-P program name] \n"
  "          [-r <n>] [-g| -n node_num] [authid|agentid]                \n"
  "                                                                     \n"
  "Where:                                                               \n"
  "      -i <instance>  attach to this instance                         \n"
  "      -u <id>        attach to instance using this id (prompt for pswd)\n"
  "      -d <dbname>    take snapshot on this database only             \n"
  "      -r <n>         take snapshot at every n seconds (default 2s)   \n"
  "      -g             take global snapshot                            \n"
  "      -n             send snapshot req to this node                  \n"
  "      -P             display appl snpashot with this prog name only  \n"
  "      authid|agentid display appls with this agentid or authid       \n";

  fprintf(stderr, "%s", usage);
  fprintf(stderr, "\n\ndb2topas 1.10 compiled on %s at %s\n", __DATE__, __TIME__);
  return 0;

} /* show_help */

/*********************************************************************************************/
static void handler(int sig)
{
  char buffer[1024];
  sprintf (buffer, "Received signal %d", sig);
  WARN_MSG(buffer);
  if (sig == SIGWINCH)
    sig_action = 1;
  else
    sig_action = 2;

}
