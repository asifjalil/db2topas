#include <math.h>
#include "db2curses.h"

/* helper functions */
int  term_rows (int fd);
int  term_cols (int fd);

/* *****************************************************************************************/
/* Helper functions to print rows on the curses screen                                     */
/* *****************************************************************************************/
void  show_curses_help(void);
int   update_dbase_screen(WINDOW *scrn, Header *header);
void  update_snapshot_screen(WINDOW *scrn, Header *header, DB2List *list);
int   print_list(WINDOW *scrn, int input_row, int max_row, Header *header, DB2List *list);

void  print_column_header_APPLS        (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_LOCKS        (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_STMTS        (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_AGENTID_SS   (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_UTILS        (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_TBSPACE      (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_TABLE        (Header *header, WINDOW *scrn, int row, int scrn_col) ;
void  print_column_header_IO           (Header *header, WINDOW *scrn, int row, int scrn_col) ;

void  print_row_APPLS(WINDOW *scrn, int row, Appl *data,  Header *header);
void  print_row_LOCKS(WINDOW *scrn, int row, Lock *data,  Header *header);
void  print_row_STMTS(WINDOW *scrn, int row, Stmt *data,  Header *header);
int   print_row_AGENTID(WINDOW *scrn, int row, Appl *data,  Header *header);
void  print_row_IO(WINDOW *scrn, int row, Appl *data,  Header *header);
void  print_row_AGENTID_SS (WINDOW *scrn, int row, Appl_SS *data,  Header *header);
void  print_row_UTILS(WINDOW *scrn, int row, Util *data,  Header *header);
void  print_row_TBSPACE(WINDOW *scrn, int row, Tbspace *data, Header *header);
void  print_row_TABLE(WINDOW *scrn, int row, Table *data, Header *header);

/******************************************************************************************/
/* Helper functions to convert various numerical result into string                       */
/******************************************************************************************/
char  *table_type_STRING(sqluint32 type, char *typeString);
char  *lock_status_STRING(sqluint32 status, char *statusString);
char  *lock_mode_STRING(sqluint32 mode, char *modeString);
char  *lock_type_STRING(sqluint32 type, char *typeString);
char  *lock_obj_STRING(sqluint32 type, char *objString);
char  *tbspace_state_STRING(sqluint32 state, char *stateString);
char  *util_type_STRING(sqluint32 type, char *typeString);
char  *util_status_STRING(sqluint32 status, char *statusString);
char  *appl_status_STRING(sqluint32 status, char *statusString);
char  *stmt_op_STRING(sqluint32 op, char *statusString);
char  *ss_status_STRING(sqluint16 status, char *statusString);
char  *cpu_STRING(sqluint64 elapsed_exec_microsec, sqluint64 cpu_microsec, char *cpuString);
float  cpu_PCT(sqluint32 elapsed_exec_microsec, sqluint32 cpu_microsec);
char  *pct_STRING(sqluint64 completed, sqluint64 total, char *pctString);
char  *hit_STRING(sqluint64 physical, sqluint64 logical, char *pctString);
char  *uint64_STRING(sqluint64 num , char *numString);
char  *uint64byte_STRING(sqluint64 num , char *numString);
char  *uint32_STRING(sqluint32 num , char *numString);
char  *uint64rate_STRING(double interval, sqluint64 num , char *numString);
char  *uint64byte_rate_STRING(double interval, sqluint64 num , char *numString);
/* 
   unint32rate_STRING is used to calculate 
   RR/s, RW/s, BPLR, BPPR, TQ-In/s, TQ-Out/s, [S,I,U,D]/s
*/
char  *uint32rate_STRING(double interval, sqluint32 num , char *numString);
/* used to calculate io read/write time        */
char  *uint32tm_STRING(sqluint32 interval, sqluint32 num , char *numString);


/******************************************************************************************/
/******************************************************************************************/
WINDOW *init_scr()
{
  /* new window to display
     snapshot in curses */
  WINDOW * scrn = NULL;
  int  term_col = 0;
  int  term_row = 0;
  int  max_row = 0;
  int  max_col = 0;
  /* make sure that we can access terminal for size calc */
  if (isatty(STDIN_FILENO) == 0) {
    fprintf(stderr, "tty error!\n");
    exit(1);
  } else {
    /* create a big enough stdscr                          
       the mother curses screen */
    setenv("COLUMNS", "134", 1);
    setenv("LINES", "100", 1);
    /* create stdscr of 134 cols and 100 rows */
    initscr();
    clear();
    noecho();             /* don't echo user keyboard input */

    curs_set(0);          /* don't display cursor */

    cbreak();            /* don't buffer keyboard input, send everything to me */

    nodelay(stdscr, 1);  /* don't wait for keyboard input */

    /* now determine terminal size                            
       assume it is less than or equal to                     
       134 cols and 100 rows */
    term_col =  term_cols(STDIN_FILENO);
    term_row =  term_rows(STDIN_FILENO);
    /* create a window to display snapshot */
    scrn = newwin(term_row, term_col, 0, 0);
    /* Get the terminal settings and make sure we have more than 132 columns per screen */
    getmaxyx(scrn, max_row, max_col);
    if (max_col <= MIN_DISPLAY_WIDTH || max_row < 6) {
      int  min_col = MIN_DISPLAY_WIDTH + 1;
      delwin(scrn);
      endwin(); /* end curses mode */
      fprintf(stderr, "This application needs minimum 6 rows and %d columns\n", min_col);
      fprintf(stderr, "Current screen only has %d rows and %d columns \nExiting...\n", max_row, max_col);
      return NULL;
    }

    keypad(stdscr, TRUE);
    return scrn;

  } /* end of else */


} /* end of init_scr */




/******************************************************************************************/
/******************************************************************************************/
int  term_rows (int fd)
{
  struct winsize size;
  if (ioctl(fd, TIOCGWINSZ, (char *) & size) < 0) {
    fprintf(stderr, "TIOCGWINSZ error");
    exit(1);
  }
  return size.ws_row;
}



/******************************************************************************************/
/******************************************************************************************/
int  term_cols (int fd)
{
  struct winsize size;
  if (ioctl(fd, TIOCGWINSZ, (char *) & size) < 0) {
    fprintf(stderr, "TIOCGWINSZ error");
    exit(1);
  }
  return size.ws_col;
}


/******************************************************************************************/
int  update_dbase_screen(WINDOW *scrn, Header *header)
{
  sqluint64 db_io_type_all_delta
      , db_io_type_read_delta 
      , db_io_type_write_delta
      , db_io_type_data_delta
      , db_io_type_idx_delta
      , db_io_type_xml_delta
      , db_io_type_temp_delta
      ;
  int  i, row, col, scrn_row, scrn_col;
  sqluint32 dio_num;
  char  buffer[1024]  ,                                                                              
  rlogString[256],
  wlogString[256],
  logString[256],
  memString[256],
  lockHeapString[256],
  utilHeapString[256],
  sortHeapString[256],
  cpuString[256],
  dioString[256],
  rbioString[256],
  rlbioString[256],
  rbioTmString[256],
  wbioString[256],
  wbioTmString[256],
  tempString[256],
  db_io_type_string[256],
  db_io_type_string1[256]
  ;

  getmaxyx(scrn, scrn_row, scrn_col);

  if (header->spin == 0 || header->spin == 2) {
    sprintf(tempString, "(-)");
    header->spin++;
  } else if (header->spin == 1) {
    sprintf(tempString, "(/)");
    header->spin++;
  } else {
    sprintf(tempString, "(\\)");
    header->spin = 0;
  }
  memset(buffer, '\0', 1024);
  strcpy(buffer, tempString);
  sprintf(tempString, "Refresh:%.2lfs ",  header->snapshot_timestamp_delta);
  strcat(buffer, tempString);

  if ((peek_snapReq(header))->node == -2)
    sprintf(tempString, "%s monitoring inst. %s(node:All)"
        , header->user_id, header->instance);
  else if ((peek_snapReq(header))->node == -1)
    sprintf(tempString, "%s monitoring inst. %s(node:%s)"
        , header->user_id, header->instance, header->db2node);
  else
    sprintf(tempString, "%s monitoring inst. %s(node:%d)"
        , header->user_id, header->instance, (peek_snapReq(header))->node);

  strcat(buffer, tempString);
  col = (min(scrn_col, MAX_DISPLAY_WIDTH) / 2) - strlen(buffer) ;
  while (col > 0) {
    strcat(buffer, " ");
    col--;
  }

  char tbspace_name [SQLUH_TABLESPACENAME_SZ+1];
  strncpy(tbspace_name, (peek_snapReq(header))->tbspace, SQLUH_TABLESPACENAME_SZ);
  switch ((peek_snapReq(header))->type) {
  case appls:
    strcat(buffer, "{APPLS}");
    break;
  case stmts:
    strcat(buffer, "{STMTS}");
    break;
  case agent_id:
    strcat(buffer, "{AGENT_ID}");
    break;
  case agent_id_detl:
    strcat(buffer, "{AGENT_ID_SS}");
    break;
  case utils:
    strcat(buffer, "{UTILS}");
    break;
  case utils_id:
    strcat(buffer, "{UTIL_ID}");
    break;
  case tbspace:
    strcat(buffer, "{TBSPACE}");
    break;
  case tbspace_id:
    sprintf(tempString,"{TBSPACE:%s}", tbspace_name);
    strcat(buffer, tempString);
    break;
  default:
    strcat(buffer, "{N/A}");
  }

  sprintf(tempString,"{%d}", num_snapReq(header));
  strcat(buffer,tempString);
  col = min(scrn_col, MAX_DISPLAY_WIDTH) - strlen(buffer) - strlen(header->myname) ;
  while (col > 0) {
    strcat(buffer, " ");
    col--;
  }

  if (header->spin == 0 || header->spin == 2)
    strcat(buffer, header->myname);
  else
    strcat(buffer,"\"h\"-help,\"q\"-quit");

  /* print db2topas header, first row */
  for (col = 0; col < scrn_col; col++)
    mvwaddch(scrn, 0, col, ' ');
  mvwaddnstr(scrn, 0, 0, buffer, scrn_col);

  memset(buffer, '\0', 1024);

  /* prepare DBASE stats */

  /* DB Cpu usg */

  if ((peek_snapReq(header))->type == appls) {                                                            
    sprintf(cpuString, "CpuType (All Appls):");                                                    
  } else if ((peek_snapReq(header))->type == agent_id || (peek_snapReq(header))->type == agent_id_detl) { 
    sprintf(cpuString, "CpuType (agent: %u):", (peek_snapReq(header))->agent_id);                                        
  } else
    sprintf(cpuString, "");

  if ( header->db_scpu_used_delta > 0 || header->db_ucpu_used_delta > 0 ) {
    int db_ucpu_used_pct = 0;
    db_ucpu_used_pct = (int) (100 * ( ((double) header->db_ucpu_used_delta)/ 
        (header->db_scpu_used_delta  +  header->db_ucpu_used_delta)
        ));
    for(i=5; i <= db_ucpu_used_pct && i<=100  ; i+=5)
      strcat(cpuString, "u");

    for(; i <=  100 ; i+=5)
      strcat(cpuString, "s");

  }

  /* DB IO usg */
  uint64rate_STRING(header->snapshot_timestamp_delta
      , delta(header->t2_db_buffered_rio, header->t1_db_buffered_rio), rbioString);
  uint64rate_STRING(header->snapshot_timestamp_delta
      , delta(header->t2_db_buffered_wio, header->t1_db_buffered_wio), wbioString);

  uint32tm_STRING(delta(header->t2_db_buffered_rio, header->t1_db_buffered_rio), 
      delta(header->t2_db_bpr_tm,header->t1_db_bpr_tm), rbioTmString);
  uint32tm_STRING(delta(header->t2_db_buffered_wio, header->t1_db_buffered_wio), 
      delta(header->t2_db_bpw_tm,header->t1_db_bpw_tm), wbioTmString);

  if ((delta(header->t2_db_direct_io_reqs, header->t1_db_direct_io_reqs)) > 0) {
    /* actual sector size is not known, so display io request instead */
    dio_num = delta(header->t2_db_direct_io_reqs, header->t1_db_direct_io_reqs);

    uint32rate_STRING(header->snapshot_timestamp_delta, dio_num, dioString);
  } else
    sprintf(dioString, "");

  /* DB Log usg */
  uint64rate_STRING(header->snapshot_timestamp_delta, 
      delta(header->t2_db_log_writes, header->t1_db_log_writes), wlogString);
  uint64rate_STRING(header->snapshot_timestamp_delta, 
      delta(header->t2_db_log_reads, header->t1_db_log_reads), rlogString);
  pct_STRING(header->db_log_used, (header->db_log_avail + header->db_log_used), logString);

  /* DB Mem usg */
  uint64byte_STRING(header->db_memusg, memString);
  uint64byte_STRING(header->db_lockheap, lockHeapString);
  uint64byte_STRING(header->db_utilheap, utilHeapString);
  uint64byte_STRING(header->db_sortheap, sortHeapString);

  /* DB IO type */
  strcpy(db_io_type_string, "IoType:");
  strcpy(db_io_type_string1,"      :");
  db_io_type_read_delta = delta(header->t2_db_io_type_read, header->t1_db_io_type_read);
  db_io_type_data_delta = delta(header->t2_db_io_type_data, header->t1_db_io_type_data);
  db_io_type_idx_delta = delta(header->t2_db_io_type_idx, header->t1_db_io_type_idx);
  db_io_type_temp_delta = delta(header->t2_db_io_type_temp, header->t1_db_io_type_temp);
  db_io_type_xml_delta = delta(header->t2_db_io_type_xml, header->t1_db_io_type_xml);

  db_io_type_write_delta = delta(header->t2_db_io_type_write, header->t1_db_io_type_write);

  /* BPLR/s */
  uint64rate_STRING(header->snapshot_timestamp_delta                            
      , db_io_type_read_delta , rlbioString);

  if ( (db_io_type_read_delta + db_io_type_write_delta) > 0) {
    sqluint64 db_io_type_read_pct = 0;
    db_io_type_read_pct = (sqluint64) (100 * (((double) db_io_type_read_delta)/ (db_io_type_read_delta + db_io_type_write_delta)));
    for(i=5; i <= db_io_type_read_pct && i<=100  ; i+=5)
      strcat(db_io_type_string, "r");

    for(; i <=  100 ; i+=5)
      strcat(db_io_type_string, "w");

  }

  if ( (db_io_type_all_delta = db_io_type_data_delta
      + db_io_type_idx_delta
      + db_io_type_temp_delta
      + db_io_type_xml_delta) > 0 ) {

    int db_io_type_data_pct = (int) (100 * (((double) db_io_type_data_delta) / db_io_type_all_delta));
    int db_io_type_idx_pct = (int) (100 * (((double) db_io_type_idx_delta) / db_io_type_all_delta));
    int db_io_type_temp_pct = (int) (100 * (((double) db_io_type_temp_delta) / db_io_type_all_delta));
    int db_io_type_xml_pct = 100 -   (db_io_type_data_pct + db_io_type_idx_pct + db_io_type_temp_pct);

    for(i=5; i <= db_io_type_data_pct  ; i+=5)
      strcat(db_io_type_string1, "d");

    for(i=5; i <= db_io_type_idx_pct  ; i+=5)
      strcat(db_io_type_string1, "i");

    for(i=5; i <= db_io_type_temp_pct  ; i+=5)
      strcat(db_io_type_string1, "t");

    for(i=5; i <= db_io_type_xml_pct  ; i+=5)
      strcat(db_io_type_string1, "x");

  }

  /****************************************************************************/
  /* build the header string for the second line */
  /*******************************************************************************/
  row = 1;
  if (header->show_dbase_list) {
    for (; row < 5; row++)
      for (col = 0; col < scrn_col; col++)
        mvwaddch(scrn, row, col, ' ');
  
    mvwaddstr(scrn,1,HANDLE,"DBASE");
  
    if (strlen((peek_snapReq(header))->dbname) > 0)
      sprintf(buffer, "%s", (peek_snapReq(header))->dbname);
    else
      sprintf(buffer, "ALL");
    mvwaddnstr(scrn,2,HANDLE,buffer,16);
  
    /* IO type */
    mvwaddstr(scrn,3, HANDLE,  db_io_type_string);
    mvwaddstr(scrn,4, HANDLE,  db_io_type_string1);

    if ( CPU + strlen(cpuString) < scrn_col)
      mvwaddstr(scrn,3,CPU,cpuString);

    sprintf(buffer, "HitRatio:%s", hit_STRING(delta(header->t2_db_buffered_rio, header->t1_db_buffered_rio)
     , db_io_type_read_delta , tempString  ));  
    mvwaddstr(scrn,4,CPU,buffer);
  
    /* Memory */
    mvwaddstr(scrn,1,STATUS_CHANGE,"Mem");
    mvwaddstr(scrn,2,STATUS_CHANGE,memString);
  
    mvwaddstr(scrn,1,STATUS_CHANGE+7,"UtlHeap");
    mvwaddstr(scrn,2,STATUS_CHANGE+7,utilHeapString);
  
    mvwaddstr(scrn,1,AUTHID,"SrtHeap");
    mvwaddstr(scrn,2,AUTHID,sortHeapString);
  
    mvwaddstr(scrn,1,STATUS,"LckHeap");
    mvwaddstr(scrn,2,STATUS,lockHeapString);
  
  
    /* Direct IO */
    mvwaddstr(scrn,1,RR,"DIO");
    mvwaddstr(scrn,2,RR,dioString);
  
    /* Buffered IO */
    if (RW + strlen("BPLR ") < scrn_col) {
      mvwaddstr(scrn,1,RW,"BPLR");
      mvwaddstr(scrn,2,RW,rlbioString);
    }
  
    if (BPLR + COL_WIDTH < scrn_col) {
      mvwaddstr(scrn,1,BPLR,"BPPR");
      mvwaddstr(scrn,2,BPLR,rbioString);
    }
  
    if (BPPR + COL_WIDTH < scrn_col) {
      mvwaddstr(scrn,1,BPPR,"BPPW");
      mvwaddstr(scrn,2,BPPR,wbioString);
    }
  
    if (TQR + COL_WIDTH < scrn_col) {
      mvwaddstr(scrn,1,TQR,"BPPR(tm)");
      mvwaddstr(scrn,2,TQR,rbioTmString);
    }
  
    if (TQW + COL_WIDTH < scrn_col) {
      mvwaddstr(scrn,1,TQW,"BPPW(tm)");
      mvwaddstr(scrn,2,TQW,wbioTmString);
    }
  
    /* log usage */
    if (SIUD + 6 < scrn_col) {
      mvwaddstr(scrn,1,SIUD,"LogR");
      mvwaddstr(scrn,2,SIUD,rlogString);
    }
  
    if (LOG < scrn_col) {
      mvwaddstr(scrn,1,SIUD+7,"LogW");
      mvwaddstr(scrn,2,SIUD+7,wlogString);
    }
  
    if (header->smallest_log_avail_node == 1000)
      sprintf(tempString, "{L:%s}", logString);
    else
      sprintf(tempString, "{L(%d):%s}"
          , header->smallest_log_avail_node, logString);
  
    if (LOG+COL_WIDTH < scrn_col) {
      mvwaddstr(scrn,1,LOG,"LogUsd");
      mvwaddstr(scrn,2,LOG,logString);
    } else
      mvwaddstr(scrn,1,scrn_col-3,">>");
  
  
    /***********************************************************************************************************/
    /* reset to 0 before taking another snapshot
       otherwise the numbers will keep increasing */
    /***********************************************************************************************************/
    header->appls_connected = 0;
    header->appls_executing = 0;
    header->db_assoc_agents = 0;
    header->db_memusg = 0;
    header->db_sortheap = 0;
    header->db_lockheap = 0;
    header->db_utilheap = 0;
  
  
    header->t1_db_buffered_rio = header->t2_db_buffered_rio;
    header->t1_db_buffered_wio = header->t2_db_buffered_wio;
    header->t2_db_buffered_rio = 0;
    header->t2_db_buffered_wio = 0;
    header->t1_db_bpr_tm = header->t2_db_bpr_tm;
    header->t1_db_bpw_tm = header->t2_db_bpw_tm;
    header->t2_db_bpr_tm = 0;
    header->t2_db_bpw_tm = 0;
  
    header->t1_db_direct_io = header->t2_db_direct_io;
    header->t2_db_direct_io = 0;
    header->t1_db_direct_io_reqs = header->t2_db_direct_io_reqs;
    header->t2_db_direct_io_reqs = 0;
  
    header->t1_db_io_type_read = header->t2_db_io_type_read;
    header->t2_db_io_type_read = 0;
    header->t1_db_io_type_data = header->t2_db_io_type_data;
    header->t2_db_io_type_data = 0;
    header->t1_db_io_type_idx = header->t2_db_io_type_idx;
    header->t2_db_io_type_idx = 0;
    header->t1_db_io_type_xml = header->t2_db_io_type_xml;
    header->t2_db_io_type_xml = 0;
    header->t1_db_io_type_temp = header->t2_db_io_type_temp;
    header->t2_db_io_type_temp = 0;
    header->t1_db_io_type_write = header->t2_db_io_type_write;
    header->t2_db_io_type_write = 0;
  
    header->t1_db_log_writes =  header->t2_db_log_writes;
    header->t2_db_log_writes = 0;
    header->t1_db_log_reads =  header->t2_db_log_reads;
    header->t2_db_log_reads = 0;
    header->db_log_avail = 0;
    header->db_log_used = 0;
    header->smallest_log_avail_node = 1000;
  } /* end of show_db_list */ 

  return row ;

} /* end of update_dbase_screen */

/******************************************************************************************/
void  update_snapshot_screen(WINDOW *scrn, Header *header, DB2List *list)
{
  int  row, col, scrn_row, scrn_col;

  /* -getmaxyx gives the number of total  
     rows and columns in a WINDOW         
     -but rows and columns start from 0   
      when printing                       
  */
  getmaxyx(scrn, scrn_row, scrn_col);

  row = update_dbase_screen(scrn, header); 

  if (db2list_get_type(list) == agent_id) {
    if (header->show_appl_list)
      row = print_list(scrn,row, scrn_row, header, list);
    if (db2list_head(list) != NULL) {
      if (header->show_stmt_list)
        row = print_list(scrn,row, scrn_row, header, 
          &(((Appl *) db2list_data(db2list_head(list)))->stmt_list));
      if (header->show_lock_list)
        row = print_list(scrn,row, scrn_row, header, 
          &(((Appl *) db2list_data(db2list_head(list)))->lock_list));
    }
  } else
    row = print_list(scrn,row, scrn_row, header, list);

  /* clear the other rows that have no new rows */
  for (; row < scrn_row; row++) {
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, row, col, ' ');
    }
  }
  return;
}
/******************************************************************************************/
int  print_list(WINDOW *scrn, int input_row, int max_row,  Header *header, DB2List *list)
{

  DB2ListElmt * countElement,
      *element;
  int  count, row, col,scrn_row,scrn_col;
  char  buffer[1024];
  Boolean highlight_list_head = FALSE;



  /* -getmaxyx gives the number of total
     rows and columns in a WINDOW
     -but rows and columns start from 0
      when printing
  */
  getmaxyx(scrn, scrn_row, scrn_col);
  if (input_row >= scrn_row)
    return scrn_row;

  if (scrn_row > max_row)
    scrn_row = max_row;

  /************************************************************************************************************/
  /* print column headers */
  /*************************************************************************************************************/
  switch (db2list_get_type(list)) {
  case appls:
  case agent_id:
    for(col=0; col < scrn_col; col++)
      mvwaddch(scrn,input_row,col,' ');
    mvwaddstr(scrn, input_row, 0, "#### Appl List ####");                      
    for(col = strlen("#### Appl List ####"); col < scrn_col; col++)
     mvwaddch(scrn, input_row, col, '#');
    input_row+=1;
    print_column_header_APPLS(header, scrn, input_row, scrn_col);
    break;
  case stmts:
    for(col=0; col < scrn_col; col++)                       
      mvwaddch(scrn,input_row,col,' ');                     
    mvwaddstr(scrn, input_row, 0, "#### Stmt List ####");                      
    for(col = strlen("#### Stmt List ####"); col < scrn_col; col++)
     mvwaddch(scrn, input_row, col, '#');
    input_row+=1;                                           

    print_column_header_STMTS(header, scrn, input_row, scrn_col);
    break;
  case agent_id_detl:
    for(col=0; col < scrn_col; col++)                               
      mvwaddch(scrn,input_row,col,' ');                             
    mvwaddstr(scrn, input_row, 0, "#### Subsection List ####");           
    for(col = strlen("#### Subsection List ####"); col < scrn_col; col++) 
     mvwaddch(scrn, input_row, col, '#');                           
    input_row+=1;                                                   
    if ( strlen((peek_snapReq(header))->stmt_text) > 0) {
      for(col=0; col < scrn_col; col++)
        mvwaddch(scrn,input_row,col,' ');
      mvwaddnstr(scrn,input_row,0, (peek_snapReq(header))->stmt_text,scrn_col);
      input_row+=1;
    }
    print_column_header_AGENTID_SS(header, scrn, input_row, scrn_col);
    break;
  case utils:
  case utils_id:
    print_column_header_UTILS(header, scrn, input_row, scrn_col);
    break;
  case tbspace:
    for(col=0; col < scrn_col; col++)                               
      mvwaddch(scrn,input_row,col,' ');                             
    mvwaddstr(scrn, input_row, 0, "#### Tbspace List ####");           
    for(col = strlen("#### Tbspace List ####"); col < scrn_col; col++) 
     mvwaddch(scrn, input_row, col, '#');                           
    input_row+=1;                                                   

    print_column_header_TBSPACE(header, scrn, input_row, scrn_col);
    break;
  case tbspace_id:
    for(col=0; col < scrn_col; col++)                               
      mvwaddch(scrn,input_row,col,' ');                             
    sprintf(buffer, "#### Table List(%s) ####", (peek_snapReq(header))->tbspace);
    mvwaddstr(scrn, input_row, 0, buffer);           
    for(col = strlen(buffer); col < scrn_col; col++) 
     mvwaddch(scrn, input_row, col, '#');                           
    input_row+=1;                                                   

    print_column_header_TABLE(header, scrn, input_row, scrn_col);
    break;
  case locklist:
    for(col=0; col < scrn_col; col++)
      mvwaddch(scrn,input_row,col,' ');
    mvwaddstr(scrn, input_row, 0, "#### Lock List ####");                      
    for(col = strlen("#### Lock List ####"); col < scrn_col; col++)
     mvwaddch(scrn, input_row, col, '#');
    input_row+=1;
    print_column_header_LOCKS(header, scrn, input_row, scrn_col);
    break;
  }

  input_row+=1;

  if (db2list_size(list) < 1 ) {                                    
    for (col = 0; col < scrn_col; col++)                            
      mvwaddch(scrn, input_row , col, ' ');                         
    mvwaddstr(scrn, input_row, 0, "==> Snapshot unavailable! <=="); 
    row = input_row + 1;                                            
    return row;                                                     
  }                                                                 

  /* -check where should we start printing the list
     -it depends on the highlighted element
  */
  if ( (element = db2list_get_highlighted_element(list)) == NULL) {
    element = db2list_head(list);
    highlight_list_head = TRUE;
  }

  for (row = input_row; element!= NULL && db2list_is_head(element) == FALSE
      ; element = db2list_prev(element)) {
    row++;
    if (row >= (scrn_row -1) )
      break;
  }

  if (db2list_is_head(element) == FALSE) {
    for (countElement =  element, count = 0; countElement != NULL 
        ; countElement = db2list_prev(countElement), count++) {
    }

    /* clear the row first                  */
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, input_row , col, ' ');
    }
    sprintf(buffer, "----------(%d)----------", count);
    mvwaddstr(scrn, input_row,0, "A");
    mvwaddstr(scrn, input_row, ((min(MAX_DISPLAY_WIDTH, scrn_col) - strlen(buffer)) / 2) - 1, buffer);


    element = db2list_next(element);
    row = input_row + 1;
  } else {
    row = input_row;
  }


  for (; element != NULL; element = db2list_next(element), row++) {
    if (row >= (scrn_row - 1))
      break;

    /****************************************/
    /* highlight the element/row */
    /*******************************************/
    if (db2list_get_type(list) == agent_id) {
      /* don't highlight */
    } else if (db2list_get_type(list) == stmts) {
      if (header->snapScr == wstmt_list  
          && (element->highlight == TRUE || highlight_list_head == TRUE)) {
        wattron(scrn, A_REVERSE);
        for (col = 0; col < scrn_col; col++)
          mvwaddch(scrn, row, col, ' ');
      } 
    } else if (db2list_get_type(list) == locklist) {
      if (header->snapScr == wlock_list  
          && (element->highlight == TRUE || highlight_list_head == TRUE)) {
        wattron(scrn, A_REVERSE);
        for (col = 0; col < scrn_col; col++)
          mvwaddch(scrn, row, col, ' ');
      } 
    } else if (element->highlight == TRUE || highlight_list_head == TRUE) {
      wattron(scrn, A_REVERSE);
      for (col = 0; col < scrn_col; col++)
        mvwaddch(scrn, row, col, ' ');
    }

    switch (db2list_get_type(list) ) {
    case appls:
      print_row_APPLS(scrn, row, db2list_data(element), header);
      break;
    case stmts:
      print_row_STMTS(scrn, row, db2list_data(element), header);
      break;
    case agent_id:
      row = print_row_AGENTID(scrn, row, db2list_data(element), header);
      /* subtract 1 b/c for loop will increment row by 1 */
      row -=1;
      break;
    case agent_id_detl:
      print_row_AGENTID_SS(scrn, row, db2list_data(element), header);
      break;
    case utils:
    case utils_id:
      print_row_UTILS(scrn, row, db2list_data(element), header);
      break;
    case tbspace:
      print_row_TBSPACE(scrn, row, db2list_data(element), header);
      break;
    case tbspace_id:
      print_row_TABLE(scrn, row, db2list_data(element), header);
      break;
    case locklist:
      print_row_LOCKS(scrn, row, db2list_data(element), header);
      break;
    }

    /****************************************/                                
    /* turn off highlight the element/row */                                           
    /*******************************************/                             
    if (db2list_get_type(list) == agent_id) {                                  
      /* don't highlight */                                                   
    } else if (db2list_get_type(list) == stmts) {                             
      if (header->snapScr == wstmt_list                                       
          && (element->highlight == TRUE || highlight_list_head == TRUE))    
        wattroff(scrn, A_REVERSE);                                             
    } else if (db2list_get_type(list) == locklist) {                          
      if (header->snapScr == wlock_list                                       
          && (element->highlight == TRUE || highlight_list_head == TRUE))    
        wattroff(scrn, A_REVERSE);                                             
    } else if (element->highlight == TRUE || highlight_list_head == TRUE) {   
      wattroff(scrn, A_REVERSE);                                               
    }                                                                         
    highlight_list_head = FALSE;

  } /* end of printing list */

  if (element != NULL) {
    for (count = 0; element != NULL       
        ; element = db2list_next(element), count++) {
    }

    sprintf(buffer, "----------(%d)----------", count);
    for(col=0; col < scrn_col; col++)
      mvwaddch(scrn, scrn_row - 1,col, ' ');

    mvwaddch(scrn, scrn_row - 1, 0,'V');
    mvwaddstr(scrn, scrn_row - 1, ((min(MAX_DISPLAY_WIDTH, scrn_col) - strlen(buffer)) / 2) - 1, buffer);
    row = scrn_row;
  }

  /* set delete flag to TRUE in each list element */
  for (element = db2list_head(list) ; element != NULL; element = db2list_next(element))
    element->delete = TRUE;

  return row ;
} /* update_snapshot_screen */


/******************************************************************************************/
/******************************************************************************************/
int  refresh_screen(WINDOW *scrn, Header *header, DB2List *list)
{
  if (scrn == NULL)
    return 1;

  if (header != NULL && list != NULL) {
    /* snapshot timestamp is in milli-seconds */                                                                    
    header->snapshot_timestamp_delta = timestampdiff(header->t2_snapshot_timestamp, header->t1_snapshot_timestamp); 
    /* convert to second */                                                                                         
    header->snapshot_timestamp_delta /= 1000000.0;                                                                  
    update_snapshot_screen(scrn, header, list);
    header->t1_snapshot_timestamp = header->t2_snapshot_timestamp; 
  } else
    return 1;
  /* display all the updates */
  wrefresh(scrn);

  return 0;
} /* refresh_screen_screen */


/******************************************************************************************/
/******************************************************************************************/
char  *stmt_op_STRING(sqluint32 op, char *opString)
{
  switch (op) {
  case SQLM_PREPARE :
    sprintf(opString, "Prep");
    break;
  case SQLM_EXECUTE :
  case SQLM_EXECUTE_IMMEDIATE:
  case SQLM_PREP_EXEC        :
    sprintf(opString, "Exec");
    break;
  case SQLM_OPEN             :
  case SQLM_PREP_OPEN        :
    sprintf(opString, "Open");
    break;
  case SQLM_FETCH            :
    sprintf(opString, "Fetch");
    break;
  case SQLM_CLOSE            :
    sprintf(opString, "Close");
    break;
  case SQLM_DESCRIBE         :
    sprintf(opString, "Dscribe");
    break;
  case SQLM_STATIC_COMMIT    :
    sprintf(opString, "Commit");
    break;
  case SQLM_PREP_COMMIT      :
    sprintf(opString, "P Commit");
    break;
  case SQLM_STATIC_ROLLBACK  :
    sprintf(opString, "RB");
    break;
  case SQLM_FREE_LOCATOR     :
    sprintf(opString, "Free Loc");
    break;
  case SQLM_CALL             :
    sprintf(opString, "Call");
    break;
  case SQLM_SELECT           :
    sprintf(opString, "Select");
    break;
  case SQLM_COMPILE          :
    sprintf(opString, "Compile");
    break;
  case SQLM_SET              :
    sprintf(opString, "Set");
    break;
  case SQLM_RUNSTATS    :
    sprintf(opString, "Runstats");
    break;
  case SQLM_REORG       :
    sprintf(opString, "Reorg");
    break;
  case SQLM_REBIND      :
    sprintf(opString, "Rebnd");
    break;
  case SQLM_REDIST      :
    sprintf(opString, "Redist");
    break;
  case SQLM_GETTA       :
  case SQLM_GETAA       :
    sprintf(opString, "Get Auth");
    break;
  case SQLM_GETNEXTCHUNK:
    sprintf(opString, "DRDA chunk");
    break;
  case SQLM_DRPPKG      :
    sprintf(opString, "Drp pkg");
    break;
  default:
    sprintf(opString, "N/A");
    break;
  }
  return opString;

} /* stmt_op_STRING */


/******************************************************************************************/
/******************************************************************************************/
/*****************************************************************/
char  *ss_status_STRING(sqluint16 status, char *statusString)
{
  switch (status) {
  case SQLM_SSEXEC:
    sprintf(statusString, "Exec");
    break;
  case SQLM_SSTQ_WAIT_TO_RCV:
    sprintf(statusString, "Rcv on TQ");
    break;
  case SQLM_SSTQ_WAIT_TO_SEND:
    sprintf(statusString, "Snd on TQ");
    break;
  case SQLM_SSCOMPLETED:
    sprintf(statusString, "Done");
    break;
  default:
    sprintf(statusString, "N/A");
    break;
  }

  return statusString;

} /* ss_status_STRING */


/******************************************************************************************/
/******************************************************************************************/
char  *appl_status_STRING (sqluint32 status, char *statusString)
{
  switch (status) {
  case SQLM_INIT:
    sprintf(statusString, "Init");
    break;
  case SQLM_CONNECTPEND:
    sprintf(statusString, "Conn Pend");
    break;
  case SQLM_CONNECTED:
    sprintf(statusString, "Connected");
    break;
  case SQLM_UOWEXEC:
    sprintf(statusString, "UOW Exec");
    break;
  case SQLM_UOWWAIT :
    sprintf(statusString, "UOW Wait");
    break;
  case SQLM_LOCKWAIT :
    sprintf(statusString, "Lock Wait");
    break;
  case SQLM_COMMIT_ACT :
    sprintf(statusString, "Commit");
    break;
  case SQLM_ROLLBACK_ACT  :
    sprintf(statusString, "RB Act");
    break;
  case SQLM_RECOMP        :
    sprintf(statusString, "SQL Recomp");
    break;
  case SQLM_COMP       :
    sprintf(statusString, "SQL Comp");
    break;
  case SQLM_INTR          :
    sprintf(statusString, "Intrpt");
    break;
  case SQLM_DISCONNECTPEND:
    sprintf(statusString, "Disconn");
    break;
  case SQLM_TPREP         :
    sprintf(statusString, "Prep TX");
    break;
  case SQLM_THCOMT :
    sprintf(statusString, "Comitted");
    break;
  case SQLM_THABRT  :
    sprintf(statusString, "Aborted");
    break;
  case SQLM_TEND          :
    sprintf(statusString, "TX Ended");
    break;
  case SQLM_CREATE_DB     :
    sprintf(statusString, "Create DB");
    break;
  case SQLM_RESTART       :
    sprintf(statusString, "Start DB");
    break;
  case SQLM_RESTORE       :
    sprintf(statusString, "Restor DB");
    break;
  case SQLM_BACKUP  :
    sprintf(statusString, "Bkup DB");
    break;
  case SQLM_LOAD          :
    sprintf(statusString, "Load");
    break;
  case SQLM_UNLOAD        :
    sprintf(statusString, "Unload");
    break;
  case SQLM_IOERROR_WAIT      :
  case SQLM_QUIESCE_TABLESPACE:
    sprintf(statusString, "Qse Tbspc");
    break;
  case SQLM_WAITFOR_REMOTE   :
    sprintf(statusString, "Rem Rqst");
    break;
  case SQLM_REMOTE_RQST       :
    sprintf(statusString, "Fed Rqst");
    break;
  case SQLM_DECOUPLED         :
    sprintf(statusString, "Decoupled");
    break;
  case SQLM_ROLLBACK_TO_SAVEPOINT:
    sprintf(statusString, "RB SavePt");
    break;
  case SQLM_AUTONOMOUS_WAIT:
    sprintf(statusString, "Rout Wait");
    break;
  case SQLM_UOWQUEUED:
    sprintf(statusString, "Queued");
    break;
  default:
    sprintf(statusString, "N/A");
    break;
  }
  return statusString;

} /* appl_status_STRING */

/******************************************************************************************/
char  *table_type_STRING(sqluint32 type, char *typeString)
{
  switch(type) {
  case SQLM_USER_TABLE   :
    sprintf(typeString, "Usr");
    break;
  case SQLM_DROPPED_TABLE:
    sprintf(typeString, "Inop");
    break;
  case SQLM_TEMP_TABLE   :
    sprintf(typeString,"Temp");
    break;
  case SQLM_CATALOG_TABLE:
    sprintf(typeString, "Catlg");
    break;
  case SQLM_REORG_TABLE  :
    sprintf(typeString, "Reorg");
    break;
  default:
    sprintf(typeString, "N/A");
  }

  return typeString;
}
/******************************************************************************************/
char *lock_status_STRING(sqluint32 status, char *statusString)
{
   switch(status) {
    case SQLM_LRBGRNT: 
      sprintf(statusString, "Granted");
      break;
    case SQLM_LRBCONV: 
      sprintf(statusString, "Wait");
      break;
    default:
      sprintf(statusString, ""); 
   }
  return statusString;
}
/******************************************************************************************/
char *lock_mode_STRING(sqluint32 mode, char *modeString)
{
  switch(mode) {
    case SQLM_LNON: 
      sprintf(modeString, ""); 
      break;
    case SQLM_LOIS: 
      sprintf(modeString, "IS");
      break; 
    case SQLM_LOIX: 
      sprintf(modeString, "IX");
      break;
    case SQLM_LOOS: 
      sprintf(modeString, "S");
      break; 
    case SQLM_LSIX: 
      sprintf(modeString, "SIX");
      break;
    case SQLM_LOOX: 
      sprintf(modeString, "X");
      break;
    case SQLM_LOIN: 
      sprintf(modeString, "IN");
      break;
    case SQLM_LOOZ: 
      sprintf(modeString, "Z");
      break;
    case SQLM_LOOU: 
      sprintf(modeString, "U");
      break;
    case SQLM_LONS: 
      sprintf(modeString, "NS");
      break;
    case SQLM_LONX: 
      sprintf(modeString, "NX");
      break;
    case SQLM_LOOW: 
      sprintf(modeString, "W");
      break;
    case SQLM_LONW: 
      sprintf(modeString, "NW");
      break;
    default:
      sprintf(modeString, "");
  }
  return modeString;
}
/******************************************************************************************/
char  *lock_type_STRING(sqluint32 type, char *typeString)                                  
{                                                                                           
  switch(type) {                                                                            
   case SQLM_INTERNAL_LOCK    :  
    sprintf(typeString, "Internal");
    break;
   case SQLM_KEYVALUE_LOCK     : 
    sprintf(typeString, "Key Val"); 
    break;
   case SQLM_SYSBOOT_LOCK      : 
    sprintf(typeString, "Boot"); 
    break;
   case SQLM_INTERNALP_LOCK    : 
    sprintf(typeString, "Int Plan");
    break;
   case SQLM_INTERNALV_LOCK    : 
    sprintf(typeString, "Int Variatn");
    break;
   case SQLM_INTERNALS_LOCK    : 
    sprintf(typeString, "Int Seq");
    break;
   case SQLM_INTERNALJ_LOCK    : 
    sprintf(typeString, "Buf Pool");
    break;
   case SQLM_INTERNALL_LOCK    : 
    sprintf(typeString, "Int Lng/Lb");
    break;
   case SQLM_INTERNALC_LOCK    : 
    sprintf(typeString, "Cat Cache");
    break;
   case SQLM_INTERNALB_LOCK    : 
    sprintf(typeString, "Online Bkup");
    break;
   case SQLM_INTERNALO_LOCK    : 
    sprintf(typeString, "Int ObjTb");
    break;
   case SQLM_INTERNALT_LOCK    : 
    sprintf(typeString, "Int TbAlt");
    break; 
   case SQLM_INTERNALQ_LOCK    : 
    sprintf(typeString, "DMS Seq");
    break;
   case SQLM_INPLACE_REORG_LOCK : 
    sprintf(typeString, "Reorg");
    break;
   case SQLM_BLOCK_LOCK        : 
    sprintf(typeString, "Block");
    break;
   case SQLM_AUTORESIZE_LOCK   : 
    sprintf(typeString, "Resize");
    break;
   case SQLM_AUTOSTORAGE_LOCK  : 
    sprintf(typeString, "Storage");
    break;
   case SQLM_XML_PATH_LOCK : 
    sprintf(typeString, "XML");
    break;
   case SQLM_TABLE_PART_LOCK   : 
    sprintf(typeString, "TB Part");
    break;
   case SQLM_TABLE_LOCK       :  
    sprintf(typeString, "Table");
    break; 
   case SQLM_ROW_LOCK          : 
    sprintf(typeString, "Row");
    break; 
   case SQLM_TABLESPACE_LOCK   : 
    sprintf(typeString, "Tbspace");
    break;
   case SQLM_EOT_LOCK          : 
    sprintf(typeString, "EOT");
    break;
   default:                                                                                  
    sprintf(typeString, "N/A");                                                             
  }                                                                                         
                                                                                            
  return typeString;                                                                        
}                                                                                           
/******************************************************************************************/
char  *lock_obj_STRING(sqluint32 type, char *objString)                                  
{                                                                                           
  switch(type) {                                                                            
   case SQLM_INTERNAL_LOCK    :  
    sprintf(objString, "Internal LK");
    break;
   case SQLM_KEYVALUE_LOCK     : 
    sprintf(objString, "Key Value LK"); 
    break;
   case SQLM_SYSBOOT_LOCK      : 
    sprintf(objString, "Sysboot TB"); 
    break;
   case SQLM_INTERNALP_LOCK    : 
    sprintf(objString, "Internal Plan");
    break;
   case SQLM_INTERNALV_LOCK    : 
    sprintf(objString, "Internal Variation");
    break;
   case SQLM_INTERNALS_LOCK    : 
    sprintf(objString, "Internal Sequence");
    break;
   case SQLM_INTERNALJ_LOCK    : 
    sprintf(objString, "Bufferpool");
    break;
   case SQLM_INTERNALL_LOCK    : 
    sprintf(objString, "Internal Lng/Lob");
    break;
   case SQLM_INTERNALC_LOCK    : 
    sprintf(objString, "Catalog Cache");
    break;
   case SQLM_INTERNALB_LOCK    : 
    sprintf(objString, "Online Backup");
    break;
   case SQLM_INTERNALO_LOCK    : 
    sprintf(objString, "Object TB");
    break;
   case SQLM_INTERNALT_LOCK    : 
    sprintf(objString, "Table Alter");
    break; 
   case SQLM_INTERNALQ_LOCK    : 
    sprintf(objString, "DMS Sequence");
    break;
   case SQLM_INPLACE_REORG_LOCK : 
    sprintf(objString, "Inplace Reorg");
    break;
   case SQLM_BLOCK_LOCK        : 
    sprintf(objString, "Block Type");
    break;
   case SQLM_AUTORESIZE_LOCK   : 
    sprintf(objString, "Auto Resize");
    break;
   case SQLM_AUTOSTORAGE_LOCK  : 
    sprintf(objString, "Auto Storage");
    break;
   case SQLM_XML_PATH_LOCK : 
    sprintf(objString, "XML Path");
    break;
   case SQLM_TABLE_PART_LOCK   : 
   case SQLM_TABLE_LOCK       :  
   case SQLM_ROW_LOCK          : 
   case SQLM_TABLESPACE_LOCK   : 
   case SQLM_EOT_LOCK          : 
   default:                                                                                  
    sprintf(objString, "N/A");                                                             
  }                                                                                         
                                                                                            
  return objString;                                                                        
}                                                                                           
/******************************************************************************************/
char  *tbspace_state_STRING(sqluint32 state, char *stateString)
{
  switch(state) {
  case 0:
    sprintf(stateString, "Normal" );
    break;
  case 1:
    sprintf(stateString,"QuiesSHR");
    break;
  case 2:
    sprintf(stateString,"QuiesUPD");
    break;
  case 4:
    sprintf(stateString,"QuiesX");
    break;
  case 8:
    sprintf(stateString,"LoadPend");
    break;
  case 16:
    sprintf(stateString,"DelPend");
    break;
  case 32:
    sprintf(stateString,"BkupPend");
    break;
  case 64:
    sprintf(stateString,"RF");
    break;
  case 128:
    sprintf(stateString,"RF Pend");
    break;
  case 256:
    sprintf(stateString,"RstrPend");
    break;
  case 512:
    sprintf(stateString,"Disable");
    break;
  case 1024:
    sprintf(stateString,"Reorg");
    break;
  case 2048:
    sprintf(stateString,"Bkup");
    break;
  case 4096:
  case 33554432:
    sprintf(stateString,"StrgeN/A");
    break;
  case 8192:
    sprintf(stateString,"RstrProg");
    break;
  case 16384:
    sprintf(stateString,"Offline");
    break;
  case 32768:
    sprintf(stateString,"DropPend");
    break;
  case 67108864:
    sprintf(stateString,"StrgRdy");
    break;
  case 134217728:
    sprintf(stateString,"StrgChg");
    break;
  case  268435456:
    sprintf(stateString,"DMSrebal");
    break;
  case 536870912:
    sprintf(stateString,"TBS del");
    break;
  case 1073741824:
    sprintf(stateString,"TBScreat");
    break;
  default:
    sprintf(stateString,"N/A");
  }

  return stateString;
}
/******************************************************************************************/
char  *util_type_STRING(sqluint32 type, char *typeString) {

  switch(type) {
  case SQLM_UTILITY_REBALANCE             :
    sprintf(typeString,"Rebal");
    break;
  case SQLM_UTILITY_BACKUP                :
    sprintf(typeString,"Bckup");
    break;
  case SQLM_UTILITY_RUNSTATS              :
    sprintf(typeString,"Runstat");
    break;
  case SQLM_UTILITY_REORG                 :
    sprintf(typeString,"Reorg");
    break;
  case SQLM_UTILITY_RESTORE               :
    sprintf(typeString,"Restor");
    break;
  case SQLM_UTILITY_CRASH_RECOVERY        :
    sprintf(typeString,"Recov");
    break;
  case SQLM_UTILITY_ROLLFORWARD_RECOVERY  :
    sprintf(typeString,"RF Recov");
    break;
  case SQLM_UTILITY_LOAD                  :
    sprintf(typeString,"Load");
    break;
  case SQLM_UTILITY_RESTART_RECREATE_INDEX:
    sprintf(typeString, "Recov IX");
    break;
  case SQLM_UTILITY_REDISTRIBUTE          :
    sprintf(typeString,"Redist");
    break;
  case SQLM_UTILITY_ASYNC_INDEX_CLEANUP   :
    sprintf(typeString,"Clean IX");
    break;
  default:
    sprintf(typeString,"N/A");
    break;
  }
  return typeString;
}

/******************************************************************************************/
char  *util_status_STRING(sqluint32 status, char *statusString) {
  switch(status) {
  case SQLM_UTILITY_STATE_EXECUTE:
    sprintf(statusString,"Exec");
    break;
  case SQLM_UTILITY_STATE_WAIT   :
    sprintf(statusString,"Wait");
    break;
  case SQLM_UTILITY_STATE_ERROR  :
    sprintf(statusString,"Err");
    break;
  default:
    sprintf(statusString,"N/A");
    break;
  }

  return statusString;
}
/******************************************************************************************/
float cpu_PCT(sqluint32 elapsed_exec_microsec, sqluint32 cpu_microsec)
{
  if (cpu_microsec == 0 || elapsed_exec_microsec == 0)
    return 0.0;
  else
    return 100 * ((float) cpu_microsec / elapsed_exec_microsec);
}
/******************************************************************************************/
char  *cpu_STRING(sqluint64 elapsed_exec_microsec, sqluint64 cpu_microsec, char *cpuString)
{
  double  temp_num;
  if (cpu_microsec == 0)
    sprintf(cpuString, "");
  else if (elapsed_exec_microsec == 0)
    sprintf(cpuString, "-1");
  else if ( cpu_microsec > elapsed_exec_microsec)
    sprintf(cpuString, "%d", cpu_microsec - elapsed_exec_microsec);
  else {
    temp_num = 100 * ( ((double) cpu_microsec) / elapsed_exec_microsec);
    if (temp_num < 0.1)
      sprintf(cpuString, "%.2f", temp_num);
    else
      sprintf(cpuString, "%.1f", temp_num);
  }

  return cpuString;

} /* cpu_STRING */


/******************************************************************************************/
char  *pct_STRING(sqluint64 completed, sqluint64 total, char *pctString)
{
  double  temp_num;
  if (total == 0 || completed == 0)
    sprintf(pctString, "");
  else {
    temp_num = ( ((double) completed) / total) * 100;
    if ( temp_num > 99.9)
      sprintf(pctString, "%.0lf%%", temp_num);
    else
      sprintf(pctString, "%.1lf%%", temp_num);
  }
  return pctString;

} /* pct_STRING */

/*****************************************************************************/
char  *hit_STRING(sqluint64 physical, sqluint64 logical, char *pctString)     
{                                                                            
  double  temp_num;                                                          
  if (logical == 0)                                          
    sprintf(pctString, "");                                                  
  else {                                                                     
    temp_num = ( 1 - (( (double) physical) / logical)) * 100;                        
    sprintf(pctString, "%.1lf%%", temp_num);                                
  }                                                                          
  return pctString;                                                          
                                                                             
} /* hit_STRING */                                                           
/******************************************************************************************/
char  *uint64byte_STRING(sqluint64 num , char *numString)
{
  double  temp_num ;
  if (num > 1000000000000)  {
    temp_num = (double) num / pow(2 ,40);
    sprintf(numString, "%.1fTB", temp_num);
  } else if (num > 1000000000)  {
    temp_num = (double) num / pow(2 ,30);
    sprintf(numString, "%.1fGB", temp_num);
  } else if (num > 1000000)  {
    temp_num = (double) num / pow(2,20);
    sprintf(numString, "%.1fMB", temp_num);
  } else if (num  > 1000) {
    temp_num = (double) num / pow(2, 10);
    sprintf(numString, "%.1fKB", temp_num);
  } else if ( num == 0) {
    sprintf(numString, "");
  } else {
    sprintf(numString, "%luB", num);
  }

  return numString;

} /* uint64byte_STRING */

/******************************************************************************************/
char  *uint64_STRING(sqluint64 num , char *numString)
{
  double  temp_num ;
  if (num > 1000000000000)  {
    temp_num = (double) num / 1000000000000;
    sprintf(numString, "%.1lfT", temp_num);
  } else if (num > 1000000000)  {
    temp_num = (double) num / 1000000000;
    sprintf(numString, "%.1lfG", temp_num);
  } else if (num > 1000000)  {
    temp_num = (double) num / 1000000;
    sprintf(numString, "%.1lfM", temp_num);
  } else if (num  > 1000) {
    temp_num = (double) num / 1000;
    sprintf(numString, "%.1lfK", temp_num);
  } else if ( num == 0) {
    sprintf(numString, "");
  } else {
    sprintf(numString, "%lu", num);
  }

  return numString;

} /* uint64_STRING */


/******************************************************************************************/
char  *uint64byte_rate_STRING(double interval, sqluint64 num , char *numString)
{
  double  temp_num ;
  if (interval == 0 || num == 0)
      sprintf(numString, "");
  else if (num > pow(2,30))  {
    temp_num = ( ((double) num) / pow(2,30)) / interval;
    sprintf(numString, "%.1lfGB", temp_num);
  } else if (num > pow(2,20))  {
    temp_num = ( ((double) num) / pow(2,20)) / interval;
    sprintf(numString, "%.1lfMB", temp_num);
  } else if (num  > pow(2,10)) {
    temp_num = ( ((double) num) / pow(2,10)) / interval;
    sprintf(numString, "%.1lfKB", temp_num);
  } else {
    temp_num = num / interval;
    sprintf(numString, "%.1lfB", temp_num);
  }

  return numString;
} /* uint64byte_rate_STRING */

/******************************************************************************************/
char  *uint64rate_STRING(double interval, sqluint64 num , char *numString)
{
  double  temp_num ;
  if (interval == 0)
    sprintf(numString, "-1");
  else if (num > 1000000000)  {
    temp_num = ( ((double) num) / 1000000000) / interval;
    sprintf(numString, "%.1lfG", temp_num);
  } else if (num > 1000000)  {
    temp_num = ( ((double) num) / 1000000) / interval;
    sprintf(numString, "%.1lfM", temp_num);
  } else if (num  > 1000) {
    temp_num = ( ((double) num) / 1000) / interval;
    sprintf(numString, "%.1lfK", temp_num);
  } else if ( num == 0) {
    sprintf(numString, "");
  } else {
    temp_num = num / interval;
    sprintf(numString, "%.2lf", temp_num);
  }

  return numString;
} /* uint64rate_STRING */


/******************************************************************************************/
char  *uint32tm_STRING(sqluint32 interval, sqluint32 num, char *numString)
{
  float temp_num;
  if (interval > 0) {
    temp_num = ( ((float) num) / interval);
    sprintf(numString, "%.2fms", temp_num);
  } else
    sprintf(numString, "");

  return numString;
}
/******************************************************************************************/
char  *uint32rate_STRING(double interval, sqluint32 num , char *numString)
{
  double temp_num ;
  if (interval == 0)
    sprintf(numString, "-1");
  else if (num > 1000000000)  {
    temp_num = ( ((double) num) / 1000000000) / interval;
    sprintf(numString, "%.1lfG", temp_num);
  } else if (num > 1000000)  {
    temp_num = ( ((double) num) / 1000000) / interval;
    sprintf(numString, "%.1lfM", temp_num);
  } else if (num  > 1000) {
    temp_num = ( ((double) num) / 1000) / interval;
    sprintf(numString, "%.1lfK", temp_num);
  } else if ( num == 0) {
    sprintf(numString, "");
  } else {
    temp_num = num / interval;
    sprintf(numString, "%.2lf", temp_num);
  }

  return numString;
} /* uint32rate_STRING */


/******************************************************************************************/
/******************************************************************************************/
char  *uint32_STRING(sqluint32 num , char *numString)
{
  float  temp_num = 0.0;
  if (num > 1000000000000.0)  {
    temp_num = ((float) num) / 1000000000000.0;
    sprintf(numString, "%.1fT", temp_num);
  } else if (num > 1000000000.0)  {
    temp_num = ((float) num) / 1000000000;
    sprintf(numString, "%.1fG", temp_num);
  } else if (num > 1000000)  {
    temp_num = ((float) num) / 1000000;
    sprintf(numString, "%.1fM", temp_num);
  } else if (num  > 1000) {
    temp_num = ((float) num) / 1000;
    sprintf(numString, "%.1fK", temp_num);
  } else if ( num == 0) {
    sprintf(numString, "");
  } else {
    sprintf(numString, "%u", num);
  }

  return numString;

} /* uint32_STRING */


/******************************************************************************/
void  print_column_header_AGENTID_SS(Header *header, WINDOW *scrn, int scrn_row, int scrn_col)
{

  int col;
  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,scrn_row,col,' ');

  /* print node number */
  if ( (peek_snapReq(header))->order_by_col == node)
    mvwaddstr(scrn, scrn_row, NODE, "*Node(ss)");
  else
    mvwaddstr(scrn, scrn_row, NODE, "Node(ss)");

  /* print how long SS is executing */
  mvwaddstr(scrn, scrn_row, SS_EXEC_TM, "Elp TM(s)");

  /* print subsection status */
  mvwaddstr(scrn, scrn_row, SS_STATUS, "SS Status(Nd|Tq)");

  /* print cpu usage */
  if ( (peek_snapReq(header))->order_by_col == cpu) {
    if (CPU + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, CPU, "*CPU%");
  } else {
    if (CPU + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, CPU, "CPU%");
  }

  /* Total # of rows recv on TQ */
  if ( (peek_snapReq(header))->order_by_col == tqr) {
    if (TQR + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQR, "*TQ-In/s");
  } else {
    if (TQR + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQR, "TQ-In/s");
  }
  /* Total # of rows sent on TQ */
  if ( (peek_snapReq(header))->order_by_col == tqw) {
    if (TQW + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQW, "*TQ-Out/s");
  } else {
    if (TQW + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQW, "TQ-Out/s");
  }

  if ( (peek_snapReq(header))->order_by_col == rr)
    mvwaddstr(scrn, scrn_row, RR, "*RR/s");
  else
    mvwaddstr(scrn, scrn_row, RR, "RR/s");

  if ( (peek_snapReq(header))->order_by_col == rw)
    mvwaddstr(scrn, scrn_row, RW, "*RW/s");
  else
    mvwaddstr(scrn, scrn_row, RW, "RW/s");

  if (SS_RR + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, scrn_row, SS_RR, "RR");

  if (SS_RW + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, scrn_row, SS_RW, "RW");


} /* print_column_header_AGENTID_SS */




/******************************************************************************/
void print_column_header_UTILS(Header *header, WINDOW *scrn, int row, int scrn_col)
{

  int col ;
  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,row, col,' ');

  mvwaddstr(scrn, row, UTIL_ID, "Id");
  mvwaddstr(scrn, row, UTIL_STARTTM, "Start Time");
  mvwaddstr(scrn, row, UTIL_TYPE, "Type");
  mvwaddstr(scrn, row, UTIL_STATUS, "Status");
  mvwaddstr(scrn, row, UTIL_PROG, "Done");

  mvwaddstr(scrn, row, UTIL_DBNAME, "DBase");
  mvwaddstr(scrn, row, UTIL_NODE, "Node");

  if ( (UTIL_DESC + COL_WIDTH) < scrn_col)
    mvwaddstr(scrn, row, UTIL_DESC, "Desc");
  else
    mvwaddstr(scrn, row, scrn_col - 3, ">>");

  return;

}
/******************************************************************************/
void print_column_header_TABLE(Header *header, WINDOW *scrn, int scrn_row, int scrn_col)
{
  int col;
  char buffer[1024];
  sqluint8 tbspace_content_type = (peek_snapReq(header))->tbspace_content_type;

  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,scrn_row,col,' ');

  mvwaddstr(scrn, scrn_row, TABNAME, "Table Name");
  mvwaddstr(scrn, scrn_row, TAB_TYPE, "Type");
  mvwaddstr(scrn, scrn_row, TAB_DAT_SZ, "Dat_SZ");
  mvwaddstr(scrn, scrn_row, TAB_IX_SZ, "Ix_SZ");


  if ( (TAB_READS + COL_WIDTH) < scrn_col)  {
    if ( (peek_snapReq(header))->order_by_col == rr )
      strcpy(buffer,"*");

    if (tbspace_content_type == SQLM_TABLESPACE_CONTENT_SYSTEMP 
        || tbspace_content_type ==  SQLM_TABLESPACE_CONTENT_USRTEMP)
      strcat(buffer, "RR/s");
    else 
      strcat(buffer, "Read/s");

    mvwaddstr(scrn,scrn_row, TAB_READS, buffer);
  }

  memset(buffer, '\0', 1024);
  if ( (TAB_WRITES + COL_WIDTH) < scrn_col ) {
    if ( (peek_snapReq(header))->order_by_col == rw )
      strcpy(buffer,"*");

    if (tbspace_content_type == SQLM_TABLESPACE_CONTENT_SYSTEMP 
        || tbspace_content_type ==  SQLM_TABLESPACE_CONTENT_USRTEMP)
      strcat(buffer, "RW/s");
    else 
      strcat(buffer, "Write/s");

    mvwaddstr(scrn,scrn_row, TAB_WRITES, buffer);

  } else 
    mvwaddstr(scrn, scrn_row, scrn_col - 3, ">>");

  return;
}
/******************************************************************************/
void print_column_header_TBSPACE(Header *header, WINDOW *scrn, int scrn_row, int scrn_col)
{

  int col;

  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,scrn_row,col, ' ');

  mvwaddstr(scrn, scrn_row, TBSPACE_NODE, "Node");
  mvwaddstr(scrn, scrn_row, TBSPACE, "Tbspace");
  mvwaddstr(scrn, scrn_row, TBSPACE_TYPE, "Type");
  mvwaddstr(scrn, scrn_row, TBSPACE_STATUS, "Status");
  mvwaddstr(scrn, scrn_row, TBSPACE_USED, "Usd");

  mvwaddstr(scrn, scrn_row, TBSPACE_TOTAL, "Total");
  mvwaddstr(scrn, scrn_row, TBSPACE_FREE, "Free");

  if ( (TBSPACE_READS + COL_WIDTH) < scrn_col)  {
    if ( (peek_snapReq(header))->order_by_col == rr )
      mvwaddstr(scrn,scrn_row, TBSPACE_READS, "*Read/s");
    else
      mvwaddstr(scrn,scrn_row, TBSPACE_READS, "Read/s");
  }

  if ( (TBSPACE_WRITES + COL_WIDTH) < scrn_col ) {
    if ( (peek_snapReq(header))->order_by_col == rw )
      mvwaddstr(scrn,scrn_row, TBSPACE_WRITES, "*Write/s");
    else 
      mvwaddstr(scrn, scrn_row, TBSPACE_WRITES, "Write/s");
  } else
    mvwaddstr(scrn, scrn_row, scrn_col - 3, ">>");

  return;

}

/******************************************************************************/
void  print_column_header_LOCKS (Header *header, WINDOW *scrn, int scrn_row, int scrn_col)
{
  int col ;
  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,scrn_row, col, ' ');

  mvwaddstr(scrn, scrn_row, HANDLE, "Node(Esc)");
  mvwaddstr(scrn, scrn_row, STATUS_CHANGE, "Agent Blocking");
  mvwaddstr(scrn, scrn_row, AUTHID, "Mode Req");
  mvwaddstr(scrn, scrn_row, STATUS, "Status");         
  mvwaddstr(scrn, scrn_row, CPU, "Mode");
  mvwaddstr(scrn, scrn_row, RR, "Count");

  if (BP < scrn_col)
    mvwaddstr(scrn, scrn_row, BP, "Type");
  if (TQR < scrn_col)
    mvwaddstr(scrn, scrn_row, TQR, "Object");
  else
    mvwaddstr(scrn, scrn_row, scrn_col -3, ">>");

  return;
} /* print_column_row_LOCKS */
/******************************************************************************/
void  print_column_header_APPLS (Header *header, WINDOW *scrn, int scrn_row, int scrn_col)
{

  int col ;
  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,scrn_row, col, ' ');

  switch (header->col1_opt) {
  case prog_nm:
    if (strlen((peek_snapReq(header))->prog_nm) > 0)
      mvwaddstr(scrn, scrn_row, PROG_NM, "ProgNm!");
    else
      mvwaddstr(scrn, scrn_row, PROG_NM, "Prog Nm");
    break;
  case client_nm:
    if (strlen((peek_snapReq(header))->client_nm) > 0)
      mvwaddstr(scrn, scrn_row, CLIENT_NM, "ClntNm!");
    else
      mvwaddstr(scrn, scrn_row, CLIENT_NM, "Clnt Nm");
    break;
  case util_id:
    mvwaddstr(scrn, scrn_row, UTIL_ID, "Util Id");
    break;
  case coord_node_num:
    mvwaddstr(scrn, scrn_row, NODE, "Coord Nd");
    break;
  case coord_pid:
    mvwaddstr(scrn, scrn_row, NODE, "CoordPid");
    break;
  case client_pid:
    mvwaddstr(scrn, scrn_row, NODE, "Clnt Pid");
    break;
  default:
    mvwaddstr(scrn, scrn_row, HANDLE, "Handle");
    break;
  }

  mvwaddstr(scrn, scrn_row, STATUS_CHANGE, "Start Time");

  if (header->col3_opt == exec_id) {
    if ( strlen((peek_snapReq(header))->exec_id) > 0)
      mvwaddstr(scrn, scrn_row, CLIENTID, "ClntId!");
    else
      mvwaddstr(scrn, scrn_row, CLIENTID, "Clnt ID");
  } else {
    if (strlen((peek_snapReq(header))->auth_id) > 0)
      mvwaddstr(scrn, scrn_row, AUTHID, "AuthId!");
    else
      mvwaddstr(scrn, scrn_row, AUTHID, "Auth ID");
  }

  /* show appl_status or stmt_op */
  if (header->col4_opt == stmt_op)
    mvwaddstr(scrn, scrn_row, STATUS, "Stmt Op");
  else
    mvwaddstr(scrn, scrn_row, STATUS, "AppState");

  /* Total # of rows recv on TQ */
  if ( (peek_snapReq(header))->order_by_col == tqr) {
    if (TQR + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQR, "*TQ-In/s");
  } else {
    if (TQR + COL_WIDTH <= scrn_col)
      mvwaddstr(scrn, scrn_row, TQR, "TQ-In/s");
  }
  /* Total # of rows sent on TQ */
  if ( (peek_snapReq(header))->order_by_col == tqw) {
    if (TQW + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, scrn_row, TQW, "*TQ-Out/s");
  } else {
    if (TQW + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, scrn_row, TQW, "TQ-Out/s");
  }

  if ( (peek_snapReq(header))->order_by_col == rr)
    mvwaddstr(scrn, scrn_row, RR, "*RR/s");
  else
    mvwaddstr(scrn, scrn_row, RR, "RR/s");

  if ( (peek_snapReq(header))->order_by_col == rw)
    mvwaddstr(scrn, scrn_row, RW, "*RW/s");
  else
    mvwaddstr(scrn, scrn_row, RW, "RW/s");

  if (SIUD + SIUD_WIDTH < scrn_col)
    mvwaddstr(scrn, scrn_row, SIUD, "[S,I,U,D]/s");

  if (header->col8_opt == bp) {
    if ( (peek_snapReq(header))->order_by_col == bppgin) {
      if (BP + COL_WIDTH + 2 < scrn_col) {
        mvwaddstr(scrn, scrn_row, BP, "BPLR");
        mvwaddstr(scrn, scrn_row, BP+7, "*BPPR");
      }
    } else {
      if (BP + COL_WIDTH + 2 < scrn_col){
        mvwaddstr(scrn, scrn_row, BP, "BPLR");
        mvwaddstr(scrn, scrn_row, BP+7, "BPPR");
      }
    }
  } else if (header->col8_opt == mem_usg) {
    if ( (peek_snapReq(header))->order_by_col == memusg) {
      if (MEMUSG + COL_WIDTH + 2 < scrn_col)
        mvwaddstr(scrn, scrn_row, MEMUSG, "*MemUsg");
    } else {
      if (MEMUSG + COL_WIDTH + 2 < scrn_col)
        mvwaddstr(scrn, scrn_row, MEMUSG, "MemUsg");
    }
  } else {
    if (MEMUSG + COL_WIDTH + 2 < scrn_col)
      mvwaddstr(scrn, scrn_row, MEMUSG, "Agents");
  }


  if ( (peek_snapReq(header))->order_by_col == cpu) {
    if (CPU + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, scrn_row, CPU, "*CPU%");
  } else {
    if (CPU + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, scrn_row, CPU, "CPU%");
  }

  if ( (LOG + COL_WIDTH) < scrn_col ) {
    if (header->col12_opt == locks)
      mvwaddstr(scrn, scrn_row, RB, "Locks");
    else if (header->col12_opt == rb)
      if ( (peek_snapReq(header))->order_by_col == rolledback)
        mvwaddstr(scrn, scrn_row, RB, "*RB%");
      else
        mvwaddstr(scrn, scrn_row, RB, "RB%");
    else if ( (peek_snapReq(header))->order_by_col == logusg)
      mvwaddstr(scrn, scrn_row, RB, "*LogUsd");
    else
      mvwaddstr(scrn, scrn_row, RB, "LogUsd");
  } else
    mvwaddstr(scrn, scrn_row, scrn_col - 3, ">>");

  return;
} /* print_column_row_APPLS */


/******************************************************************************/
void  print_column_header_STMTS (Header *header, WINDOW *scrn,int row, int scrn_col)
{

  int col;
  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn, row, col, ' ');

  if ( (peek_snapReq(header))->type == agent_id)
    mvwaddstr(scrn, row, HANDLE, "Cost");
  else
    mvwaddstr(scrn, row, HANDLE, "Handle");
  mvwaddstr(scrn, row, STATUS_CHANGE, "Start Time");

  if ((peek_snapReq(header))->type == agent_id)
    mvwaddstr(scrn, row, AUTHID, "Sorts");
  else if (header->col3_opt == exec_id) {
    if ( strlen((peek_snapReq(header))->exec_id) > 0)
      mvwaddstr(scrn, row, CLIENTID, "ClntId!");
    else
      mvwaddstr(scrn, row, CLIENTID, "Clnt ID");
  } else {
    if (strlen((peek_snapReq(header))->auth_id) > 0)
      mvwaddstr(scrn, row, AUTHID, "AuthId!");
    else
      mvwaddstr(scrn, row, AUTHID, "Auth ID");
  }


  /* show stmt_op */
  mvwaddstr(scrn, row, STATUS, "Stmt Op");

  if ( (peek_snapReq(header))->order_by_col == bppgin) {
    if (BP + COL_WIDTH + 2 < scrn_col) {
      mvwaddstr(scrn, row, BP, "BPLR");
      mvwaddstr(scrn, row, BP+7, "*BPPR");
    }
  } else {
    if (BP + COL_WIDTH + 2 < scrn_col){
      mvwaddstr(scrn, row, BP, "BPLR");
      mvwaddstr(scrn, row, BP+7, "BPPR");
    }
  }


  if ( (peek_snapReq(header))->order_by_col == cpu) {
    if (CPU + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, row, CPU, "*CPU%");
  } else {
    if (CPU + COL_WIDTH < scrn_col)
      mvwaddstr(scrn, row, CPU, "CPU%");
  }

  /* show RR/RW */
  if ( (peek_snapReq(header))->order_by_col == rr)
    mvwaddstr(scrn, row, RR, "*RR/s");
  else 
    mvwaddstr(scrn, row, RR, "RR/s");

  if ( (peek_snapReq(header))->order_by_col == rw)
    mvwaddstr(scrn, row, RW, "*RW/s");
  else 
    mvwaddstr(scrn, row, RW, "RW/s");

  if (STMT_SIUD + SIUD_WIDTH < scrn_col)
    mvwaddstr(scrn, row, STMT_SIUD, "[S,I,U,D]/s");

  if (STMT_TEXT + SIUD_WIDTH < scrn_col)
    mvwaddstr(scrn, row, STMT_TEXT, "Text");

  return;
} /* print_column_header_STMTS */

/************************************************************************************************/
void print_column_header_IO (Header *header, WINDOW *scrn, int row, int scrn_col)
{

  int i;
  for (i = 0; i < scrn_col; i++)
    mvwaddch(scrn, row, i, ' ');
  mvwaddstr(scrn, row, BPR_TM, "BpR_tm");
  mvwaddstr(scrn, row, BPW_TM, "BpW_tm");
  mvwaddstr(scrn, row, DIOR_TM, "DioR_tm");
  mvwaddstr(scrn, row, DIOW_TM, "DioW_tm");
  mvwaddstr(scrn, row, BPLDR, "BpLDR");
  mvwaddstr(scrn, row, BPLIR, "BpLIR");
  mvwaddstr(scrn, row, BPPDR, "BpPDR");
  mvwaddstr(scrn, row, BPPIR, "BpPIR");
  mvwaddstr(scrn, row, BPDW, "BpDW");
  if (BPLTR + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, row, BPLTR, "BpLTR");
  if (BPIW + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, row, BPIW, "BpIW");
  if (DIOR + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, row, DIOR, "DIOR");
  if (DIOW + COL_WIDTH < scrn_col)
    mvwaddstr(scrn, row, DIOW, "DIOW");
  else
    mvwaddstr(scrn, row, scrn_col - 3, ">>");

  return;

}

/************************************************************************************************/
void print_row_IO (WINDOW *scrn, int row, Appl *listData,  Header *header)
{
  char buffer[1024];
  int i, scrn_row = 0, scrn_col = 0;

  getmaxyx(scrn, scrn_row, scrn_col);
  /* clear the line before printing */
  for (i = 0; i < scrn_col; i++)
    mvwaddch(scrn, row, i, ' ');

  mvwaddstr(scrn, row, BPR_TM, uint32tm_STRING((listData->bppdr_delta + listData->bppir_delta), 
      listData->bpr_tm_delta, buffer));
  mvwaddstr(scrn, row, BPW_TM -1, "|");
  mvwaddstr(scrn, row, BPW_TM, uint32tm_STRING((listData->bpdw_delta + listData->bpiw_delta), 
      listData->bpw_tm_delta, buffer));
  mvwaddstr(scrn, row, DIOR_TM -1, "|");
  mvwaddstr(scrn, row, DIOR_TM, uint32tm_STRING(listData->dio_reads_delta, 
      listData->dio_read_tm_delta, buffer));
  mvwaddstr(scrn, row, DIOW_TM -1, "|");
  mvwaddstr(scrn, row, DIOW_TM, uint32tm_STRING(listData->dio_writes_delta, 
      listData->dio_write_tm_delta, buffer));

  mvwaddstr(scrn, row, BPLIR-1, "|");
  mvwaddnstr(scrn, row, BPLIR, uint32rate_STRING(header->snapshot_timestamp_delta,                        
      listData->bplir_delta, buffer), COL_WIDTH );

  mvwaddstr(scrn,row, BPLDR-1,"|");
  mvwaddnstr(scrn, row, BPLDR, uint32rate_STRING(header->snapshot_timestamp_delta,                        
      (listData->bpldr_delta), buffer), COL_WIDTH);

  mvwaddstr(scrn, row, BPDW-1, "|");
  mvwaddnstr(scrn, row, BPDW, uint32rate_STRING(header->snapshot_timestamp_delta,                        
      listData->bpdw_delta, buffer), COL_WIDTH );

  mvwaddstr(scrn, row, BPPDR-1, "|");
  mvwaddnstr(scrn, row, BPPDR, uint32rate_STRING(header->snapshot_timestamp_delta,                        
      listData->bppdr_delta, buffer), COL_WIDTH );

  mvwaddstr(scrn, row, BPPIR-1, "|");
  mvwaddnstr(scrn, row, BPPIR, uint32rate_STRING(header->snapshot_timestamp_delta,                        
      listData->bppir_delta, buffer), COL_WIDTH );

  if (BPLTR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, BPLTR-1, "|");
    mvwaddnstr(scrn, row, BPLTR, uint32rate_STRING(header->snapshot_timestamp_delta,                       
        listData->bpltdir_delta, buffer), COL_WIDTH );
  }

  if (BPIW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, BPIW-1, "|");
    mvwaddnstr(scrn, row, BPIW, uint32rate_STRING(header->snapshot_timestamp_delta,                        
        listData->bpiw_delta, buffer), COL_WIDTH );
  }

  if (DIOR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, DIOR-1, "|");
    mvwaddnstr(scrn, row, BPIW, uint32rate_STRING(header->snapshot_timestamp_delta,                        
        listData->dio_read_reqs_delta, buffer), COL_WIDTH );
  }

  if (DIOW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, DIOW-1, "|");
    mvwaddnstr(scrn, row, BPIW, uint32rate_STRING(header->snapshot_timestamp_delta,                        
        listData->dio_write_reqs_delta, buffer), COL_WIDTH );
  }
}
/************************************************************************************************/
void  print_row_TABLE(WINDOW *scrn, int row, Table *listData, Header *header)
{
  int  scrn_row, scrn_col, col;
  char buffer[1024];
  sqluint32 tbspace_pg_sz = (peek_snapReq(header))->tbspace_pg_sz;
  sqluint8 tbspace_content_type = (peek_snapReq(header))->tbspace_content_type;
  getmaxyx(scrn, scrn_row, scrn_col);

  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print table name */
  strncpy(buffer, strtrim(listData->tabschema), TABSCHEMA_SZ);
  strcat(buffer,".");
  strncat(buffer, strtrim(listData->tabname), TABNAME_SZ);
  if (listData->data_partition_id != 9999) {
    char temp[128] ;
    sprintf(temp, "[%d]",listData->data_partition_id);
    strcat(buffer,temp);
  }
  mvwaddnstr(scrn, row, TABNAME, buffer, TAB_TYPE - TABNAME);

  /* print table type */
  mvwaddstr(scrn, row, TAB_TYPE -1, "|");
  mvwaddstr(scrn, row, TAB_TYPE, table_type_STRING(listData->type, buffer));

  /* print table size */
  mvwaddstr(scrn, row, TAB_DAT_SZ -1, "|");
  mvwaddstr(scrn, row, TAB_DAT_SZ 
      ,uint64byte_STRING( listData->data_sz * tbspace_pg_sz,buffer));

  mvwaddstr(scrn, row, TAB_IX_SZ -1, "|");
  mvwaddstr(scrn, row, TAB_IX_SZ 
      ,uint64byte_STRING( listData->ix_sz * tbspace_pg_sz,buffer));

  if ( (TAB_READS + COL_WIDTH) < scrn_col)  {

    mvwaddstr(scrn, row, TAB_READS -1, "|");
    if (tbspace_content_type == SQLM_TABLESPACE_CONTENT_SYSTEMP   
        || tbspace_content_type ==  SQLM_TABLESPACE_CONTENT_USRTEMP
        || listData->avg_row_sz < 0 ) {
      mvwaddstr(scrn, row, TAB_READS, uint64rate_STRING(header->snapshot_timestamp_delta,
          listData->rr_delta, buffer));
    } else {
      mvwaddstr(scrn, row, TAB_READS, uint64byte_rate_STRING(header->snapshot_timestamp_delta,
          listData->rr_delta * listData->avg_row_sz, buffer));
    }

  }

  if ( (TAB_WRITES + COL_WIDTH) < scrn_col)  {

    mvwaddstr(scrn, row, TAB_WRITES -1, "|");
    if (tbspace_content_type == SQLM_TABLESPACE_CONTENT_SYSTEMP   
        || tbspace_content_type ==  SQLM_TABLESPACE_CONTENT_USRTEMP
        || listData->avg_row_sz < 0 )
      mvwaddstr(scrn, row, TAB_WRITES, uint64rate_STRING(header->snapshot_timestamp_delta,
          listData->rw_delta, buffer));
    else {
      mvwaddstr(scrn, row, TAB_WRITES, uint64byte_rate_STRING(header->snapshot_timestamp_delta,
          listData->rw_delta * listData->avg_row_sz, buffer));
    }

  }

  return;

}
/************************************************************************************************/
void print_row_TBSPACE(WINDOW *scrn, int row, Tbspace *listData, Header *header)
{
  int  scrn_row, scrn_col, col;
  char buffer[1024];

  getmaxyx(scrn, scrn_row, scrn_col);

  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print the node number */
  if ( (peek_snapReq(header))->type == tbspace
      && (peek_snapReq(header))->node == -2)
    mvwaddstr(scrn, row, TBSPACE_NODE, "All");
  else {
    sprintf(buffer, "%u", listData->node);
    mvwaddstr(scrn, row, TBSPACE_NODE, buffer);
  }

  /* print tablespace name */
  mvwaddnstr(scrn,row, TBSPACE, listData->name, 16);

  /* print tablespace state */
  mvwaddstr(scrn,row,TBSPACE_STATUS-1,"|");
  mvwaddnstr(scrn,row,TBSPACE_STATUS,tbspace_state_STRING(listData->state,buffer),COL_WIDTH);

  /* print tablespace type-content_type */
  if (listData->type == SQLM_TABLESPACE_TYP_DMS)
    sprintf(buffer,"DMS-");
  else
    sprintf(buffer,"SMS-");

  switch(listData->content_type) {
  case SQLM_TABLESPACE_CONTENT_LONG:
    strcat(buffer,"Long");
    break;
  case SQLM_TABLESPACE_CONTENT_SYSTEMP:
    strcat(buffer,"Stmp");
    break;
  case  SQLM_TABLESPACE_CONTENT_USRTEMP:
    strcat(buffer,"Utmp");
    break;
  default:
    strcat(buffer,"Any");
  }

  mvwaddstr(scrn, row, TBSPACE_TYPE-1,"|");
  mvwaddstr(scrn, row, TBSPACE_TYPE,buffer);

  /* print %usd */
  mvwaddstr(scrn, row, TBSPACE_USED-1,"|");
  if (listData->type == SQLM_TABLESPACE_TYP_DMS)
    mvwaddstr(scrn, row, TBSPACE_USED, 
        pct_STRING(listData->t1_used_pg, listData->t1_total_pg,buffer));
  else
    mvwaddstr(scrn, row, TBSPACE_USED, 
        pct_STRING((sqluint64) listData->t1_used_pg * listData->pg_sz, listData->t1_fs_total_sz
        ,buffer));

  /* print total size */
  mvwaddstr(scrn, row, TBSPACE_TOTAL-1,"|");
  if (listData->type == SQLM_TABLESPACE_TYP_DMS)
    uint64byte_STRING((sqluint64) listData->t1_total_pg * listData->pg_sz,buffer);
  else
    uint64byte_STRING(listData->t1_fs_total_sz,buffer);

  mvwaddstr(scrn, row, TBSPACE_TOTAL, buffer);

  /* print free size */
  mvwaddstr(scrn, row, TBSPACE_FREE-1,"|");
  if (listData->type == SQLM_TABLESPACE_TYP_DMS)
    uint64byte_STRING(( (sqluint64)listData->t1_total_pg - listData->t1_used_pg)
        * (sqluint64) listData->pg_sz ,buffer);
  else
    /* may not be the same as (t1_ts_total_sz -  (listData->t1_used_pg * listData->pg_sz))
         if the file system is used for files other than the tablespace file system container */
    uint64byte_STRING( (sqluint64) (listData->t1_fs_total_sz - listData->t1_fs_used_sz )
        ,buffer);

  mvwaddstr(scrn, row, TBSPACE_FREE, buffer);

  /* print read/write rate */
  if ( (TBSPACE_READS+COL_WIDTH) < scrn_col) {
    mvwaddstr(scrn, row, TBSPACE_READS-1, "|");
    uint64byte_rate_STRING(header->snapshot_timestamp_delta,listData->reads_delta, buffer);
    mvwaddstr(scrn, row, TBSPACE_READS, buffer);
  }

  if ( (TBSPACE_WRITES+COL_WIDTH) < scrn_col) {
    mvwaddstr(scrn, row, TBSPACE_WRITES-1, "|");
    uint64byte_rate_STRING(header->snapshot_timestamp_delta,listData->writes_delta, buffer);
    mvwaddstr(scrn, row, TBSPACE_WRITES, buffer);
  }

  return;
}
/************************************************************************************************/
void print_row_UTILS(WINDOW *scrn, int row, Util *listData, Header *header)
{

  int  scrn_row, scrn_col, col;
  char buffer[1024];

  getmaxyx(scrn, scrn_row, scrn_col);

  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print util id */
  sprintf(buffer, "%u", listData->id);
  mvwaddstr(scrn, row, UTIL_ID,buffer);


  /* print util start time */
  mvwaddstr(scrn,row, UTIL_STARTTM-1, "|");
  mvwaddstr(scrn, row, UTIL_STARTTM, 
      time_STRING(listData->start_time, buffer));

  /* print util type */
  mvwaddstr(scrn, row, UTIL_TYPE-1, "|");
  mvwaddstr(scrn, row, UTIL_TYPE, util_type_STRING(listData->type, buffer));

  /* print util status */
  mvwaddstr(scrn, row, UTIL_STATUS-1, "|");
  mvwaddstr(scrn, row, UTIL_STATUS, util_status_STRING(listData->state, buffer));

  /* print util progress */
  mvwaddstr(scrn, row, UTIL_PROG-1,"|");
  if (strlen (pct_STRING(listData->progress_completed
      , listData->progress_total
      , buffer)) > 0)
    mvwaddstr(scrn, row, UTIL_PROG, buffer);
  else
    mvwaddstr(scrn, row, UTIL_PROG, "N/A");
  listData->progress_total = 0;
  listData->progress_completed = 0;

  /* print util dbname */
  mvwaddstr(scrn,row, UTIL_DBNAME-1, "|");
  mvwaddstr(scrn,row, UTIL_DBNAME, listData->dbname);

  /* print the node number */
  mvwaddstr(scrn, row, UTIL_NODE-1, "|");
  if ( (peek_snapReq(header))->type == utils 
      && (peek_snapReq(header))->node == -2)
    mvwaddstr(scrn, row, UTIL_NODE, "All");
  else {
    sprintf(buffer, "%u", listData->node);
    mvwaddstr(scrn, row, UTIL_NODE, buffer);
  }

  /*print util desc */
  if ( (UTIL_DESC + COL_WIDTH) < scrn_col ) {
    mvwaddstr(scrn,row, UTIL_DESC -1, "|");
    mvwaddnstr(scrn, row, UTIL_DESC, listData->desc, scrn_col - UTIL_DESC);
  }

  return;


}
/************************************************************************************************/
void  print_row_AGENTID_SS(WINDOW *scrn, int row, Appl_SS *listData, Header *header)
{

  char  buffer[1024];
  int  scrn_row, scrn_col, col;

  getmaxyx(scrn, scrn_row, scrn_col);

  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print node number */
  sprintf(buffer, "%u(%u)", listData->ss_node_number,listData->ss_number);
  mvwaddstr(scrn, row, NODE, buffer);


  /* print how long ss is running */
  sprintf(buffer, "%'u", listData->ss_exec_time,listData->ss_number);
  mvwaddstr(scrn,row, SS_EXEC_TM, buffer);

  /* print  ss status */
  ss_status_STRING(listData->ss_status, buffer);
  if ( (listData->ss_status == SQLM_SSTQ_WAIT_TO_RCV
      || listData->ss_status == SQLM_SSTQ_WAIT_TO_SEND)){
    char mybuffer[256];
    if (listData->tq_wait_for_any)
      sprintf(mybuffer,"%s(ANY|%d)", buffer, listData->tq_id_waiting_on);
    else
      sprintf(mybuffer,"%s(%d|%d)", buffer, listData->tq_node_waiting_for, listData->tq_id_waiting_on);
    strcpy(buffer, mybuffer);
  }
  mvwaddstr(scrn, row, SS_STATUS, buffer);

  /* print cpu% */
  mvwaddstr(scrn, row, CPU - 1, "|");
  mvwaddnstr(scrn, row, CPU, cpu_STRING((header->db_ucpu_used_delta + header->db_scpu_used_delta), 
      (listData->ss_ucpu_used_delta + listData->ss_scpu_used_delta), buffer), COL_WIDTH);

  /* print RR/s */
  mvwaddstr(scrn, row, RR - 1, "|");
  mvwaddnstr(scrn, row, RR, uint32rate_STRING(header->snapshot_timestamp_delta, listData->ss_rows_read_delta, buffer),
      COL_WIDTH);

  /* print RW/s */
  mvwaddstr(scrn, row, RW - 1, "|");
  mvwaddnstr(scrn, row, RW, uint32rate_STRING(header->snapshot_timestamp_delta, listData->ss_rows_written_delta, buffer),
      COL_WIDTH);

  if (SS_RR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, SS_RR - 1, "|");
    mvwaddstr(scrn, row, SS_RR,uint32_STRING(listData->ss_rows_read,buffer));
  }

  if (SS_RW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, SS_RW - 1, "|");
    mvwaddstr(scrn, row, SS_RW,uint32_STRING(listData->ss_rows_written,buffer));
  }

  /* print network in */
  if (TQR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQR - 1, "|");
    mvwaddnstr(scrn, row, TQR, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_read_delta, buffer), COL_WIDTH);
  }

  /* print network out */
  if (TQW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQW - 1, "|");
    mvwaddnstr(scrn, row, TQW, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_written_delta, buffer), COL_WIDTH);
  }

} /* print_row_AGENTID_SS */


/******************************************************************************/
void  print_row_LOCKS(WINDOW *scrn, int row, Lock *data,  Header *header)
{
  int  scrn_row, scrn_col, col;      
  char buffer[1024]; 

  getmaxyx(scrn, scrn_row, scrn_col);

  for(col=0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');
                                     

  sprintf(buffer, "%u(%#x)", data->node, data->lock_escal); 
  mvwaddstr(scrn, row, HANDLE, buffer);

  sprintf(buffer, "%d", data->agent_id_holding_lk); 
  mvwaddstr(scrn, row, STATUS_CHANGE, buffer);
  mvwaddstr(scrn, row, AUTHID, lock_mode_STRING(data->lock_mode_requested, buffer));

  mvwaddstr(scrn, row, STATUS, lock_status_STRING(data->lock_status, buffer));         
  mvwaddstr(scrn, row, CPU, lock_mode_STRING(data->lock_mode, buffer));

  sprintf(buffer, "%'d", data->count);
  mvwaddstr(scrn, row, RR, buffer);

  if (BP < scrn_col) {
    mvwaddstr(scrn, row, BP, lock_type_STRING(data->lock_obj_type, buffer));
  }
  if (TQR < scrn_col) {
    if (strlen(data->tabschema) > 0 && strlen(data->tabname) > 0)                            
      sprintf(buffer, "%s.%s(%d)", data->tabschema, data->tabname, data->data_partition_id); 
    else if (strlen(data->tbspace) > 0)                                                      
      sprintf(buffer, "%s", data->tbspace);                                                  
    else                                                                                     
      lock_obj_STRING(data->lock_obj_type, buffer);                                          
    mvwaddnstr(scrn, row, TQR, buffer, scrn_col - TQR);
  }

  return;
} /* print_row_LOCKS */
/************************************************************************************************/
void  print_row_APPLS(WINDOW *scrn, int row, Appl *listData, Header *header)
{

  char  buffer[1024];
  int  scrn_row, scrn_col, col;

  getmaxyx(scrn, scrn_row, scrn_col);
  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print appl handle|coor node num| coord agent pid| client pid| prog_nm|client_nm */
  if (header->col1_opt == prog_nm) {
    if (strlen(listData->prog_nm) > COL_WIDTH)
      *(listData->prog_nm + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, HANDLE, ( (Appl * ) listData)->prog_nm, COL_WIDTH);
  } else if (header->col1_opt == client_nm) {
    if (strlen(listData->client_nm) > COL_WIDTH)
      *(listData->client_nm + COL_WIDTH - 1) = '$';
    mvwaddnstr(scrn, row, HANDLE, ( (Appl * ) listData)->client_nm, COL_WIDTH);
  } else if (header->col1_opt == coord_node_num) {
    sprintf(buffer, "%u", listData->coord_node_num);
    if (db2list_size(&(listData->stmt_list)) > 0)
      strcat(buffer, "?");
    mvwaddstr(scrn, row, NODE, buffer);
  } else if (header->col1_opt == coord_pid) {
    sprintf(buffer, "%u", listData->coord_pid);
    if (listData->num_stmts > 0)
      strcat(buffer, "?");
    mvwaddstr(scrn, row, NODE, buffer);
  } else if (header->col1_opt == client_pid) {
    sprintf(buffer, "%u", listData->client_pid);
    if (listData->num_stmts > 0)
      strcat(buffer, "?");
    mvwaddstr(scrn, row, NODE, buffer);
  } else {
    sprintf(buffer, "%u", listData->appl_handle);
    if (listData->num_stmts > 0)
      strcat(buffer, "?");
    if (listData->appl_handle == header->agent_id_oldest_xact)
      strcat(buffer, "*");
    mvwaddstr(scrn, row, HANDLE, buffer);
  }


  /* print when uow started or stopped */
  if ((listData->uow_stop).seconds > 0)
    mvwaddstr(scrn, row, STATUS_CHANGE
        , time_STRING(listData->uow_stop, buffer));
  else
    mvwaddstr(scrn, row, STATUS_CHANGE
        , time_STRING(listData->uow_start, buffer));


  /* print auth id|exec id */
  if (header->col3_opt == exec_id) {
    if (strlen(listData->exec_id) > COL_WIDTH)
      *(listData->exec_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->exec_id, COL_WIDTH);
  } else {
    if (strlen(listData->auth_id) > COL_WIDTH)
      *(listData->auth_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->auth_id, COL_WIDTH);
  }


  /* print appl status | stmt op 

     if uow_stop_time is 0 that means the dynamic
     sql stmt is still running so display the stmt op
     instead of appl status
  */
  if (header->col4_opt == stmt_op)
    mvwaddnstr(scrn, row, STATUS, stmt_op_STRING(listData->stmt_op, buffer), COL_WIDTH);
  else
    mvwaddnstr(scrn, row, STATUS, appl_status_STRING(listData->appl_status, buffer), COL_WIDTH);

  /* print cpu% */
  mvwaddstr(scrn, row, CPU - 1, "|");
  mvwaddnstr(scrn, row, CPU, cpu_STRING( (header->db_ucpu_used_delta + header->db_scpu_used_delta) , 
      (listData->ucpu_used_delta + listData->scpu_used_delta), buffer), COL_WIDTH);

  /* print RR/s */
  mvwaddstr(scrn, row, RR - 1, "|");
  mvwaddnstr(scrn, row, RR, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_read_delta, buffer), COL_WIDTH);

  /* print RW/s */
  mvwaddstr(scrn, row, RW - 1, "|");
  mvwaddnstr(scrn, row, RW, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_written_delta, buffer), COL_WIDTH);

  /* print BPLR,BPPR|Priv_memusg */
  if (BP + COL_WIDTH + 2 < scrn_col) {
    mvwaddstr(scrn, row, BPLR - 1, "|");
    mvwaddstr(scrn, row, BPPR - 1, "|");
    if (header->col8_opt == bp) {
      mvwaddnstr(scrn, row, BPLR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bpldr_delta + listData->bplir_delta + listData->bpltdir_delta), buffer), COL_WIDTH );
      mvwaddnstr(scrn, row, BPPR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bppdr_delta + listData->bppir_delta), buffer), COL_WIDTH);
    } else if (header->col8_opt == mem_usg)
      mvwaddstr(scrn, row, MEMUSG, uint64byte_STRING(listData->privagent_memusg, buffer));
    else
      mvwaddstr(scrn, row, MEMUSG, uint32_STRING(listData->nagents, buffer));
  }

  /* print network in */
  if (TQR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQR - 1, "|");
    mvwaddnstr(scrn, row, TQR, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_read_delta, buffer), COL_WIDTH);
  }

  /* print network out */
  if (TQW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQW - 1, "|");
    mvwaddnstr(scrn, row, TQW, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_written_delta, buffer), COL_WIDTH);
  }

  /* print select, insert,update,delete /s */
  if (SIUD + SIUD_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, SIUD - 1, "|");
    if (listData->rows_selected_delta > 0 || listData->rows_inserted_delta > 0
        || listData->rows_updated_delta > 0 || listData->rows_deleted_delta > 0) {
      char  siud_buffer[1024];
      strcpy(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_selected_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_inserted_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_updated_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_deleted_delta, buffer));
      mvwaddnstr(scrn, row, SIUD, siud_buffer, SIUD_WIDTH);
    }
  }

  /* print the log space usage | RB% | Locks Held */
  if (LOG + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, LOG - 1, "|");
    if (header->col12_opt == locks) {
      sprintf(buffer, "%'u", listData->locks_held);
      mvwaddstr(scrn, row, LOG, buffer);
    } else if (header->col12_opt == log_usg) {
      if (listData->uow_log_space_used > 0)
        mvwaddstr(scrn, row, LOG
            , uint64byte_STRING(listData->uow_log_space_used, buffer));
    } else
      mvwaddstr(scrn, row, RB, pct_STRING(listData->rollback_progress_completed
          , listData->rollback_progress_total, buffer));
  }

} /* print_row_APPLS */

/************************************************************************************************/
void  print_row_STMTS(WINDOW *scrn, int row, Stmt *listData, Header *header)
{

  char  buffer[1024];
  int  scrn_row, scrn_col, col;

  getmaxyx(scrn, scrn_row, scrn_col);
  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print appl handle*/
  sprintf(buffer, "%u", listData->appl_handle);
  if (listData->appl_handle == header->agent_id_oldest_xact)
    strcat(buffer, "*");

  if ( (peek_snapReq(header))->type == agent_id) {
    sprintf(buffer, "%'lu", listData->query_cost);
    if (strlen(buffer) > COL_WIDTH) {
      buffer[COL_WIDTH] = '!';
      buffer[COL_WIDTH+1] = '\0';
    }
  }

  mvwaddstr(scrn, row, HANDLE, buffer);

  /* print when stmt started */
  mvwaddstr(scrn, row, STATUS_CHANGE
      , time_STRING(listData->stmt_start, buffer));


  /* print auth id|exec id */
  if ( (peek_snapReq(header))->type == agent_id)  {
    sprintf(buffer, "%'lu", listData->stmt_sorts);
    mvwaddnstr(scrn, row, AUTHID, buffer, COL_WIDTH);
  } else if (header->col3_opt == exec_id) {
    if (strlen(listData->exec_id) > COL_WIDTH)
      *(listData->exec_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->exec_id, COL_WIDTH);
  } else {
    if (strlen(listData->auth_id) > COL_WIDTH)
      *(listData->auth_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->auth_id, COL_WIDTH);
  }


  /* print stmt op 

  */
  mvwaddnstr(scrn, row, STATUS, stmt_op_STRING(listData->stmt_op, buffer), COL_WIDTH);

  /* print cpu% */
  mvwaddstr(scrn, row, CPU - 1, "|");
  mvwaddnstr(scrn, row, CPU, cpu_STRING( (header->db_ucpu_used_delta + header->db_scpu_used_delta), 
      (listData->ucpu_used_delta + listData->scpu_used_delta), buffer), COL_WIDTH);

  /* print RR/s */
  mvwaddstr(scrn, row, RR - 1, "|");
  mvwaddnstr(scrn, row, RR, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_read_delta, buffer), COL_WIDTH);

  /* print RW/s */
  mvwaddstr(scrn, row, RW - 1, "|");
  mvwaddnstr(scrn, row, RW, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_written_delta, buffer), COL_WIDTH);

  /* print BPLR,BPPR|Priv_memusg */
  if (BP + COL_WIDTH + 2 < scrn_col) {
    mvwaddstr(scrn, row, BPLR - 1, "|");
    mvwaddstr(scrn, row, BPPR - 1, "|");
    if (header->col8_opt == bp) {
      mvwaddnstr(scrn, row, BPLR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bpldr_delta + listData->bplir_delta + listData->bpltdir_delta), buffer), COL_WIDTH );
      mvwaddnstr(scrn, row, BPPR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bppdr_delta + listData->bppir_delta), buffer), COL_WIDTH);
    }
  }

  /* print select, insert,update,delete /s */
  if (STMT_SIUD + SIUD_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, STMT_SIUD - 1, "|");
    if (listData->rows_selected_delta > 0 || listData->rows_inserted_delta > 0
        || listData->rows_updated_delta > 0 || listData->rows_deleted_delta > 0) {
      char  siud_buffer[1024];
      strcpy(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_selected_delta, buffer)); 
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_inserted_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_updated_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_deleted_delta, buffer));
      mvwaddnstr(scrn, row, STMT_SIUD, siud_buffer, SIUD_WIDTH);
    }
  }

  if (STMT_TEXT < scrn_col) {
    mvwaddstr(scrn, row, STMT_TEXT - 1, "|");
    mvwaddnstr(scrn,row,STMT_TEXT,listData->stmt_text,scrn_col-STMT_TEXT);
  }

  return;

} /* print_row_STMTS */

/************************************************************************************************/
int  print_row_AGENTID(WINDOW *scrn, int row, Appl *listData, Header *header)
{

  DB2ListElmt *element = NULL;
  char  buffer[1024];
  int  scrn_row, scrn_col, col, i;

  getmaxyx(scrn, scrn_row, scrn_col);

  /* cleanup before printing the row) */
  for(col = 0; col < scrn_col; col++)
    mvwaddch(scrn,row, col, ' ');

  /* print appl handle|coor node num| coord agent pid| client pid| prog_nm|client_nm */
  if (header->col1_opt == prog_nm) {
    if (strlen(listData->prog_nm) > COL_WIDTH)
      *(listData->prog_nm + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, HANDLE, ( (Appl * ) listData)->prog_nm, COL_WIDTH);
  } else if (header->col1_opt == client_nm) {
    if (strlen(listData->client_nm) > COL_WIDTH)
      *(listData->client_nm + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, HANDLE, ( (Appl * ) listData)->client_nm, COL_WIDTH);
  } else if (header->col1_opt == coord_node_num) {
    sprintf(buffer, "%u", listData->coord_node_num);
    mvwaddstr(scrn, row, NODE, buffer);
  } else if (header->col1_opt == coord_pid) {
    sprintf(buffer, "%u", listData->coord_pid);
    mvwaddstr(scrn, row, NODE, buffer);
  } else if (header->col1_opt == client_pid) {
    sprintf(buffer, "%u", listData->client_pid);
    mvwaddstr(scrn, row, NODE, buffer);
  } else {
    sprintf(buffer, "%u", listData->appl_handle);
    if (listData->appl_handle == header->agent_id_oldest_xact)
      strcat(buffer, "*");
    mvwaddstr(scrn, row, HANDLE, buffer);
  }


  /* print when uow started or stopped */
  if ((listData->uow_stop).seconds > 0)
    mvwaddstr(scrn, row, STATUS_CHANGE
        , time_STRING(listData->uow_stop, buffer));
  else
    mvwaddstr(scrn, row, STATUS_CHANGE
        , time_STRING(listData->uow_start, buffer));


  /* print auth id|exec id */
  if (header->col3_opt == exec_id) {
    if (strlen(listData->exec_id) > COL_WIDTH)
      *(listData->exec_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->exec_id, COL_WIDTH);
  } else {
    if (strlen(listData->auth_id) > COL_WIDTH)
      *(listData->auth_id + COL_WIDTH - 1) = '*';
    mvwaddnstr(scrn, row, AUTHID, listData->auth_id, COL_WIDTH);
  }


  /* print appl status | stmt op 

     if uow_stop_time is 0 that means the dynamic
     sql stmt is still running so display the stmt op
     instead of appl status
  */
  if (header->col4_opt == stmt_op)
    mvwaddnstr(scrn, row, STATUS, stmt_op_STRING(listData->stmt_op, buffer), COL_WIDTH);
  else
    mvwaddnstr(scrn, row, STATUS, appl_status_STRING(listData->appl_status, buffer), COL_WIDTH);

  /* print cpu% */
  mvwaddstr(scrn, row, CPU - 1, "|");
  mvwaddnstr(scrn, row, CPU, cpu_STRING( (header->db_ucpu_used_delta + header->db_scpu_used_delta), 
      (listData->ucpu_used_delta + listData->scpu_used_delta), buffer), COL_WIDTH);

  /* print RR/s */
  mvwaddstr(scrn, row, RR - 1, "|");
  mvwaddnstr(scrn, row, RR, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_read_delta, buffer), COL_WIDTH);

  /* print RW/s */
  mvwaddstr(scrn, row, RW - 1, "|");
  mvwaddnstr(scrn, row, RW, 
      uint32rate_STRING(header->snapshot_timestamp_delta, listData->rows_written_delta, buffer), COL_WIDTH);

  /* print BPLR,BPPR|Priv_memusg */
  if (BP + COL_WIDTH + 2 < scrn_col) {
    mvwaddstr(scrn, row, BPLR - 1, "|");
    mvwaddstr(scrn, row, BPPR - 1, "|");
    if (header->col8_opt == bp) {
      mvwaddnstr(scrn, row, BPLR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bpldr_delta + listData->bplir_delta + listData->bpltdir_delta), buffer), COL_WIDTH);
      mvwaddnstr(scrn, row, BPPR, uint32rate_STRING(header->snapshot_timestamp_delta,
          (listData->bppdr_delta + listData->bppir_delta), buffer), COL_WIDTH);
    } else if (header->col8_opt == mem_usg)
      mvwaddstr(scrn, row, MEMUSG, uint64byte_STRING(listData->privagent_memusg, buffer));
    else 
      mvwaddstr(scrn, row, MEMUSG, uint32_STRING(listData->nagents, buffer));
  }

  /* print network in */
  if (TQR + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQR - 1, "|");
    mvwaddnstr(scrn, row, TQR, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_read_delta, buffer), COL_WIDTH);
  }

  /* print network out */
  if (TQW + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, TQW - 1, "|");
    mvwaddnstr(scrn, row, TQW, uint32rate_STRING(header->snapshot_timestamp_delta,
        listData->tq_rows_written_delta, buffer), COL_WIDTH);
  }

  /* print select, insert,update,delete /s */
  if (SIUD + SIUD_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, SIUD - 1, "|");
    if (listData->rows_selected_delta > 0 || listData->rows_inserted_delta > 0
        || listData->rows_updated_delta > 0 || listData->rows_deleted_delta > 0) {
      char  siud_buffer[1024];
      strcpy(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_selected_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_inserted_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_updated_delta, buffer));
      strcat(siud_buffer, ",");

      strcat(siud_buffer, uint32rate_STRING(header->snapshot_timestamp_delta
          , listData->rows_deleted_delta, buffer));
      mvwaddnstr(scrn, row, SIUD, siud_buffer, SIUD_WIDTH);
    }
  }

  /* print the log space usage | RB% */
  if (LOG + COL_WIDTH < scrn_col) {
    mvwaddstr(scrn, row, LOG - 1, "|");
    if (header->col12_opt == locks)
      mvwaddstr(scrn, row, LOG
          , uint32_STRING(listData->locks_held, buffer));
    if (header->col12_opt == log_usg) {
      if (listData->uow_log_space_used > 0)
        mvwaddstr(scrn, row, LOG
            , uint64byte_STRING(listData->uow_log_space_used, buffer));
    } else
      mvwaddstr(scrn, row, RB, pct_STRING(listData->rollback_progress_completed
          , listData->rollback_progress_total, buffer));
  }

  /* ******************************************* */
  /* print detail application activity           */
  /* print buffered and non-buffered io activity */
  /* ******************************************* */
  wattroff(scrn, A_REVERSE);  /* turn off highlight which was turned on when appl row was printed */
  if (row + 10 < scrn_row) {
    row+=1;
    for (col=0; col < scrn_col;col++)
      mvwaddch(scrn,row,col,'-');

    row+=1;
    for (col = 0; col < scrn_col; col++)
      mvwaddch(scrn, row, col, ' ');
    print_column_header_IO(header, scrn, row, scrn_col);

    row+=1;
    for (col = 0; col < scrn_col; col++)
      mvwaddch(scrn, row, col, ' ');
    print_row_IO(scrn, row, listData, header);

    /* ************************************************************ */
    /* Cpu used                                                     */
    /* ************************************************************ */
    row+=1;
    /* clear the row first                */
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, row, col, ' ');
    }
    sprintf(buffer, "{appl|stmt|ss} => CPU (sec.ms):{%.6f|%.6f|%.6f}", 
        ( (listData->appl_ucpu_used).seconds 
        + ( ((double) (listData->appl_ucpu_used).microsec)/1000000)
        )
        + 
        ( (listData->appl_scpu_used).seconds 
        + ( ((double) (listData->appl_scpu_used).microsec)/1000000)
        )
        , ( (listData->stmt_cpu_used).seconds + 
        ( ((double) (listData->stmt_cpu_used).microsec)/1000000))
        , ( (listData->ss_cpu_used).seconds + 
        ( ((double) (listData->ss_cpu_used).microsec)/1000000))
        );
    mvwaddstr(scrn, row, HANDLE, buffer);

    /* ************************************************************ */
    /* Rows read and written                                        */
    /* ************************************************************ */
    row+=1;
    /* clear the row first                */
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, row, col, ' ');
    }
    sprintf(buffer, "{appl}(stmt) => RS|RR|RW:{%'lu|%'lu|%'lu}(%'lu|%'lu|%'lu)", 
        listData->appl_rows_selected
        ,listData->appl_rows_read 
        ,listData->appl_rows_written
        ,listData->stmt_rows_selected
        ,listData->stmt_rows_read 
        ,listData->stmt_rows_written
        );
    mvwaddstr(scrn, row, HANDLE, buffer);

    /* ************************************************************ */
    /* Rows inserted/updated/deleted                                */
    /* ************************************************************ */
    row+=1;
    /* clear the row first                */
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, row, col, ' ');
    }
    /* print appl info b/c stmt info is always 0 */
    sprintf(buffer, "appl => RI|RU|RD:{%'lu|%'lu|%'lu}", 
        listData->appl_rows_inserted
        ,listData->appl_rows_updated 
        ,listData->appl_rows_deleted);
    mvwaddstr(scrn, row, HANDLE, buffer);
    /* ************************************************************ */
    /* Bufferpool usage                                             */
    /* ************************************************************ */
    row+=1;
    /* clear the row first                */
    for (col = 0; col < scrn_col; col++) {
      mvwaddch(scrn, row, col, ' ');
    }
    sprintf(buffer, "{appl}(stmt) => BPLDR|BPLIR|BPLTDIR:{%'lu|%'lu|%'lu}(%'lu|%'lu|%'lu)", 
        listData->appl_bpldr
        ,listData->appl_bplir
        ,listData->appl_bpltdir
        ,listData->stmt_bpldr
        ,listData->stmt_bplir
        ,listData->stmt_bpltdir
        );
    mvwaddstr(scrn, row, HANDLE, buffer);
    row+=1;
  } else
    row+=1;

  return row;

} /* print_row_AGENTID */

/**************************************************************************************************/

void  read_screen_input(int keyboard, WINDOW *scrn, Header *header, DB2List *list)
{

  int  scrn_row
  , scrn_col
  , col, row ;
  char  msg[256]
  , buffer[256]
  , *pBuffer;
  DB2ListElmt * element;
  sqluint32 agentid = 0;
  SnapReq snapReq = *(peek_snapReq(header));


BEGIN:
  if (keyboard != KEY_DOWN && keyboard != KEY_UP && keyboard != 10) {
    nodelay(stdscr, 0); /* wait for user input */
    echo(); /* echo user input */

    curs_set(1); /* show cursor */

    getmaxyx(scrn, scrn_row, scrn_col);
    for (row = scrn_row -1 ; row < scrn_row ; row++) {
      for (col = 0; col < scrn_col; col++)
        mvwaddch(scrn, row, col, ' ');
    }
  }

  switch (keyboard) {
  case 47:
    /* Input is "/" -- Search for the string in the list */
    switch ((peek_snapReq(header))->type) {
    case tbspace_id:
      strcpy(msg,"/");
      mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
      wrefresh(scrn); /* display mesg */
      wgetnstr(scrn, buffer, TABSCHEMA_SZ + TABNAME_SZ);
      /* change the match function */
      db2list_upd_match(list,search_table);
      if ( (element = db2list_lookup(list, buffer)) != NULL) {
        WARN_MSG("Found!");
        list->highlighted_element = element;
      }

      db2list_upd_match(list,match_table);
      break;
    }
    break;
  case 73:
    /* Input is "I" -- Show DB2 connect id or auth id */
    switch ((peek_snapReq(header))->type) {
    case appls:
      if (header->col3_opt == exec_id)
        strncpy(msg, "Show DB2 Connect Id(leave blank to show all):", 256);
      else 
        strncpy(msg, "Show Client Id(leave blank to show all):", 256);

      mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
      wrefresh(scrn); /* display mesg */

      wgetnstr(scrn, buffer, USERID_SZ);
      if (header->col3_opt == exec_id) {
        strncpy(snapReq.auth_id, strtrim(buffer), USERID_SZ);
        strcpy(snapReq.exec_id, "");
        header->col3_opt = auth_id;
      } else {
        strncpy(snapReq.exec_id, strtrim(buffer), USERID_SZ);
        strcpy(snapReq.auth_id, "");
        header->col3_opt = exec_id;
      }
      push_snapReq(header, &snapReq);
      break;
    case agent_id:
      if (header->col3_opt == exec_id)
        header->col3_opt = auth_id;
      else
        header->col3_opt = exec_id;
      break;
    }
    break;
  case 65:
    /* Input is "A" -- turn on/off appl list */
    switch ((peek_snapReq(header))->type) {
    case agent_id: 
      if (header->show_appl_list)
        header->show_appl_list = FALSE; 
      else
        header->show_appl_list = TRUE; 
      break;  
    }  
  case 66:
    /* Input is "B" -- show BP usage column */
    switch ((peek_snapReq(header))->type) {
    case appls:
    case agent_id:
      header->col8_opt = bp;
      break;
    }
    break;
  case 68:
    /* Input is "D" -- turn on/off DBASE header */
    if (header->show_dbase_list == TRUE)
     header->show_dbase_list = FALSE; 
    else
     header->show_dbase_list = TRUE; 
    break; 
  case 87:
    /* Input is "W" -- Write monitor stream in a file*/
    header->dump = TRUE;
    break;
  case 78:
    /* Input is "N" -- Show client name instead of appl handle */
    switch ((peek_snapReq(header))->type) {
    case appls:
      switch (header->col1_opt) {
      case client_nm:
        header->col1_opt = coord_node_num;
        break;
      default:
        strncpy(msg, "Show Client Name(leave blank to show all):", 256);

        mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
        wrefresh(scrn); /* display mesg */


        wgetnstr(scrn, buffer, USERID_SZ);
        strncpy(snapReq.client_nm, strtrim(buffer), USERID_SZ);
        snapReq.agent_id = 0;
        strcpy(snapReq.prog_nm, "");
        header->col1_opt = client_nm;
        push_snapReq(header, &snapReq);
        break;
      }
      break;
    case agent_id:
      switch (header->col1_opt) {
      case client_nm:
        header->col1_opt = coord_node_num;
        break;
      default:
        header->col1_opt = client_nm;
        break;
      }
      break;
    }
    break;
  case 80:
    /* Input is "P" -- Show Prog name  */
    switch ((peek_snapReq(header))->type ) {
    case appls:
      switch (header->col1_opt) {
      case prog_nm:
        header->col1_opt = client_pid;
        break;
      case client_pid:
        header->col1_opt = coord_pid;
        break;
      default :
        strncpy(msg, "Show Prog Name(leave blank to show all):", 256);

        mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
        wrefresh(scrn); /* display mesg */


        strcpy(snapReq.client_nm, "");
        snapReq.agent_id = 0;
        wgetnstr(scrn, buffer, USERID_SZ);
        strncpy(snapReq.prog_nm, strtrim(buffer), USERID_SZ);
        header->col1_opt = prog_nm;
        push_snapReq(header, &snapReq);
      }
      break;
    case agent_id:
      switch (header->col1_opt) {
      case prog_nm:
        header->col1_opt = client_pid;
        break;
      case client_pid:
        header->col1_opt = coord_pid;
        break;
      default:
        header->col1_opt = prog_nm;
        break;
      }
      break;
    }
    break;
  case 72:
    /* Input is "H" -- Show Appl Handle */
    switch ((peek_snapReq(header))->type) {
    case appls:
    case agent_id:
      header->col1_opt = appl_handle;
      break;
    }
    break;
  case 84:
    /* Input is "T" -- take tablespace snapshot */
    if ( snapReq.type != tbspace
        && snapReq.type != tbspace_id) {
      snapReq.type = tbspace;
      snapReq.order_by_col = rr;
      snapReq.order = desc;
      if (strlen(snapReq.dbname) == 0) {
        strncpy(msg, "Enter dbase(leave blank to use default):", 256);

        mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
        wrefresh(scrn); /* display mesg */

        wgetnstr(scrn, buffer, 256);
        if (strlen(strtrim(buffer)) > 0)
          strncpy(snapReq.dbname, strtrim(buffer), SQL_DBNAME_SZ);
        else
          /* use default dbase */
          strncpy(snapReq.dbname, getenv("DB2DBDFT"), SQL_DBNAME_SZ);
      }
      if (strlen(snapReq.dbname) > 0 && push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case 85:
    /* Input is "U" -- take instance snapshot to list utilities */
    if ( snapReq.type != utils
        && snapReq.type != utils_id) {
      snapReq.type = utils;
      if ( push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case 103:
    /* Input is "g" -- take global snapshot */
    if (snapReq.node != -2) {
      snapReq.node = -2;
      if ( push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case 104:
    /* Input is "h" -- show help */
    show_curses_help();
    break;
  case 110:
    /* Input is "n" -- take current or specific node snapshot */
    strncpy(msg, "Request Appl Snapshot from this Node(leave blank to use cur node):", 256);

    mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
    wrefresh(scrn); /* display mesg */


    wgetnstr(scrn, buffer, 256);
    for (pBuffer = strtrim(buffer); *pBuffer != '\0'; pBuffer++) {
      if (!isdigit(*pBuffer))
        break;

    }
    if (*pBuffer == '\0' && strlen(strtrim(buffer)) > 0 )
      snapReq.node = atoi(strtrim(buffer));
    else
      snapReq.node = -1;

    if ( push_snapReq(header, &snapReq) == 0)
      header->reinit_DB2SnapReq = TRUE;
    break;
  case 76:
    /* Input is "L" -- show the log|lock usage column 
                    -- turn on/off lock list */
    switch ((peek_snapReq(header))->type) {
    case appls:
      if (header->col12_opt == log_usg)
        header->col12_opt = locks;
      else 
        header->col12_opt = log_usg;
      break;
    case agent_id:
      if (header->col12_opt == log_usg) {
        header->col12_opt = locks;
        header->show_lock_list = TRUE; 
      } else {
        header->col12_opt = log_usg;
        header->show_lock_list = FALSE;
      }
      break;
    }
    break;
  case 82:
    /* Input is "R" -- show the uow % rolled back column */
    switch ((peek_snapReq(header))->type) {
    case appls:
    case agent_id:
      header->col12_opt = rb;
      break;
    }
    break;
  case 77:
    /* Input is "M" -- show memory usage column */
    switch ((peek_snapReq(header))->type) {
    case appls:
    case agent_id:
      if (header->col8_opt == mem_usg)
        header->col8_opt = num_agents;
      else
        header->col8_opt = mem_usg;
      break;
    }
    break;
  case 83:
    /* Input is "S" --  turn on/off stmt list */
    if ( (peek_snapReq(header))->type == appls) {
      snapReq.type = stmts;
      snapReq.agent_id = 0;

      if ( push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    } else if ( (peek_snapReq(header))->type == agent_id) {
      if (header->show_stmt_list)
        header->show_stmt_list = FALSE; 
      else 
        header->show_stmt_list = TRUE; 
    }
    break;
  case 79:
    /* Input is  "O" -- show stmt operation */
    switch ((peek_snapReq(header))->type) {
    case appls:
    case agent_id:
      header->col4_opt = stmt_op;
      break;
    }
    break;
  case 88:
    /* Input is "X" -- get new refresh interval */
    strncpy(msg, "Refresh interval:", 256);

    mvwprintw(scrn, scrn_row -1, 0, "%s", msg);
    wrefresh(scrn); /* display mesg */


    wgetnstr(scrn, buffer, 256);
    for (pBuffer = strtrim(buffer); *pBuffer != '\0'; pBuffer++) {
      if (!isdigit(*pBuffer))
        break;

    }
    if (*pBuffer == '\0' 
        && strlen(strtrim(buffer)) > 0 
        && atoi(strtrim(buffer)) > 0 )
      header->interval = atoi(strtrim(buffer));
    break;
  case 90:
    /* Input is "Z" -- toggle/untoggle highlighted row */
    if (header->mark == TRUE)
      header->mark = FALSE;
    else
      header->mark = TRUE;

    update_list_highlight(header, list,-1);
    break;
  case 27:
    /* Input is "esc" -- pop a snapshot request */
    if ( (pop_snapReq(header)) == 0 )
      header->reinit_DB2SnapReq = TRUE;
    break;
  case KEY_DOWN:
    header->mark = TRUE;
    /* scroll down without taking snapshots */
SCROLL_DOWN:
    while (keyboard == KEY_DOWN) {
      update_list_highlight(header, list, keyboard);
      update_snapshot_screen(scrn,header, list);
      wrefresh(scrn);
      nodelay(stdscr, 0); /* wait for user input */
      keyboard = getch();
    }
    if (keyboard == KEY_UP)
      goto SCROLL_UP;
    else {
      nodelay(stdscr, 1); /* stop waiting for user input */
      goto BEGIN;
    }
    break;
  case KEY_UP:
    /* set mark to true, which is required for scrolling */
    header->mark = TRUE;
    /* loop, which allows us to scroll up
         without taking snapshots
      */
SCROLL_UP:
    while ( keyboard == KEY_UP) {
      update_list_highlight(header, list, keyboard);
      update_snapshot_screen(scrn,header, list);
      wrefresh(scrn);
      nodelay(stdscr, 0); /* wait for user input */
      keyboard = getch();
    }
    if (keyboard == KEY_DOWN)
      goto SCROLL_DOWN;
    else {
      nodelay(stdscr, 1); /* stop waiting for user input */
      goto BEGIN;
    }
    break;
  case 9:
    /* user pressed tab */
    if (db2list_get_type(list) == agent_id && header->snapScr == wlock_list)
      header->snapScr = wstmt_list;
    else if (db2list_get_type(list) == agent_id && header->snapScr == wstmt_list)
      header->snapScr = wlock_list;
    break;
  case 10:
    /* user pressed enter or newline */
    update_snapshot_request(header, list);
    break;
  case 43:
    /* Input is "+" -- sort in asc order */
    if (snapReq.order != asc) {
      snapReq.order = asc;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 45:
    /* Input is "-" -- sort in desc order */
    if (snapReq.order != desc) {
      snapReq.order = desc;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 99:
    /* Input is "c" -- sort by cpu usg */
    if (snapReq.order_by_col != cpu 
        && snapReq.type != utils && snapReq.type != utils_id) {
      snapReq.order_by_col = cpu;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 114:
    /* Input is "r" -- sort by rows read */
    if (snapReq.order_by_col != rr 
        && snapReq.type != utils && snapReq.type != utils_id) {
      snapReq.order_by_col = rr;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 119:
    /* Input is "w" -- sort by rows written */
    if (snapReq.order_by_col != rw
        && snapReq.type != utils && snapReq.type != utils_id) {
      snapReq.order_by_col = rw;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 115:
    /* Input is "s" -- sort by rows selected */
    if (snapReq.order_by_col !=  sel
        && snapReq.type == appls ) {
      snapReq.order_by_col = sel;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 105:
    /* Input is "i" -- sort by rows inserted */
    if (snapReq.order_by_col !=  ins
        && snapReq.type == appls ) {
      snapReq.order_by_col = ins;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 117:
    /* Input is "u" -- sort by rows updated */
    if (snapReq.order_by_col != upd 
        && snapReq.type == appls ) {
      snapReq.order_by_col = upd;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 100:
    /* Input is "d" -- sort by rows deleted */
    if (snapReq.order_by_col != del
        && snapReq.type == appls ) {
      snapReq.order_by_col = del;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  case 112:
    /* Input is "p" -- sort by dbpartitionnum */
    if (snapReq.order_by_col != node
        && (snapReq.type == agent_id_detl || snapReq.type==utils_id) ) {
      snapReq.order_by_col = node;
      pop_snapReq(header);
      push_snapReq(header, &snapReq);
    }
    break;
  } /* end of (switch) reading keyboard */

  if (keyboard != KEY_UP && keyboard != KEY_DOWN && keyboard != 10) {
    nodelay(stdscr, 1);      /* stop waiting for user intput */
    noecho();                /* don't echo user input */
    curs_set(0);             /* no cursor */

  }

  return;
} /* read_screen_input */


/******************************************************************************************/
void show_curses_help(void)
{
  FILE *fp = NULL;

  if ( (fp = popen(PAGER, "w")) == NULL) {
    WARN_MSG("popen error!");
    return ;
  }
  def_prog_mode();
  endwin();

  fprintf(fp, "db2topas -- help (Press Key) \n");
  fprintf(fp, "\"A\" -- show DB2 connect id or Auth id\n");
  fprintf(fp, "\"B\" -- show Bufferpool usage \n");
  fprintf(fp, "\"C\" -- show login id on Client \n");
  fprintf(fp, "\"H\" -- show application Handle\n");
  fprintf(fp, "\"L\" -- show the Log space usage \n");
  fprintf(fp, "\"M\" -- show application private memory usage \n");
  fprintf(fp, "\"N\" -- show client Name|Coord Partition Num| \n");
  fprintf(fp, "\"O\" -- show application statement Operation \n");
  fprintf(fp, "\"P\" -- show Program name|Client PID|Coord Agent PID \n");
  fprintf(fp, "\"R\" -- show the uow % Rolled back  \n");
  fprintf(fp, "\"S\" -- show application Status during appl snapshot \n");
  fprintf(fp, "\"T\" -- take Tablespace snapshot\n");
  fprintf(fp, "\"U\" -- take instance snapshot to list Utilities such as load\n");
  fprintf(fp, "\"X\" -- get new refresh interval \n");
  fprintf(fp, "\"g\" -- take global snapshot \n");
  fprintf(fp, "\"n\" -- take snapshot from node n \n" );
  fprintf(fp, "\"ESC\"      -- return to previous snapshot request\n");
  fprintf(fp, "\"KEY UP\"   -- move up the snapshot output list \n");
  fprintf(fp, "\"KEY DOWN\" -- move down the snapshot output list \n");
  fprintf(fp, "\"Enter\"    -- get detail information about the highlighted row \n");
  fprintf(fp, "    -During application snapshot (APPLS) Enter will isolate \n");
  fprintf(fp, "     the application and take snapshot of that application (AGENT_ID) only\n");
  fprintf(fp, "    -During AGENT_ID snapshot Enter will display application\n");
  fprintf(fp, "     subsection snapshot (AGENT_ID_SS)\n");
  fprintf(fp, "    -During AGENT_ID_SS snapshot Enter will display command line application\n");
  fprintf(fp, "     snapshot of that application from that database partition\n");
  fprintf(fp, "    -During utility snapshot (UTILITY) Enter will isolate the highlighted \n");
  fprintf(fp, "     utility and only display that (UTILITY_ID)\n");
  fprintf(fp, "    -During tablespace snapshot (TBSPACE) Enter will isolate that tablespace\n");
  fprintf(fp, "     and show the table snapshot (TABLE) of the tables from that tablespace\n");
  fprintf(fp, "\"+\" -- sort rows in ascending order \n");
  fprintf(fp, "\"-\" -- sort rows in descending order \n");
  fprintf(fp, "\"c\" -- sort rows by cpu usg\n");
  fprintf(fp, "\"d\" -- sort rows by num rows deleted \n");
  fprintf(fp, "\"i\" -- sort rows by num rows inserted \n");
  fprintf(fp, "\"p\" -- sort rows by dbpartitionnum \n");
  fprintf(fp, "\"r\" -- sort rows by rows read(RR/s|Read/s) \n");
  fprintf(fp, "\"s\" -- sort rows by num rows selected \n");
  fprintf(fp, "\"u\" -- sort rows by num rows updated \n");
  fprintf(fp, "\"w\" -- sort rows by rows written(RW/s|Write/s) \n");

  if (pclose(fp) == -1)
    WARN_MSG("pclose error");

  reset_prog_mode();

  return;
}
