#ifndef DB2TOPASUTIL_H
  #define DB2TOPASUTIL_H

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <unistd.h>   
#include <sys/param.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>

#include <sqlenv.h>
#include <sqlda.h> 
#include <sqlca.h> 
#include <sqlmon.h>
#include <sqlcli1.h> 
#include <sqlutil.h> 
#include <db2ApiDf.h>

#include "stack.h"

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef max
  #define max(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef min
  #define min(A, B) ((A) > (B) ? (B) : (A))
#endif

#ifndef delta
  #define delta(A,B) ((A) > (B) ? ((A) - (B)) : 0)
#endif

#ifndef SQL_DIAGPATH_SZ        
  #define SQL_DIAGPATH_SZ  512   
#endif                         

#ifndef UTIL_DESC_SZ
  #define UTIL_DESC_SZ 1024
#endif

#ifndef TABSCHEMA_SZ
  #define TABSCHEMA_SZ 128
#endif

#ifndef TABNAME_SZ
  #define TABNAME_SZ 128
#endif
/* Boolean data type */ 
#ifndef Boolean         
  typedef int Boolean;  
  #ifndef FALSE         
    #define FALSE 0     
  #endif                
  #ifndef  TRUE         
    #define TRUE 1      
  #endif                
#endif                  

#define PAGER "${PAGER:-more}"

#define USERID_SZ 128
#define STMT_SZ 256
#define MAX_STMTS 100
#define PSWD_SZ 14
#define MAXLINE 1024

#if (defined(DB2NT))
  #define PATH_SEP "\\"
#else /* UNIX */
  #define PATH_SEP "/"
#endif

/**************************************************************/
/* macro for printing error mesg                              */
/**************************************************************/
#define ERR_MSG(MSG_STR)                                      \
do {                                                          \
  db2AdminMsgWriteStruct msg;                                 \
  struct                 sqlca msg_sqlca;                     \
                                                              \
  msg.iMsgType = STRING_MSG ;                                 \
  msg.iComponent = 0;                                         \
  msg.iFunction = 0;                                          \
  msg.iProbeID = __LINE__;                                    \
  msg.piData_title = __FILE__;                                \
  msg.piData = MSG_STR;                                       \
  msg.iError_type = DB2LOG_ERROR;                             \
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);   \
  return -1;                                                  \
} while (0)


/**************************************************************/
/* macro for printing warning mesg                            */
/**************************************************************/
#define WARN_MSG(MSG_STR)                                     \
do {                                                          \
  db2AdminMsgWriteStruct msg;                                 \
  struct                 sqlca msg_sqlca;                     \
                                                              \
  msg.iMsgType = STRING_MSG ;                                 \
  msg.iComponent = 0;                                         \
  msg.iFunction = 0;                                          \
  msg.iProbeID = __LINE__;                                    \
  msg.piData_title = __FILE__;                                \
  msg.piData = MSG_STR;                                       \
  msg.iError_type = DB2LOG_WARNING;                           \
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);   \
} while (0)

/**************************************************************/
/* macro for printing informational mesg                      */
/**************************************************************/
#define INFO_MSG(MSG_STR)                                     \
do {                                                          \
  db2AdminMsgWriteStruct msg;                                 \
  struct                 sqlca msg_sqlca;                     \
                                                              \
  msg.iMsgType = STRING_MSG ;                                 \
  msg.iComponent = 0;                                         \
  msg.iFunction = 0;                                          \
  msg.iProbeID = __LINE__;                                    \
  msg.piData_title = __FILE__;                                \
  msg.piData = MSG_STR;                                       \
  msg.iError_type = DB2LOG_INFORMATION;                       \
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);   \
} while (0)   

/**************************************************************/
/* macro for DB2_API checking */
/**************************************************************/
#define DB2_API_CHECK(MSG_STR)                                \
do {                                                          \
  SqlInfoPrint(MSG_STR, &sqlca, __LINE__, __FILE__);          \
  if (sqlca.sqlcode < 0)                                      \
  {                                                           \
    return 1;                                                 \
  }                                                           \
} while (0)

/**************************************************************/
/* macro for environment handle checking */         
/**************************************************************/
#define ENV_HANDLE_CHECK(henv, cliRC)                         \
do {                                                          \
  if (cliRC != SQL_SUCCESS)                                   \
  {                                                           \
    rc = HandleInfoPrint(SQL_HANDLE_ENV, henv,                \
                         cliRC, __LINE__, __FILE__);          \
    if (rc != 0) return rc;                                   \
  }                                                           \
} while (0)
                                                    
/**************************************************************/
/* macro for connection handle checking */          
/**************************************************************/
#define DBC_HANDLE_CHECK(hdbc, cliRC)                         \
do {                                                          \
  if (cliRC != SQL_SUCCESS)                                   \
  {                                                           \
    rc = HandleInfoPrint(SQL_HANDLE_DBC, hdbc,                \
                         cliRC, __LINE__, __FILE__);          \
    if (rc != 0) return rc;                                   \
  }                                                           \
} while (0)
/**************************************************************/
/* macro for statement handle checking */           
/**************************************************************/
#define STMT_HANDLE_CHECK(hstmt, hdbc, cliRC)                 \
do {                                                          \
  if (cliRC != SQL_SUCCESS)                                   \
  {                                                           \
    rc = HandleInfoPrint(SQL_HANDLE_STMT, hstmt,              \
                         cliRC, __LINE__, __FILE__);          \
    if (rc == 2) StmtResourcesFree(hstmt);                    \
    if (rc != 0) TransRollback(hdbc);                         \
    if (rc != 0) return rc;                                   \
  }                                                           \
} while (0)
/**************************************************************/
/* functions used in ..._CHECK macros */
/**************************************************************/
void SqlInfoPrint(char *, struct sqlca *, int, char *);
int HandleInfoPrint(SQLSMALLINT, SQLHANDLE, SQLRETURN, int, char *); 
/**************************************************************/
/* lock used to snychronize threads                           */
/**************************************************************/
typedef struct {
        pthread_mutexattr_t lock_attr;
        pthread_mutex_t lock;
        pthread_cond_t cond;
        int free;
        int wanted;
} long_lock_t;

/**************************************************************/
/* snapshot screen where key up and key down should work      */
/**************************************************************/
typedef enum SnapScr_ {
   wstmt_list
  ,wlock_list
} SnapScr; 

/**************************************************************/
/* Type of snapshot requested                                 */
/**************************************************************/
typedef enum SnapReqType_ { 
                   appls
                  ,stmts 
                  ,locklist 
                  ,agent_id
                  ,agent_id_detl
                  ,agent_id_cmdline
                  ,utils
                  ,utils_id
                  ,tbspace
                  ,tbspace_id
                  ,unknown } SnapReqType;

/**************************************************************/
/* Column used to order rows                                  */
/**************************************************************/
typedef enum OrderByCol_ { cpu
                  , tqr
                  , tqw
                  , rr
                  , rw
                  , sel
                  , ins
                  , upd
                  , del
                  , memusg
                  , bppgin
                  , logusg
                  , node
                  , nagents
                  , rolledback } OrderByCol;

/**************************************************************/
/* How rows are ordered                                       */
/**************************************************************/
typedef enum Order_ { asc
                     , desc } Order;

/**************************************************************/
/* Struct to hold snapshot request                            */
/**************************************************************/
typedef struct SnapReq_ {
  SnapReqType type;
  sqluint32   agent_id;                   /* show this specific appl handle only */  
  sqluint32   util_id;
  sqluint32   tbspace_id;
  sqluint32   tbspace_pg_sz;
  sqluint8    tbspace_content_type;
  char        tbspace [SQLUH_TABLESPACENAME_SZ+1]   ;
  char        exec_id[USERID_SZ + 1];     /* show appl with this exec_id only    */    
  char        auth_id[USERID_SZ + 1];     /* show appl with this auth_id only    */    
  char        prog_nm[USERID_SZ + 1];     /* show appl with this prog_nm only    */  
  char        client_nm[USERID_SZ + 1];   /* show appl with this client_nm       */  
  char        dbname[SQL_DBNAME_SZ + 1];  /* db being monitored; if NULL then         
                                              monitor all db under the instance  */     
  int       node;                         /* take app snapshot from this node        
                                             -2 means global                         
                                             -1 means current node                   
                                             +n means node n  */                       
   sqluint16  stmt_node_number;
   sqluint32  stmt_section_number;
   char       sequence_no [SQLM_SEQ_SZ + 1];
   char       stmt_text[STMT_SZ + 1];
  OrderByCol order_by_col;                                                              
  Order     order;
} SnapReq;

/**************************************************************/
/* Type of data displayed in col1 of the curses screen        */ 
/**************************************************************/
typedef enum Col1_opt_ { prog_nm
                        ,client_nm
                        ,util_id
                        ,coord_node_num
                        ,coord_pid
                        ,client_pid
                        ,appl_handle } Col1_opt;

/**************************************************************/
/* Type of data displayed in col3 of the curses screen        */ 
/**************************************************************/
typedef enum Col3_opt_ { exec_id, auth_id} Col3_opt;

/**************************************************************/
/* Type of data displayed in col4 of the curses screen        */ 
/**************************************************************/
typedef enum Col4_opt_ { stmt_op, appl_status} Col4_opt;

/**************************************************************/
/* Type of data displayed in col8 of the curses screen        */ 
/**************************************************************/
typedef enum Col8_opt_ { bp, mem_usg, num_agents} Col8_opt;

/**************************************************************/
/* Type of data displayed in col12 of the curses screen        */ 
/**************************************************************/
typedef enum Col12_opt_ { rb, log_usg,locks} Col12_opt;


            
                 

/**************************************************************/
/* -header that's displayed constantly on the curses screen  
   -use to store cmdline options or input from curses screen 
                                                             
*/                                                           

/**************************************************************/
typedef struct Header_ {                                                                       
  SnapScr   snapScr;
  Stack     snap_req_stack;                /* Stack to hold type snapshot req    */
  char      myname[256]  ;                 /* program name                       */ 
  int       spin;                                                                      
  char      user_id[USERID_SZ+1];          /* user_id used to attach to instance */    
  char      instance[SQL_INSTNAME_SZ + 1]; /* instance being monitored           */    
  char      pswd[PSWD_SZ + 1];                                                         
  char      hostname[MAXHOSTNAMELEN + 1];  /* hostname where the prog started    */                                              
  char      db2node[5];                                                                
  unsigned int interval;                      /* snapshot or refresh interval       */    
                                                                                       
                                                                                       

  Col1_opt  col1_opt;
  Col3_opt  col3_opt;
  Col4_opt  col4_opt;
  Col8_opt  col8_opt;
  Col12_opt col12_opt;

  Boolean  reinit_DB2SnapReq;               /* reinitialize DB2 snapshot request  */  
  /*******************************************************************************/
  /* variables used to summarize information about DBASE or APPL
     in the header
  */ 
  /*******************************************************************************/
  sqlm_timestamp t1_snapshot_timestamp;
  sqlm_timestamp t2_snapshot_timestamp;
  double     snapshot_timestamp_delta;
  sqluint32  appls_connected;                                               
  sqluint32  appls_executing;                                               
  sqluint32  db_assoc_agents; /* -number of agents associated to applications 
                                 -summed up from APPL snapshots
                                 -used to calculate cpu % */
                                 

  sqluint64  db_memusg;                                                     
  sqluint64  db_genheap;                                                     
  sqluint32  db_sortheap;
  sqluint64  db_lockheap;
  sqluint64  db_utilheap;

  
                                                                             
  sqluint64  db_ucpu_used_delta ;
  sqluint64  db_scpu_used_delta ;
                                                                             
  sqluint64  t1_db_bpr_tm;
  sqluint64  t2_db_bpr_tm;
  sqluint64  t1_db_bpw_tm;
  sqluint64  t2_db_bpw_tm;

  sqluint64  t1_db_buffered_rio; 
  sqluint64  t1_db_buffered_wio; 
  sqluint64  t2_db_buffered_rio;                     
  sqluint64  t2_db_buffered_wio;                     

  sqluint64  t1_db_direct_io ;
  sqluint64  t2_db_direct_io;                         
  sqluint64  t1_db_direct_io_reqs ;
  sqluint64  t2_db_direct_io_reqs;               

  sqluint64  t1_db_io_type_read;
  sqluint64  t2_db_io_type_read;
  sqluint64  t1_db_io_type_write;
  sqluint64  t2_db_io_type_write;
  sqluint64  t1_db_io_type_data;
  sqluint64  t2_db_io_type_data;
  sqluint64  t1_db_io_type_idx;
  sqluint64  t2_db_io_type_idx;
  sqluint64  t1_db_io_type_temp;
  sqluint64  t2_db_io_type_temp;
  sqluint64  t1_db_io_type_xml;
  sqluint64  t2_db_io_type_xml;
  sqluint64  t1_db_io_type_dio;
  sqluint64  t2_db_io_type_dio;
                                                                             
  sqluint64  t1_db_log_reads;
  sqluint64  t2_db_log_reads;
  sqluint64  t1_db_log_writes;
  sqluint64  t2_db_log_writes;
  sqluint64  db_log_avail;                                                  
  sqluint64  db_log_used;                                                   
  sqluint16  smallest_log_avail_node;                                    
  sqluint32  agent_id_oldest_xact;

  Boolean   mark;                         /* toggle the currently highlighted row if true */ 
  Boolean   dump;                         /* dump monitor stream in a file                */ 

  int       dbase_scrn_row;               /* number of rows used by database screen       */

  Boolean   sequence_no_found;
  Boolean   section_number_found; 
  Boolean   node_number_found; 
  char      *pStmtStart;

  Boolean   show_dbase_list;              /* turn on/off database screen at the top       */
  Boolean   show_appl_list;               /* turn on/off appl list                        */
  Boolean   show_lock_list;               /* turn on/off lock list                        */
  Boolean   show_stmt_list;               /* turn on/off stmt list                        */
 
} Header;                                                                            

/**************************************************************/
/* Public utility functions */
/**************************************************************/
int CLIAppInit(char dbAlias[],            
               char user[],               
               char pswd[],               
               SQLHANDLE *pHenv,          
               SQLHANDLE *pHdbc,          
               SQLPOINTER autocommitValue);
int       CLIAppTerm(SQLHANDLE * pHenv, SQLHANDLE * pHdbc, char dbAlias[]);
int       StmtResourcesFree(SQLHANDLE);
void      TransRollback(SQLHANDLE);   
int       InstanceAttach(char * , char *, char *);
int       InstanceDetach(char *);

Header        *init_header();
void           free_header(Header *);
const SnapReq *peek_snapReq(Header *);
int            push_snapReq(Header *, const SnapReq *);
int            pop_snapReq(Header *);
int            num_snapReq(Header *);

char    *strrtrim(char *);
char    *strltrim(char *);
char    *strtrim(char *);
char    *strrev(char *); 
char    *basename(const char*);
void    chomp(char *);

/*
void long_lock_init(long_lock_t *);
void long_lock_destroy(long_lock_t *);
void long_lock(long_lock_t *);
int long_trylock(long_lock_t *);
void long_unlock(long_lock_t *);
*/
char  *time_STRING(sqlm_timestamp timestamp, char *timeString);
sqluint32 timediff(sqlm_time, sqlm_time);
sqluint32 timestampdiff(sqlm_timestamp, sqlm_timestamp);



#ifdef __cplusplus
  }
#endif


#endif /* DB2TOPASUTIL_H */

