#ifndef DB2SNAP_H
  #define DB2SNAP_H

#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>   
#include <sqlutil.h>  
#include <sqlenv.h>   
#include <sqlmon.h>   
#include <db2ApiDf.h> 
#include <pthread.h>
#include "stack.h"
#include "db2topasutil.h"  
#include "db2list.h"

#define NUMELEMENTS 975 
#define NUMTYPES  60
#define SNAPSHOT_BUFFER_UNIT_SZ 1024
#define BLANKS "                                                                             "
#define DASHES "----------------------------------------------------------------------"

#define sqlmCheckRC(rc) \
{\
  if ( rc ) \
  { \
     printf( "Addsnapshot returned error=%d sqlcode =%d\n", rc, sqlca.sqlcode );\
  } \
  if (sqlca.sqlcode != 0L) \
  { \
    SqlInfoPrint("db2AddSnapshotRequest", &sqlca, __LINE__, __FILE__); \
    if (sqlca.sqlcode < 0L) \
    { \
      printf("\n%s",DASHES); \
      printf("\ndb2GetSnapshotSize SQLCODE is %d. Exiting.", sqlca.sqlcode); \
      printf("\n%s\n",DASHES); \
      return((int)sqlca.sqlcode); \
    } \
  } \
}


/***********************************************************************************************/
/* -lock struct is used to hold information about a lock and lock-wait 
*/
/***********************************************************************************************/
typedef struct Lock_ {
  SnapReqType      snap_req_type ; /* type of snapshot */
  sqluint32        agent_id; /* lock into about this agent_id */
  sqluint32        ss_number ; /* subsection number waiting for lock */
  sqluint16        node; 
  
  sqluint32        agent_id_holding_lk;
  
  sqluint32        lock_obj_type    ;
  sqluint32        lock_mode;
  sqluint32        lock_mode_requested;
  sqluint32        lock_status;
  sqluint8         lock_escal;
  sqluint32        count; /* how many of this type of lock being held */
  char             tabschema[TABSCHEMA_SZ + 1];
  char             tabname[TABNAME_SZ + 1];
  sqluint16        data_partition_id;  /* data partition id of the table */
  char             tbspace [SQLUH_TABLESPACENAME_SZ+1] ;
  

} Lock;

/***********************************************************************************************/
/* -table struct is used to hold all information about a table
*/
/***********************************************************************************************/
typedef struct Table_ {
  SnapReqType      snap_req_type ; /* type of snapshot */
  sqluint32        tbspace_id;
  char             tabschema[TABSCHEMA_SZ + 1];
  char             tabname[TABNAME_SZ + 1];
  sqluint32        type;
  sqluint16        data_partition_id;
  sqlint16         avg_row_sz;
  sqluint64        rr;                   /* rows read                       */
  sqluint64        rr_delta;
  sqluint64        rw;                   /* rows written                    */
  sqluint64        rw_delta;
  sqluint64        data_sz;              /* in pages; includes LOB          */
  sqluint64        ix_sz;                /* in pages                         */
  
} Table;
/***********************************************************************************************/
/* -tbspace struct is used to hold all information about a tablespace
*/
/***********************************************************************************************/
typedef struct Tbspace_ {
  SnapReqType       snap_req_type ; /* type of snapshot */
  sqluint32         id;
  sqluint32         pg_sz;
  sqluint32         state;
  sqluint32         t1_total_pg;
  sqluint32         t2_total_pg;
  sqluint64         t1_fs_total_sz;
  sqluint64         t2_fs_total_sz;
  sqluint32         t1_used_pg;
  sqluint64         t1_fs_used_sz;
  sqluint64         t2_fs_used_sz;
  sqluint32         t2_used_pg;
  sqluint32         t1_free_pg;
  sqluint32         t2_free_pg;
  sqluint16         node;
  sqluint8          type;
  sqluint8          content_type;
  char              name[SQLUH_TABLESPACENAME_SZ+1] ;

  /* Data stats */
  sqluint64         l_bp_d_reads;
  sqluint64         p_bp_d_reads;
  sqluint64         p_bp_d_read_reqs;
  sqluint64         a_bp_d_reads;
  sqluint64         bp_d_writes;
  sqluint64         a_bp_d_writes;
  sqluint64         l_bp_d_reads_delta;
  sqluint64         p_bp_d_reads_delta;
  sqluint64         p_bp_d_read_req_delta;
  sqluint64         a_bp_d_reads_delta;
  sqluint64         p_bp_d_write_delta;
  sqluint64         a_bp_d_write_delta;

  /* Index stats */
  sqluint64         l_bp_i_reads;         
  sqluint64         p_bp_i_reads;         
  sqluint64         p_bp_i_read_reqs;     
  sqluint64         a_bp_i_reads;         
  sqluint64         bp_i_writes;          
  sqluint64         a_bp_i_writes;        
  sqluint64         l_bp_i_reads_delta;   
  sqluint64         p_bp_i_reads_delta;   
  sqluint64         p_bp_i_read_req_delta;
  sqluint64         a_bp_i_reads_delta;   
  sqluint64         p_bp_i_write_delta;   
  sqluint64         a_bp_i_write_delta;   

  /* Data/Index read write time */
  sqluint64        bp_read_tm; 
  sqluint64        bp_write_tm; 
  sqluint64        bp_read_tm_delta; 
  sqluint64        bp_write_tm_delta; 

  /* TEMP stats */
  sqluint64         l_bp_t_reads;
  sqluint64         p_bp_t_reads;
  sqluint64         a_bp_t_reads;
  sqluint64         l_bp_t_reads_delta;
  sqluint64         p_bp_t_reads_delta;
  sqluint64         a_bp_t_reads_delta;

  /* XML stats */
  sqluint64         l_bp_x_reads;         
  sqluint64         p_bp_x_reads;         
  sqluint64         p_bp_x_read_reqs;     
  sqluint64         a_bp_x_reads;         
  sqluint64         bp_x_writes;          
  sqluint64         a_bp_x_writes;        
  sqluint64         l_bp_x_reads_delta;   
  sqluint64         p_bp_x_reads_delta;   
  sqluint64         p_bp_x_read_req_delta;
  sqluint64         a_bp_x_reads_delta;   
  sqluint64         p_bp_x_write_delta;   
  sqluint64         a_bp_x_write_delta;   

  sqluint64         t1_bp_reads;
  sqluint64         t2_bp_reads;
  sqluint64         bp_reads_delta;

  sqluint64         t1_bp_writes;
  sqluint64         t2_bp_writes;
  sqluint64         bp_writes_delta;

  sqluint64         t1_direct_reads;
  sqluint64         t2_direct_reads;
  sqluint64         direct_reads_delta;

  sqluint64         t1_direct_writes;
  sqluint64         t2_direct_writes;
  sqluint64         direct_writes_delta;

  sqluint64         t1_direct_read_reqs;
  sqluint64         t2_direct_read_reqs;
  sqluint64         direct_read_reqs_delta;

  sqluint64         t1_direct_write_reqs;
  sqluint64         t2_direct_write_reqs;
  sqluint64         direct_write_reqs_delta;

  sqluint64         reads_delta;
  sqluint64         writes_delta;
 
} Tbspace;

/***********************************************************************************************/
/* -util struct is used to hold all information
    that are returned by the "list utility" command
*/
/***********************************************************************************************/

typedef struct Util_ {
  SnapReqType       snap_req_type ; /* type of snapshot */
  sqluint32         id; /* uniq id of an utility invocation */
  sqluint32         type;
  sqluint32         state;
  sqluint16         node; 

  sqlm_timestamp    start_time;

  char              dbname[SQL_DBNAME_SZ+1];

  char              desc[UTIL_DESC_SZ + 1];

  sqluint64         progress_total;
  sqluint64         progress_completed;

} Util; 


/**************************************************************************************************/
/*
  -appl_ss is used to hold
   subsection output of an appl
*/
/**************************************************************************************************/

typedef struct Appl_ss_ {
  SnapReqType        snap_req_type ; /* type of snapshot */

  sqluint32          appl_handle; 
  sqluint16          ss_number;                         
  sqluint16          ss_status;                       
  sqluint16          ss_node_number;                   /* Node where the subsection is executing */
  sqluint32          stmt_section_number;              /* subsection of this stmt number         */

  sqluint32          ss_exec_time;                  /* Execution elapsed time in seconds      */

  sqlm_time          ss_ucpu_used;
  sqlm_time          ss_scpu_used;
  sqluint64          ss_ucpu_used_delta; 
  sqluint64          ss_scpu_used_delta; 

  sqluint64          ss_rows_read;                                                           
  sqluint32          ss_rows_read_delta;                                                     

  sqluint64          ss_rows_written;                                                        
  sqluint32          ss_rows_written_delta;                                                   

  sqluint64          tq_rows_read;                /* Total # rows received on tablequeues   */ 
  sqluint32          tq_rows_read_delta;                                     
                                                                                           
  sqluint64          tq_rows_written;             /* Total # rows sent on tablequeues       */                                          
  sqluint32          tq_rows_written_delta;                                                

  sqluint32          tq_cur_spills;
  sqluint32          tq_tot_spills;

  sqluint16          tq_node_waiting_for;
  sqluint16          tq_wait_for_any;
  sqluint16          tq_id_waiting_on;

  sqluint32          num_agents;                     /* # of agents currently working on subs  */

} Appl_SS ;

/************************************************************************************************/
/* -Stmt struct represents DB2  stmt snapshot information                                
   of one application                                                                     
   -The structs are maintained in a double-linked list                                    
*/                                                                                        
/************************************************************************************************/
typedef struct Stmt_ {                                                                             
  SnapReqType        snap_req_type ; /* type of snapshot */
  sqluint32          appl_handle ;                                                        
  sqluint16          stmt_node_number; 
  sqluint32          section_number;                                                         
  char               sequence_no[SQLM_SEQ_SZ+1];

  char               auth_id[USERID_SZ+1];            /* DB connect id                            */
  char               exec_id[USERID_SZ+1];            /* Client login id                          */                                                 

  sqlm_timestamp     stmt_start;
  sqluint32          stmt_op;                                                         

                                                                                          
  sqlm_time          stmt_ucpu_used;
  sqlm_time          stmt_scpu_used;
  sqluint64          ucpu_used_delta;                                                           
  sqluint64          scpu_used_delta;                                                           

  sqluint32          nagents; /* num_assoc_agents                                      */

  sqluint64          stmt_rows_read;                                                           
  sqluint32          rows_read_delta; /* based on appl_rows_read                       */                                                    

  sqluint64          stmt_rows_written;                                                        
  sqluint32          rows_written_delta;  /* based on appl_rows_written                */                                                 
                                                                                           
                     /* bpXXX where bp means buffer pool                               */
  sqluint64          stmt_bpldr; /* logical data reads                                 */
  sqluint32          bpldr_delta;
  sqluint64          stmt_bppdr; /* physical data reads                                */
  sqluint32          bppdr_delta;

                     /* bpXXX where bp means buffer pool                               */
  sqluint64          stmt_bplir; /* logical index reads                                */
  sqluint32          bplir_delta;
  sqluint64          stmt_bppir; /* physical index reads                               */ 
  sqluint32          bppir_delta;

                     /* bpXXX where bp means buffer pool                               */
  sqluint64          stmt_bpltdir;
  sqluint32          bpltdir_delta;

  sqluint64          stmt_rows_selected;                                                        
  sqluint32          rows_selected_delta;                                                  
                                                                                           
  sqluint64          stmt_rows_inserted;                                                        
  sqluint32          rows_inserted_delta;                                                  
                                                                                           
  sqluint64          stmt_rows_updated;                                                         
  sqluint32          rows_updated_delta;                                                   
                                                                                           
  sqluint64          stmt_rows_deleted;                                                         
  sqluint32          rows_deleted_delta;                                                   

  sqluint64          stmt_sorts;
  sqluint64          stmt_total_sort_time;
  sqluint64          stmt_sort_overflows;

  sqluint32          query_cost;
  sqluint32          query_card; 
  char               stmt_text[STMT_SZ + 1];
} Stmt ;                                                                      

/************************************************************************************************/
/* -Appl struct represents DB2 application-level snapshot                                 
   of one application                                                                     
   -The structs are maintained in a single-linked list                                    
*/                                                                                        
/************************************************************************************************/
typedef struct Appl_ {                                                                             
  SnapReqType        snap_req_type ; /* type of snapshot */
  sqluint32          appl_handle ;                                                        
  sqluint32          coord_node_num;
  sqluint32          coord_pid;
  sqluint32          client_pid;
  char               prog_nm[USERID_SZ+1];  
  char               client_nm[USERID_SZ+1];

  sqlm_timestamp     uow_start;
  sqlm_timestamp     uow_stop;

  char               auth_id[USERID_SZ+1]; /* DB connect id                            */
  char               exec_id[USERID_SZ+1]; /* Client login id                          */                                                 

  sqluint32          appl_status;                                                         
  sqluint32          stmt_op;                                                         

                                                                                          
  sqlm_time          elapsed_exec_time; /* SQLM_ELM_ELAPSED_EXEC_TIME                  */
  sqluint32          elapsed_exec_time_delta; 
  sqlm_time          appl_ucpu_used;
  sqlm_time          appl_scpu_used;
  sqlm_time          stmt_cpu_used;
  sqlm_time          ss_cpu_used;
  sqluint64          ucpu_used_delta;        /* based on appl_cpu_used                  */                                                   
  sqluint64          scpu_used_delta;        /* based on appl_cpu_used                  */                                                   

  sqluint32          nagents; /* num_assoc_agents                                      */

  sqluint64          appl_rows_read;                                                           
  sqluint64          stmt_rows_read;                                                           
  sqluint32          rows_read_delta; /* based on appl_rows_read                       */                                                    

  sqluint64          appl_rows_written;                                                        
  sqluint64          stmt_rows_written;                                                        
  sqluint32          rows_written_delta;  /* based on appl_rows_written                */                                                 
                                                                                           
                     /* bpXXX where bp means buffer pool                               */
  sqluint64          appl_bpldr; /* logical data reads                                 */
  sqluint64          stmt_bpldr; /* logical data reads                                 */
  sqluint32          bpldr_delta;
  sqluint64          appl_bppdr; /* physical data reads                                */
  sqluint64          stmt_bppdr; /* physical data reads                                */
  sqluint32          bppdr_delta;
  sqluint64          bpdw;      /* data writes                                         */
  sqluint32          bpdw_delta;  

                     /* bpXXX where bp means buffer pool                               */
  sqluint64          appl_bplir; /* logical index reads                                */
  sqluint64          stmt_bplir; /* logical index reads                                */
  sqluint32          bplir_delta;
  sqluint64          appl_bppir; /* physical index reads                               */ 
  sqluint64          stmt_bppir; /* physical index reads                               */ 
  sqluint32          bppir_delta;
  sqluint64          bpiw; /* index writes                                             */
  sqluint32          bpiw_delta; 

                     /* bpXXX where bp means buffer pool                               */
  sqluint64          appl_bpltdir;
  sqluint64          stmt_bpltdir;
  sqluint32          bpltdir_delta;

                     /* buffer pool read/write time                                    */
  sqluint64          bpr_tm ;/* read time                                               */
  sqluint32          bpr_tm_delta;
  sqluint64          bpw_tm; /* write time                                              */
  sqluint32          bpw_tm_delta;

  sqluint64          dio_reads;
  sqluint32          dio_reads_delta;
  sqluint64          dio_read_reqs;
  sqluint32          dio_read_reqs_delta;

  sqluint64          dio_writes;
  sqluint32          dio_writes_delta; 
  sqluint64          dio_write_reqs;
  sqluint32          dio_write_reqs_delta;

  sqluint64          dio_read_tm;
  sqluint32          dio_read_tm_delta;
  sqluint64          dio_write_tm;
  sqluint32          dio_write_tm_delta; 
                                                                                           
                     /* TQ means table queue                                            */
  sqluint64          tq_rows_read;                                                         
  sqluint32          tq_rows_read_delta;                                                   
                                                                                           
  sqluint64          tq_rows_written;                                                      
  sqluint32          tq_rows_written_delta;                                                
                                                                                           
  sqluint64          appl_rows_selected;                                                        
  sqluint64          stmt_rows_selected;                                                        
  sqluint32          rows_selected_delta;                                                  
                                                                                           
  sqluint64          appl_rows_inserted;                                                        
  sqluint64          stmt_rows_inserted;                                                        
  sqluint32          rows_inserted_delta;                                                  
                                                                                           
  sqluint64          appl_rows_updated;                                                         
  sqluint64          stmt_rows_updated;                                                         
  sqluint32          rows_updated_delta;                                                   
                                                                                           
  sqluint64          appl_rows_deleted;                                                         
  sqluint64          stmt_rows_deleted;                                                         
  sqluint32          rows_deleted_delta;                                                   

  sqluint32	     locks_held;
  sqluint64          uow_log_space_used;
  sqluint64          rollback_progress_total;
  sqluint64          rollback_progress_completed;
  sqluint64          privagent_memusg;            /* Amt used by agents per appl    */
  sqluint32          query_cost;
  sqluint32          query_card; 

  /* record num stmts during APPLS snapshot and don't populate stmt_list */
  int                num_stmts;
  DB2List            stmt_list;
  DB2List            lock_list;

} Appl ;                                                                      

/******************************************************************************************/
/* Struct to pass arguments to and return results from take_db2_snapshot function         */ 
/******************************************************************************************/
typedef struct TakeDb2SnapshotArgs_ {
  /* args ****************************/
  Header                 *header;
  sqluint32              buffer_sz;
  char                   *buffer_ptr;
  db2AddSnapshotRqstData db2SnapReq; /* used to send snapshot request to DB2 API */
  Boolean                cancel;
  long_lock_t            snap_lock;

  /* result ************************/
  void                  *status;

} TakeDb2SnapshotArgs;
/******************************************************************************************/
/* Struct to pass arguments to and return results from getreclen         function         */ 
/******************************************************************************************/
typedef  struct GetReclenArgs_ {                              
    char dbAlias [ SQL_DBNAME_SZ + 1 ]; 
    char user [USERID_SZ + 1];          
    char pswd [PSWD_SZ + 1];            
    Table *table;                       
 } GetReclenArgs;                         



/******************************************************************************************/
/* public functions */
/******************************************************************************************/

int     search_table(const void *table,const void *str);
int     match_table(const void *, const void *);

int free_snapshot_memory(struct sqlma *ma_ptr, char *buffer_ptr);

int init_snapRequest(db2AddSnapshotRqstData *snapReq 
                     ,char **buffer_ptr              
                     ,sqluint32 *buffer_sz           
                     ,SnapReqType snap_req_type
                     ,const int node                       
                     ,const char *dbname                   
                     ,const sqluint32 agentid
                     ,DB2List *list )
;

void * take_db2_snapshot(void *);
int  getreclen(void *);

int parse_monitor_stream(Header *header
                         , DB2List *list
                         , char *buffer_ptr);

void sort_list(Header *, DB2List *);

void update_list_highlight(Header *, DB2List *, int);

void update_snapshot_request(Header *, DB2List *);

#endif
