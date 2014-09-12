#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
/************************************/
/* these headers are required
   to handle agent_id_detl snapshot
*/
/************************************/
#include <sys/wait.h>
#include <ncurses.h>

#include "db2topasutil.h"
#include "db2snap.h"

/*****************************************************************************************
    private helper functions 
******************************************************************************************/
int     init_element_names(char *[]);
int     init_element_types(char *[]);

Lock    *init_lock(void);
Stmt    *init_stmt(void);
Appl    *init_appl(void);
Appl_SS *init_appl_ss(void);
Util    *init_util(void);
Tbspace *init_tbspace(void);
Table   *init_table(void);

void    free_appl(void *);

int     copy_stmt(Stmt * , const Stmt *);
int     copy_appl(Appl * , const Appl *);
int     copy_appl_ss(Appl_SS * , const Appl_SS *);
int     copy_util(Util *, const Util *);
int     copy_tbspace(Tbspace *, const Tbspace *);
int     copy_table(Table *, const Table *);

int     match_appl(const void * , const void *);
int     match_stmt(const void * , const void *);
int     match_lock(const void * , const void *);
int     match_appl_ss(const void *, const void *);
int     match_util(const void *, const void *);
int     match_util_detl(const void *, const void *);
int     match_tbspace(const void *, const void *);

void    delta_tbspace(Tbspace *);
void    reclen_table(Header *, DB2List *);
int     getreclen_select(SQLHANDLE,char [], char [], sqlint16 *);

int     reset_table(Table *);
int     reset_tbspace(Tbspace *);
int     reset_util(Util *);
int     reset_appl_ss(Appl_SS *);
int     reset_appl(Appl *);
int     reset_stmt(Stmt *);
int     reset_lock(Lock *);

int  compare_appl_cpu_asc(const void *, const void *);
int  compare_appl_cpu_desc(const void *, const void *);
int  compare_stmt_cpu_asc(const void *, const void *);
int  compare_stmt_cpu_desc(const void *, const void *);
int  compare_appl_tqr_asc(const void *, const void *);
int  compare_appl_tqr_desc(const void *, const void *);
int  compare_appl_tqw_asc(const void *, const void *);
int  compare_appl_tqw_desc(const void *, const void *);
int  compare_appl_rr_asc(const void *, const void *);
int  compare_appl_rr_desc(const void *, const void *);
int  compare_stmt_rr_asc(const void *, const void *);
int  compare_stmt_rr_desc(const void *, const void *);
int  compare_appl_rw_asc(const void *, const void *);
int  compare_appl_rw_desc(const void *, const void *);
int  compare_stmt_rw_asc(const void *, const void *);
int  compare_stmt_rw_desc(const void *, const void *);
int  compare_appl_nagents_asc(const void *, const void *);
int  compare_appl_nagents_desc(const void *, const void *);
int  compare_appl_memusg_asc(const void *, const void *);
int  compare_appl_memusg_desc(const void *, const void *);
int  compare_appl_bp_asc(const void *, const void *);
int  compare_appl_bp_desc(const void *, const void *);
int  compare_stmt_bp_asc(const void *, const void *);
int  compare_stmt_bp_desc(const void *, const void *);
int  compare_appl_logusg_asc(const void *, const void *);
int  compare_appl_logusg_desc(const void *, const void *);
int  compare_appl_rb_asc(const void *, const void *);
int  compare_appl_rb_desc(const void *, const void *);

int  compare_utils_id_node_asc(const void *, const void *);
int  compare_utils_id_node_desc(const void *, const void *);


int  compare_tbspace_rr_asc(const void *, const void *);
int  compare_tbspace_rr_desc(const void *, const void *);
int  compare_tbspace_rw_asc(const void *, const void *);
int  compare_tbspace_rw_desc(const void *, const void *);

int  compare_table_rr_asc(const void *, const void *);
int  compare_table_rr_desc(const void *, const void *);
int  compare_table_rw_asc(const void *, const void *);
int  compare_table_rw_desc(const void *, const void *);

int  compare_appl_handle(const void *, const void *);



char *get_util_desc(char *
, size_t
, char *);

Boolean add_appl(const SnapReq *
, Appl *);

Boolean add_util(const SnapReq *
, Util *);

Boolean add_table(const SnapReq *
, Table *);

char  *init_snapshot_buffer(int node
, sqluint32 *buffer_sz
, db2AddSnapshotRqstData *snapReq);

sqlm_header_info *print_monitor_stream(char *prefix
, char *pStart
, char *pEnd
, sqluint32 logicalGroup
, FILE *fp
, char *elementName[]
, char *elementType[]);

sqlm_header_info *parse_monitor_stream_DBASE(Header *header
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_APPLS(Header *header
, DB2List *list
, Appl *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_STMTS(Header *header
, DB2List *list
, Stmt *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *preparse_monitor_stream_AGENTID_SS  (Header  *header
, char *pStart                                                          
, char *pEnd                                                            
, sqluint32 logicalGroup) ;

sqlm_header_info *parse_monitor_stream_AGENTID_SS(Header *header
, DB2List *list
, Appl_SS *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_UTILS(Header *header
, DB2List *list
, Util *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_TBSPACE(Header *header
, DB2List *list
, Tbspace *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_TABLE(Header *header
, DB2List *list
, Table *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);

sqlm_header_info *parse_monitor_stream_LOCKS(Header *header
, DB2List *list
, Lock *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup);
/****************************************************************************************/
Lock *init_lock(void)
{
  int rc = 0;
  Lock *lock = NULL;
  lock = (Lock *) malloc (sizeof(Lock));
  if (lock == NULL)
    return NULL;

  rc = reset_lock(lock);
  if (rc > 0) {
    free(lock);
    return NULL;
  }
  lock->snap_req_type = locklist;
  lock->agent_id = 0;

  return lock;


}
/****************************************************************************************/
int reset_lock(Lock *lock)
{

  lock->ss_number = 0;
  lock->node = 1000;
  lock->agent_id_holding_lk = 0;

  lock->lock_obj_type = 0;
  lock->lock_mode = 999;
  lock->lock_mode_requested = 999;
  lock->lock_status = 0;
  lock->count = 0;
  lock->lock_escal = 0;
  strcpy(lock->tabschema,"");
  strcpy(lock->tabname,"");
  strcpy(lock->tbspace,"");
  lock->data_partition_id = 0;

  return 0;
}
/****************************************************************************************/
Stmt *init_stmt(void)
{

  int  rc = 0;
  Stmt * stmt = NULL;

  stmt = (Stmt * ) malloc(sizeof(Stmt));
  if (stmt == NULL)
    return NULL;

  stmt->snap_req_type = stmts;

  /* don't reset handle,exec_id,auth_id
     so we can keep track of the appl that we are parsing
  */

  strcpy(stmt->exec_id, "");
  strcpy(stmt->auth_id, "");
  stmt->appl_handle = 0;
  strcpy(stmt->sequence_no,"");

  rc = reset_stmt(stmt);
  if (rc != 0) {
    return NULL;
  }

  return stmt;
} /*init_stmt*/

/****************************************************************************************/
Appl *init_appl(void)
{

  int  rc = 0, i = 0;
  Appl * appl = NULL;

  appl = (Appl * ) malloc(sizeof(Appl));
  if (appl == NULL)
    return NULL;

  appl->snap_req_type = appls;
  /*
  appl->num_stmts = 0;
  for (i=0; i < MAX_STMTS; i++)
    appl->stmt_text_arr[i] = NULL;
  */

  db2list_init(&(appl->stmt_list), match_stmt, free, stmts);
  db2list_init(&(appl->lock_list), match_lock, free, locklist);

  rc = reset_appl(appl);
  if (rc != 0) {
    return NULL;
  }

  return appl;
} /*init_appl*/


/****************************************************************************************/
Appl_SS *init_appl_ss(void)
{

  int  rc = 0;
  Appl_SS * appl_ss = NULL;

  appl_ss = (Appl_SS * ) malloc(sizeof(Appl_SS));
  if (appl_ss == NULL)
    return NULL;

  rc = reset_appl_ss(appl_ss);
  if (rc != 0) {
    free(appl_ss);
    return NULL;
  }

  appl_ss->appl_handle = 0;
  return appl_ss;
} /*init_appl_ss*/




/*************************************************************************************/
void free_appl(void *v) {

  Appl *appl = v;
  /* destroy stmt list first */

  if (appl != NULL) {
    db2list_destroy(&(appl->stmt_list));
    db2list_destroy(&(appl->lock_list));
  }
  /* now free APPL struct */
  free(appl);

}
/*************************************************************************************/
Table *init_table(void)
{
  int rc = 0;
  Table *table = NULL;

  table = (Table * ) malloc(sizeof(Table));
  if (table == NULL)
    return NULL;

  table->snap_req_type = tbspace_id;
  rc = reset_table(table);
  if (rc != 0) {
    free(table);
    table = NULL;
  }

  return table;

}
/*************************************************************************************/
Tbspace *init_tbspace(void)
{
  int rc = 0;
  Tbspace * TBspace = NULL;

  TBspace = (Tbspace * ) malloc(sizeof(Tbspace));
  if (TBspace == NULL)
    return NULL;

  TBspace->snap_req_type = tbspace;
  rc = reset_tbspace(TBspace);
  if (rc != 0) {
    free(TBspace);
    TBspace = NULL;
  }

  return TBspace;
}
/*************************************************************************************/
int reset_table(Table *table)
{

  table->tbspace_id = 1000000000;
  strcpy(table->tabschema , "");
  strcpy(table->tabname, "");
  table->type =  999999;
  table->data_partition_id = 9999;
  table->avg_row_sz = -3;
  table->rr = 0;
  table->rr_delta = 0;
  table->rw = 0;
  table->rw_delta = 0;
  table->data_sz = 0;
  table->ix_sz = 0;

  return 0;

}
/*************************************************************************************/
int reset_tbspace(Tbspace *tbspace)
{
  tbspace->id = 0;
  tbspace->pg_sz = 0;
  tbspace->state = 0;
  tbspace->t1_total_pg = 0;
  tbspace->t2_total_pg = 0;
  tbspace->t1_fs_total_sz = 0;
  tbspace->t2_fs_total_sz = 0;
  tbspace->t1_used_pg = 0;
  tbspace->t2_used_pg = 0;
  tbspace->t1_fs_used_sz = 0;
  tbspace->t2_fs_used_sz = 0;
  tbspace->t1_free_pg = 0;
  tbspace->t2_free_pg = 0;
  tbspace->node = 0;
  tbspace->type = 0;
  tbspace->content_type = 0;
  strcpy(tbspace->name, "") ;

  tbspace->t1_bp_reads = 0;
  tbspace->t2_bp_reads = 0;
  tbspace->bp_reads_delta = 0;

  tbspace->t1_bp_writes = 0;
  tbspace->t2_bp_writes = 0;
  tbspace->bp_writes_delta = 0;

  tbspace->t1_direct_reads = 0;
  tbspace->t2_direct_reads = 0;
  tbspace->direct_reads_delta = 0;

  tbspace->t1_direct_writes = 0;
  tbspace->t2_direct_writes = 0;
  tbspace->direct_writes_delta = 0;

  tbspace->t1_direct_read_reqs = 0;
  tbspace->t2_direct_read_reqs = 0;
  tbspace->direct_read_reqs_delta = 0;

  tbspace->t1_direct_write_reqs = 0;
  tbspace->t2_direct_write_reqs = 0;
  tbspace->direct_write_reqs_delta = 0;

  tbspace->reads_delta = 0;
  tbspace->writes_delta = 0;

  return 0;
}
/*************************************************************************************/
int copy_tbspace(Tbspace *tbspace1, const Tbspace *tbspace2)
{
  tbspace1->id = tbspace2->id;
  tbspace1->pg_sz = tbspace2->pg_sz;
  tbspace1->state = tbspace2->state;
  tbspace1->t2_total_pg = tbspace2->t2_total_pg;
  tbspace1->t2_fs_total_sz = tbspace2->t2_fs_total_sz;
  tbspace1->t2_fs_used_sz = tbspace2->t2_fs_used_sz;
  tbspace1->t2_used_pg = tbspace2->t2_used_pg;
  tbspace1->t2_free_pg = tbspace2->t2_free_pg;
  tbspace1->node = tbspace2->node;
  tbspace1->type = tbspace2->type;
  tbspace1->content_type = tbspace2->content_type;
  strncpy(tbspace1->name, tbspace2->name, SQLUH_TABLESPACENAME_SZ);

  tbspace1->t2_bp_reads += tbspace2->t2_bp_reads;
  tbspace1->t2_bp_writes += tbspace2->t2_bp_writes;

  tbspace1->t2_direct_reads += tbspace2->t2_direct_reads;
  tbspace1->t2_direct_writes += tbspace2->t2_direct_writes;

  tbspace1->t2_direct_read_reqs += tbspace2->t2_direct_read_reqs;
  tbspace1->t2_direct_write_reqs += tbspace2->t2_direct_write_reqs;

  return 0;
}
/*************************************************************************************/
int copy_table(Table *table1, const Table *table2)
{

  char buffer[1024];
  if (strlen(table1->tabschema) == 0)
    strncpy(table1->tabschema, table2->tabschema, TABSCHEMA_SZ);

  if (strlen(table1->tabname) == 0)
    strncpy(table1->tabname, table2->tabname, TABNAME_SZ);

  table1->tbspace_id = table2->tbspace_id;
  table1->type = table2->type;
  table1->data_partition_id = table2->data_partition_id;
  table1->data_sz = table2->data_sz;
  table1->ix_sz = table2->ix_sz;

  table1->rr_delta = delta(table2->rr, table1->rr);
  table1->rw_delta = delta(table2->rw, table1->rw);
  table1->rr = table2->rr;
  table1->rw = table2->rw;


  return 0;

}
/*************************************************************************************/
void delta_tbspace(Tbspace *tbspace)
{
  tbspace->t1_total_pg = tbspace->t2_total_pg;
  tbspace->t2_total_pg = 0;
  tbspace->t1_fs_total_sz = tbspace->t2_fs_total_sz;
  tbspace->t2_fs_total_sz = 0;
  tbspace->t1_fs_used_sz = tbspace->t2_fs_used_sz;
  tbspace->t2_fs_used_sz = 0;
  tbspace->t1_used_pg = tbspace->t2_used_pg;
  tbspace->t2_used_pg = 0;
  tbspace->t1_free_pg = tbspace->t2_free_pg;
  tbspace->t2_free_pg = 0;

  tbspace->bp_reads_delta = delta(tbspace->t2_bp_reads, tbspace->t1_bp_reads);
  tbspace->t1_bp_reads = tbspace->t2_bp_reads;
  tbspace->t2_bp_reads = 0;

  tbspace->bp_writes_delta = delta(tbspace->t2_bp_writes, tbspace->t1_bp_writes);
  tbspace->t1_bp_writes = tbspace->t2_bp_writes;
  tbspace->t2_bp_writes = 0;

  tbspace->direct_reads_delta = delta(tbspace->t2_direct_reads, tbspace->t1_direct_reads);
  tbspace->t1_direct_reads = tbspace->t2_direct_reads;
  tbspace->t2_direct_reads = 0;

  tbspace->direct_writes_delta = delta(tbspace->t2_direct_writes, tbspace->t1_direct_writes);
  tbspace->t1_direct_writes = tbspace->t2_direct_writes;
  tbspace->t2_direct_writes = 0;

  tbspace->direct_read_reqs_delta = delta(tbspace->t2_direct_read_reqs, tbspace->t1_direct_read_reqs);
  tbspace->t1_direct_read_reqs = tbspace->t2_direct_read_reqs;
  tbspace->t2_direct_read_reqs = 0;

  tbspace->direct_write_reqs_delta = delta(tbspace->t2_direct_write_reqs, tbspace->t1_direct_write_reqs);
  tbspace->t1_direct_write_reqs = tbspace->t2_direct_write_reqs;
  tbspace->t2_direct_write_reqs = 0;

  tbspace->reads_delta = tbspace->pg_sz * tbspace->bp_reads_delta;
  tbspace->reads_delta += (512 * tbspace->direct_reads_delta);

  tbspace->writes_delta = tbspace->pg_sz * tbspace->bp_writes_delta;
  tbspace->writes_delta += (512 * tbspace->direct_writes_delta);

  return;
}

/*************************************************************************************/
Util *init_util(void)
{

  int  rc = 0;
  Util * util = NULL;

  util = (Util * ) malloc(sizeof(Util));
  if (util == NULL)
    return NULL;

  util->snap_req_type = utils;
  rc = reset_util(util);
  if (rc != 0) {
    free(util);
    return NULL;
  }

  return util;
} /*init_util*/


/************************************************************************************/
int  reset_util(Util *util)
{

  if (util == NULL)
    return - 1;

  util->id = 0;
  util->type = 0;
  util->state = 0;
  util->node = 1000;

  (util->start_time).seconds = 0;
  (util->start_time).microsec = 0;

  strcpy(util->dbname, "");
  strcpy(util->desc, "");
  util->progress_total = 0;
  util->progress_completed = 0;

  return 0;

} /*reset_util*/


/*********************************************************************************/
int  reset_appl_ss(Appl_SS *appl_ss)
{

  if (appl_ss == NULL)
    return - 1;

  appl_ss->ss_node_number = 1000;
  appl_ss->ss_number = 999;
  appl_ss->ss_status = 0;

  appl_ss->ss_exec_time = 0;

  (appl_ss->ss_ucpu_used).seconds = 0;
  (appl_ss->ss_ucpu_used).microsec = 0;
  (appl_ss->ss_scpu_used).seconds = 0;
  (appl_ss->ss_scpu_used).microsec = 0;

  appl_ss->ss_rows_read = 0;

  appl_ss->ss_rows_written = 0;

  appl_ss->tq_rows_read = 0;

  appl_ss->tq_rows_written = 0;

  appl_ss->tq_cur_spills = 0;
  appl_ss->tq_tot_spills = 0;
  appl_ss->tq_node_waiting_for = 1000;
  appl_ss->tq_wait_for_any = 0;
  appl_ss->tq_id_waiting_on = 1000;

  appl_ss->num_agents = 0;

  return 0;

} /*reset_appl_ss*/


/*******************************************************************************/
int  reset_stmt(Stmt *stmt)
{

  if (stmt == NULL)
    return - 1;

  stmt->section_number = 999999;
  stmt->stmt_node_number = 1000;

  (stmt->stmt_start).seconds = 0;
  (stmt->stmt_start).microsec = 0;

  stmt->stmt_op = 0;

  (stmt->stmt_ucpu_used).seconds = 0;
  (stmt->stmt_ucpu_used).microsec = 0;
  (stmt->stmt_scpu_used).seconds = 0;
  (stmt->stmt_scpu_used).microsec = 0;
  stmt->ucpu_used_delta = 0;
  stmt->scpu_used_delta = 0;
  stmt->nagents = 0;

  stmt->stmt_rows_read = 0;
  stmt->rows_read_delta = 0;

  stmt->stmt_rows_written = 0;
  stmt->rows_written_delta = 0;

  /* buffer pool data logical/physical read; write */
  stmt->stmt_bpldr = 0;
  stmt->bpldr_delta = 0;
  stmt->stmt_bppdr = 0;
  stmt->bppdr_delta = 0;
  /* buffer pool index logical/physical read; write */
  stmt->stmt_bplir = 0;
  stmt->bplir_delta = 0;
  stmt->stmt_bppir = 0;
  stmt->bppir_delta;
  /* buffer pool temporary data/index logical read */
  stmt->stmt_bpltdir = 0;
  stmt->bpltdir_delta = 0;

  stmt->stmt_rows_selected = 0;
  /* stmt->rows_selected_delta = 0; */

  stmt->stmt_rows_inserted = 0;
  stmt->rows_inserted_delta = 0;

  stmt->stmt_rows_updated = 0;
  stmt->rows_updated_delta = 0;

  stmt->stmt_rows_deleted = 0;
  stmt->rows_deleted_delta = 0;

  stmt->stmt_sorts = 0;
  stmt->stmt_total_sort_time = 0;
  stmt->stmt_sort_overflows = 0;

  stmt->query_cost = 0;
  stmt->query_card = 0;

  strcpy(stmt->stmt_text,"");

  return 0;

} /*reset_stmt*/

/*******************************************************************************/
int  reset_appl(Appl *appl)
{

  int i = 0;
  if (appl == NULL)
    return - 1;

  appl->appl_handle = 0 ;
  appl->coord_node_num = 1000;
  appl->coord_pid = 0;
  appl->client_pid = 0;
  strcpy(appl->prog_nm, "");
  strcpy(appl->client_nm, "");

  (appl->uow_start).seconds = 0;
  (appl->uow_start).microsec = 0;
  (appl->uow_stop).seconds = 0;
  (appl->uow_stop).microsec = 0;

  strcpy(appl->auth_id, "");
  strcpy(appl->exec_id, "");

  appl->appl_status = 0;
  appl->stmt_op = 0;

  (appl->elapsed_exec_time).seconds = 0;
  (appl->elapsed_exec_time).microsec = 0;
  (appl->appl_ucpu_used).seconds = 0;
  (appl->appl_ucpu_used).microsec = 0;
  (appl->appl_scpu_used).seconds = 0;
  (appl->appl_scpu_used).microsec = 0;
  (appl->stmt_cpu_used).seconds = 0;
  (appl->stmt_cpu_used).microsec = 0;
  (appl->ss_cpu_used).seconds = 0;
  (appl->ss_cpu_used).microsec = 0;
  appl->ucpu_used_delta = 0;
  appl->scpu_used_delta = 0;
  appl->nagents = 0;

  appl->appl_rows_read = 0;
  appl->stmt_rows_read = 0;
  appl->rows_read_delta = 0;

  appl->appl_rows_written = 0;
  appl->stmt_rows_written = 0;
  appl->rows_written_delta = 0;

  /* buffer pool data logical/physical read; write */
  appl->appl_bpldr = 0;
  appl->stmt_bpldr = 0;
  appl->bpldr_delta = 0;
  appl->appl_bppdr = 0;
  appl->stmt_bppdr = 0;
  appl->bppdr_delta = 0;
  appl->bpdw = 0;
  appl->bpdw_delta = 0;
  /* buffer pool index logical/physical read; write */
  appl->appl_bplir = 0;
  appl->stmt_bplir = 0;
  appl->bplir_delta = 0;
  appl->appl_bppir = 0;
  appl->stmt_bppir = 0;
  appl->bppir_delta;
  appl->bpiw = 0;
  appl->bpiw_delta = 0;
  /* buffer pool temporary data/index logical read */
  appl->appl_bpltdir = 0;
  appl->stmt_bpltdir = 0;
  appl->bpltdir_delta = 0;
  /* buffer pool read/write time */
  appl->bpr_tm = 0;
  appl->bpr_tm_delta = 0;
  appl->bpw_tm = 0;
  appl->bpw_tm_delta = 0;

  /* non-bufferpool io */
  appl->dio_reads = 0;
  appl->dio_reads_delta = 0;
  appl->dio_read_reqs = 0;
  appl->dio_read_reqs_delta = 0;

  appl->dio_writes = 0;
  appl->dio_writes_delta = 0;
  appl->dio_write_reqs = 0;
  appl->dio_write_reqs_delta = 0;

  appl->dio_read_tm = 0;
  appl->dio_read_tm_delta = 0;
  appl->dio_write_tm = 0;
  appl->dio_write_tm_delta = 0;

  appl->tq_rows_read = 0;
  appl->tq_rows_read_delta = 0;

  appl->tq_rows_written = 0;
  appl->tq_rows_written_delta = 0;

  appl->appl_rows_selected = 0;
  appl->stmt_rows_selected = 0;
  appl->rows_selected_delta = 0;

  appl->appl_rows_inserted = 0;
  appl->stmt_rows_inserted = 0;
  appl->rows_inserted_delta = 0;

  appl->appl_rows_updated = 0;
  appl->stmt_rows_updated = 0;
  appl->rows_updated_delta = 0;

  appl->appl_rows_deleted = 0;
  appl->stmt_rows_deleted = 0;
  appl->rows_deleted_delta = 0;

  appl->locks_held = 0;
  appl->uow_log_space_used = 0;

  appl->rollback_progress_total = 0;
  appl->rollback_progress_completed = 0;

  appl->privagent_memusg = 0;
  appl->query_cost = 0;
  appl->query_card = 0;

  appl->num_stmts = 0;

  /*  cleanup the stmt list and create new list while parsing stmt stream */
  db2list_destroy(&(appl->stmt_list));
  db2list_init(&(appl->stmt_list), match_stmt, free, stmts);
  /*  cleanup the lock list and create new list while parsing stmt stream */
  db2list_destroy(&(appl->lock_list));
  db2list_init(&(appl->lock_list), match_lock, free, locklist);

  return 0;

} /*reset_appl*/

/********************************************************************************/
Boolean add_appl(const SnapReq *snapReq, Appl *appl)
{

  Boolean add =  FALSE;

  if (strlen(snapReq->exec_id) == 0                                     
      && strlen(snapReq->auth_id) == 0                                    
      && strlen(snapReq->prog_nm) == 0                                    
      && strlen(snapReq->client_nm) == 0)
    return TRUE;

  if (strlen(snapReq->exec_id) > 0 
      && strncasecmp(snapReq->exec_id, appl->exec_id, USERID_SZ) == 0)
    add = TRUE;
  else if (strlen(snapReq->exec_id) > 0)
    return FALSE;

  if (strlen(snapReq->auth_id) > 0 
      && strncasecmp(snapReq->auth_id, appl->auth_id, USERID_SZ) == 0)
    add = TRUE;
  else if (strlen(snapReq->auth_id) > 0)
    return FALSE;

  if (strlen(snapReq->prog_nm) > 0 
      && strncasecmp(snapReq->prog_nm, appl->prog_nm, USERID_SZ) == 0)
    add = TRUE;
  else if (strlen(snapReq->prog_nm) > 0)
    return FALSE;

  if (strlen(snapReq->client_nm) > 0 
      && strncasecmp(snapReq->client_nm, appl->client_nm, USERID_SZ) == 0)
    add = TRUE;
  else if (strlen(snapReq->client_nm) > 0)
    return FALSE;

  return add;
} /* add_appl */


/********************************************************************************/
Boolean add_util(const SnapReq *snapReq, Util *util)
{

  Boolean add =  FALSE;

  if (snapReq->util_id == 0                                     
      && strlen(snapReq->dbname) == 0 )
    return TRUE;

  if (snapReq->util_id > 0
      && util->id == snapReq->util_id)
    add = TRUE;
  else if (snapReq->util_id > 0)
    return FALSE;

  if (strlen(snapReq->dbname) > 0 
      && strncasecmp(snapReq->dbname, util->dbname, SQL_DBNAME_SZ) == 0)
    add = TRUE;
  else if (strlen(snapReq->dbname) > 0)
    return FALSE;

  return add;
} /* add_util */

/********************************************************************************/
Boolean add_table(const SnapReq *snapReq, Table *table)
{
  if (table->tbspace_id == snapReq->tbspace_id)
    return TRUE;

  return FALSE;

}
/********************************************************************************/
int  copy_util(Util *util1, const Util *util2)
{

  char  *pSpace;

  if (util1 == NULL || util2 == NULL)
    return - 1;

  util1->id = util2->id;
  util1->type = util2->type;
  util1->state = util2->state;
  util1->node = util2->node;

  if ( (util2->start_time).seconds > (util1->start_time).seconds)
    util1->start_time = util2->start_time;


  strncpy(util1->dbname, util2->dbname, SQL_DBNAME_SZ);

  if (strlen(util2->desc) > 0 && strlen(util1->desc) == 0)
    strncpy(util1->desc, util2->desc, UTIL_DESC_SZ);

  util1->progress_total += util2->progress_total;
  util1->progress_completed += util2->progress_completed;

  return 0;
} /*copy_util*/


/********************************************************************************/
int  copy_appl_ss(Appl_SS *appl_ss1, const Appl_SS *appl_ss2)
{

  if (appl_ss1 == NULL || appl_ss2 == NULL)
    return - 1;

  appl_ss1->appl_handle = appl_ss2->appl_handle;
  appl_ss1->ss_number = appl_ss2->ss_number;
  appl_ss1->ss_status = appl_ss2->ss_status;

  appl_ss1->ss_node_number = appl_ss2->ss_node_number;

  appl_ss1->ss_exec_time = appl_ss2->ss_exec_time;

  appl_ss1->ss_ucpu_used_delta =  delta( timediff(appl_ss2->ss_ucpu_used, appl_ss1->ss_ucpu_used)
      , 0);
  appl_ss1->ss_scpu_used_delta =  delta( timediff(appl_ss2->ss_scpu_used, appl_ss1->ss_scpu_used)
      , 0);
  appl_ss1->ss_ucpu_used = appl_ss2->ss_ucpu_used;
  appl_ss1->ss_scpu_used = appl_ss2->ss_scpu_used;


  appl_ss1->ss_rows_read_delta = delta(appl_ss2->ss_rows_read, appl_ss1->ss_rows_read);
  appl_ss1->ss_rows_read = appl_ss2->ss_rows_read;

  appl_ss1->ss_rows_written_delta = delta(appl_ss2->ss_rows_written, appl_ss1->ss_rows_written);
  appl_ss1->ss_rows_written = appl_ss2->ss_rows_written;

  appl_ss1->tq_rows_read_delta = delta(appl_ss2->tq_rows_read, appl_ss1->tq_rows_read);
  appl_ss1->tq_rows_read = appl_ss2->tq_rows_read;

  appl_ss1->tq_rows_written_delta = delta(appl_ss2->tq_rows_written, appl_ss1->tq_rows_written);
  appl_ss1->tq_rows_written = appl_ss2->tq_rows_written;


  appl_ss1->tq_cur_spills = appl_ss2->tq_cur_spills;
  appl_ss1->tq_tot_spills = appl_ss2->tq_tot_spills;

  appl_ss1->tq_node_waiting_for = appl_ss2->tq_node_waiting_for;
  appl_ss1->tq_wait_for_any = appl_ss2->tq_wait_for_any;
  appl_ss1->tq_id_waiting_on = appl_ss2->tq_id_waiting_on;

  appl_ss1->num_agents = appl_ss2->num_agents;

  return 0;

} /*copy_appl_ss*/


/********************************************************************************/
int copy_lock(Lock *lock1, const Lock *lock2)
{
  if (lock1 == NULL || lock2 == NULL)
    return -1;

  lock1->agent_id = lock2->agent_id;
  lock1->ss_number = lock2->ss_number;
  lock1->node = lock2->node;
  lock1->agent_id_holding_lk = lock2->agent_id_holding_lk;

  lock1->lock_obj_type = lock2->lock_obj_type;
  lock1->lock_mode = lock2->lock_mode;
  lock1->lock_mode_requested = lock2->lock_mode_requested;
  lock1->lock_status = lock2->lock_status;
  lock1->lock_escal = lock2->lock_escal;
  if (lock2->count > 0)
    lock1->count= lock2->count;
  else 
    lock1->count+=1;


  strncpy(lock1->tabschema, lock2->tabschema, TABSCHEMA_SZ);
  strncpy(lock1->tabname, lock2->tabname, TABSCHEMA_SZ);
  strncpy(lock1->tbspace, lock2->tbspace, SQLUH_TABLESPACENAME_SZ);
  lock1->data_partition_id = lock2->data_partition_id;

  return 0;

}
/********************************************************************************/
int  copy_stmt(Stmt *stmt1, const Stmt *stmt2) 
{
  if (stmt1 == NULL || stmt2 == NULL)
    return - 1;

  stmt1->appl_handle = stmt2->appl_handle;
  strncpy(stmt1->auth_id, stmt2->auth_id, USERID_SZ);
  strncpy(stmt1->exec_id, stmt2->exec_id, USERID_SZ);
  strncpy(stmt1->sequence_no, stmt2->sequence_no, SQLM_SEQ_SZ);

  stmt1->section_number = stmt2->section_number;
  stmt1->stmt_node_number = stmt2->stmt_node_number;

  stmt1->stmt_start = stmt2->stmt_start;

  stmt1->stmt_op = stmt2->stmt_op;

  /* cpu used ***********************************************/
  /* time difference is in microsec */
  stmt1->nagents = stmt2->nagents;
  stmt1->ucpu_used_delta                                        
      = delta( timediff(stmt2->stmt_ucpu_used, stmt1->stmt_ucpu_used), 0);
  stmt1->scpu_used_delta                                        
      = delta( timediff(stmt2->stmt_scpu_used, stmt1->stmt_scpu_used), 0);

  stmt1->stmt_ucpu_used = stmt2->stmt_ucpu_used;
  stmt1->stmt_scpu_used = stmt2->stmt_scpu_used;

  /* rows read ****************************************************/
  stmt1->rows_read_delta  
      = delta(stmt2->stmt_rows_read, stmt1->stmt_rows_read);
  stmt1->stmt_rows_read = stmt2->stmt_rows_read;

  /* rows written *************************************************/
  stmt1->rows_written_delta 
      = delta(stmt2->stmt_rows_written, stmt1->stmt_rows_written);
  stmt1->stmt_rows_written = stmt2->stmt_rows_written;

  /* BP activity ******************************************************/
  /* logical */
  stmt1->bpldr_delta 
      = delta(stmt2->stmt_bpldr, stmt1->stmt_bpldr);
  stmt1->bplir_delta 
      = delta(stmt2->stmt_bplir, stmt1->stmt_bplir);
  stmt1->bpltdir_delta
      = delta(stmt2->stmt_bpltdir, stmt1->stmt_bpltdir);

  /* physical */
  stmt1->bppdr_delta 
      = delta(stmt2->stmt_bppdr, stmt1->stmt_bppdr);
  stmt1->bppir_delta 
      = delta(stmt2->stmt_bppir, stmt1->stmt_bppir);

  stmt1->stmt_bpldr = stmt2->stmt_bpldr;
  stmt1->stmt_bplir = stmt2->stmt_bplir;
  stmt1->stmt_bpltdir = stmt2->stmt_bpltdir;

  stmt1->stmt_bppdr = stmt2->stmt_bppdr;
  stmt1->stmt_bppir = stmt2->stmt_bppir;

  /* rows selected ****************************************************/
  stmt1->rows_selected_delta 
      = delta (stmt2->stmt_rows_selected, stmt1->stmt_rows_selected);
  stmt1->stmt_rows_selected = stmt2->stmt_rows_selected;

  /* rows inserted, deleted, updated **********************************/
  stmt1->rows_inserted_delta
      = delta (stmt2->stmt_rows_inserted, stmt1->stmt_rows_inserted);
  stmt1->stmt_rows_inserted = stmt2->stmt_rows_inserted;

  stmt1->rows_deleted_delta 
      = delta(stmt2->stmt_rows_deleted, stmt1->stmt_rows_deleted);
  stmt1->stmt_rows_deleted = stmt2->stmt_rows_deleted;

  stmt1->rows_updated_delta 
      = delta(stmt2->stmt_rows_updated, stmt1->stmt_rows_updated);
  stmt1->stmt_rows_updated = stmt2->stmt_rows_updated;

  stmt1->stmt_sorts = stmt2->stmt_sorts;
  stmt1->stmt_total_sort_time = stmt2->stmt_total_sort_time;
  stmt1->stmt_sort_overflows = stmt2->stmt_sort_overflows;
  stmt1->query_cost = stmt2->query_cost;
  stmt1->query_card = stmt2->query_card;

  strncpy(stmt1->stmt_text, stmt2->stmt_text,STMT_SZ);

  return 0;
}
/********************************************************************************/
int  copy_appl(Appl *appl1, const Appl *appl2)
{

  DB2ListElmt *element = NULL, *new_element = NULL;
  int i = 0;
  if (appl1 == NULL || appl2 == NULL)
    return - 1;

  appl1->appl_handle = appl2->appl_handle;
  appl1->coord_node_num = appl2->coord_node_num;
  appl1->coord_pid = appl2->coord_pid;
  appl1->client_pid = appl2->client_pid;
  strncpy(appl1->prog_nm, appl2->prog_nm, USERID_SZ);
  strncpy(appl1->client_nm, appl2->client_nm, USERID_SZ);

  appl1->uow_start = appl2->uow_start;
  appl1->uow_stop = appl2->uow_stop;

  strncpy(appl1->auth_id, appl2->auth_id, USERID_SZ);
  strncpy(appl1->exec_id, appl2->exec_id, USERID_SZ);

  appl1->appl_status = appl2->appl_status;
  appl1->stmt_op = appl2->stmt_op;

  /* cpu used ***********************************************/
  /* time difference is in microsec */
  appl1->nagents = appl2->nagents;
  appl1->elapsed_exec_time_delta
      = delta( timediff(appl2->elapsed_exec_time, appl1->elapsed_exec_time), 0);

  appl1->ucpu_used_delta                                        
      = delta( timediff(appl2->appl_ucpu_used, appl1->appl_ucpu_used), 0);
  appl1->scpu_used_delta                                        
      = delta( timediff(appl2->appl_scpu_used, appl1->appl_scpu_used), 0);

  appl1->appl_ucpu_used = appl2->appl_ucpu_used;
  appl1->appl_scpu_used = appl2->appl_scpu_used;

  appl1->stmt_cpu_used = appl2->stmt_cpu_used;
  appl1->ss_cpu_used = appl2->ss_cpu_used;
  appl1->elapsed_exec_time = appl2->elapsed_exec_time;

  /* rows read ****************************************************/
  appl1->rows_read_delta  
      = delta(appl2->appl_rows_read, appl1->appl_rows_read);
  appl1->appl_rows_read = appl2->appl_rows_read;
  appl1->stmt_rows_read = appl2->stmt_rows_read;

  /* rows written *************************************************/
  appl1->rows_written_delta 
      = delta(appl2->appl_rows_written, appl1->appl_rows_written);
  appl1->appl_rows_written = appl2->appl_rows_written;
  appl1->stmt_rows_written = appl2->stmt_rows_written;

  /* BP activity ******************************************************/
  /* logical */
  appl1->bpldr_delta 
      = delta(appl2->appl_bpldr, appl1->appl_bpldr);
  appl1->bplir_delta 
      = delta(appl2->appl_bplir, appl1->appl_bplir);
  appl1->bpltdir_delta
      = delta(appl2->appl_bpltdir, appl1->appl_bpltdir);

  /* physical */
  appl1->bppdr_delta 
      = delta(appl2->appl_bppdr, appl1->appl_bppdr);
  appl1->bppir_delta 
      = delta(appl2->appl_bppir, appl1->appl_bppir);
  appl1->bpdw_delta
      = delta(appl2->bpdw, appl1->bpdw);
  appl1->bpiw_delta
      = delta(appl2->bpiw, appl1->bpiw);

  /*time */
  appl1->bpr_tm_delta
      = delta(appl2->bpr_tm , appl1->bpr_tm);
  appl1->bpw_tm_delta
      = delta(appl2->bpw_tm, appl1->bpw_tm);


  appl1->appl_bpldr = appl2->appl_bpldr;
  appl1->stmt_bpldr = appl2->stmt_bpldr;
  appl1->appl_bplir = appl2->appl_bplir;
  appl1->stmt_bplir = appl2->stmt_bplir;
  appl1->appl_bpltdir = appl2->appl_bpltdir;
  appl1->stmt_bpltdir = appl2->stmt_bpltdir;

  appl1->appl_bppdr = appl2->appl_bppdr;
  appl1->stmt_bppdr = appl2->stmt_bppdr;
  appl1->appl_bppir = appl2->appl_bppir;
  appl1->stmt_bppir = appl2->stmt_bppir;

  appl1->bpdw = appl2->bpdw;
  appl1->bpiw = appl2->bpiw;

  appl1->bpr_tm = appl2->bpr_tm;
  appl1->bpw_tm = appl2->bpw_tm;

  /* non-BP activity **********************************************/
  appl1->dio_reads_delta
      = delta(appl2->dio_reads, appl1->dio_reads);
  appl1->dio_read_reqs_delta
      = delta(appl2->dio_read_reqs, appl1->dio_read_reqs);

  appl1->dio_writes_delta
      = delta(appl2->dio_writes, appl1->dio_writes);
  appl1->dio_write_reqs_delta
      = delta(appl2->dio_write_reqs, appl1->dio_write_reqs);

  appl1->dio_read_tm_delta
      = delta(appl2->dio_read_tm, appl1->dio_read_tm);
  appl1->dio_write_tm_delta
      = delta(appl2->dio_write_tm, appl1->dio_write_tm);

  appl1->dio_reads = appl2->dio_reads;
  appl1->dio_read_reqs = appl2->dio_read_reqs;

  appl1->dio_writes = appl2->dio_writes;
  appl1->dio_write_reqs = appl2->dio_write_reqs;

  appl1->dio_read_tm = appl2->dio_read_tm;
  appl1->dio_write_tm = appl2->dio_write_tm;

  /* tq read ******************************************************/
  appl1->tq_rows_read_delta 
      = delta(appl2->tq_rows_read, appl1->tq_rows_read);
  appl1->tq_rows_read = appl2->tq_rows_read;

  /* tq written *****************************************************/
  appl1->tq_rows_written_delta 
      = delta(appl2->tq_rows_written, appl1->tq_rows_written);
  appl1->tq_rows_written = appl2->tq_rows_written;

  /* rows selected ****************************************************/
  appl1->rows_selected_delta 
      = delta (appl2->appl_rows_selected, appl1->appl_rows_selected);
  appl1->appl_rows_selected = appl2->appl_rows_selected;
  appl1->stmt_rows_selected = appl2->stmt_rows_selected;

  /* rows inserted, deleted, updated **********************************/
  appl1->rows_inserted_delta
      = delta (appl2->appl_rows_inserted, appl1->appl_rows_inserted);
  appl1->appl_rows_inserted = appl2->appl_rows_inserted;
  appl1->stmt_rows_inserted = appl2->stmt_rows_inserted;

  appl1->rows_deleted_delta 
      = delta(appl2->appl_rows_deleted, appl1->appl_rows_deleted);
  appl1->appl_rows_deleted = appl2->appl_rows_deleted;
  appl1->stmt_rows_deleted = appl2->stmt_rows_deleted;

  appl1->rows_updated_delta 
      = delta(appl2->appl_rows_updated, appl1->appl_rows_updated);
  appl1->appl_rows_updated = appl2->appl_rows_updated;
  appl1->stmt_rows_updated = appl2->stmt_rows_updated;

  appl1->locks_held = appl2->locks_held;
  appl1->uow_log_space_used = appl2->uow_log_space_used;
  appl1->rollback_progress_total = appl2->rollback_progress_total;
  appl1->rollback_progress_completed = appl2->rollback_progress_completed;

  appl1->privagent_memusg = appl2->privagent_memusg;

  appl1->query_cost = appl2->query_cost;
  appl1->query_card = appl2->query_card;


  appl1->num_stmts = appl2->num_stmts;

  if (db2list_size(&(appl2->stmt_list)) > 0) {                                    
    /* copy the stmt list */                                                      
    for(element = db2list_head(&(appl2->stmt_list)); element != NULL;             
        element = db2list_next(element)) {                                        
      if (db2list_data(element) != NULL) {                                        
        new_element = db2list_lookup(&(appl1->stmt_list), db2list_data(element)); 
        if (new_element != NULL)                                                  
          copy_stmt(db2list_data(new_element), db2list_data(element));            
        else {                                                                    
          Stmt* newStmtData = init_stmt();                                        
          copy_stmt(newStmtData, db2list_data(element));                          
          db2list_ins_next(&(appl1->stmt_list), db2list_tail(&(appl1->stmt_list)),
              newStmtData);                                                       
        }                                                                         
      }                                                                           
    }                                                                             
  } /* end of copying stmts */                                                    
  /* clean up stmts that finished */                                    
  db2list_cleanup(&(appl1->stmt_list));                                           

  if (db2list_size(&(appl2->lock_list)) > 0) {
    /* copy the lock list */
    for(element = db2list_head(&(appl2->lock_list)); element != NULL; 
        element = db2list_next(element)) {
      if (db2list_data(element) != NULL) {
        new_element = db2list_lookup(&(appl1->lock_list), db2list_data(element));
        if (new_element != NULL) {
          copy_lock(db2list_data(new_element), db2list_data(element));
        } else {
          Lock* newLockData = init_lock();
          copy_lock(newLockData, db2list_data(element));
          db2list_ins_next(&(appl1->lock_list), db2list_tail(&(appl1->lock_list)),
              newLockData);
        }
      }
    }
  } /* end of copying locks */
  /* clean up locks that have been released */
  db2list_cleanup(&(appl1->lock_list));


  return 0;
} /* copy_appl */



/********************************************************************************/
/* populate db2AddSnapshotRqstData  based on the information in Header          */
/* allocate buffer based on db2AddSnapshotRqstData                              */
/*  by calling init_snapshot_buffer                                             */
/********************************************************************************/
int  init_snapRequest(
db2AddSnapshotRqstData *snapReq
, char **buffer_ptr
, sqluint32 *buffer_sz 
, SnapReqType snap_req_type
, const int node
, const char *dbname
, const sqluint32 agentid
, DB2List *list
)
{

  int  rc = 0;  /* return code */
  char mesg[1024];

  struct sqlca sqlca;
  /************************************************************************/
  /* free memory and then initialize */
  /************************************************************************/
  free_snapshot_memory(snapReq->pioRequestData, *buffer_ptr);
  memset(snapReq, 0, sizeof(db2AddSnapshotRqstData));
  memset(&sqlca  , 0, sizeof(sqlca));
  snapReq->pioRequestData = NULL;

  db2list_destroy(list);

  /************************************************************************/
  /* initialize the list */
  /************************************************************************/
  if (snap_req_type == appls || snap_req_type == agent_id) {
    db2list_init(list, match_appl, free_appl, snap_req_type);
  } else if (snap_req_type == stmts) {
    db2list_init(list, match_stmt, free, snap_req_type);
  } else if (snap_req_type == agent_id_detl) {
    db2list_init(list, match_appl_ss, free, snap_req_type);
  } else if (snap_req_type == agent_id_cmdline) {
    /* no need for a list b/c the snapshot
       data will be printed to stdout directly
    */
  } else if (snap_req_type == utils) {
    db2list_init(list, match_util, free, snap_req_type);
  } else if(snap_req_type == utils_id) {
    db2list_init(list, match_util_detl, free, snap_req_type);
  } else if (snap_req_type == tbspace) {
    db2list_init(list, match_tbspace, free, snap_req_type);
  } else if (snap_req_type == tbspace_id) {
    db2list_init(list, match_table, free, snap_req_type);
  } else {
    /* snapshot request doesn't make sense */
  }

  /************************************************************************/
  /* Request dbase snapshot unless */
  /************************************************************************/
  if (snap_req_type != agent_id_cmdline ) {
    if (strlen(dbname) > 0) {
      /* get snapshot for one dbase        */
      snapReq->iRequestType  = SQLMA_DBASE;
      snapReq->iQualType     = SQLM_INSTREAM_ELM_DBNAME;
      snapReq->piQualData    = (void *) dbname;
      rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
      sqlmCheckRC(rc);
    } else {
      /* get snapshot for all dbase        */
      snapReq->iRequestType  = SQLMA_DBASE_ALL;
      rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
      sqlmCheckRC(rc);
    }
  } /* end of dbase snapshot request */

  /************************************************************************/
  /* there was a problem requesting dbase snapshot */
  /************************************************************************/
  if (rc != 0)  {
    return rc;
  }

  /************************************************************************/
  /* Request instance(utility)|agent_id|application snapshot */
  /************************************************************************/
  if ( snap_req_type == agent_id && agentid > 0) {
    char  pAgentid [1024];
    sprintf(pAgentid, "%u", agentid);
    snapReq->iQualType = SQLM_INSTREAM_ELM_AGENTID;
    snapReq->piQualData = (void * ) pAgentid;
    snapReq->iRequestType = SQLMA_AGENT_ID;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
    /* get lock snapshot */
    snapReq->iQualType = SQLM_INSTREAM_ELM_AGENTID;
    snapReq->piQualData = (void * ) pAgentid;
    snapReq->iRequestType = SQLMA_APPL_LOCKS_AGENT_ID;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if (( snap_req_type == agent_id_cmdline 
      || snap_req_type == agent_id_detl
      || snap_req_type == stmts )
      && agentid > 0) {
    char  pAgentid [1024];
    sprintf(pAgentid, "%u", agentid);
    snapReq->iQualType = SQLM_INSTREAM_ELM_AGENTID;
    snapReq->piQualData = (void * ) pAgentid;
    snapReq->iRequestType = SQLMA_AGENT_ID;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if ((snap_req_type == appls || snap_req_type == stmts) && strlen(dbname) > 0) {
    /* get snapshot for all applications on dbname */
    snapReq->iQualType     = SQLM_INSTREAM_ELM_DBNAME;
    snapReq->piQualData = (void * ) dbname;
    snapReq->iRequestType  = SQLMA_DBASE_APPLS;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);

    /* get snapshot for all remote applications on dbname */
    snapReq->iQualType     = SQLM_INSTREAM_ELM_DBNAME;
    snapReq->piQualData = (void * ) dbname;
    snapReq->iRequestType  = SQLMA_DBASE_APPLS_REMOTE;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if (snap_req_type == appls || snap_req_type == stmts) {
    /* get snapshot for all applications */
    snapReq->iRequestType  = SQLMA_APPL_ALL;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);

    /* get snapshot for all remote applications */
    snapReq->iRequestType  = SQLMA_APPL_REMOTE_ALL;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if (snap_req_type == utils || snap_req_type == utils_id) {
    /* get snapshot for instance */
    snapReq->iRequestType = SQLMA_DB2;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if (snap_req_type == tbspace) {
    /* get snapshot for tablespaces */
    snapReq->iQualType     = SQLM_INSTREAM_ELM_DBNAME;
    snapReq->piQualData = (void * ) dbname;
    snapReq->iRequestType = SQLMA_DBASE_TABLESPACES;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else if (snap_req_type == tbspace_id) {
    /* get snapshot for tables */
    snapReq->iQualType     = SQLM_INSTREAM_ELM_DBNAME;
    snapReq->piQualData = (void * ) dbname;
    snapReq->iRequestType = SQLMA_DBASE_TABLES;
    rc = db2AddSnapshotRequest(db2Version970, snapReq, &sqlca);
    sqlmCheckRC(rc);
  } else {
    /* snapshot request doesn't make sense */
    rc = - 1;
  }

  /************************************************************************/
  /* Problem requesting instance|agent_id|application|tbspace snapshot    */
  /************************************************************************/
  if (rc != 0)  {
    return rc;
  }

  /************************************************************************/
  /* allocate buffer to hold monitor stream */
  /************************************************************************/
  *buffer_ptr = init_snapshot_buffer(node, buffer_sz, snapReq);
   if (*buffer_ptr == NULL || *buffer_sz <= 0L) {
     return - 1;
   }

  /************************************************************************/
  /* every thing is OK :-) */
  /************************************************************************/
  return rc;
} /* end of init_snapRequest */


/******************************************************************************************/
/* allocate buffer to hold snapshot data                                                  */
/*  -return pointer to the buffer                                                          */
/******************************************************************************************/
char  *init_snapshot_buffer(int node
, sqluint32 *buffer_sz
, db2AddSnapshotRqstData *snapReq)
{

  int  rc = 0;
  struct sqlca sqlca;
  char  *buffer_ptr = '\0';  /* buffer returned from db2GetSnapshot call */

  char  mesg[1024];
  db2GetSnapshotSizeData getSnapshotSizeParam;

  memset(&sqlca  , 0, sizeof(sqlca));

  /* call the db2GetSnapshotDataSize API to determine the size of the */
  /* buffer required to store the snapshot monitor data */
  /*    first, set the values of the db2GetSnapshotDataSize structure */
  getSnapshotSizeParam.piSqlmaData    = snapReq->pioRequestData;
  getSnapshotSizeParam.poBufferSize   = buffer_sz;
  getSnapshotSizeParam.iVersion       = SQLM_CURRENT_VERSION;
  if (node == -2)
    getSnapshotSizeParam.iNodeNumber    = SQLM_ALL_NODES;
  else if (node >= 0)
    getSnapshotSizeParam.iNodeNumber    = node ;
  else
    getSnapshotSizeParam.iNodeNumber    = SQLM_CURRENT_NODE;

  getSnapshotSizeParam.iSnapshotClass = SQLM_CLASS_DEFAULT;

  /*    second, call the db2GetSnapshotDataSize API */
  rc = db2GetSnapshotSize(db2Version970, &getSnapshotSizeParam, &sqlca);
  /* exit function if the db2GetSnapshotSize call returns a non-zero value */
  if (rc != 0) {
    sprintf(mesg, "Return code from db2GetSnapshotSize is %d. Exiting.", rc);
    WARN_MSG(mesg);
    free_snapshot_memory(NULL, buffer_ptr);
    return NULL;
  }

  /* examine the sqlcode and react accordingly: */
  /*   if 0, then continue */
  /*   if positive, then print sqlca info and continue */
  /*   if negative, then print sqlca info, clear memory and exit function */
  if (sqlca.sqlcode != 0L) {
    SqlInfoPrint("db2GetSnapshotSize", &sqlca, __LINE__, __FILE__);
    if (sqlca.sqlcode < 0L) {
      sprintf(mesg, "db2GetSnapshotSize SQLCODE is %d. Exiting.", sqlca.sqlcode);
      WARN_MSG(mesg);
      free_snapshot_memory(NULL, buffer_ptr);
      return NULL;
    }
  }

  if (*buffer_sz == 0) {
    sprintf(mesg, "Estimated buffer size is zero. Exiting.");
    WARN_MSG(mesg);
    free_snapshot_memory( NULL, buffer_ptr);
    return NULL;
  }

  /* allocate memory to a buffer to hold snapshot monitor data. */
  buffer_ptr = (char *) malloc(*buffer_sz);
  if (buffer_ptr == NULL) {
    sprintf(mesg, "Error allocating memory for buffer area. Exiting.");
    WARN_MSG(mesg);
    free_snapshot_memory( NULL, buffer_ptr);
    return NULL;
  }
  /* clear the buffer */
  memset(buffer_ptr, '\0', *buffer_sz);

  return buffer_ptr;

} /* end of init_snapshot_buffer */


/***************************************************************************/
/* take snapshot  db2 snapshot                                             */
/* a worker pthread is used                                                */
/***************************************************************************/
void *  take_db2_snapshot  (void * args)
{
  int   rc = 0, try = 0;  /* return code */
  TakeDb2SnapshotArgs * pArgs = (TakeDb2SnapshotArgs *) args;
  int  node =  SQLM_CURRENT_NODE;
  struct sqlm_collected collected;  /* returned sqlm_collected structure */
  struct sqlca sqlca;
  sqluint32 outputFormat;
  db2GetSnapshotData getSnapshotParam;
  char  mesg[1024];
  time_t start_time;


  if ((pArgs->buffer_sz) <= 0 || pArgs->buffer_ptr == NULL) {
    sprintf(mesg, "Wrong buffer size %d or buffer_ptr is NULL", (pArgs->buffer_sz) );
    WARN_MSG(mesg);
    pthread_exit((void *) -1);
  }
  memset(&collected, '\0', sizeof(struct sqlm_collected ));
  /* **********************************************************************/
  /* clear the buffer                                                     */
  /* **********************************************************************/
  memset((pArgs->buffer_ptr), '\0', (pArgs->buffer_sz) );


  /* start_time = time(NULL); */
  /* call the db2GetSnapshot API to capture a snapshot and store the */
  /* monitor data in the buffer pointed to by "buffer_ptr". */
  /*    first, set the values of the db2GetSnapshot structure */
  getSnapshotParam.piSqlmaData = (pArgs->db2SnapReq).pioRequestData;
  getSnapshotParam.poCollectedData = &collected;
  getSnapshotParam.iBufferSize = (pArgs->buffer_sz);
  getSnapshotParam.poBuffer = (pArgs->buffer_ptr);
  getSnapshotParam.iVersion = SQLM_CURRENT_VERSION;
  getSnapshotParam.iStoreResult = 0;
  node = (int) (peek_snapReq(pArgs->header))->node;
  if (node == -2)
    getSnapshotParam.iNodeNumber    = SQLM_ALL_NODES;
  else if (node >= 0)
    getSnapshotParam.iNodeNumber    = node ;
  else
    getSnapshotParam.iNodeNumber    = SQLM_CURRENT_NODE;

  getSnapshotParam.poOutputFormat = &outputFormat;
  getSnapshotParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
  /*    second, call the db2GetSnapshot API */
  rc = db2GetSnapshot(db2Version970, &getSnapshotParam, &sqlca);

  while ( sqlca.sqlcode == 1606 && try++ < 5 ) {
    sprintf(mesg, "Buffer size for snapshot data is too small(%d)\n", (pArgs->buffer_sz) );
    strcat(mesg, "Re-allocating memory for snapshot monitor data.");
    WARN_MSG(mesg);

    /* deallocate memory assigned to the buffer */
    free_snapshot_memory(NULL, (pArgs->buffer_ptr));
    /* allocate new buffer                      */
    pArgs->buffer_ptr = init_snapshot_buffer(node
        , &(pArgs->buffer_sz)
        , &(pArgs->db2SnapReq));


    if (pArgs->buffer_ptr == NULL) {
      sprintf(mesg, "Error allocating memory for buffer area. Exiting.");
      WARN_MSG(mesg);
      pthread_exit((void *) -1);
    } else {
      sprintf(mesg, "Re-allocated memory for snapshot monitor data(new size:%d)", (pArgs->buffer_sz) );
      WARN_MSG(mesg);
    }

    getSnapshotParam.iBufferSize = pArgs->buffer_sz;
    getSnapshotParam.poBuffer = pArgs->buffer_ptr;

    /* get snapshot */
    rc = db2GetSnapshot(db2Version970, &getSnapshotParam, &sqlca);
  }

  /* exit function if the db2GetSnapshot call returns a non-zero value */
  if (rc != 0) {
    free_snapshot_memory(NULL, pArgs->buffer_ptr);
    sprintf(mesg, "Return code from db2GetSnapshot is %d. Exiting.", rc);
    WARN_MSG(mesg);
    pthread_exit((void *) -1);
  }

  /* examine the sqlcode and react accordingly: */
  /*   if 0, then continue */
  /*   if positive, then print the sqlca info and continue */
  /*   if negative, then print the sqlca info and exit function */
  if (sqlca.sqlcode != 0L) {
    SqlInfoPrint("db2GetSnapshot", &sqlca, __LINE__, __FILE__);
    if ( sqlca.sqlcode < 0L) {
      sprintf(mesg, "db2GetSnapshot SQLCODE is %d. buffer sz %d. node %d Exiting."
          , sqlca.sqlcode, pArgs->buffer_sz, node );
      WARN_MSG(mesg);
      free_snapshot_memory(NULL, pArgs->buffer_ptr);
      pthread_exit((void *) -1);
    }
  }

  pthread_exit((void *) 0);

} /*take_db2_snapshot*/

/*************************************************************************/
/*  parse snapshot stream                                                */
/*************************************************************************/
int parse_monitor_stream(Header *header                   
, DB2List *list                  
, char *buffer_ptr )
{
  int rc = 0;
  DB2ListElmt *element;
  char  mesg[1024];
  time_t start_time;
  // -- fprintf(stderr,"took snapshot %g sec\n", difftime(time(NULL), start_time));
  // -- start_time = time(NULL);
  /*************************************************************************/
  /*  parse or print snapshot stream                                       */
  /*************************************************************************/
  if (header->dump == TRUE) {
    FILE *fp;
    fp = fopen("db2topas.txt", "w");
    if (fp == NULL)
      WARN_MSG("db2topas.txt open failed");
    else {
      char  *elementName[NUMELEMENTS];
      char  *elementType[NUMTYPES];
      init_element_names(elementName);
      init_element_types(elementType);
      print_monitor_stream(" ", buffer_ptr, NULL, 0, fp, elementName, elementType);
    }
    header->dump = FALSE;
  }
  else if ( (peek_snapReq(header))->type == stmts ) {
    Stmt *tempListData =  init_stmt();
    parse_monitor_stream_DBASE(header, 
        buffer_ptr
        , NULL
        , 0
        , 0);
    if (tempListData != NULL) {
      parse_monitor_stream_STMTS(header
          , list
          , tempListData
          , buffer_ptr
          , NULL
          , 0
          , 0);

      /* add the last stmt */
      if ( tempListData->appl_handle > 0 && tempListData->section_number  != 999999) {
        element = db2list_lookup(list, tempListData);
        if ( element != NULL) {
          copy_stmt(db2list_data(element), tempListData);
        } else {
          Stmt* newStmtData = init_stmt();
          copy_stmt(newStmtData, tempListData);
          db2list_ins_next(list, db2list_tail(list), newStmtData);
        }
        reset_stmt(tempListData);
      }
      free(tempListData);
    } /* end of tempListData Null */

  } else if ( (peek_snapReq(header))->type == appls || (peek_snapReq(header))->type == agent_id)  {

    Appl *tempListData = init_appl();
    parse_monitor_stream_DBASE(header, 
        buffer_ptr
        , NULL
        , 0
        , 0);
    if (tempListData != NULL) {
      parse_monitor_stream_APPLS(header
          , list
          , tempListData
          , buffer_ptr
          , NULL
          , 0
          , 0);

      if ( (peek_snapReq(header))->type == agent_id) {
        Stmt *stmtData = init_stmt();
        if (stmtData != NULL)
          parse_monitor_stream_STMTS( header                 
              , &(tempListData->stmt_list)
              , stmtData                   
              , buffer_ptr                   
              , NULL                    
              , 0
              , 0     );

        /* add the last stmt */
        if (   stmtData  != NULL                                       
            && stmtData->appl_handle > 0
            && stmtData->section_number  != 999999) {
          element = db2list_lookup(&(tempListData->stmt_list), stmtData);
          if ( element != NULL) {
            copy_stmt(db2list_data(element), stmtData);
          } else {
            Stmt* newStmtData = init_stmt();
            copy_stmt(newStmtData, stmtData);
            db2list_ins_next(&(tempListData->stmt_list), 
                db2list_tail(&(tempListData->stmt_list)), newStmtData);
          }
          free(stmtData);
        }

        Lock *lockData = init_lock();
        if (lockData != NULL) {
          /* get locks for this agent_id 
             providing up front b/c agentid info in appl_lock_list 
             can be after the lock info */
          lockData->agent_id = (peek_snapReq(header))->agent_id;
          parse_monitor_stream_LOCKS(header
              , &(tempListData->lock_list)
              , lockData                                
              , buffer_ptr                              
              , NULL                                    
              , 0                                       
              , 0     );
          /* add the last lock */
          if ( lockData->agent_id > 0                                    
              && lockData->lock_obj_type > 0) {
            element = db2list_lookup(&(tempListData->lock_list), lockData);
            if ( element != NULL) {
              copy_lock(db2list_data(element), lockData);
            } else {
              Lock* newLockData = init_lock();
              copy_lock(newLockData, lockData);
              db2list_ins_next(&(tempListData->lock_list),                   
                  db2list_tail(&(tempListData->lock_list)), newLockData);
            }
            free(lockData);
          }
        }
      }
      /* add the last appl snapshot to the list */
      if ( tempListData  != NULL
          && tempListData->appl_handle > 0 
          && add_appl(peek_snapReq(header), tempListData) == TRUE) {
        element = db2list_lookup(list, tempListData);
        if ( element != NULL) {
          copy_appl(db2list_data(element), tempListData);
        } else {
          Appl * newApplData = init_appl();
          copy_appl(newApplData, tempListData);
          db2list_ins_next(list, db2list_tail(list), newApplData);
        }
        reset_appl(tempListData);
      }

      free_appl(tempListData);
    } /*end of tempListData NULL */

  } else if ( (peek_snapReq(header))->type == agent_id_detl) {
    DB2ListElmt * element;
    parse_monitor_stream_DBASE(header, 
        buffer_ptr
        , NULL
        , 0
        , 0);
    header->pStmtStart = NULL;
    header->sequence_no_found = FALSE;
    header->section_number_found = FALSE;
    header->node_number_found = FALSE;

    preparse_monitor_stream_AGENTID_SS(header, buffer_ptr, NULL, 0);
    if (header->pStmtStart != NULL) {
      Appl_SS *tempListData = init_appl_ss();
      tempListData->appl_handle = (peek_snapReq(header))->agent_id;
      char *pData, *pEnd, buffer[1024];

      sqlm_header_info * pHeader = (sqlm_header_info * )header->pStmtStart;

      pData = header->pStmtStart + sizeof(sqlm_header_info);
      pEnd =  pData + pHeader->size;
      if (tempListData != NULL) {
        parse_monitor_stream_AGENTID_SS (header
            , list
            , tempListData
            , header->pStmtStart
            , pEnd
            , 0
            , 0);

        /* add the last appl subsection to the list */
        if ( ((Appl_SS * ) tempListData)->ss_node_number < 1000) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_appl_ss(db2list_data(element), tempListData);
          } else {
            Appl_SS * newApplData = init_appl_ss();
            copy_appl_ss(newApplData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newApplData);
          }
          reset_appl_ss(tempListData);
        }
        free(tempListData);
      }
    }

  } else if ( (peek_snapReq(header))->type == agent_id_cmdline) {
    sqluint32 agent_id = (peek_snapReq(header))->agent_id;
    int node = (peek_snapReq(header))->node;
    char cmd[1024];

    if (strlen(header->pswd) > 0)
      sprintf(cmd, "db2 attach to %s user %s  using %s; db2 get snapshot for application agentid %d at dbpartitionnum %d|more",
          header->instance, header->user_id, header->pswd, agent_id,node );
    else
      sprintf(cmd, "db2 attach to %s; db2 get snapshot for application agentid %d at dbpartitionnum %d|more",
          header->instance, agent_id, node);

    /* leave curses temporarily */
    def_prog_mode();
    endwin();

    if ( (system(cmd)) < 0 )
      WARN_MSG("system cmd failed");

    /* restart curses */
    reset_prog_mode();
    return 0;
  } else if ( (peek_snapReq(header))->type == utils
      || (peek_snapReq(header))->type == utils_id ) {
    Util *tempListData = init_util();
    parse_monitor_stream_DBASE(header 
        , buffer_ptr
        , NULL
        , 0
        , 0);
    if (tempListData != NULL) {
      parse_monitor_stream_UTILS (header
          , list
          , tempListData
          , buffer_ptr
          , NULL
          , 0
          , 0);
      /* add the list util to the list */
      if ( tempListData != NULL 
          && ((Util * ) tempListData)->id > 0
          && add_util(peek_snapReq(header), tempListData) == TRUE) {
        element = db2list_lookup(list, tempListData);
        if (element != NULL) {
          copy_util(db2list_data(element), tempListData);
        } else {
          Util * newUtilData = init_util();
          copy_util(newUtilData, tempListData);
          db2list_ins_next(list, db2list_tail(list), newUtilData);
        }
        reset_util(tempListData);
      } else
        reset_util(tempListData);
      free(tempListData);
    }

  } else if ( (peek_snapReq(header))->type == tbspace ) {

    DB2ListElmt * element;
    Tbspace *tempListData = init_tbspace();

    parse_monitor_stream_DBASE(header                         
        , buffer_ptr                     
        , NULL                          
        , 0                             
        , 0);
    if (tempListData != NULL) {
      parse_monitor_stream_TBSPACE (header                        
          , list                        
          , tempListData                
          , buffer_ptr                 
          , NULL                        
          , 0                           
          , 0);

      /* add the last tbspace to the list */
      if ( tempListData != NULL                                       
          && strlen(((Tbspace * ) tempListData)->name) > 0) {
        element = db2list_lookup(list, tempListData);
        if (element != NULL) {
          copy_tbspace(db2list_data(element), tempListData);
        } else {
          Tbspace * newTbspaceData = init_tbspace();
          copy_tbspace(newTbspaceData, tempListData);
          db2list_ins_next(list, db2list_tail(list), newTbspaceData);
        }
        reset_tbspace(tempListData);
      } else 
        reset_tbspace(tempListData);
      free(tempListData);
    }

    /* calculate rate */
    for (element = db2list_head(list); element != NULL; element = db2list_next(element))
      delta_tbspace(db2list_data(element));
  } else if ( (peek_snapReq(header))->type == tbspace_id ) {
    DB2ListElmt * element;
    Table *tempListData = init_table();

    parse_monitor_stream_DBASE(header                         
        , buffer_ptr                     
        , NULL                          
        , 0                             
        , 0);
    if (tempListData != NULL) {
      parse_monitor_stream_TABLE (header                        
          , list                        
          , tempListData                
          , buffer_ptr                 
          , NULL                        
          , 0                           
          , 0);

      /* add the last table to the list */
      if ( tempListData != NULL                                       
          && strlen(((Table * ) tempListData)->tabname) > 0
          && add_table(peek_snapReq(header), tempListData) ) {
        element = db2list_lookup(list, tempListData);
        if (element != NULL) {
          copy_table(db2list_data(element), tempListData);
        } else {
          Table * newTableData = init_table();
          copy_table(newTableData, tempListData);
          db2list_ins_next(list, db2list_tail(list), newTableData);
        }
        reset_table(tempListData);
      } else 
        reset_table(tempListData);

      /* get avg record length */
      reclen_table(header, list);
      free(tempListData);
    }

  } else {
    /* snapshot request doesn't make sense */
    return - 1;

  }

  // -- fprintf(stderr,"parsed snapshot %g sec\n", difftime(time(NULL), start_time));
  // -- start_time = time(NULL);

  return rc;
} /* parse_monitor_stream  */


/***************************************************************************/
/* print snapshot data to stdout instead of parsing and saving in a list   */
/***************************************************************************/
sqlm_header_info *print_monitor_stream(char *prefix,
char *pStart,
char *pEnd,
sqluint32 logicalGroup,
FILE *fp,
char *elementName[],
char *elementType[])
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char   *pData
  , *pElementName
  , *pElementType
  , *pLogicalGroup;
  char   elementNameBuffer[24]
  , elementTypeBuffer[24]
  , logicalGroupBuffer[24]
  , buffer[1024];
  sqlm_timestamp timestamp;

  if (fp == NULL)
    return NULL;
  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine the actual element name */
    if (pHeader->element >= NUMELEMENTS  || 
        (!(pElementName = elementName[pHeader->element]))) {
      /* if the element name is not defined, display the number */
      sprintf(elementNameBuffer, "Element Number %d", pHeader->element);
      pElementName = elementNameBuffer;
    }
    /* determine the actual logical group name */
    if (logicalGroup >= NUMELEMENTS  || 
        (!(pLogicalGroup = elementName[logicalGroup]))) {
      /* if the element name is not defined, display the number */
      sprintf(logicalGroupBuffer, "Logical Group %d", logicalGroup);
      pLogicalGroup = logicalGroupBuffer;
    }

    /* determine the actual element type */
    if (pHeader->type >= NUMTYPES || 
        (!(pElementType = elementType[pHeader->type]))) {
      // if the element type is not defined, display the number 
      sprintf(elementTypeBuffer, "Element Type %d", pHeader->type);
      pElementType = elementTypeBuffer;
    }

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  newPrefix[80];
      char  *pNewEnd;

      fprintf(fp, "%s Logical Grouping  %s   (size %d)\n", prefix, 
          pElementName,
          pHeader->size);

      /* indent the data for this logical group to indicate nesting */
      strncpy(newPrefix, BLANKS, strlen(prefix) + 2);
      newPrefix[strlen(prefix)+2] = '\0';
      pNewEnd = pData + (pHeader->size);

      /* call ParseMonitorStream recursively to parse and print this */
      /* nested logical grouping */
      pHeader = print_monitor_stream(newPrefix
          , pData
          , pNewEnd
          , pHeader->element
          , fp
          , elementName
          , elementType);
    } else
    {
      /* not a logical grouping, therefore print this monitor element */

      if (logicalGroup == SQLM_ELM_UOW_START_TIME
          || logicalGroup == SQLM_ELM_UOW_STOP_TIME 
          || logicalGroup == SQLM_ELM_TIME_STAMP
          || logicalGroup == SQLM_ELM_SWITCH_SET_TIME
          || logicalGroup == SQLM_ELM_APPL_CON_TIME
          || logicalGroup == SQLM_ELM_CONN_COMPLETE_TIME 
          || logicalGroup == SQLM_ELM_LAST_RESET
          || logicalGroup == SQLM_ELM_PREV_UOW_STOP_TIME
          || logicalGroup == SQLM_ELM_STATUS_CHANGE_TIME
          || logicalGroup == SQLM_ELM_STMT_START
          || logicalGroup == SQLM_ELM_STMT_STOP
          ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          timestamp.seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          timestamp.microsec = *(sqluint32 * )pData;
          fprintf(fp,"%s Data: %s\n",prefix, time_STRING(timestamp,buffer)) ;
          break;
        }
      } else if (pHeader->type == SQLM_TYPE_U8BIT) {
        sqluint8 uch = *(sqluint8 * )pData;
        fprintf(fp, "%s Data %s (type: %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%c\n", uch);
      } else if (pHeader->type == SQLM_TYPE_8BIT) {
        sqlint8 ch = *(sqlint8 * )pData;
        fprintf(fp, "%s Data %s (type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%d\n", ch);
      } else if (pHeader->type == SQLM_TYPE_U16BIT) {
        sqluint16 i = *(sqluint16 * )pData;
        fprintf(fp, "%s Data %s(type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%u\n", i);
      } else if (pHeader->type == SQLM_TYPE_16BIT) {
        sqlint16 i = *(sqlint16 * )pData;
        fprintf(fp, "%s Data %s(type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%d\n", i);
      } else if (pHeader->type == SQLM_TYPE_U32BIT) {
        sqluint32 i = *(sqluint32 * )pData;
        fprintf(fp, "%s Data %s(type: %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%u\n", i);
      } else if (pHeader->type == SQLM_TYPE_32BIT) {
        sqlint32 i = *(sqlint32 * )pData;
        fprintf(fp, "%s Data %s(type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%d\n", i);
      } else if (pHeader->type == SQLM_TYPE_U64BIT) {
        sqluint64 i = *(sqluint64 * )pData;
        fprintf(fp, "%s Data %s(type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%lu\n", i);
      } else if (pHeader->type == SQLM_TYPE_64BIT) {
        sqlint64 i = *(sqlint64 * )pData;
        fprintf(fp, "%s Data %s(type %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "%ld\n", i);
      } else if (pHeader->type == SQLM_TYPE_STRING) {
        /* print out the char string data */
        fprintf(fp, "%s Data %s(type: %s): ", prefix, pElementName, pElementType);
        fprintf(fp, "\"%.*s\"\n", pHeader->size, pData);
      } else
      {
        /* dump out the data in hex format */
        int  i, j;
        fprintf(fp, "%s Data %s: ", prefix, pElementName);
        fprintf(fp, "0x");
        for (i = 0; i < pHeader->size; i++) {
          j = (char)*(pData + i);
          fprintf(fp, "%.2x", j);
        }
        fprintf(fp, "\n");
      }

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* print_monitor_stream */


/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_DBASE   (Header  *header,
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      if (pHeader->element == SQLM_ELM_DB2)
        primaryLogicalGroup = SQLM_ELM_DB2;
      else if (pHeader->element == SQLM_ELM_DBASE)
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_APPL)
        primaryLogicalGroup = SQLM_ELM_APPL;
      else if (pHeader->element == SQLM_ELM_TABLESPACE)
        primaryLogicalGroup = SQLM_ELM_TABLESPACE;
      else if (pHeader->element == SQLM_ELM_COLLECTED)
        primaryLogicalGroup = SQLM_ELM_COLLECTED;


      /* call function recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_DBASE(header 
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_DBASE) {
        /* add up all elements b/c the snapshot can be for all dbase */
        switch (pHeader->element) {
        case SQLM_ELM_NUM_ASSOC_AGENTS:
          header->db_assoc_agents += *(sqluint32 *)pData;
          break;
        case SQLM_ELM_SORT_HEAP_ALLOCATED:
        case SQLM_ELM_SORT_SHRHEAP_ALLOCATED:
          /* sort heap allocated are in 4K pages
             so convert into bytes */
          header->db_sortheap += (1024 * 4 * (*(sqluint32 * )pData));
          break;
        case SQLM_ELM_POOL_READ_TIME:
          header->t2_db_bpr_tm += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_WRITE_TIME:
          header->t2_db_bpw_tm += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_LOG_READS:
          header->t2_db_log_reads += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_LOG_WRITES:
          header->t2_db_log_writes += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TOTAL_LOG_USED:
          /* add up dbase log usage for all dbase  */
          header->db_log_used += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TOTAL_LOG_AVAILABLE:
          /* add up dbase log usage for all dbase  */
          header->db_log_avail += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_APPL_ID_OLDEST_XACT:
          header->agent_id_oldest_xact = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_SMALLEST_LOG_AVAIL_NODE:
          header->smallest_log_avail_node = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_APPLS_CUR_CONS:
          header->appls_connected += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_APPLS_IN_DB2:
          header->appls_executing += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_DIRECT_READS:
          header->t2_db_direct_io += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITES:
          header->t2_db_direct_io += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_READ_REQS:
        case SQLM_ELM_DIRECT_WRITE_REQS:
          header->t2_db_direct_io_reqs += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_L_READS:
          header->t2_db_io_type_data += *(sqluint64 * )pData;
          header->t2_db_io_type_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_L_READS:
          header->t2_db_io_type_idx += *(sqluint64 * )pData;
          header->t2_db_io_type_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_TEMP_DATA_L_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_L_READS:
          header->t2_db_io_type_temp += *(sqluint64 * )pData;
          header->t2_db_io_type_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_XDA_L_READS:
        case SQLM_ELM_POOL_TEMP_XDA_L_READS:
          header->t2_db_io_type_xml = *(sqluint64 * )pData;
          header->t2_db_io_type_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_P_READS:
        case SQLM_ELM_POOL_INDEX_P_READS:
        case SQLM_ELM_POOL_TEMP_DATA_P_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_P_READS:
        case SQLM_ELM_POOL_XDA_P_READS:
        case SQLM_ELM_POOL_TEMP_XDA_P_READS:
          header->t2_db_buffered_rio += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_WRITES:
          header->t2_db_io_type_data += *(sqluint64 * )pData;
          header->t2_db_io_type_write += *(sqluint64 * )pData;
          header->t2_db_buffered_wio += *(sqluint64 * )pData;
        case SQLM_ELM_POOL_INDEX_WRITES:
          header->t2_db_io_type_idx += *(sqluint64 * )pData;
          header->t2_db_io_type_write += *(sqluint64 * )pData;
          header->t2_db_buffered_wio += *(sqluint64 * )pData;
        case SQLM_ELM_POOL_XDA_WRITES:
          header->t2_db_io_type_xml += *(sqluint64 * )pData;
          header->t2_db_io_type_write += *(sqluint64 * )pData;
          header->t2_db_buffered_wio += *(sqluint64 * )pData;
          break;
        }
      }   /* end of SQLM_ELM_DBASE */ else if (primaryLogicalGroup == SQLM_ELM_DBASE 
          && logicalGroup == SQLM_ELM_MEMORY_POOL) {
        switch (pHeader->element) {
        case SQLM_ELM_POOL_CUR_SIZE:
          header->db_memusg += *(sqluint64 * )pData;
          header->db_genheap = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_ID :
          switch( *(sqluint32 * )pData) {
          case SQLM_HEAP_LOCK_MGR:
            header->db_lockheap += header->db_genheap;
            header->db_genheap = 0;
          case SQLM_HEAP_UTILITY:
          case SQLM_HEAP_STATISTICS:
            header->db_utilheap += header->db_genheap;
            header->db_genheap = 0;
            break;
          case SQLM_HEAP_APPL_CONTROL:
            /* subtract appl_ctl_heap b/c appgroup_mem accounts for it */
            header->db_memusg = header->db_memusg - header->db_genheap;
            header->db_genheap = 0;
            break;
          }
          break;
        }
      } /* end of SQLM_ELM_MEMORY_POOL */
      else if (logicalGroup == SQLM_ELM_TIME_STAMP ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (header->t2_snapshot_timestamp).seconds = *(sqluint32 *) pData;
          break;
        case SQLM_ELM_MICROSEC:
          (header->t2_snapshot_timestamp).microsec = *(sqluint32 *) pData;
          break;
        }
      }

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_DBASE */


/*************************************************************************************/
/****************************************************************************/
/* parse_monitor_stream_LOCKS                                                */
/*                                                                          */
/****************************************************************************/
sqlm_header_info *parse_monitor_stream_LOCKS   (Header  *header,
DB2List   *list,
Lock   *tempListData, 
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData, temp[USERID_SZ +1];
  DB2ListElmt  *element;
  char buffer[1024];

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      /* *************************************************
        -start of a lock
        -possibly add data to the list
        -reset temp data to erase old values
        -keep track of type of snapshot output
         using primaryLogicalGroup 
      *****************************************************/
      if (pHeader->element == SQLM_ELM_LOCK || pHeader->element == SQLM_ELM_APPL_LOCK_LIST) {
        if (pHeader->element == SQLM_ELM_APPL_LOCK_LIST)
          primaryLogicalGroup = SQLM_ELM_APPL_LOCK_LIST;
        if (tempListData->agent_id > 0 && tempListData->lock_obj_type > 0) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_lock(db2list_data(element), tempListData);
          } else {
            Lock * newLockData = init_lock();
            copy_lock(newLockData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newLockData);
          }
          reset_lock(tempListData);
        }
      } else if (pHeader->element == SQLM_ELM_DBASE)
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_APPL)
        primaryLogicalGroup = SQLM_ELM_APPL;
      else if (pHeader->element == SQLM_ELM_COLLECTED)
        primaryLogicalGroup = SQLM_ELM_COLLECTED;


      /* nested logical grouping */
      pHeader = parse_monitor_stream_LOCKS(header, list, tempListData
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_APPL_LOCK_LIST) {
        switch (pHeader->element) {
        case SQLM_ELM_AGENT_ID:
          tempListData->agent_id = *(sqluint32 * )pData;
          break;
        }
      } else if (logicalGroup == SQLM_ELM_LOCK  || 
          (primaryLogicalGroup == SQLM_ELM_APPL_LOCK_LIST && logicalGroup == SQLM_ELM_LOCK_WAIT) ) {
        switch (pHeader->element) {
        case SQLM_ELM_AGENT_ID_HOLDING_LK:
          tempListData->agent_id_holding_lk = *(sqluint32 *)pData;
          break;
        case SQLM_ELM_LOCK_OBJECT_TYPE:
          tempListData->lock_obj_type = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_LOCK_MODE:
          tempListData->lock_mode = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_LOCK_MODE_REQUESTED:
          tempListData->lock_mode_requested = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_LOCK_STATUS:
          tempListData->lock_status = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_SUBSECTION_NUMBER:
          tempListData->ss_number = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_NODE_NUMBER:
          tempListData->node = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_LOCK_ESCALATION:
          tempListData->lock_escal = *(sqluint8 * )pData;
          break;
        case SQLM_ELM_TABLE_NAME:
          sprintf(buffer, "%.*s",pHeader->size,pData);
          strncpy(tempListData->tabname, strtrim(buffer), TABNAME_SZ);
          break;
        case SQLM_ELM_TABLE_SCHEMA:
          sprintf(buffer, "%.*s",pHeader->size,pData);
          strncpy(tempListData->tabschema, strtrim(buffer), TABSCHEMA_SZ);
          break;
        case SQLM_ELM_TABLESPACE_NAME:
          sprintf(buffer, "%.*s",pHeader->size,pData);
          strncpy(tempListData->tbspace, strtrim(buffer), SQLUH_TABLESPACENAME_SZ);
          break;
        case SQLM_ELM_DATA_PARTITION_ID:
          tempListData->data_partition_id = *(sqluint16 *)pData;
          break;
        }
      } /* end of ELM_LOCK and ELM_LOCK_WAIT */

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_LOCKS */
/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_STMTS   (Header  *header,
DB2List   *list,
Stmt   *tempListData, 
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData, temp[USERID_SZ + 1];
  DB2ListElmt  *element;
  size_t size;
  int i;
  Boolean comment = FALSE;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);
      if (pHeader->element == SQLM_ELM_APPL) {
        primaryLogicalGroup = SQLM_ELM_APPL;
        /* start of a new a new application; add the stmt from the previous stmt */
        if ( tempListData  != NULL                                         
            && tempListData->appl_handle > 0
            && tempListData->section_number  != 999999) {
          element = db2list_lookup(list, tempListData);
          if ( element != NULL) {
            copy_stmt(db2list_data(element), tempListData);
          } else {
            Stmt* newStmtData = init_stmt();
            copy_stmt(newStmtData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newStmtData);
          }
          reset_stmt(tempListData);
        }
        tempListData->appl_handle = 0;
        strcpy(tempListData->auth_id,"");
        strcpy(tempListData->exec_id,"");
        strcpy(tempListData->sequence_no,"");
        reset_stmt(tempListData);
      } else if (pHeader->element == SQLM_ELM_DBASE)
        /* start of a DBASE snapshot */
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_STMT) {
        primaryLogicalGroup = SQLM_ELM_STMT;
        /* start of a new stmt with in an application */
        if ( tempListData  != NULL                                      
            && tempListData->appl_handle > 0
            && tempListData->section_number  != 999999) {
          element = db2list_lookup(list, tempListData);
          if ( element != NULL) {
            copy_stmt(db2list_data(element), tempListData);
          } else {
            Stmt* newStmtData = init_stmt();
            copy_stmt(newStmtData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newStmtData);
          }
          reset_stmt(tempListData);
        }
      }

      /* call ParseMonitorStream4Appl recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_STMTS(header, list, tempListData
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_APPL_INFO) {
        switch (pHeader->element) {
        case SQLM_ELM_AGENT_ID:
          tempListData->appl_handle = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_AUTH_ID:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->auth_id, strtrim(temp), USERID_SZ);
          break;
        case SQLM_ELM_EXECUTION_ID:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->exec_id, strtrim(temp), USERID_SZ);
          break;
        case SQLM_ELM_SEQUENCE_NO:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->sequence_no, strtrim(temp), SQLM_SEQ_SZ);
          break;
        }
      } else if (logicalGroup == SQLM_ELM_STMT_START)  {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->stmt_start).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->stmt_start).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_STMT_START */ 
          else if (logicalGroup == SQLM_ELM_STMT_USR_CPU_TIME)  {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->stmt_ucpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->stmt_ucpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } else if ( logicalGroup == SQLM_ELM_STMT_SYS_CPU_TIME) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->stmt_scpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->stmt_scpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_STMT_CPU_TIME */ 
          else if (logicalGroup == SQLM_ELM_STMT) {
        switch (pHeader->element) {
        case SQLM_ELM_ROWS_READ:
          tempListData->stmt_rows_read = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_WRITTEN:
          tempListData->stmt_rows_written = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_STMT_SORTS:
          tempListData->stmt_sorts = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TOTAL_SORT_TIME:
          tempListData->stmt_total_sort_time = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_SORT_OVERFLOWS:
          tempListData->stmt_sort_overflows = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_DELETED:
          tempListData->stmt_rows_deleted = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_UPDATED:
          tempListData->stmt_rows_updated = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_INSERTED:
          tempListData->stmt_rows_inserted = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_SELECTED:
        case SQLM_ELM_FETCH_COUNT:  
          tempListData->stmt_rows_selected += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_TEMP_DATA_L_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_L_READS:
        case SQLM_ELM_POOL_TEMP_XDA_L_READS:
          tempListData->stmt_bpltdir += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_L_READS:
        case SQLM_ELM_POOL_XDA_L_READS:
          tempListData->stmt_bpldr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_L_READS:
          tempListData->stmt_bplir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_P_READS:
        case SQLM_ELM_POOL_XDA_P_READS:
          tempListData->stmt_bppdr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_P_READS:
          tempListData->stmt_bppir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_QUERY_COST_ESTIMATE:
          tempListData->query_cost = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_QUERY_CARD_ESTIMATE:
          tempListData->query_card = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_STMT_OPERATION:
          tempListData->stmt_op = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_NUM_AGENTS:
          tempListData->nagents = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_SECTION_NUMBER:
          tempListData->section_number = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_STMT_NODE_NUMBER: 
          tempListData->stmt_node_number = *(sqluint16 *)pData;
          break;
        case SQLM_ELM_STMT_TEXT:
          /* only copy the first STMT_SZ characters */
          if (pHeader->size > 0) {
            for(size = 0, i=0; i < pHeader->size && size < STMT_SZ;i++) {
              if (size == 0 && isspace(pData[i]))
                continue;
              else if ( (i+1 < pHeader->size) && pData[i] == '*' && pData[i+1]
                  == '/') {
                comment = FALSE;
                i++;
                continue;
              } else if ( (i+1 < pHeader->size) && pData[i] == '/' && pData[i+1]
                  == '*') {
                comment = TRUE;
                i++;
                continue;
              } else if (!comment && size > 0 && isspace(pData[i])
                  && !isspace(tempListData->stmt_text[size-1])) {
                tempListData->stmt_text[size] = ' ';
                size++;
              } else if (!comment && !isspace(pData[i]))  {
                tempListData->stmt_text[size] = pData[i];
                size++;
              }
            }
            tempListData->stmt_text[size] = '\0';
          }
          break;
        }
      } /* end of SQLM_ELM_STMT */


      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_STMTS */

/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_APPLS   (Header  *header,
DB2List   *list,
Appl   *tempListData, 
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData, temp[USERID_SZ + 1];
  DB2ListElmt  *element;
  int size,i;
  Boolean comment = FALSE;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      /* *************************************************
        -start of an application snapshot 
        -possibly add temp to appl list
        -reset temp appl node to erase old values
        -keep track of type of snapshot output
         using primaryLogicalGroup 
      *****************************************************/
      if (pHeader->element == SQLM_ELM_APPL) {
        primaryLogicalGroup = SQLM_ELM_APPL;
        if (tempListData != NULL 
            && tempListData->appl_handle > 0
            && add_appl(peek_snapReq(header), tempListData) == TRUE ) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_appl(db2list_data(element), tempListData);
          } else {
            Appl * newApplData = init_appl();
            copy_appl(newApplData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newApplData);
          }
          reset_appl(tempListData);
        } else
          reset_appl(tempListData); /* throw away info if add_appl is FALSE */
      } else if (pHeader->element == SQLM_ELM_DBASE) {
        /* start of a DBASE snapshot */
        primaryLogicalGroup = SQLM_ELM_DBASE;
      }

      /* call ParseMonitorStream4Appl recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_APPLS(header, list, tempListData
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_APPL) {
        switch (pHeader->element) {
        case SQLM_ELM_LOCKS_HELD :
          tempListData->locks_held = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_UOW_LOG_SPACE_USED:
          tempListData->uow_log_space_used = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_READ:
          tempListData->appl_rows_read = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_WRITTEN:
          tempListData->appl_rows_written = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_TEMP_DATA_L_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_L_READS:
        case SQLM_ELM_POOL_TEMP_XDA_L_READS:
          tempListData->appl_bpltdir += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_L_READS:
        case SQLM_ELM_POOL_XDA_L_READS:
          tempListData->appl_bpldr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_L_READS:
          tempListData->appl_bplir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_P_READS:
        case SQLM_ELM_POOL_XDA_P_READS:
          tempListData->appl_bppdr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_P_READS:
          tempListData->appl_bppir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_WRITES:
        case SQLM_ELM_POOL_XDA_WRITES:
          tempListData->bpdw += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_WRITES:
          tempListData->bpiw = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_READ_TIME:
          tempListData->bpr_tm = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_WRITE_TIME:
          tempListData->bpw_tm = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_READS:
          tempListData->dio_reads = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITES:
          tempListData->dio_writes = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_READ_REQS :
          tempListData->dio_read_reqs = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITE_REQS:
          tempListData->dio_write_reqs = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_READ_TIME :
          tempListData->dio_read_tm = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITE_TIME:
          tempListData->dio_write_tm = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_DELETED:
          tempListData->appl_rows_deleted = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_UPDATED:
          tempListData->appl_rows_updated = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_INSERTED:
          tempListData->appl_rows_inserted = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_SELECTED:
          tempListData->appl_rows_selected = *(sqluint64 * )pData;
          break;
        } /* end  of switch */
      } /* end of SQLM_ELM_APPL */ else if (logicalGroup == SQLM_ELM_APPL_INFO) {
        switch (pHeader->element) {
        case SQLM_ELM_NUM_ASSOC_AGENTS:
          tempListData->nagents = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_AGENT_ID:
          tempListData->appl_handle = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_APPL_STATUS:
          tempListData->appl_status = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_AUTH_ID:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->auth_id, strtrim(temp), USERID_SZ);
          break;
        case SQLM_ELM_EXECUTION_ID:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->exec_id, strtrim(temp), USERID_SZ);
          break;
        case SQLM_ELM_APPL_NAME:
          sprintf(temp, "%.*s",pHeader->size,pData);
          strncpy( tempListData->prog_nm, strtrim(temp), USERID_SZ);
          break;
        case SQLM_ELM_CLIENT_NNAME:
          if (pHeader->size > 0)  {
            sprintf(temp, "%.*s",pHeader->size,pData);
            strncpy( tempListData->client_nm, strtrim(temp), USERID_SZ);
          } else
            strncpy(tempListData->client_nm, "N/A", USERID_SZ);
          break;
        case SQLM_ELM_COORD_NODE_NUM:
          tempListData->coord_node_num = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_CLIENT_PID:
          tempListData->client_pid = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_COORD_AGENT_PID:
          tempListData->coord_pid = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_APPL_INFO */ else if (logicalGroup == SQLM_ELM_UOW_START_TIME  
          && primaryLogicalGroup == SQLM_ELM_APPL) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->uow_start).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->uow_start).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_UOW_START */ else if (logicalGroup == SQLM_ELM_UOW_STOP_TIME  
          && primaryLogicalGroup == SQLM_ELM_APPL ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->uow_stop).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->uow_stop).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_UOW_STOP */ 
          else if ( logicalGroup == SQLM_ELM_AGENT_USR_CPU_TIME ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->appl_ucpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->appl_ucpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } else if (logicalGroup == SQLM_ELM_AGENT_SYS_CPU_TIME) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->appl_scpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->appl_scpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_AGENT_CPU_TIME */ else if ( (logicalGroup == SQLM_ELM_STMT_USR_CPU_TIME 
          || logicalGroup == SQLM_ELM_STMT_SYS_CPU_TIME)) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->stmt_cpu_used).seconds += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->stmt_cpu_used).microsec += *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_STMT_CPU_TIME */ else if ( (logicalGroup == SQLM_ELM_SS_SYS_CPU_TIME
          || logicalGroup == SQLM_ELM_SS_USR_CPU_TIME)) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->ss_cpu_used).seconds += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->ss_cpu_used).microsec += *(sqluint32 * )pData;
          break;
        }
      } else if (logicalGroup == SQLM_ELM_STMT && primaryLogicalGroup ==
          SQLM_ELM_APPL) {
        switch (pHeader->element) {
          /* sum up all statements in an application */
        case SQLM_ELM_ROWS_READ:
          tempListData->stmt_rows_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_WRITTEN:
          tempListData->stmt_rows_written += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_DELETED:
          tempListData->stmt_rows_deleted += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_UPDATED:
          tempListData->stmt_rows_updated += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_INSERTED:
          tempListData->stmt_rows_inserted += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_SELECTED:
        case SQLM_ELM_FETCH_COUNT:
          tempListData->stmt_rows_selected += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_TEMP_XDA_L_READS:
        case SQLM_ELM_POOL_TEMP_DATA_L_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_L_READS:
          tempListData->stmt_bpltdir += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_XDA_L_READS:
        case SQLM_ELM_POOL_DATA_L_READS:
          tempListData->stmt_bpldr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_L_READS:
          tempListData->stmt_bplir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_P_READS:
        case SQLM_ELM_POOL_XDA_P_READS:
          tempListData->stmt_bppdr += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_INDEX_P_READS:
          tempListData->stmt_bppir = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_QUERY_COST_ESTIMATE:
          tempListData->query_cost += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_QUERY_CARD_ESTIMATE:
          tempListData->query_card += *(sqluint32 * )pData;
          break;
        case SQLM_ELM_STMT_TEXT:
          if (pHeader->size > 0)
            tempListData->num_stmts+=1;
          break;
        }
      } /* end of SQLM_ELM_STMT */ else if (logicalGroup == SQLM_ELM_SUBSECTION) {
        /* there can be multiple subsections and nodes */
        switch (pHeader->element) {
        case SQLM_ELM_TQ_ROWS_READ:
          tempListData->tq_rows_read += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TQ_ROWS_WRITTEN:
          tempListData->tq_rows_written += *(sqluint64 * )pData;
          break;
        }
      } /* end of SQLM_ELM_SUBSECTION */ else if (logicalGroup == SQLM_ELM_ROLLBACK_PROGRESS) {
        switch (pHeader->element) {
        case SQLM_ELM_PROGRESS_TOTAL_UNITS:
          tempListData->rollback_progress_total += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_PROGRESS_COMPLETED_UNITS:
          tempListData->rollback_progress_completed += *(sqluint64 * )pData;
          break;
        }
      } /* end of SQLM_ELM_ROLLBACK_PROGRESS */ else if (logicalGroup == SQLM_ELM_MEMORY_POOL 
          && primaryLogicalGroup == SQLM_ELM_APPL) {
        /* -this memory pool information is from appl snapshot
          instead of database snapshot output
         -appl snapshot reports private memory usage 
          of each application
      */
        switch (pHeader->element) {
        case SQLM_ELM_POOL_CUR_SIZE:
          tempListData->privagent_memusg += *(sqluint64 * )pData;
          header->db_memusg += *(sqluint64 * )pData;
          break;
        }
      } /* end of SQLM_ELM_MEMORY_POOL */


      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_APPLS */


/****************************************************************************/
/* preparse_monitor_stream_AGENTID_SS                                       */
/*                                                                          */
/****************************************************************************/
sqlm_header_info *preparse_monitor_stream_AGENTID_SS  (Header  *header,
char *pStart,
char *pEnd,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  sqlm_header_info * pStmtHeader;
  char  *pData;
  char buffer[1024];
  sqluint32 myLogicalGroup = 0;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      /* nested logical grouping */
      pStmtHeader = pHeader;
      pHeader = preparse_monitor_stream_AGENTID_SS(header,pData, pNewEnd, pHeader->element);
      if (pStmtHeader->element == SQLM_ELM_STMT && header->section_number_found && header->node_number_found)
        header->pStmtStart = (char *) pStmtHeader;
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_APPL_INFO) {
        if (pHeader->element == SQLM_ELM_SEQUENCE_NO) {
          strncpy(buffer, pData, SQLM_SEQ_SZ);
          if ( strncmp ((peek_snapReq(header))->sequence_no , buffer, SQLM_SEQ_SZ) == 0)
            header->sequence_no_found = TRUE;
        }
      } else if (logicalGroup == SQLM_ELM_STMT) {
        if (pHeader->element == SQLM_ELM_STMT_NODE_NUMBER) {
          if ( header->sequence_no_found  
              && ((peek_snapReq(header))->stmt_node_number == *(sqluint16 *)pData)
              )
            header->node_number_found = TRUE;
        }
        if (pHeader->element == SQLM_ELM_SECTION_NUMBER) {
          if ( header->sequence_no_found  
              && ((peek_snapReq(header))->stmt_section_number == *(sqluint32 *)pData)
              )
            header->section_number_found = TRUE;
        }
      }

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* preparse_monitor_stream_AGENTID_SS */

/****************************************************************************/
/* parse_monitor_stream_AGENTID_SS                                          */
/*                                                                          */
/****************************************************************************/
sqlm_header_info *parse_monitor_stream_AGENTID_SS   (Header  *header,
DB2List   *list,
Appl_SS   *tempListData, 
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData, temp[USERID_SZ +1];
  DB2ListElmt  *element;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      /* *************************************************
        -start of an application subsection 
        -possibly add data to the list
        -reset temp data to erase old values
        -keep track of type of snapshot output
         using primaryLogicalGroup 
      *****************************************************/
      if (pHeader->element == SQLM_ELM_SUBSECTION) {
        primaryLogicalGroup = SQLM_ELM_SUBSECTION;
        if (tempListData->ss_node_number < 1000) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_appl_ss(db2list_data(element), tempListData);
          } else {
            Appl_SS * newApplData = init_appl_ss();
            copy_appl_ss(newApplData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newApplData);
          }
          reset_appl_ss(tempListData);
        }
      } else if (pHeader->element == SQLM_ELM_DBASE)
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_APPL)
        primaryLogicalGroup = SQLM_ELM_APPL;
      else if (pHeader->element == SQLM_ELM_COLLECTED)
        primaryLogicalGroup = SQLM_ELM_COLLECTED;


      /* nested logical grouping */
      pHeader = parse_monitor_stream_AGENTID_SS(header, list, tempListData
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_SS_USR_CPU_TIME ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->ss_ucpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->ss_ucpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } else if (logicalGroup == SQLM_ELM_SS_SYS_CPU_TIME ) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->ss_scpu_used).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->ss_scpu_used).microsec = *(sqluint32 * )pData;
          break;
        }
      } /* end of SQLM_ELM_SS_CPU_TIME */ 
          else if (logicalGroup == SQLM_ELM_SUBSECTION) {
        switch (pHeader->element) {
        case SQLM_ELM_ROWS_READ:
          tempListData->ss_rows_read = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_ROWS_WRITTEN:
          tempListData->ss_rows_written = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TQ_ROWS_READ:
          tempListData->tq_rows_read = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_TQ_ROWS_WRITTEN:
          tempListData->tq_rows_written = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_SS_EXEC_TIME:
          tempListData->ss_exec_time = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_SS_NUMBER:
          tempListData->ss_number = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_SS_STATUS:
          tempListData->ss_status = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_SS_NODE_NUMBER:
          tempListData->ss_node_number = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_TQ_TOT_SEND_SPILLS:
          tempListData->tq_tot_spills = *(sqluint32 *) pData;
          break;
        case SQLM_ELM_TQ_CUR_SEND_SPILLS:
          tempListData->tq_cur_spills = *(sqluint32 *) pData;
          break;
        case SQLM_ELM_TQ_ID_WAITING_ON:
          tempListData->tq_id_waiting_on = *(sqluint16 *) pData;
          break;
        case SQLM_ELM_TQ_NODE_WAITED_FOR:
          tempListData->tq_node_waiting_for = *(sqluint16 *) pData;
          break;
        case SQLM_ELM_TQ_WAIT_FOR_ANY:
          tempListData->tq_wait_for_any = *(sqluint16 *) pData;
          break;
        }
      } /* end of SQLM_ELM_SUBSECTION */ 
          else if (primaryLogicalGroup == SQLM_ELM_SUBSECTION &&
          logicalGroup == SQLM_ELM_AGENT) {
        if (pHeader->element == SQLM_ELM_AGENT_PID)
          tempListData->num_agents +=1 ;
      }

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_AGENTID_SS */


/***************************************************************************/
/* InitElementNames                                                        */
/* Initialize the array of element names based on the defines in sqlmon.h. */
/***************************************************************************/
int  init_element_names(char *elementName[])
{
  int  rc = 0;
  int  arraySize = NUMELEMENTS *sizeof(elementName[0]);
  char  *pArray = (char *) &elementName[0];

  /* zero the entire array to ensure unset values are null */
  strncpy(pArray, "", arraySize);

  /* set the individual element names (they are defined in sqlmon.h) */
  elementName[1] = "SQLM_ELM_DB2";
  elementName[2] = "SQLM_ELM_FCM";
  elementName[3] = "SQLM_ELM_FCM_NODE";
  elementName[4] = "SQLM_ELM_APPL_INFO";
  elementName[5] = "SQLM_ELM_APPL";
  elementName[6] = "SQLM_ELM_DCS_APPL_INFO";
  elementName[7] = "SQLM_ELM_DCS_APPL";
  elementName[8] = "SQLM_ELM_DCS_STMT";
  elementName[9] = "SQLM_ELM_SUBSECTION";
  elementName[10] = "SQLM_ELM_AGENT";
  elementName[11] = "SQLM_ELM_LOCK_WAIT";
  elementName[12] = "SQLM_ELM_DCS_DBASE";
  elementName[13] = "SQLM_ELM_DBASE";
  elementName[14] = "SQLM_ELM_ROLLFORWARD";
  elementName[15] = "SQLM_ELM_TABLE";
  elementName[16] = "SQLM_ELM_LOCK";
  elementName[17] = "SQLM_ELM_TABLESPACE";
  elementName[18] = "SQLM_ELM_BUFFERPOOL";
  elementName[19] = "SQLM_ELM_DYNSQL";
  elementName[20] = "SQLM_ELM_COLLECTED";
  elementName[21] = "SQLM_ELM_SWITCH_LIST";
  elementName[22] = "SQLM_ELM_UOW_SW";
  elementName[23] = "SQLM_ELM_STATEMENT_SW";
  elementName[24] = "SQLM_ELM_TABLE_SW";
  elementName[25] = "SQLM_ELM_BUFFPOOL_SW";
  elementName[26] = "SQLM_ELM_LOCK_SW";
  elementName[27] = "SQLM_ELM_SORT_SW";
  elementName[28] = "SQLM_ELM_TABLE_LIST";
  elementName[29] = "SQLM_ELM_TABLESPACE_LIST";
  elementName[30] = "SQLM_ELM_DYNSQL_LIST";
  elementName[31] = "SQLM_ELM_APPL_LOCK_LIST";
  elementName[32] = "SQLM_ELM_DB_LOCK_LIST";
  elementName[33] = "SQLM_ELM_STMT";
  elementName[34] = "SQLM_ELM_DBASE_REMOTE";
  elementName[35] = "SQLM_ELM_APPL_REMOTE";
  elementName[36] = "SQLM_ELM_APPL_ID_INFO";
  elementName[37] = "SQLM_ELM_STMT_TRANSMISSIONS";
  elementName[38] = "SQLM_ELM_TIMESTAMP_SW";
  elementName[39] = "SQLM_ELM_TABLE_REORG";
  elementName[40] = "SQLM_ELM_MEMORY_POOL";
  elementName[41] = "SQLM_ELM_TABLESPACE_QUIESCER";
  elementName[42] = "SQLM_ELM_TABLESPACE_CONTAINER";
  elementName[43] = "SQLM_ELM_TABLESPACE_RANGE";
  elementName[44] = "SQLM_ELM_TABLESPACE_RANGE_CONTAINER";
  elementName[45] = "SQLM_ELM_TABLESPACE_NODEINFO";
  elementName[46] = "SQLM_ELM_HEALTH_INDICATOR";
  elementName[47] = "SQLM_ELM_HEALTH_INDICATOR_HIST";
  elementName[48] = "SQLM_ELM_BUFFERPOOL_NODEINFO";
  elementName[49] = "SQLM_ELM_UTILITY";
  elementName[50] = "SQLM_ELM_HI_OBJ_LIST";
  elementName[51] = "SQLM_ELM_HI_OBJ_LIST_HIST";
  elementName[52] = "SQLM_ELM_PROGRESS";
  elementName[53] = "SQLM_ELM_PROGRESS_LIST";
  elementName[54] = "SQLM_ELM_HADR";
  elementName[55] = "SQLM_ELM_DETAIL_LOG";
  elementName[56] = "SQLM_ELM_ROLLBACK_PROGRESS";
  elementName[57] = "SQLM_ELM_DB_STORAGE_GROUP";
  elementName[58] = "SQLM_ELM_DB_STO_PATH_INFO";
  elementName[59] = "SQLM_ELM_MEMORY_POOL_LIST";
  elementName[59] = "SQLM_MAX_LOGICAL_ELEMENT";
  elementName[100] = "SQLM_ELM_EVENT_DB";
  elementName[101] = "SQLM_ELM_EVENT_CONN";
  elementName[102] = "SQLM_ELM_EVENT_TABLE";
  elementName[103] = "SQLM_ELM_EVENT_STMT";
  elementName[104] = "SQLM_ELM_EVENT_XACT";
  elementName[105] = "SQLM_ELM_EVENT_DEADLOCK";
  elementName[106] = "SQLM_ELM_EVENT_DLCONN";
  elementName[107] = "SQLM_ELM_EVENT_TABLESPACE";
  elementName[108] = "SQLM_ELM_EVENT_DBHEADER";
  elementName[109] = "SQLM_ELM_EVENT_START";
  elementName[110] = "SQLM_ELM_EVENT_CONNHEADER";
  elementName[111] = "SQLM_ELM_EVENT_OVERFLOW";
  elementName[112] = "SQLM_ELM_EVENT_BUFFERPOOL";
  elementName[113] = "SQLM_ELM_EVENT_SUBSECTION";
  elementName[114] = "SQLM_ELM_EVENT_LOG_HEADER";
  elementName[115] = "SQLM_ELM_EVENT_CONTROL";
  elementName[116] = "SQLM_ELM_EVENT_LOCK_LIST";
  elementName[117] = "SQLM_ELM_EVENT_DETAILED_DLCONN";
  elementName[118] = "SQLM_ELM_EVENT_CONNMEMUSE";
  elementName[119] = "SQLM_ELM_EVENT_DBMEMUSE";
  elementName[120] = "SQLM_ELM_EVENT_STMT_HISTORY";
  elementName[121] = "SQLM_ELM_EVENT_DATA_VALUE";
  elementName[122] = "SQLM_ELM_EVENT_ACTIVITY";
  elementName[123] = "SQLM_ELM_EVENT_ACTIVITYSTMT";
  elementName[124] = "SQLM_ELM_EVENT_ACTIVITYVALS";
  elementName[125] = "SQLM_ELM_EVENT_SCSTATS";
  elementName[126] = "SQLM_ELM_EVENT_WCSTATS";
  elementName[127] = "SQLM_ELM_EVENT_WLSTATS";
  elementName[128] = "SQLM_ELM_EVENT_QSTATS";
  elementName[129] = "SQLM_ELM_EVENT_HISTOGRAMBIN";
  elementName[130] = "SQLM_ELM_EVENT_THRESHOLD_VIOLATIONS";
  elementName[200] = "SQLM_ELM_TIME_STAMP";
  elementName[201] = "SQLM_ELM_STATUS_CHANGE_TIME";
  elementName[202] = "SQLM_ELM_GW_CON_TIME";
  elementName[203] = "SQLM_ELM_PREV_UOW_STOP_TIME";
  elementName[204] = "SQLM_ELM_UOW_START_TIME";
  elementName[205] = "SQLM_ELM_UOW_STOP_TIME";
  elementName[206] = "SQLM_ELM_STMT_START";
  elementName[207] = "SQLM_ELM_STMT_STOP";
  elementName[208] = "SQLM_ELM_LAST_RESET";
  elementName[209] = "SQLM_ELM_DB2START_TIME";
  elementName[210] = "SQLM_ELM_DB_CONN_TIME";
  elementName[211] = "SQLM_ELM_LAST_BACKUP";
  elementName[212] = "SQLM_ELM_LOCK_WAIT_START_TIME";
  elementName[213] = "SQLM_ELM_APPL_CON_TIME";
  elementName[214] = "SQLM_ELM_CONN_COMPLETE_TIME";
  elementName[215] = "SQLM_ELM_DISCONN_TIME";
  elementName[216] = "SQLM_ELM_EVENT_TIME";
  elementName[217] = "SQLM_ELM_START_TIME";
  elementName[218] = "SQLM_ELM_STOP_TIME";
  elementName[219] = "SQLM_ELM_RF_TIMESTAMP";
  elementName[220] = "SQLM_ELM_CONN_TIME";
  elementName[221] = "SQLM_ELM_FIRST_OVERFLOW_TIME";
  elementName[222] = "SQLM_ELM_LAST_OVERFLOW_TIME";
  elementName[223] = "SQLM_ELM_GW_EXEC_TIME";
  elementName[224] = "SQLM_ELM_AGENT_USR_CPU_TIME";
  elementName[225] = "SQLM_ELM_AGENT_SYS_CPU_TIME";
  elementName[226] = "SQLM_ELM_SS_USR_CPU_TIME";
  elementName[227] = "SQLM_ELM_SS_SYS_CPU_TIME";
  elementName[228] = "SQLM_ELM_USER_CPU_TIME";
  elementName[229] = "SQLM_ELM_TOTAL_EXEC_TIME";
  elementName[230] = "SQLM_ELM_SWITCH_SET_TIME";
  elementName[231] = "SQLM_ELM_ELAPSED_EXEC_TIME";
  elementName[232] = "SQLM_ELM_SELECT_TIME";
  elementName[233] = "SQLM_ELM_INSERT_TIME";
  elementName[234] = "SQLM_ELM_UPDATE_TIME";
  elementName[235] = "SQLM_ELM_DELETE_TIME";
  elementName[236] = "SQLM_ELM_CREATE_NICKNAME_TIME";
  elementName[237] = "SQLM_ELM_PASSTHRU_TIME";
  elementName[238] = "SQLM_ELM_STORED_PROC_TIME";
  elementName[239] = "SQLM_ELM_REMOTE_LOCK_TIME";
  elementName[240] = "SQLM_ELM_NETWORK_TIME_TOP";
  elementName[241] = "SQLM_ELM_NETWORK_TIME_BOTTOM";
  elementName[242] = "SQLM_ELM_TABLESPACE_REBALANCER_START_TIME";
  elementName[243] = "SQLM_ELM_TABLESPACE_REBALANCER_RESTART_TIME";
  elementName[244] = "SQLM_ELM_TABLESPACE_MIN_RECOVERY_TIME";
  elementName[245] = "SQLM_ELM_HI_TIMESTAMP";
  elementName[245] = "SQLM_MAX_TIME_STAMP";
  elementName[300] = "SQLM_ELM_SECONDS";
  elementName[301] = "SQLM_ELM_MICROSEC";
  elementName[302] = "SQLM_ELM_AGENT_ID";
  elementName[303] = "SQLM_ELM_SERVER_DB2_TYPE";
  elementName[304] = "SQLM_ELM_SERVER_PRDID";
  elementName[305] = "SQLM_ELM_SERVER_NNAME";
  elementName[306] = "SQLM_ELM_SERVER_INSTANCE_NAME";
  elementName[307] = "SQLM_ELM_NODE_NUMBER";
  elementName[308] = "SQLM_ELM_TIME_ZONE_DISP";
  elementName[309] = "SQLM_ELM_SERVER_VERSION";
  elementName[310] = "SQLM_ELM_APPL_STATUS";
  elementName[311] = "SQLM_ELM_CODEPAGE_ID";
  elementName[312] = "SQLM_ELM_STMT_TEXT";
  elementName[313] = "SQLM_ELM_APPL_NAME";
  elementName[314] = "SQLM_ELM_APPL_ID";
  elementName[315] = "SQLM_ELM_SEQUENCE_NO";
  elementName[316] = "SQLM_ELM_AUTH_ID";
  elementName[316] = "SQLM_ELM_PRIMARY_AUTH_ID";
  elementName[317] = "SQLM_ELM_CLIENT_NNAME";
  elementName[318] = "SQLM_ELM_CLIENT_PRDID";
  elementName[319] = "SQLM_ELM_INPUT_DB_ALIAS";
  elementName[320] = "SQLM_ELM_CLIENT_DB_ALIAS";
  elementName[321] = "SQLM_ELM_DB_NAME";
  elementName[322] = "SQLM_ELM_DB_PATH";
  elementName[323] = "SQLM_ELM_NUM_ASSOC_AGENTS";
  elementName[324] = "SQLM_ELM_COORD_NODE_NUM";
  elementName[325] = "SQLM_ELM_AUTHORITY_LVL";
  elementName[326] = "SQLM_ELM_EXECUTION_ID";
  elementName[327] = "SQLM_ELM_CORR_TOKEN";
  elementName[328] = "SQLM_ELM_CLIENT_PID";
  elementName[329] = "SQLM_ELM_CLIENT_PLATFORM";
  elementName[330] = "SQLM_ELM_CLIENT_PROTOCOL";
  elementName[331] = "SQLM_ELM_COUNTRY_CODE";
  elementName[331] = "SQLM_ELM_TERRITORY_CODE";
  elementName[332] = "SQLM_ELM_COORD_AGENT_PID";
  elementName[333] = "SQLM_ELM_GW_DB_ALIAS";
  elementName[334] = "SQLM_ELM_OUTBOUND_COMM_ADDRESS";
  elementName[335] = "SQLM_ELM_INBOUND_COMM_ADDRESS";
  elementName[336] = "SQLM_ELM_OUTBOUND_COMM_PROTOCOL";
  elementName[337] = "SQLM_ELM_DCS_DB_NAME";
  elementName[338] = "SQLM_ELM_HOST_DB_NAME";
  elementName[339] = "SQLM_ELM_HOST_PRDID";
  elementName[340] = "SQLM_ELM_OUTBOUND_APPL_ID";
  elementName[341] = "SQLM_ELM_OUTBOUND_SEQUENCE_NO";
  elementName[342] = "SQLM_ELM_DCS_APPL_STATUS";
  elementName[343] = "SQLM_ELM_HOST_CCSID";
  elementName[344] = "SQLM_ELM_OUTPUT_STATE";
  elementName[345] = "SQLM_ELM_COUNT";
  elementName[346] = "SQLM_ELM_ROWS_SELECTED";
  elementName[347] = "SQLM_ELM_SQL_STMTS";
  elementName[348] = "SQLM_ELM_FAILED_SQL_STMTS";
  elementName[349] = "SQLM_ELM_COMMIT_SQL_STMTS";
  elementName[350] = "SQLM_ELM_ROLLBACK_SQL_STMTS";
  elementName[351] = "SQLM_ELM_INBOUND_BYTES_RECEIVED";
  elementName[352] = "SQLM_ELM_OUTBOUND_BYTES_SENT";
  elementName[353] = "SQLM_ELM_OUTBOUND_BYTES_RECEIVED";
  elementName[354] = "SQLM_ELM_INBOUND_BYTES_SENT";
  elementName[355] = "SQLM_ELM_STMT_OPERATION";
  elementName[356] = "SQLM_ELM_SECTION_NUMBER";
  elementName[357] = "SQLM_ELM_LOCK_NODE";
  elementName[358] = "SQLM_ELM_CREATOR";
  elementName[359] = "SQLM_ELM_PACKAGE_NAME";
  elementName[360] = "SQLM_ELM_APPL_IDLE_TIME";
  elementName[361] = "SQLM_ELM_OPEN_CURSORS";
  elementName[362] = "SQLM_ELM_UOW_COMP_STATUS";
  elementName[363] = "SQLM_ELM_SEQUENCE_NO_HOLDING_LK";
  elementName[364] = "SQLM_ELM_ROLLED_BACK_AGENT_ID";
  elementName[365] = "SQLM_ELM_ROLLED_BACK_APPL_ID";
  elementName[366] = "SQLM_ELM_ROLLED_BACK_SEQUENCE_NO";
  elementName[367] = "SQLM_ELM_XID";
  elementName[368] = "SQLM_ELM_TPMON_CLIENT_USERID";
  elementName[369] = "SQLM_ELM_TPMON_CLIENT_WKSTN";
  elementName[370] = "SQLM_ELM_TPMON_CLIENT_APP";
  elementName[371] = "SQLM_ELM_TPMON_ACC_STR";
  elementName[372] = "SQLM_ELM_QUERY_COST_ESTIMATE";
  elementName[373] = "SQLM_ELM_QUERY_CARD_ESTIMATE";
  elementName[374] = "SQLM_ELM_FETCH_COUNT";
  /* ROWS_RETURNED is an alias of FETCH_COUNT    */
  /* elementName[374] = "SQLM_ELM_ROWS_RETURNED" */
  elementName[375] = "SQLM_ELM_GW_TOTAL_CONS";
  elementName[376] = "SQLM_ELM_GW_CUR_CONS";
  elementName[377] = "SQLM_ELM_GW_CONS_WAIT_HOST";
  elementName[378] = "SQLM_ELM_GW_CONS_WAIT_CLIENT";
  elementName[379] = "SQLM_ELM_GW_CONNECTIONS_TOP";
  elementName[380] = "SQLM_ELM_SORT_HEAP_ALLOCATED";
  elementName[381] = "SQLM_ELM_POST_THRESHOLD_SORTS";
  elementName[382] = "SQLM_ELM_PIPED_SORTS_REQUESTED";
  elementName[383] = "SQLM_ELM_PIPED_SORTS_ACCEPTED";
  elementName[384] = "SQLM_ELM_DL_CONNS";
  elementName[385] = "SQLM_ELM_REM_CONS_IN";
  elementName[386] = "SQLM_ELM_REM_CONS_IN_EXEC";
  elementName[387] = "SQLM_ELM_LOCAL_CONS";
  elementName[388] = "SQLM_ELM_LOCAL_CONS_IN_EXEC";
  elementName[389] = "SQLM_ELM_CON_LOCAL_DBASES";
  elementName[390] = "SQLM_ELM_AGENTS_REGISTERED";
  elementName[391] = "SQLM_ELM_AGENTS_WAITING_ON_TOKEN";
  elementName[392] = "SQLM_ELM_DB2_STATUS";
  elementName[393] = "SQLM_ELM_AGENTS_REGISTERED_TOP";
  elementName[394] = "SQLM_ELM_AGENTS_WAITING_TOP";
  elementName[395] = "SQLM_ELM_COMM_PRIVATE_MEM";
  elementName[396] = "SQLM_ELM_IDLE_AGENTS";
  elementName[397] = "SQLM_ELM_AGENTS_FROM_POOL";
  elementName[398] = "SQLM_ELM_AGENTS_CREATED_EMPTY_POOL";
  elementName[399] = "SQLM_ELM_AGENTS_TOP";
  elementName[400] = "SQLM_ELM_COORD_AGENTS_TOP";
  elementName[401] = "SQLM_ELM_MAX_AGENT_OVERFLOWS";
  elementName[402] = "SQLM_ELM_AGENTS_STOLEN";
  elementName[403] = "SQLM_ELM_PRODUCT_NAME";
  elementName[404] = "SQLM_ELM_COMPONENT_ID";
  elementName[405] = "SQLM_ELM_SERVICE_LEVEL";
  elementName[406] = "SQLM_ELM_POST_THRESHOLD_HASH_JOINS";
  elementName[407] = "SQLM_ELM_BUFF_FREE";
  elementName[408] = "SQLM_ELM_BUFF_FREE_BOTTOM";
  elementName[409] = "SQLM_ELM_MA_FREE";
  elementName[410] = "SQLM_ELM_MA_FREE_BOTTOM";
  elementName[411] = "SQLM_ELM_CE_FREE";
  elementName[412] = "SQLM_ELM_CE_FREE_BOTTOM";
  elementName[413] = "SQLM_ELM_RB_FREE";
  elementName[414] = "SQLM_ELM_RB_FREE_BOTTOM";
  elementName[416] = "SQLM_ELM_CONNECTION_STATUS";
  elementName[417] = "SQLM_ELM_TOTAL_BUFFERS_SENT";
  elementName[418] = "SQLM_ELM_TOTAL_BUFFERS_RCVD";
  elementName[419] = "SQLM_ELM_LOCKS_HELD";
  elementName[420] = "SQLM_ELM_LOCK_WAITS";
  elementName[421] = "SQLM_ELM_LOCK_WAIT_TIME";
  elementName[422] = "SQLM_ELM_LOCK_LIST_IN_USE";
  elementName[423] = "SQLM_ELM_DEADLOCKS";
  elementName[424] = "SQLM_ELM_LOCK_ESCALS";
  elementName[425] = "SQLM_ELM_X_LOCK_ESCALS";
  elementName[426] = "SQLM_ELM_LOCKS_WAITING";
  elementName[427] = "SQLM_ELM_TOTAL_SORTS";
  elementName[428] = "SQLM_ELM_TOTAL_SORT_TIME";
  elementName[429] = "SQLM_ELM_SORT_OVERFLOWS";
  elementName[430] = "SQLM_ELM_ACTIVE_SORTS";
  elementName[431] = "SQLM_ELM_POOL_DATA_L_READS";
  elementName[432] = "SQLM_ELM_POOL_DATA_P_READS";
  elementName[433] = "SQLM_ELM_POOL_DATA_WRITES";
  elementName[434] = "SQLM_ELM_POOL_INDEX_L_READS";
  elementName[435] = "SQLM_ELM_POOL_INDEX_P_READS";
  elementName[436] = "SQLM_ELM_POOL_INDEX_WRITES";
  elementName[437] = "SQLM_ELM_POOL_READ_TIME";
  elementName[438] = "SQLM_ELM_POOL_WRITE_TIME";
  elementName[439] = "SQLM_ELM_FILES_CLOSED";
  elementName[440] = "SQLM_ELM_DYNAMIC_SQL_STMTS";
  elementName[441] = "SQLM_ELM_STATIC_SQL_STMTS";
  elementName[442] = "SQLM_ELM_SELECT_SQL_STMTS";
  elementName[443] = "SQLM_ELM_DDL_SQL_STMTS";
  elementName[444] = "SQLM_ELM_UID_SQL_STMTS";
  elementName[445] = "SQLM_ELM_INT_AUTO_REBINDS";
  elementName[446] = "SQLM_ELM_INT_ROWS_DELETED";
  elementName[447] = "SQLM_ELM_INT_ROWS_UPDATED";
  elementName[448] = "SQLM_ELM_INT_COMMITS";
  elementName[449] = "SQLM_ELM_INT_ROLLBACKS";
  elementName[450] = "SQLM_ELM_INT_DEADLOCK_ROLLBACKS";
  elementName[451] = "SQLM_ELM_ROWS_DELETED";
  elementName[452] = "SQLM_ELM_ROWS_INSERTED";
  elementName[453] = "SQLM_ELM_ROWS_UPDATED";
  elementName[454] = "SQLM_ELM_BINDS_PRECOMPILES";
  elementName[455] = "SQLM_ELM_LOCKS_HELD_TOP";
  elementName[456] = "SQLM_ELM_NUM_NODES_IN_DB2_INSTANCE";
  elementName[457] = "SQLM_ELM_TOTAL_CONS";
  elementName[458] = "SQLM_ELM_APPLS_CUR_CONS";
  elementName[459] = "SQLM_ELM_APPLS_IN_DB2";
  elementName[460] = "SQLM_ELM_SEC_LOG_USED_TOP";
  elementName[461] = "SQLM_ELM_TOT_LOG_USED_TOP";
  elementName[462] = "SQLM_ELM_SEC_LOGS_ALLOCATED";
  elementName[463] = "SQLM_ELM_POOL_ASYNC_INDEX_READS";
  elementName[464] = "SQLM_ELM_POOL_DATA_TO_ESTORE";
  elementName[465] = "SQLM_ELM_POOL_INDEX_TO_ESTORE";
  elementName[466] = "SQLM_ELM_POOL_INDEX_FROM_ESTORE";
  elementName[467] = "SQLM_ELM_POOL_DATA_FROM_ESTORE";
  elementName[468] = "SQLM_ELM_DB_STATUS";
  elementName[469] = "SQLM_ELM_LOCK_TIMEOUTS";
  elementName[470] = "SQLM_ELM_CONNECTIONS_TOP";
  elementName[471] = "SQLM_ELM_DB_HEAP_TOP";
  elementName[472] = "SQLM_ELM_POOL_ASYNC_DATA_READS";
  elementName[473] = "SQLM_ELM_POOL_ASYNC_DATA_WRITES";
  elementName[474] = "SQLM_ELM_POOL_ASYNC_INDEX_WRITES";
  elementName[475] = "SQLM_ELM_POOL_ASYNC_READ_TIME";
  elementName[476] = "SQLM_ELM_POOL_ASYNC_WRITE_TIME";
  elementName[477] = "SQLM_ELM_POOL_ASYNC_DATA_READ_REQS";
  elementName[478] = "SQLM_ELM_POOL_LSN_GAP_CLNS";
  elementName[479] = "SQLM_ELM_POOL_DRTY_PG_STEAL_CLNS";
  elementName[480] = "SQLM_ELM_POOL_DRTY_PG_THRSH_CLNS";
  elementName[481] = "SQLM_ELM_DIRECT_READS";
  elementName[482] = "SQLM_ELM_DIRECT_WRITES";
  elementName[483] = "SQLM_ELM_DIRECT_READ_REQS";
  elementName[484] = "SQLM_ELM_DIRECT_WRITE_REQS";
  elementName[485] = "SQLM_ELM_DIRECT_READ_TIME";
  elementName[486] = "SQLM_ELM_DIRECT_WRITE_TIME";
  elementName[487] = "SQLM_ELM_INT_ROWS_INSERTED";
  elementName[488] = "SQLM_ELM_LOG_READS";
  elementName[489] = "SQLM_ELM_LOG_WRITES";
  elementName[490] = "SQLM_ELM_PKG_CACHE_LOOKUPS";
  elementName[491] = "SQLM_ELM_PKG_CACHE_INSERTS";
  elementName[492] = "SQLM_ELM_CAT_CACHE_LOOKUPS";
  elementName[493] = "SQLM_ELM_CAT_CACHE_INSERTS";
  elementName[494] = "SQLM_ELM_CAT_CACHE_OVERFLOWS";
  elementName[495] = "SQLM_ELM_CAT_CACHE_HEAP_FULL";
  elementName[496] = "SQLM_ELM_CATALOG_NODE";
  elementName[497] = "SQLM_ELM_TOTAL_SEC_CONS";
  elementName[498] = "SQLM_ELM_DB_LOCATION";
  elementName[499] = "SQLM_ELM_SERVER_PLATFORM";
  elementName[500] = "SQLM_ELM_CATALOG_NODE_NAME";
  elementName[501] = "SQLM_ELM_PREFETCH_WAIT_TIME";
  elementName[502] = "SQLM_ELM_APPL_SECTION_LOOKUPS";
  elementName[503] = "SQLM_ELM_APPL_SECTION_INSERTS";
  elementName[504] = "SQLM_ELM_TOTAL_HASH_JOINS";
  elementName[505] = "SQLM_ELM_TOTAL_HASH_LOOPS";
  elementName[506] = "SQLM_ELM_HASH_JOIN_OVERFLOWS";
  elementName[507] = "SQLM_ELM_HASH_JOIN_SMALL_OVERFLOWS";
  elementName[508] = "SQLM_ELM_UOW_LOCK_WAIT_TIME";
  elementName[509] = "SQLM_ELM_STMT_TYPE";
  elementName[510] = "SQLM_ELM_CURSOR_NAME";
  elementName[511] = "SQLM_ELM_UOW_LOG_SPACE_USED";
  elementName[512] = "SQLM_ELM_OPEN_REM_CURS";
  elementName[513] = "SQLM_ELM_OPEN_REM_CURS_BLK";
  elementName[514] = "SQLM_ELM_REJ_CURS_BLK";
  elementName[515] = "SQLM_ELM_ACC_CURS_BLK";
  elementName[516] = "SQLM_ELM_VERSION";
  elementName[517] = "SQLM_ELM_EVENT_MONITOR_NAME";
  elementName[518] = "SQLM_ELM_SQL_REQS_SINCE_COMMIT";
  elementName[520] = "SQLM_ELM_BYTE_ORDER";
  elementName[521] = "SQLM_ELM_PREP_TIME_WORST";
  elementName[522] = "SQLM_ELM_ROWS_READ";
  /* ROWS_FETCHED is an alias of ROWS_READ      */
  /* elementName[522] = "SQLM_ELM_ROWS_FETCHED" */
  elementName[523] = "SQLM_ELM_ROWS_WRITTEN";
  elementName[523] = "SQLM_ELM_ROWS_MODIFIED";
  elementName[524] = "SQLM_ELM_OPEN_LOC_CURS";
  elementName[525] = "SQLM_ELM_OPEN_LOC_CURS_BLK";
  elementName[526] = "SQLM_ELM_COORD_NODE";
  elementName[526] = "SQLM_ELM_COORD_PARTITION_NUM";
  elementName[527] = "SQLM_ELM_NUM_AGENTS";
  elementName[528] = "SQLM_ELM_ASSOCIATED_AGENTS_TOP";
  elementName[529] = "SQLM_ELM_APPL_PRIORITY";
  elementName[530] = "SQLM_ELM_APPL_PRIORITY_TYPE";
  elementName[531] = "SQLM_ELM_DEGREE_PARALLELISM";
  elementName[532] = "SQLM_ELM_STMT_SORTS";
  elementName[533] = "SQLM_ELM_STMT_USR_CPU_TIME";
  elementName[534] = "SQLM_ELM_STMT_SYS_CPU_TIME";
  elementName[535] = "SQLM_ELM_SS_NUMBER";
  elementName[536] = "SQLM_ELM_SS_STATUS";
  elementName[537] = "SQLM_ELM_SS_NODE_NUMBER";
  elementName[538] = "SQLM_ELM_SS_EXEC_TIME";
  elementName[539] = "SQLM_ELM_PREP_TIME_BEST";
  elementName[540] = "SQLM_ELM_NUM_COMPILATIONS";
  elementName[541] = "SQLM_ELM_TQ_NODE_WAITED_FOR";
  elementName[542] = "SQLM_ELM_TQ_WAIT_FOR_ANY";
  elementName[543] = "SQLM_ELM_TQ_ID_WAITING_ON";
  elementName[544] = "SQLM_ELM_TQ_TOT_SEND_SPILLS";
  elementName[545] = "SQLM_ELM_TQ_CUR_SEND_SPILLS";
  elementName[546] = "SQLM_ELM_TQ_MAX_SEND_SPILLS";
  elementName[547] = "SQLM_ELM_TQ_ROWS_READ";
  elementName[548] = "SQLM_ELM_TQ_ROWS_WRITTEN";
  elementName[549] = "SQLM_ELM_AGENT_PID";
  elementName[550] = "SQLM_ELM_LOCK_ESCALATION";
  elementName[551] = "SQLM_ELM_SUBSECTION_NUMBER";
  elementName[552] = "SQLM_ELM_LOCK_MODE";
  elementName[553] = "SQLM_ELM_LOCK_OBJECT_TYPE";
  elementName[554] = "SQLM_ELM_NUM_EXECUTIONS";
  elementName[555] = "SQLM_ELM_TABLE_NAME";
  elementName[556] = "SQLM_ELM_TABLE_SCHEMA";
  elementName[557] = "SQLM_ELM_TABLESPACE_NAME";
  elementName[558] = "SQLM_ELM_AGENT_ID_HOLDING_LK";
  elementName[559] = "SQLM_ELM_APPL_ID_HOLDING_LK";
  elementName[561] = "SQLM_ELM_TABLE_FILE_ID";
  elementName[562] = "SQLM_ELM_TABLE_TYPE";
  elementName[563] = "SQLM_ELM_OVERFLOW_ACCESSES";
  elementName[564] = "SQLM_ELM_PAGE_REORGS";
  elementName[565] = "SQLM_ELM_SQLCABC";
  elementName[566] = "SQLM_ELM_LOCK_STATUS";
  elementName[567] = "SQLM_ELM_LOCK_OBJECT_NAME";
  elementName[568] = "SQLM_ELM_RF_TYPE";
  elementName[569] = "SQLM_ELM_RF_LOG_NUM";
  elementName[570] = "SQLM_ELM_RF_STATUS";
  elementName[571] = "SQLM_ELM_TS_NAME";
  elementName[572] = "SQLM_ELM_BP_NAME";
  elementName[573] = "SQLM_ELM_STMT_NODE_NUMBER";
  elementName[574] = "SQLM_ELM_PARTIAL_RECORD";
  elementName[575] = "SQLM_ELM_SYSTEM_CPU_TIME";
  elementName[576] = "SQLM_ELM_SQLCA";
  elementName[577] = "SQLM_ELM_SQLCODE";
  elementName[578] = "SQLM_ELM_SQLERRML";
  elementName[579] = "SQLM_ELM_SQLERRMC";
  elementName[580] = "SQLM_ELM_SQLERRP";
  elementName[581] = "SQLM_ELM_SQLERRD";
  elementName[582] = "SQLM_ELM_SQLWARN";
  elementName[583] = "SQLM_ELM_SQLSTATE";
  elementName[584] = "SQLM_ELM_UOW_STATUS";
  elementName[585] = "SQLM_ELM_TOTAL_SYS_CPU_TIME";
  elementName[586] = "SQLM_ELM_TOTAL_USR_CPU_TIME";
  elementName[587] = "SQLM_ELM_LOCK_MODE_REQUESTED";
  elementName[588] = "SQLM_ELM_INACTIVE_GW_AGENTS";
  elementName[589] = "SQLM_ELM_NUM_GW_CONN_SWITCHES";
  elementName[590] = "SQLM_ELM_GW_COMM_ERRORS";
  elementName[591] = "SQLM_ELM_GW_COMM_ERROR_TIME";
  elementName[592] = "SQLM_ELM_GW_CON_START_TIME";
  elementName[593] = "SQLM_ELM_CON_RESPONSE_TIME";
  elementName[594] = "SQLM_ELM_CON_ELAPSED_TIME";
  elementName[595] = "SQLM_ELM_HOST_RESPONSE_TIME";
  elementName[596] = "SQLM_ELM_PKG_CACHE_NUM_OVERFLOWS";
  elementName[597] = "SQLM_ELM_PKG_CACHE_SIZE_TOP";
  elementName[598] = "SQLM_ELM_APPL_ID_OLDEST_XACT";
  elementName[599] = "SQLM_ELM_TOTAL_LOG_USED";
  elementName[600] = "SQLM_ELM_TOTAL_LOG_AVAILABLE";
  elementName[601] = "SQLM_ELM_STMT_ELAPSED_TIME";
  elementName[602] = "SQLM_ELM_UOW_ELAPSED_TIME";
  elementName[603] = "SQLM_ELM_SQLCAID";
  elementName[604] = "SQLM_ELM_SMALLEST_LOG_AVAIL_NODE";
  elementName[605] = "SQLM_ELM_DISCONNECTS";
  elementName[606] = "SQLM_ELM_CREATE_NICKNAME";
  elementName[607] = "SQLM_ELM_PASSTHRUS";
  elementName[608] = "SQLM_ELM_STORED_PROCS";
  elementName[609] = "SQLM_ELM_SP_ROWS_SELECTED";
  elementName[610] = "SQLM_ELM_DATASOURCE_NAME";
  elementName[611] = "SQLM_ELM_REMOTE_LOCKS";
  elementName[612] = "SQLM_ELM_BLOCKING_CURSOR";
  elementName[613] = "SQLM_ELM_OUTBOUND_BLOCKING_CURSOR";
  elementName[614] = "SQLM_ELM_INSERT_SQL_STMTS";
  elementName[615] = "SQLM_ELM_UPDATE_SQL_STMTS";
  elementName[616] = "SQLM_ELM_DELETE_SQL_STMTS";
  elementName[617] = "SQLM_ELM_UNREAD_PREFETCH_PAGES";
  elementName[618] = "SQLM_ELM_AGENT_STATUS";
  elementName[619] = "SQLM_ELM_NUM_TRANSMISSIONS";
  elementName[620] = "SQLM_ELM_OUTBOUND_BYTES_SENT_TOP";
  elementName[621] = "SQLM_ELM_OUTBOUND_BYTES_RECEIVED_TOP";
  elementName[622] = "SQLM_ELM_OUTBOUND_BYTES_SENT_BOTTOM";
  elementName[623] = "SQLM_ELM_OUTBOUND_BYTES_RECEIVED_BOTTOM";
  elementName[624] = "SQLM_ELM_MAX_DATA_SENT_128";
  elementName[625] = "SQLM_ELM_MAX_DATA_SENT_256";
  elementName[626] = "SQLM_ELM_MAX_DATA_SENT_512";
  elementName[627] = "SQLM_ELM_MAX_DATA_SENT_1024";
  elementName[628] = "SQLM_ELM_MAX_DATA_SENT_2048";
  elementName[629] = "SQLM_ELM_MAX_DATA_SENT_4096";
  elementName[630] = "SQLM_ELM_MAX_DATA_SENT_8192";
  elementName[631] = "SQLM_ELM_MAX_DATA_SENT_16384";
  elementName[632] = "SQLM_ELM_MAX_DATA_SENT_31999";
  elementName[633] = "SQLM_ELM_MAX_DATA_SENT_64000";
  elementName[634] = "SQLM_ELM_MAX_DATA_SENT_GT64000";
  elementName[635] = "SQLM_ELM_MAX_DATA_RECEIVED_128";
  elementName[636] = "SQLM_ELM_MAX_DATA_RECEIVED_256";
  elementName[637] = "SQLM_ELM_MAX_DATA_RECEIVED_512";
  elementName[638] = "SQLM_ELM_MAX_DATA_RECEIVED_1024";
  elementName[639] = "SQLM_ELM_MAX_DATA_RECEIVED_2048";
  elementName[640] = "SQLM_ELM_MAX_DATA_RECEIVED_4096";
  elementName[641] = "SQLM_ELM_MAX_DATA_RECEIVED_8192";
  elementName[642] = "SQLM_ELM_MAX_DATA_RECEIVED_16384";
  elementName[643] = "SQLM_ELM_MAX_DATA_RECEIVED_31999";
  elementName[644] = "SQLM_ELM_MAX_DATA_RECEIVED_64000";
  elementName[645] = "SQLM_ELM_MAX_DATA_RECEIVED_GT64000";
  elementName[646] = "SQLM_ELM_MAX_TIME_2_MS";
  elementName[647] = "SQLM_ELM_MAX_TIME_4_MS";
  elementName[648] = "SQLM_ELM_MAX_TIME_8_MS";
  elementName[649] = "SQLM_ELM_MAX_TIME_16_MS";
  elementName[650] = "SQLM_ELM_MAX_TIME_32_MS";
  elementName[651] = "SQLM_ELM_MAX_TIME_GT32_MS";
  elementName[652] = "SQLM_ELM_DEADLOCK_ID";
  elementName[653] = "SQLM_ELM_DEADLOCK_NODE";
  elementName[654] = "SQLM_ELM_PARTICIPANT_NO";
  elementName[655] = "SQLM_ELM_PARTICIPANT_NO_HOLDING_LK";
  elementName[656] = "SQLM_ELM_ROLLED_BACK_PARTICIPANT_NO";
  elementName[657] = "SQLM_ELM_SQLERRD1";
  elementName[658] = "SQLM_ELM_SQLERRD2";
  elementName[659] = "SQLM_ELM_SQLERRD3";
  elementName[660] = "SQLM_ELM_SQLERRD4";
  elementName[661] = "SQLM_ELM_SQLERRD5";
  elementName[662] = "SQLM_ELM_SQLERRD6";
  elementName[663] = "SQLM_ELM_EVMON_ACTIVATES";
  elementName[664] = "SQLM_ELM_EVMON_FLUSHES";
  elementName[665] = "SQLM_ELM_SQL_REQ_ID";
  elementName[666] = "SQLM_ELM_MESSAGE";
  elementName[667] = "SQLM_ELM_MESSAGE_TIME";
  elementName[668] = "SQLM_ELM_VECTORED_IOS";
  elementName[669] = "SQLM_ELM_PAGES_FROM_VECTORED_IOS";
  elementName[670] = "SQLM_ELM_BLOCK_IOS";
  elementName[671] = "SQLM_ELM_PAGES_FROM_BLOCK_IOS";
  elementName[672] = "SQLM_ELM_PHYSICAL_PAGE_MAPS";
  elementName[673] = "SQLM_ELM_LOCKS_IN_LIST";
  elementName[674] = "SQLM_ELM_REORG_PHASE";
  elementName[675] = "SQLM_ELM_REORG_MAX_PHASE";
  elementName[676] = "SQLM_ELM_REORG_CURRENT_COUNTER";
  elementName[677] = "SQLM_ELM_REORG_MAX_COUNTER";
  elementName[678] = "SQLM_ELM_REORG_TYPE";
  elementName[679] = "SQLM_ELM_REORG_STATUS";
  elementName[680] = "SQLM_ELM_REORG_COMPLETION";
  elementName[681] = "SQLM_ELM_REORG_START";
  elementName[682] = "SQLM_ELM_REORG_END";
  elementName[683] = "SQLM_ELM_REORG_PHASE_START";
  elementName[684] = "SQLM_ELM_REORG_INDEX_ID";
  elementName[685] = "SQLM_ELM_REORG_TBSPC_ID";
  elementName[686] = "SQLM_ELM_POOL_ID";
  elementName[687] = "SQLM_ELM_POOL_CUR_SIZE";
  elementName[688] = "SQLM_ELM_POOL_CONFIG_SIZE";
  elementName[688] = "SQLM_ELM_POOL_MAX_SIZE";
  elementName[689] = "SQLM_ELM_POOL_WATERMARK";
  elementName[690] = "SQLM_ELM_TABLESPACE_ID";
  elementName[691] = "SQLM_ELM_TABLESPACE_TYPE";
  elementName[692] = "SQLM_ELM_TABLESPACE_CONTENT_TYPE";
  elementName[693] = "SQLM_ELM_TABLESPACE_STATE";
  elementName[694] = "SQLM_ELM_TABLESPACE_PAGE_SIZE";
  elementName[695] = "SQLM_ELM_TABLESPACE_EXTENT_SIZE";
  elementName[696] = "SQLM_ELM_TABLESPACE_PREFETCH_SIZE";
  elementName[697] = "SQLM_ELM_TABLESPACE_CUR_POOL_ID";
  elementName[698] = "SQLM_ELM_TABLESPACE_NEXT_POOL_ID";
  elementName[699] = "SQLM_ELM_TABLESPACE_TOTAL_PAGES";
  elementName[700] = "SQLM_ELM_TABLESPACE_USABLE_PAGES";
  elementName[701] = "SQLM_ELM_TABLESPACE_USED_PAGES";
  elementName[702] = "SQLM_ELM_TABLESPACE_FREE_PAGES";
  elementName[703] = "SQLM_ELM_TABLESPACE_PAGE_TOP";
  elementName[704] = "SQLM_ELM_TABLESPACE_PENDING_FREE_PAGES";
  elementName[705] = "SQLM_ELM_TABLESPACE_REBALANCER_MODE";
  elementName[706] = "SQLM_ELM_TABLESPACE_REBALANCER_EXTENTS_REMAINING";
  elementName[707] = "SQLM_ELM_TABLESPACE_REBALANCER_EXTENTS_PROCESSED";
  elementName[708] = "SQLM_ELM_TABLESPACE_REBALANCER_LAST_EXTENT_MOVED";
  elementName[709] = "SQLM_ELM_TABLESPACE_REBALANCER_PRIORITY";
  elementName[710] = "SQLM_ELM_TABLESPACE_NUM_QUIESCERS";
  elementName[711] = "SQLM_ELM_TABLESPACE_STATE_CHANGE_OBJECT_ID";
  elementName[712] = "SQLM_ELM_TABLESPACE_STATE_CHANGE_TS_ID";
  elementName[713] = "SQLM_ELM_TABLESPACE_NUM_CONTAINERS";
  elementName[714] = "SQLM_ELM_TABLESPACE_NUM_RANGES";
  elementName[715] = "SQLM_ELM_QUIESCER_STATE";
  elementName[716] = "SQLM_ELM_QUIESCER_AGENT_ID";
  elementName[717] = "SQLM_ELM_QUIESCER_TS_ID";
  elementName[718] = "SQLM_ELM_QUIESCER_OBJ_ID";
  elementName[719] = "SQLM_ELM_QUIESCER_AUTH_ID";
  elementName[720] = "SQLM_ELM_CONTAINER_ID";
  elementName[721] = "SQLM_ELM_CONTAINER_TYPE";
  elementName[722] = "SQLM_ELM_CONTAINER_TOTAL_PAGES";
  elementName[723] = "SQLM_ELM_CONTAINER_USABLE_PAGES";
  elementName[724] = "SQLM_ELM_CONTAINER_STRIPE_SET";
  elementName[725] = "SQLM_ELM_CONTAINER_ACCESSIBLE";
  elementName[726] = "SQLM_ELM_CONTAINER_NAME";
  elementName[727] = "SQLM_ELM_RANGE_STRIPE_SET_NUMBER";
  elementName[728] = "SQLM_ELM_RANGE_NUMBER";
  elementName[729] = "SQLM_ELM_RANGE_OFFSET";
  elementName[730] = "SQLM_ELM_RANGE_MAX_PAGE_NUMBER";
  elementName[731] = "SQLM_ELM_RANGE_MAX_EXTENT";
  elementName[732] = "SQLM_ELM_RANGE_START_STRIPE";
  elementName[733] = "SQLM_ELM_RANGE_END_STRIPE";
  elementName[734] = "SQLM_ELM_RANGE_ADJUSTMENT";
  elementName[735] = "SQLM_ELM_RANGE_NUM_CONTAINERS";
  elementName[736] = "SQLM_ELM_RANGE_CONTAINER_ID";
  elementName[737] = "SQLM_ELM_CONSISTENCY_TOKEN";
  elementName[738] = "SQLM_ELM_PACKAGE_VERSION_ID";
  elementName[739] = "SQLM_ELM_LOCK_NAME";
  elementName[740] = "SQLM_ELM_LOCK_COUNT";
  elementName[741] = "SQLM_ELM_LOCK_HOLD_COUNT";
  elementName[742] = "SQLM_ELM_LOCK_ATTRIBUTES";
  elementName[743] = "SQLM_ELM_LOCK_RELEASE_FLAGS";
  elementName[744] = "SQLM_ELM_LOCK_CURRENT_MODE";
  elementName[745] = "SQLM_ELM_TABLESPACE_FS_CACHING";
  elementName[751] = "SQLM_ELM_BP_TBSP_USE_COUNT";
  elementName[752] = "SQLM_ELM_BP_PAGES_LEFT_TO_REMOVE";
  elementName[753] = "SQLM_ELM_BP_CUR_BUFFSZ";
  elementName[754] = "SQLM_ELM_BP_NEW_BUFFSZ";
  elementName[755] = "SQLM_ELM_SORT_HEAP_TOP";
  elementName[756] = "SQLM_ELM_SORT_SHRHEAP_ALLOCATED";
  elementName[757] = "SQLM_ELM_SORT_SHRHEAP_TOP";
  elementName[758] = "SQLM_ELM_SHR_WORKSPACE_SIZE_TOP";
  elementName[759] = "SQLM_ELM_SHR_WORKSPACE_NUM_OVERFLOWS";
  elementName[760] = "SQLM_ELM_SHR_WORKSPACE_SECTION_LOOKUPS";
  elementName[761] = "SQLM_ELM_SHR_WORKSPACE_SECTION_INSERTS";
  elementName[762] = "SQLM_ELM_PRIV_WORKSPACE_SIZE_TOP";
  elementName[763] = "SQLM_ELM_PRIV_WORKSPACE_NUM_OVERFLOWS";
  elementName[764] = "SQLM_ELM_PRIV_WORKSPACE_SECTION_LOOKUPS";
  elementName[765] = "SQLM_ELM_PRIV_WORKSPACE_SECTION_INSERTS";
  elementName[766] = "SQLM_ELM_CAT_CACHE_SIZE_TOP";
  elementName[767] = "SQLM_ELM_PARTITION_NUMBER";
  elementName[768] = "SQLM_ELM_NUM_TRANSMISSIONS_GROUP";
  elementName[769] = "SQLM_ELM_NUM_INDOUBT_TRANS";
  elementName[770] = "SQLM_ELM_UTILITY_DBNAME";
  elementName[771] = "SQLM_ELM_UTILITY_ID";
  elementName[772] = "SQLM_ELM_UTILITY_TYPE";
  elementName[773] = "SQLM_ELM_UTILITY_PRIORITY";
  elementName[774] = "SQLM_ELM_UTILITY_START_TIME";
  elementName[775] = "SQLM_ELM_UTILITY_DESCRIPTION";
  elementName[776] = "SQLM_ELM_POOL_ASYNC_INDEX_READ_REQS";
  elementName[777] = "SQLM_ELM_SESSION_AUTH_ID";
  elementName[778] = "SQLM_ELM_SQL_CHAINS";
  elementName[779] = "SQLM_ELM_POOL_TEMP_DATA_L_READS";
  elementName[780] = "SQLM_ELM_POOL_TEMP_DATA_P_READS";
  elementName[781] = "SQLM_ELM_POOL_TEMP_INDEX_L_READS";
  elementName[782] = "SQLM_ELM_POOL_TEMP_INDEX_P_READS";
  elementName[783] = "SQLM_ELM_MAX_TIME_1_MS";
  elementName[784] = "SQLM_ELM_MAX_TIME_100_MS";
  elementName[785] = "SQLM_ELM_MAX_TIME_500_MS";
  elementName[786] = "SQLM_ELM_MAX_TIME_GT500_MS";
  elementName[787] = "SQLM_ELM_LOG_TO_REDO_FOR_RECOVERY";
  elementName[788] = "SQLM_ELM_POOL_NO_VICTIM_BUFFER";
  elementName[789] = "SQLM_ELM_LOG_HELD_BY_DIRTY_PAGES";
  elementName[790] = "SQLM_ELM_PROGRESS_DESCRIPTION";
  elementName[791] = "SQLM_ELM_PROGRESS_START_TIME";
  elementName[792] = "SQLM_ELM_PROGRESS_WORK_METRIC";
  elementName[793] = "SQLM_ELM_PROGRESS_TOTAL_UNITS";
  elementName[794] = "SQLM_ELM_PROGRESS_COMPLETED_UNITS";
  elementName[795] = "SQLM_ELM_PROGRESS_SEQ_NUM";
  elementName[796] = "SQLM_ELM_PROGRESS_LIST_CUR_SEQ_NUM";
  elementName[797] = "SQLM_ELM_HADR_ROLE";
  elementName[798] = "SQLM_ELM_HADR_STATE";
  elementName[799] = "SQLM_ELM_HADR_SYNCMODE";
  elementName[800] = "SQLM_ELM_HADR_CONNECT_STATUS";
  elementName[801] = "SQLM_ELM_HADR_CONNECT_TIME";
  elementName[802] = "SQLM_ELM_HADR_HEARTBEAT";
  elementName[803] = "SQLM_ELM_HADR_LOCAL_HOST";
  elementName[804] = "SQLM_ELM_HADR_LOCAL_SERVICE";
  elementName[805] = "SQLM_ELM_HADR_REMOTE_HOST";
  elementName[806] = "SQLM_ELM_HADR_REMOTE_SERVICE";
  elementName[807] = "SQLM_ELM_HADR_TIMEOUT";
  elementName[808] = "SQLM_ELM_HADR_PRIMARY_LOG_FILE";
  elementName[809] = "SQLM_ELM_HADR_PRIMARY_LOG_PAGE";
  elementName[810] = "SQLM_ELM_HADR_PRIMARY_LOG_LSN";
  elementName[811] = "SQLM_ELM_HADR_STANDBY_LOG_FILE";
  elementName[812] = "SQLM_ELM_HADR_STANDBY_LOG_PAGE";
  elementName[813] = "SQLM_ELM_HADR_STANDBY_LOG_LSN";
  elementName[814] = "SQLM_ELM_HADR_LOG_GAP";
  elementName[815] = "SQLM_ELM_HADR_REMOTE_INSTANCE";
  elementName[816] = "SQLM_ELM_DATA_OBJECT_PAGES";
  elementName[817] = "SQLM_ELM_INDEX_OBJECT_PAGES";
  elementName[818] = "SQLM_ELM_LOB_OBJECT_PAGES";
  elementName[819] = "SQLM_ELM_LONG_OBJECT_PAGES";
  elementName[820] = "SQLM_ELM_LOCK_TIMEOUT_VAL";
  elementName[821] = "SQLM_ELM_LOG_WRITE_TIME";
  elementName[822] = "SQLM_ELM_LOG_READ_TIME";
  elementName[823] = "SQLM_ELM_NUM_LOG_WRITE_IO";
  elementName[824] = "SQLM_ELM_NUM_LOG_READ_IO";
  elementName[825] = "SQLM_ELM_NUM_LOG_PART_PAGE_IO";
  elementName[826] = "SQLM_ELM_NUM_LOG_BUFF_FULL";
  elementName[827] = "SQLM_ELM_NUM_LOG_DATA_IN_BUFF";
  elementName[828] = "SQLM_ELM_LOG_FILE_NUM_FIRST";
  elementName[829] = "SQLM_ELM_LOG_FILE_NUM_LAST";
  elementName[830] = "SQLM_ELM_LOG_FILE_NUM_CURR";
  elementName[831] = "SQLM_ELM_LOG_FILE_ARCHIVE";
  elementName[832] = "SQLM_ELM_NANOSEC";
  elementName[833] = "SQLM_ELM_STMT_HISTORY_ID";
  elementName[834] = "SQLM_ELM_STMT_LOCK_TIMEOUT";
  elementName[835] = "SQLM_ELM_STMT_ISOLATION";
  elementName[836] = "SQLM_ELM_COMP_ENV_DESC";
  elementName[837] = "SQLM_ELM_STMT_VALUE_TYPE";
  elementName[838] = "SQLM_ELM_STMT_VALUE_ISREOPT";
  elementName[839] = "SQLM_ELM_STMT_VALUE_ISNULL";
  elementName[840] = "SQLM_ELM_STMT_VALUE_DATA";
  elementName[841] = "SQLM_ELM_STMT_VALUE_INDEX";
  elementName[842] = "SQLM_ELM_STMT_FIRST_USE_TIME";
  elementName[843] = "SQLM_ELM_STMT_LAST_USE_TIME";
  elementName[844] = "SQLM_ELM_STMT_NEST_LEVEL";
  elementName[845] = "SQLM_ELM_STMT_INVOCATION_ID";
  elementName[846] = "SQLM_ELM_STMT_QUERY_ID";
  elementName[847] = "SQLM_ELM_STMT_SOURCE_ID";
  elementName[848] = "SQLM_ELM_STMT_PKGCACHE_ID";
  elementName[849] = "SQLM_ELM_INACT_STMTHIST_SZ";
  elementName[850] = "SQLM_ELM_NUM_DB_STORAGE_PATHS";
  elementName[851] = "SQLM_ELM_DB_STORAGE_PATH";
  elementName[852] = "SQLM_ELM_TABLESPACE_INITIAL_SIZE";
  elementName[853] = "SQLM_ELM_TABLESPACE_CURRENT_SIZE";
  elementName[854] = "SQLM_ELM_TABLESPACE_MAX_SIZE";
  elementName[855] = "SQLM_ELM_TABLESPACE_INCREASE_SIZE_PERCENT";
  elementName[856] = "SQLM_ELM_TABLESPACE_INCREASE_SIZE";
  elementName[857] = "SQLM_ELM_TABLESPACE_LAST_RESIZE_TIME";
  elementName[858] = "SQLM_ELM_TABLESPACE_USING_AUTO_STORAGE";
  elementName[859] = "SQLM_ELM_TABLESPACE_AUTO_RESIZE_ENABLED";
  elementName[860] = "SQLM_ELM_TABLESPACE_LAST_RESIZE_FAILED";
  elementName[861] = "SQLM_ELM_BP_ID";
  elementName[862] = "SQLM_ELM_REORG_LONG_TBSPC_ID";
  elementName[863] = "SQLM_ELM_DATA_PARTITION_ID";
  elementName[864] = "SQLM_ELM_PROGRESS_LIST_ATTR";
  elementName[865] = "SQLM_ELM_REORG_ROWSCOMPRESSED";
  elementName[866] = "SQLM_ELM_REORG_ROWSREJECTED";
  elementName[867] = "SQLM_ELM_CH_FREE";
  elementName[868] = "SQLM_ELM_CH_FREE_BOTTOM";
  elementName[869] = "SQLM_ELM_UTILITY_STATE";
  elementName[870] = "SQLM_ELM_UTILITY_INVOKER_TYPE";
  elementName[871] = "SQLM_ELM_POST_SHRTHRESHOLD_SORTS";
  elementName[872] = "SQLM_ELM_POST_SHRTHRESHOLD_HASH_JOINS";
  elementName[873] = "SQLM_ELM_ACTIVE_HASH_JOINS";
  elementName[874] = "SQLM_ELM_POOL_SECONDARY_ID";
  elementName[875] = "SQLM_ELM_FS_ID";
  elementName[876] = "SQLM_ELM_FS_TOTAL_SZ";
  elementName[877] = "SQLM_ELM_FS_USED_SZ";
  elementName[878] = "SQLM_ELM_STO_PATH_FREE_SZ";
  elementName[879] = "SQLM_ELM_POOL_XDA_L_READS";
  elementName[880] = "SQLM_ELM_POOL_XDA_P_READS";
  elementName[881] = "SQLM_ELM_POOL_XDA_WRITES";
  elementName[882] = "SQLM_ELM_POOL_TEMP_XDA_L_READS";
  elementName[883] = "SQLM_ELM_POOL_TEMP_XDA_P_READS";
  elementName[884] = "SQLM_ELM_POOL_ASYNC_XDA_READS";
  elementName[885] = "SQLM_ELM_POOL_ASYNC_XDA_WRITES";
  elementName[886] = "SQLM_ELM_POOL_ASYNC_XDA_READ_REQS";
  elementName[887] = "SQLM_ELM_XDA_OBJECT_PAGES";
  elementName[888] = "SQLM_ELM_XQUERY_STMTS";
  elementName[889] = "SQLM_ELM_TRUSTED_AUTH_ID";
  elementName[890] = "SQLM_ELM_HADR_PEER_WINDOW_END";
  elementName[891] = "SQLM_ELM_HADR_PEER_WINDOW";
  elementName[892] = "SQLM_ELM_BLOCKS_PENDING_CLEANUP";
  elementName[893] = "SQLM_ELM_AUTHORITY_BITMAP";
  elementName[894] = "SQLM_ELM_TOTAL_OLAP_FUNCS";
  elementName[895] = "SQLM_ELM_POST_THRESHOLD_OLAP_FUNCS";
  elementName[896] = "SQLM_ELM_ACTIVE_OLAP_FUNCS";
  elementName[897] = "SQLM_ELM_OLAP_FUNC_OVERFLOWS";
  elementName[898] = "SQLM_ELM_SERVICE_CLASS_ID";
  elementName[899] = "SQLM_ELM_SERVICE_SUPERCLASS_NAME";
  elementName[900] = "SQLM_ELM_SERVICE_SUBCLASS_NAME";
  elementName[901] = "SQLM_ELM_WORK_ACTION_SET_ID";
  elementName[902] = "SQLM_ELM_WORK_ACTION_SET_NAME";
  elementName[903] = "SQLM_ELM_DB_WORK_ACTION_SET_ID";
  elementName[904] = "SQLM_ELM_SC_WORK_ACTION_SET_ID";
  elementName[905] = "SQLM_ELM_WORK_CLASS_ID";
  elementName[906] = "SQLM_ELM_WORK_CLASS_NAME";
  elementName[907] = "SQLM_ELM_DB_WORK_CLASS_ID";
  elementName[908] = "SQLM_ELM_SC_WORK_CLASS_ID";
  elementName[909] = "SQLM_ELM_WORKLOAD_ID";
  elementName[910] = "SQLM_ELM_WORKLOAD_OCCURRENCE_ID";
  elementName[911] = "SQLM_ELM_WORKLOAD_NAME";
  elementName[912] = "SQLM_ELM_TEMP_TABLESPACE_TOP";
  elementName[913] = "SQLM_ELM_ROWS_RETURNED_TOP";
  elementName[914] = "SQLM_ELM_CONCURRENT_ACT_TOP";
  elementName[915] = "SQLM_ELM_CONCURRENT_CONNECTION_TOP";
  elementName[916] = "SQLM_ELM_COST_ESTIMATE_TOP";
  elementName[917] = "SQLM_ELM_STATISTICS_TIMESTAMP";
  elementName[918] = "SQLM_ELM_ACT_TOTAL";
  elementName[919] = "SQLM_ELM_WLO_COMPLETED_TOTAL";
  elementName[920] = "SQLM_ELM_CONCURRENT_WLO_TOP";
  elementName[921] = "SQLM_ELM_CONCURRENT_WLO_ACT_TOP";
  elementName[922] = "SQLM_ELM_TOP";
  elementName[923] = "SQLM_ELM_BOTTOM";
  elementName[924] = "SQLM_ELM_HISTOGRAM_TYPE";
  elementName[925] = "SQLM_ELM_BIN_ID";
  elementName[926] = "SQLM_ELM_ACTIVITY_ID";
  elementName[927] = "SQLM_ELM_ACTIVITY_SECONDARY_ID";
  elementName[928] = "SQLM_ELM_UOW_ID";
  elementName[929] = "SQLM_ELM_PARENT_ACTIVITY_ID";
  elementName[930] = "SQLM_ELM_PARENT_UOW_ID";
  elementName[931] = "SQLM_ELM_TIME_OF_VIOLATION";
  elementName[932] = "SQLM_ELM_ACTIVITY_COLLECTED";
  elementName[933] = "SQLM_ELM_ACTIVITY_TYPE";
  elementName[934] = "SQLM_ELM_THRESHOLD_PREDICATE";
  elementName[935] = "SQLM_ELM_THRESHOLD_ACTION";
  elementName[936] = "SQLM_ELM_THRESHOLD_MAXVALUE";
  elementName[937] = "SQLM_ELM_THRESHOLD_QUEUESIZE";
  elementName[938] = "SQLM_ELM_COORD_ACT_COMPLETED_TOTAL";
  elementName[939] = "SQLM_ELM_COORD_ACT_ABORTED_TOTAL";
  elementName[940] = "SQLM_ELM_COORD_ACT_REJECTED_TOTAL";
  elementName[941] = "SQLM_ELM_COORD_ACT_LIFETIME_TOP";
  elementName[942] = "SQLM_ELM_ACT_EXEC_TIME";
  elementName[943] = "SQLM_ELM_TIME_CREATED";
  elementName[944] = "SQLM_ELM_TIME_STARTED";
  elementName[945] = "SQLM_ELM_TIME_COMPLETED";
  elementName[946] = "SQLM_ELM_SECTION_ENV";
  elementName[947] = "SQLM_ELM_ACTIVATE_TIMESTAMP";
  elementName[948] = "SQLM_ELM_NUM_THRESHOLD_VIOLATIONS";
  elementName[949] = "SQLM_ELM_ARM_CORRELATOR";
  elementName[950] = "SQLM_ELM_PREP_TIME";
  elementName[951] = "SQLM_ELM_QUEUE_SIZE_TOP";
  elementName[953] = "SQLM_ELM_QUEUE_ASSIGNMENTS_TOTAL";
  elementName[954] = "SQLM_ELM_QUEUE_TIME_TOTAL";
  elementName[955] = "SQLM_ELM_LAST_WLM_RESET";
  elementName[956] = "SQLM_ELM_THRESHOLD_DOMAIN";
  elementName[957] = "SQLM_ELM_THRESHOLD_NAME";
  elementName[958] = "SQLM_ELM_THRESHOLDID";
  elementName[959] = "SQLM_ELM_NUMBER_IN_BIN";
  elementName[960] = "SQLM_ELM_COORD_ACT_LIFETIME_AVG";
  elementName[961] = "SQLM_ELM_COORD_ACT_QUEUE_TIME_AVG";
  elementName[962] = "SQLM_ELM_COORD_ACT_EXEC_TIME_AVG";
  elementName[963] = "SQLM_ELM_COORD_ACT_EST_COST_AVG";
  elementName[964] = "SQLM_ELM_COORD_ACT_INTERARRIVAL_TIME_AVG";
  elementName[965] = "SQLM_ELM_REQUEST_EXEC_TIME_AVG";
  elementName[966] = "SQLM_ELM_STATS_CACHE_SIZE";
  elementName[967] = "SQLM_ELM_STATS_FABRICATIONS";
  elementName[968] = "SQLM_ELM_STATS_FABRICATE_TIME";
  elementName[969] = "SQLM_ELM_SYNC_RUNSTATS";
  elementName[970] = "SQLM_ELM_SYNC_RUNSTATS_TIME";
  elementName[971] = "SQLM_ELM_ASYNC_RUNSTATS";
  elementName[972] = "SQLM_ELM_POOL_LIST_ID";
  elementName[973] = "SQLM_ELM_IS_SYSTEM_APPL";
  elementName[974] = "SQLM_ELM_INSERT_TIMESTAMP";

  return rc;
} /* init_element_names */


/***************************************************************************/
/* InitElementTypes                                                        */
/* Initialize the array of element types based on the defines in sqlmon.h. */
/***************************************************************************/
int  init_element_types(char *elementType[])
{
  int  rc = 0;
  int  arraySize = NUMTYPES *sizeof(elementType[0]);
  char  *pArray = (char *) &elementType[0];

  /* zero the entire array to ensure unset values are null */
  strncpy(pArray, "", arraySize);

  /* set the individual element types (defined in sqlmon.h) */
  elementType[1]  = "SQLM_TYPE_HEADER";
  elementType[50] = "SQLM_TYPE_STRING";
  elementType[51] = "SQLM_TYPE_U8BIT" ;
  elementType[52] = "SQLM_TYPE_8BIT" ;
  elementType[53] = "SQLM_TYPE_16BIT";
  elementType[54] = "SQLM_TYPE_U16BIT";
  elementType[55] = "SQLM_TYPE_32BIT";
  elementType[56] = "SQLM_TYPE_U32BIT";
  elementType[57] = "SQLM_TYPE_U64BIT";
  elementType[58] = "SQLM_TYPE_64BIT";
  elementType[59] = "SQLM_TYPE_HANDLE";

  return rc;

} /* InitElementTypes */


/***************************************************************************/
/* free_snapshot_memory                                                    */
/* General cleanup routine to release memory buffers.                      */
/***************************************************************************/
int  free_snapshot_memory(struct sqlma *ma_ptr, char *buffer_ptr)
{
  int  rc = 0;

  /* free output buffer */
  if (buffer_ptr != NULL)
    free(buffer_ptr);

  /* free sqlma */
  if (ma_ptr != NULL)
    free(ma_ptr);

  return rc;
}  /* free_snapshot_memory */


/*******************************************************************************/
int  match_appl(const void *appl1, const void *appl2)
{

  return  ((Appl * ) appl1)->appl_handle  
      - ((Appl * )appl2)->appl_handle;

} /*match_appl*/

/*******************************************************************************/
int  match_lock(const void *lockx, const void *locky)
{
  Lock *lock1 = (Lock *) lockx;
  Lock *lock2 = (Lock *) locky;

  if (lock1->agent_id == lock2->agent_id 
      && lock1->node == lock2->node
      ) {
    if (lock1->lock_obj_type != lock2->lock_obj_type)
      return -1;
    else if (lock1->lock_obj_type == SQLM_TABLE_LOCK
        || lock1->lock_obj_type == SQLM_ROW_LOCK
        || lock1->lock_obj_type == SQLM_BLOCK_LOCK
        || lock1->lock_obj_type == SQLM_TABLE_PART_LOCK
        || lock1->lock_obj_type == SQLM_EOT_LOCK) {
        if (lock1->lock_mode == lock2->lock_mode
            && lock1->lock_status == lock2->lock_status
            && lock1->data_partition_id == lock2->data_partition_id
            && strncmp(lock1->tabschema, lock2->tabschema, TABSCHEMA_SZ) == 0
            && strncmp(lock1->tabname, lock2->tabname, TABNAME_SZ) == 0 )
           return 0;
        else 
           return -1; 
    }  else if (lock1->lock_obj_type ==  SQLM_TABLESPACE_LOCK) {
        if (lock1->lock_mode == lock2->lock_mode
            && lock1->lock_status == lock2->lock_status
            && strncmp(lock1->tbspace, lock2->tbspace, SQLUH_TABLESPACENAME_SZ) == 0)
        return 0;
        else
        return -1;
    }  else if ( lock1->lock_obj_type == lock2->lock_obj_type
                 && lock1->lock_mode == lock2->lock_mode 
                 && lock1->lock_status == lock2->lock_status )
      return 0;
    else
      return -1;

  } else
    return -1;

}
/*******************************************************************************/
int  match_stmt(const void *stmt1, const void *stmt2)
{

  if (
      (((Stmt * ) stmt1)->appl_handle 
      == ((Stmt * )stmt2)->appl_handle)
      && 
      (((Stmt * ) stmt1)->stmt_node_number  
      == ((Stmt * )stmt2)->stmt_node_number)
      &&
      (((Stmt * ) stmt1)->section_number  
      == ((Stmt * )stmt2)->section_number)
      &&
      (strncmp(((Stmt * ) stmt1)->sequence_no  
      ,((Stmt * )stmt2)->sequence_no, SQLM_SEQ_SZ) == 0 )
      )
    return 0 ;
  else
    return -1;

} /*match_appl*/

/*******************************************************************************/
int  match_tbspace(const void *tbspace1, const void *tbspace2)
{

  return ((Tbspace * ) tbspace1)->id 
      - ((Tbspace * )tbspace2)->id;

} /*match_tbspace*/
/*******************************************************************************/
int match_table(const void *table1, const void *table2)
{

  if (strncmp( ((Table *) table1)->tabschema, ((Table *) table2)->tabschema, TABSCHEMA_SZ) == 0
      && strncmp( ((Table *) table1)->tabname,   ((Table *) table2)->tabname, TABNAME_SZ) == 0
      && ((Table *) table1)->data_partition_id == ((Table *) table2)->data_partition_id )
    return 0;
  else
    return -1;

}
/*******************************************************************************/
int search_table(const void *table1, const void *str)
{
  char tabname [ TABSCHEMA_SZ + TABNAME_SZ + 1];
  strncpy(tabname, strtrim(((Table *) table1)->tabschema), TABSCHEMA_SZ);
  strcat(tabname,".");
  strncat(tabname, strtrim(((Table *) table1)->tabname), TABNAME_SZ);
  if ( strstr(tabname, str) != NULL)
    return 0;
  else
    return 1;
}
/*******************************************************************************/
int  match_util(const void *util1, const void *util2)
{

  return  ((Util * ) util1)->id 
      - ((Util * )util2)->id;

} /*match_util*/

/*******************************************************************************/
int  match_util_detl(const void *util1, const void *util2)
{

  return 
      (((Util * ) util1)->id - ((Util * )util2)->id )
      + (((Util * ) util1)->node - ((Util * )util2)->node )
      ;

} /*match_util_detl*/
/********************************************************************************/
int  match_appl_ss(const void *appl_ss1, const void *appl_ss2)
{


  if ( ((Appl_SS * ) appl_ss1)->ss_node_number 
      == ((Appl_SS * )appl_ss2)->ss_node_number 
      && 
      ((Appl_SS * ) appl_ss1)->ss_number 
      == ((Appl_SS * )appl_ss2)->ss_number)
    return 0;
  else
    return -1;

  ;

} /*match_appl_ss*/


/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_TBSPACE   (Header  *header
, DB2List *list
, Tbspace *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData;
  DB2ListElmt *element;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      if (pHeader->element == SQLM_ELM_DB2)
        primaryLogicalGroup = SQLM_ELM_DB2;
      else if (pHeader->element == SQLM_ELM_DBASE)
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_APPL)
        primaryLogicalGroup = SQLM_ELM_APPL;
      else if (pHeader->element == SQLM_ELM_TABLESPACE) {
        primaryLogicalGroup = SQLM_ELM_TABLESPACE;

        /* add the tbspace to the list */
        if ( tempListData != NULL                                            
            && strlen(((Tbspace * ) tempListData)->name) > 0 ) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_tbspace(db2list_data(element), tempListData);
          } else {
            Tbspace * newTbspaceData = init_tbspace();
            copy_tbspace(newTbspaceData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newTbspaceData);
          }
          reset_tbspace(tempListData);
        } else 
          reset_tbspace(tempListData);
      }
      /* call function recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_TBSPACE(header, list, tempListData 
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_TABLESPACE) {
        switch (pHeader->element) {
        case SQLM_ELM_TABLESPACE_ID:
          tempListData->id = *(sqluint32 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_PAGE_SIZE:
          tempListData->pg_sz = *(sqluint32 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_TYPE:
          tempListData->type = *(sqluint8 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_CONTENT_TYPE:
          tempListData->content_type = *(sqluint8 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_NAME:
          if (pHeader->size >= SQLUH_TABLESPACENAME_SZ) {
            strncpy(tempListData->name, pData, SQLUH_TABLESPACENAME_SZ);
            (tempListData->name)[SQLUH_TABLESPACENAME_SZ] = '\0';
          } else {
            strncpy(tempListData->name, pData, pHeader->size);
            (tempListData->name)[pHeader->size] = '\0';
          } 
          break;
        case SQLM_ELM_DIRECT_READS:
          tempListData->t2_direct_reads = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITES:
          tempListData->t2_direct_writes = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_READ_REQS:
          tempListData->t2_direct_read_reqs = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_DIRECT_WRITE_REQS:
          tempListData->t2_direct_write_reqs = *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_P_READS:
        case SQLM_ELM_POOL_INDEX_P_READS:
        case SQLM_ELM_POOL_TEMP_DATA_P_READS:
        case SQLM_ELM_POOL_TEMP_INDEX_P_READS:
        case SQLM_ELM_POOL_XDA_P_READS:
        case SQLM_ELM_POOL_TEMP_XDA_P_READS:
          tempListData->t2_bp_reads += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_POOL_DATA_WRITES:
        case SQLM_ELM_POOL_INDEX_WRITES:
        case SQLM_ELM_POOL_XDA_WRITES:
          tempListData->t2_bp_writes += *(sqluint64 * )pData;
          break;
        }
      }   /* end of SQLM_ELM_TABLESPACE */ 
          else if (primaryLogicalGroup == SQLM_ELM_TABLESPACE
          && logicalGroup == SQLM_ELM_TABLESPACE_NODEINFO) {
        switch (pHeader->element) {
        case SQLM_ELM_TABLESPACE_STATE:
          tempListData->state = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_TABLESPACE_TOTAL_PAGES:
          tempListData->t2_total_pg += *(sqluint32 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_USED_PAGES:
        case SQLM_ELM_TABLESPACE_PENDING_FREE_PAGES:
          tempListData->t2_used_pg += *(sqluint32 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_FREE_PAGES:
          tempListData->t2_free_pg += *(sqluint32 *)pData;
          break;
        case SQLM_ELM_NODE_NUMBER:
          tempListData->node = *(sqluint16 *)pData;
          break;
        }
      } /* end of SQLM_ELM_TABLESPACE_NODEINFO */
      else if (primaryLogicalGroup == SQLM_ELM_TABLESPACE
          && logicalGroup == SQLM_ELM_TABLESPACE_CONTAINER) {
        switch(pHeader->element) {
        case SQLM_ELM_FS_TOTAL_SZ:
          tempListData->t2_fs_total_sz += *(sqluint64 *)pData;
          break;
        case SQLM_ELM_FS_USED_SZ:
          tempListData->t2_fs_used_sz += *(sqluint64 *)pData;
          break;
        }
      }

      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_TBSPACE */
/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_TABLE   (Header  *header
, DB2List *list
, Table *tempListData
, char *pStart
, char *pEnd
, sqluint32 primaryLogicalGroup
, sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData;
  DB2ListElmt *element;
  char buffer[TABSCHEMA_SZ + TABNAME_SZ + 1];

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      if (pHeader->element == SQLM_ELM_DB2)
        primaryLogicalGroup = SQLM_ELM_DB2;
      else if (pHeader->element == SQLM_ELM_DBASE)
        primaryLogicalGroup = SQLM_ELM_DBASE;
      else if (pHeader->element == SQLM_ELM_APPL)
        primaryLogicalGroup = SQLM_ELM_APPL;
      else if (pHeader->element == SQLM_ELM_TABLE) {
        primaryLogicalGroup = SQLM_ELM_TABLE;

        /* add the table to the list */
        if ( tempListData != NULL                                            
            && strlen(tempListData->tabname) > 0 
            && add_table(peek_snapReq(header), tempListData) ) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL) {
            copy_table(db2list_data(element), tempListData);
          } else {
            Table * newTableData = init_table();
            copy_table(newTableData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newTableData);
          }
          reset_table(tempListData);
        } else 
          reset_table(tempListData);
      }
      /* call function recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_TABLE(header, list, tempListData 
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_TABLE) {
        switch (pHeader->element) {
        case SQLM_ELM_ROWS_WRITTEN:
          tempListData->rw = *(sqluint64 *)pData;
          break;
        case SQLM_ELM_ROWS_READ:
          tempListData->rr = *(sqluint64 *)pData;
          break;
        case SQLM_ELM_TABLE_TYPE:
          tempListData->type = *(sqluint32 *)pData;
          break;
        case SQLM_ELM_DATA_OBJECT_PAGES:
          tempListData->data_sz = *(sqluint64 *)pData;
          break;
        case SQLM_ELM_INDEX_OBJECT_PAGES:
          tempListData->ix_sz = *(sqluint64 *)pData;
          break;
        case SQLM_ELM_TABLESPACE_ID:
          tempListData->tbspace_id = *(sqluint32 *)pData;
          break;
        case SQLM_ELM_DATA_PARTITION_ID:
          tempListData->data_partition_id = *(sqluint16 *)pData;
          break;
        case SQLM_ELM_TABLE_NAME:
          sprintf(buffer, "%.*s",pHeader->size,pData);
          strncpy(tempListData->tabname, strtrim(buffer), TABNAME_SZ);
          break;
        case SQLM_ELM_TABLE_SCHEMA:
          sprintf(buffer, "%.*s",pHeader->size,pData);
          strncpy(tempListData->tabschema, strtrim(buffer), TABSCHEMA_SZ);
          break;
        }
      }   /* end of SQLM_ELM_TABLE */


      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_TABLE */
/*************************************************************************************/
sqlm_header_info *parse_monitor_stream_UTILS   (Header  *header,
DB2List *list,
Util *tempListData,
char *pStart,
char *pEnd,
sqluint32 primaryLogicalGroup,
sqluint32 logicalGroup)
{
  sqlm_header_info * pHeader = (sqlm_header_info * )pStart;
  char  *pData , temp [ SQL_DBNAME_SZ + 1];
  DB2ListElmt  *element;

  /* "pEnd" is NULL only when called at the "SQLM_ELM_COLLECTED" level, so */
  /* because this is the beginning of the monitor data stream, calculate */
  /* the memory location where the monitor data stream buffer ends */
  if (!pEnd)
    pEnd = pStart +                  /* start of monitor stream  */
    pHeader->size +           /* add size in the "collected" header */
    sizeof(sqlm_header_info); /* add size of header itself */

  /* parse and print the data for the current logical grouping */
  /* elements in the current logical grouping will be parsed until "pEnd" */
  while ((char * )pHeader < pEnd) {
    /* point to the data which appears immediately after the header */
    pData = (char * )pHeader + sizeof(sqlm_header_info);

    /* determine if the current unit of data is a nested logical grouping */
    if (pHeader->type == SQLM_TYPE_HEADER) {
      char  *pNewEnd;
      pNewEnd = pData + (pHeader->size);

      if (pHeader->element == SQLM_ELM_DBASE) {
        /* start of a DBASE snapshot */
        primaryLogicalGroup = SQLM_ELM_DBASE;
      } else if (pHeader->element == SQLM_ELM_APPL) {
        primaryLogicalGroup = SQLM_ELM_APPL;
      } else if (pHeader->element == SQLM_ELM_UTILITY) {
        primaryLogicalGroup = SQLM_ELM_UTILITY;
        if (tempListData != NULL
            && tempListData->id > 0
            && add_util(peek_snapReq(header), tempListData) == TRUE) {
          element = db2list_lookup(list, tempListData);
          if (element != NULL)
            copy_util(db2list_data(element), tempListData);
          else {
            Util * newUtilData = init_util();
            copy_util(newUtilData, tempListData);
            db2list_ins_next(list, db2list_tail(list), newUtilData);
          }
          reset_util(tempListData);
        } else 
          reset_util(tempListData);
      }

      /* call function recursively to parse this */
      /* nested logical grouping */
      pHeader = parse_monitor_stream_UTILS(header, list, tempListData 
          , pData, pNewEnd, primaryLogicalGroup, pHeader->element);
    } else
    {
      /* not a logical grouping, therefore extract this monitor element */

      if (logicalGroup == SQLM_ELM_UTILITY) {
        switch (pHeader->element) {
        case SQLM_ELM_UTILITY_ID:
          /* add up dbase log usage for all dbase  */
          tempListData->id = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_UTILITY_TYPE:
          /* add up dbase log usage for all dbase  */
          tempListData->type = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_UTILITY_STATE:
          tempListData->state = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_NODE_NUMBER:
          tempListData->node = *(sqluint16 * )pData;
          break;
        case SQLM_ELM_UTILITY_DBNAME:
          strncpy(temp, pData, SQL_DBNAME_SZ);
          strncpy(tempListData->dbname, strtrim(temp), SQL_DBNAME_SZ);
          break;
        case SQLM_ELM_UTILITY_DESCRIPTION:
          get_util_desc(pData,pHeader->size, tempListData->desc);
          break;
        }
      }   /* end of SQLM_ELM_UTILITY */ else if (primaryLogicalGroup == SQLM_ELM_UTILITY
          && logicalGroup == SQLM_ELM_UTILITY_START_TIME) {
        switch (pHeader->element) {
        case SQLM_ELM_SECONDS:
          (tempListData->start_time).seconds = *(sqluint32 * )pData;
          break;
        case SQLM_ELM_MICROSEC:
          (tempListData->start_time).microsec = *(sqluint32 * )pData;
          break;
        }
      }   /* end of SQLM_ELM_UTILITY_START_TIME */ else if (primaryLogicalGroup == SQLM_ELM_UTILITY
          && logicalGroup == SQLM_ELM_PROGRESS) {
        switch (pHeader->element) {
        case SQLM_ELM_PROGRESS_COMPLETED_UNITS:
          tempListData->progress_completed += *(sqluint64 * )pData;
          break;
        case SQLM_ELM_PROGRESS_TOTAL_UNITS:
          tempListData->progress_total += *(sqluint64 * )pData;
          break;
        }
      }


      /* increment past the data to the next header */
      pHeader = (sqlm_header_info * )(pData + pHeader->size);
    }
  }

  /* return the current memory location once the current logical grouping */
  /* has been parsed */
  return (pHeader);
} /* parse_monitor_stream_UTILS */
/*************************************************************************************/
char *get_util_desc(char *pData, size_t length, char *buffer) {

  if (length == 0)
    return buffer;

  if (strstr(pData, "LOAD") != NULL) {
    char  *p;
    char  *util_desc = malloc(sizeof(char) * (length + 1));
    char  *tablename =  malloc(sizeof(char) * (length + 1));

    if (util_desc == NULL || tablename == NULL)
      return buffer;

    strcpy(buffer, "LOAD" );
    strncpy(util_desc, pData, length);
    util_desc[length] = '\0';
    strrtrim(util_desc);
    strrev(util_desc);
    // extract table name but all words are reversed                                                  
        p = strtok(util_desc, " ");
    strcpy(tablename, p);
    p = strtok(NULL, " ");
    strcat(tablename, p);
    // trim white space                                                       
        strrtrim(tablename);
    strcat(buffer, " ");
    // append the table name but reverse the name so schema name appears first
        strcat(buffer, strrev(tablename));

    free(util_desc);
    free(tablename);
  } else  {
    if (length >= UTIL_DESC_SZ) {
      strncpy(buffer, pData, UTIL_DESC_SZ);
      buffer[UTIL_DESC_SZ] = '\0';
    } else {
      strncpy(buffer, pData, length);
      buffer[length] = '\0';
    }
  }

  return buffer;
}

/*************************************************************************************/
int  compare_appl_cpu_asc(const void *p, const void *q)
{
  return 
      ( (((Appl *) p)->ucpu_used_delta + ((Appl *) p)->scpu_used_delta) - 
      (((Appl *) q)->ucpu_used_delta + ((Appl *) q)->scpu_used_delta)  )
      ;

}
/*************************************************************************************/
int  compare_appl_cpu_desc(const void *p, const void *q)
{
  return 
      ( (((Appl *) q)->ucpu_used_delta + ((Appl *) q)->scpu_used_delta) - 
      (((Appl *) p)->ucpu_used_delta + ((Appl *) p)->scpu_used_delta)  )
      ;

}
/*************************************************************************************/
int  compare_stmt_cpu_asc(const void *p, const void *q)
{
  return 
      (((Stmt *) p)->ucpu_used_delta + ((Stmt *) p)->scpu_used_delta) - 
      (((Stmt *) q)->ucpu_used_delta + ((Stmt *) q)->scpu_used_delta)
      ;

}
/*************************************************************************************/
int  compare_stmt_cpu_desc(const void *p, const void *q)
{
  return 
      (((Stmt *) q)->ucpu_used_delta + ((Stmt *) q)->scpu_used_delta) - 
      (((Stmt *) p)->ucpu_used_delta + ((Stmt *) p)->scpu_used_delta)
      ;

}
/*************************************************************************************/
int  compare_appl_ss_cpu_asc(const void *p, const void *q)
{
  return 
      (((Appl_SS *) p)->ss_ucpu_used_delta + ((Appl_SS *) p)->ss_scpu_used_delta) - 
      (((Appl_SS *) q)->ss_ucpu_used_delta + ((Appl_SS *) q)->ss_scpu_used_delta)
      ;
}
/*************************************************************************************/
int  compare_appl_ss_cpu_desc(const void *p, const void *q)
{
  return 
      (((Appl_SS *) q)->ss_ucpu_used_delta + ((Appl_SS *) q)->ss_scpu_used_delta) - 
      (((Appl_SS *) p)->ss_ucpu_used_delta + ((Appl_SS *) p)->ss_scpu_used_delta)
      ;
}
/*************************************************************************************/
int  compare_appl_tqr_asc(const void *p, const void *q)
{
  return 
      ((Appl *) p)->tq_rows_read_delta - 
      ((Appl *) q)->tq_rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_tqr_desc(const void *p, const void *q)
{
  return 
      ((Appl *) q)->tq_rows_read_delta  -
      ((Appl *) p)->tq_rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_ss_tqr_asc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) p)->tq_rows_read_delta - 
      ((Appl_SS *) q)->tq_rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_ss_tqr_desc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) q)->tq_rows_read_delta  -
      ((Appl_SS *) p)->tq_rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_tqw_asc(const void *p, const void *q)
{
  return 
      ((Appl *) p)->tq_rows_written_delta - 
      ((Appl *) q)->tq_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_tqw_desc(const void *p, const void *q)
{
  return 
      ((Appl *) q)->tq_rows_written_delta  -
      ((Appl *) p)->tq_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_ss_tqw_asc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) p)->tq_rows_written_delta - 
      ((Appl_SS *) q)->tq_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_ss_tqw_desc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) q)->tq_rows_written_delta  -
      ((Appl_SS *) p)->tq_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_rr_asc(const void *p, const void *q)
{
  return 
      ((Appl *) p)->rows_read_delta - 
      ((Appl *) q)->rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_rr_desc(const void *p, const void *q)
{
  return 
      ((Appl *) q)->rows_read_delta  -
      ((Appl *) p)->rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_ss_rr_asc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) p)->ss_rows_read_delta - 
      ((Appl_SS *) q)->ss_rows_read_delta;
}
/*************************************************************************************/
int  compare_appl_ss_rr_desc(const void *p, const void *q)
{
  return 
      ((Appl_SS *) q)->ss_rows_read_delta  -
      ((Appl_SS *) p)->ss_rows_read_delta;
}

/*************************************************************************************/
int  compare_appl_rw_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->rows_written_delta -                                                      
      ((Appl *) q)->rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_rw_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->rows_written_delta  -                                                  
      ((Appl *) p)->rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_ss_rw_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl_SS *) p)->ss_rows_written_delta -                                                      
      ((Appl_SS *) q)->ss_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_ss_rw_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl_SS *) q)->ss_rows_written_delta  -                                                  
      ((Appl_SS *) p)->ss_rows_written_delta;
}
/*************************************************************************************/
int  compare_appl_bp_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->bpldr_delta + ((Appl *) p)->bplir_delta -                                                   
      ((Appl *) q)->bpldr_delta + ((Appl *) q)->bplir_delta ;
}
/*************************************************************************************/
int  compare_appl_bp_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->bpldr_delta + ((Appl *) q)->bplir_delta -                                                   
      ((Appl *) p)->bpldr_delta + ((Appl *) p)->bplir_delta ;
}
/*************************************************************************************/
int  compare_appl_logusg_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->uow_log_space_used -                                                   
      ((Appl *) q)->uow_log_space_used;
}
/*************************************************************************************/
int  compare_appl_logusg_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->uow_log_space_used  -                                                  
      ((Appl *) p)->uow_log_space_used;
}
/*************************************************************************************/
int  compare_appl_memusg_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->privagent_memusg -                                                   
      ((Appl *) q)->privagent_memusg;
}
/*************************************************************************************/
int  compare_appl_memusg_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->privagent_memusg  -                                                  
      ((Appl *) p)->privagent_memusg;
}
/*************************************************************************************/
int  compare_appl_sel_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->rows_selected_delta -                                                   
      ((Appl *) q)->rows_selected_delta;
}
/*************************************************************************************/
int  compare_appl_sel_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->rows_selected_delta  -                                                  
      ((Appl *) p)->rows_selected_delta;
}
/*************************************************************************************/
int  compare_appl_ins_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->rows_inserted_delta -                                                   
      ((Appl *) q)->rows_inserted_delta;
}
/*************************************************************************************/
int  compare_appl_ins_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->rows_inserted_delta  -                                                  
      ((Appl *) p)->rows_inserted_delta;
}
/*************************************************************************************/
int  compare_appl_upd_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->rows_updated_delta -                                                   
      ((Appl *) q)->rows_updated_delta;
}
/*************************************************************************************/
int  compare_appl_upd_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->rows_updated_delta  -                                                  
      ((Appl *) p)->rows_updated_delta;
}
/*************************************************************************************/
int  compare_appl_del_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) p)->rows_deleted_delta -                                                   
      ((Appl *) q)->rows_deleted_delta;
}
/*************************************************************************************/
int  compare_appl_del_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl *) q)->rows_deleted_delta  -                                                  
      ((Appl *) p)->rows_deleted_delta;
}

/*************************************************************************************/
int  compare_appl_ss_node_asc(const void *p, const void *q)
{
  return                                                                                 
      (((Appl_SS *) p)->ss_node_number -                                                   
      ((Appl_SS *) q)->ss_node_number)
      +
      (((Appl_SS *) p)->ss_number -                                                   
      ((Appl_SS *) q)->ss_number)
      ;
}
/*************************************************************************************/
int  compare_appl_ss_node_desc(const void *p, const void *q)
{
  return                                                                                 
      (((Appl_SS *) q)->ss_node_number -                                                  
      ((Appl_SS *) p)->ss_node_number)
      +
      (((Appl_SS *) q)->ss_number -                                                  
      ((Appl_SS *) p)->ss_number)

      ;
}
/*************************************************************************************/
int  compare_appl_ss_nagents_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl_SS *) p)->num_agents -                                                   
      ((Appl_SS *) q)->num_agents;
}
/*************************************************************************************/
int  compare_appl_ss_nagents_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Appl_SS *) q)->num_agents -                                                  
      ((Appl_SS *) p)->num_agents;
}
/*************************************************************************************/
int  compare_utils_id_node_asc(const void *p, const void *q)
{
  return                                                                                 
      ((Util *) p)->node -                                                   
      ((Util *) q)->node;
}
/*************************************************************************************/
int  compare_utils_id_node_desc(const void *p, const void *q)
{
  return                                                                                 
      ((Util *) q)->node -                                                  
      ((Util *) p)->node;
}
/*************************************************************************************/
int  compare_tbspace_rr_asc(const void *p, const void *q)
{
  return                                                                               
      ((Tbspace *) p)->reads_delta  -                                                
      ((Tbspace *) q)->reads_delta;
}
/*************************************************************************************/
int  compare_tbspace_rr_desc(const void *p, const void *q)
{
  return                                                                               
      ((Tbspace *) q)->reads_delta  -                                                
      ((Tbspace *) p)->reads_delta;
}
/*************************************************************************************/
int  compare_tbspace_rw_asc(const void *p, const void *q)
{
  return                                                                               
      ((Tbspace *) p)->writes_delta  -                                                
      ((Tbspace *) q)->writes_delta;
}
/*************************************************************************************/
int  compare_tbspace_rw_desc(const void *p, const void *q)
{
  return                                                                               
      ((Tbspace *) q)->writes_delta  -                                                
      ((Tbspace *) p)->writes_delta;
}
/*************************************************************************************/
int  compare_table_rr_asc(const void *p, const void *q)
{
  return                                                                               
      ((Table *) p)->rr_delta  -                                                
      ((Table *) q)->rr_delta;
}
/*************************************************************************************/
int  compare_table_rr_desc(const void *p, const void *q)
{
  return                                                                               
      ((Table *) q)->rr_delta  -                                                
      ((Table *) p)->rr_delta;
}
/*************************************************************************************/
int  compare_table_rw_asc(const void *p, const void *q)
{
  return                                                                               
      ((Table *) p)->rw_delta  -                                                
      ((Table *) q)->rw_delta;
}
/*************************************************************************************/
int  compare_table_rw_desc(const void *p, const void *q)
{
  return                                                                               
      ((Table *) q)->rw_delta  -                                                
      ((Table *) p)->rw_delta;
}



/********************************************************************************/
void update_snapshot_request(Header *header, DB2List *list)
{
  DB2ListElmt * element;
  sqluint32 agentid = 0;
  /* define snapReq struct and fill it up with the current snapReq values */
  SnapReq snapReq = *(peek_snapReq(header));
  strncpy(snapReq.sequence_no, (peek_snapReq(header))->sequence_no, SQLM_SEQ_SZ);
  strncpy(snapReq.stmt_text, (peek_snapReq(header))->stmt_text, STMT_SZ);


  if (db2list_get_type(list) == agent_id && header->snapScr == wlock_list) {
    list = &(((Appl *) db2list_data(db2list_head(list)))->lock_list);
  } else if (db2list_get_type(list) == agent_id) {
    list = &(((Appl *) db2list_data(db2list_head(list)))->stmt_list);
  }

  if (db2list_size(list) < 1)
    return;

  if ((element = db2list_get_highlighted_element(list)) == NULL)
    element = db2list_head(list);

  switch (db2list_get_type(list)) {
  case appls:
    if ( element != NULL) {
      snapReq.agent_id =  ((Appl * ) db2list_data(element))->appl_handle;
      snapReq.type = agent_id;

      strcpy(snapReq.prog_nm, "");
      strcpy(snapReq.client_nm, "");
      strcpy(snapReq.auth_id, "");
      strcpy(snapReq.exec_id, "");
      if (push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case locklist: 
    if ( element != NULL) {                                               
      snapReq.agent_id =  ((Lock * ) db2list_data(element))->agent_id_holding_lk; 
      snapReq.type = agent_id;                                            
                                                                          
      strcpy(snapReq.prog_nm, "");                                      
      strcpy(snapReq.client_nm, "");                                    
      strcpy(snapReq.auth_id, "");                                      
      strcpy(snapReq.exec_id, "");                                      
      if ( snapReq.agent_id > 0 && push_snapReq(header, &snapReq) == 0)                            
        header->reinit_DB2SnapReq = TRUE;                                 
    }                                                                     
    break;
  case stmts:
    if ( element != NULL) {
      snapReq.agent_id =  ((Stmt * ) db2list_data(element))->appl_handle;
      snapReq.stmt_node_number =  ((Stmt * ) db2list_data(element))->stmt_node_number;
      snapReq.stmt_section_number =  ((Stmt * ) db2list_data(element))->section_number;
      strncpy( snapReq.sequence_no,  ((Stmt * ) db2list_data(element))->sequence_no, SQLM_SEQ_SZ);
      strncpy(snapReq.stmt_text , ((Stmt * ) db2list_data(element))->stmt_text, STMT_SZ);
      snapReq.type = agent_id_detl;

      strcpy(snapReq.prog_nm, "");
      strcpy(snapReq.client_nm, "");
      strcpy(snapReq.auth_id, "");
      strcpy(snapReq.exec_id, "");
      if (push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case agent_id_detl:
    if ( element != NULL) {
      snapReq.agent_id =  ((Appl_SS * ) db2list_data(element))->appl_handle; 
      snapReq.node =  ((Appl_SS * ) db2list_data(element))->ss_node_number;
      snapReq.type = agent_id_cmdline;

      strcpy(snapReq.prog_nm, "");
      strcpy(snapReq.client_nm, "");
      strcpy(snapReq.auth_id, "");
      strcpy(snapReq.exec_id, "");
      if (push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case utils:
    if ( element != NULL) {
      snapReq.util_id =  ((Util * ) db2list_data(element))->id;
      snapReq.type = utils_id;

      if ( push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  case tbspace:
    if ( element != NULL) {
      snapReq.tbspace_id =  ((Tbspace * ) db2list_data(element))->id;
      snapReq.tbspace_pg_sz =  ((Tbspace * ) db2list_data(element))->pg_sz;
      snapReq.tbspace_content_type =  ((Tbspace * ) db2list_data(element))->content_type;
      strncpy( snapReq.tbspace, ((Tbspace * ) db2list_data(element))->name, SQLUH_TABLESPACENAME_SZ);
      snapReq.type = tbspace_id;

      if ( push_snapReq(header, &snapReq) == 0)
        header->reinit_DB2SnapReq = TRUE;
    }
    break;
  }
  return;
}
/********************************************************************************/
void update_list_highlight(Header *header, DB2List *list, int keyboard)
{

  DB2ListElmt *element ;

  if (db2list_size(list) < 1)
    return;

  if (db2list_get_type(list) == agent_id && header->snapScr == wlock_list) 
    list = &(((Appl *) db2list_data(db2list_head(list)))->lock_list);        
  else if (db2list_get_type(list) == agent_id)                            
    list = &(((Appl *) db2list_data(db2list_head(list)))->stmt_list);        

  element = db2list_get_highlighted_element(list);
  if (element == NULL && header->mark == TRUE) {
    element = db2list_head(list);
    db2list_set_highlight(element);
    db2list_set_list_highlight(list, element);
  } else if (element != NULL && header->mark == FALSE) {
    db2list_unset_highlight(element);
    db2list_set_list_highlight(list, NULL);
  }

  if ( keyboard == KEY_UP) {
    if ( (element = db2list_get_highlighted_element(list)) != NULL   
        && db2list_prev(element) != NULL ) {
      element->highlight = FALSE;
      (db2list_prev(element))->highlight = TRUE;
      list->highlighted_element = db2list_prev(element);
    }
  } else if ( keyboard == KEY_DOWN) {
    if ( (element = db2list_get_highlighted_element(list)) != NULL 
        && db2list_next(element) != NULL ) {
      element->highlight = FALSE;
      (db2list_next(element))->highlight = TRUE;
      list->highlighted_element = db2list_next(element);
    }
  }
  return;
}
/********************************************************************************/
int compare_appl_handle(const void *p, const void *q)
{
  return
      (peek_snapReq((Header *) p))->agent_id -               
      ((Appl *)  q)->appl_handle ;
}

/********************************************************************************/
void sort_list(Header *header, DB2List *list)
{

  DB2ListElmt *element;
  OrderByCol order_by_col = (peek_snapReq(header))->order_by_col;
  Order order = (peek_snapReq(header))->order;
  int (*compare) (const void *, const void *) = NULL;
  header->db_ucpu_used_delta = 0;
  header->db_scpu_used_delta = 0;

  /* remove disconnected appls|utils|appl_ss */
  db2list_cleanup(list);

  if ( (peek_snapReq(header))->type == appls || (peek_snapReq(header))->type == agent_id) {
    for(element = db2list_head(list); element != NULL; 
        element = db2list_next(element)) {
      header->db_ucpu_used_delta += ((Appl *) db2list_data(element))->ucpu_used_delta;
      header->db_scpu_used_delta += ((Appl *) db2list_data(element))->scpu_used_delta;
    }
  } else if ((peek_snapReq(header))->type == agent_id_detl) {
    for(element = db2list_head(list); element != NULL; 
        element = db2list_next(element)) {
      header->db_ucpu_used_delta += ((Appl_SS *) db2list_data(element))->ss_ucpu_used_delta;
      header->db_scpu_used_delta += ((Appl_SS *) db2list_data(element))->ss_scpu_used_delta;
    }
  } else if ((peek_snapReq(header))->type == stmts) {
    for(element = db2list_head(list); element != NULL; 
        element = db2list_next(element)) {
      header->db_ucpu_used_delta += ((Stmt *) db2list_data(element))->ucpu_used_delta;
      header->db_scpu_used_delta += ((Stmt *) db2list_data(element))->scpu_used_delta;
    }
  }

  switch((peek_snapReq(header))->type) {
  case appls:
  case agent_id:
    switch(order_by_col) {
tqr:
      if (order == asc)
        compare = compare_appl_tqr_asc;
      else
        compare = compare_appl_tqr_desc;
      break;
tqw:
      if (order == asc)
        compare = compare_appl_tqw_asc;
      else
        compare = compare_appl_tqw_desc;
      break;
rr:
      if (order == asc)
        compare = compare_appl_rr_asc;
      else
        compare = compare_appl_rr_desc;
      break;
rw:
      if (order == asc)
        compare = compare_appl_rw_asc;
      else
        compare = compare_appl_rw_desc;
      break;
sel:
      if (order == asc)
        compare = compare_appl_sel_asc;
      else
        compare = compare_appl_sel_desc;
      break;
ins:
      if (order == asc)
        compare = compare_appl_ins_asc;
      else
        compare = compare_appl_ins_desc;
      break;
upd:
      if (order == asc)
        compare = compare_appl_upd_asc;
      else
        compare = compare_appl_upd_desc;
      break;
del:
      if (order == asc)
        compare = compare_appl_del_asc;
      else
        compare = compare_appl_del_desc;
      break;
memusg:
      if (order == asc)
        compare = compare_appl_memusg_asc;
      else
        compare = compare_appl_memusg_desc;
      break;
bppgin:
      if (order == asc)
        compare = compare_appl_bp_asc;
      else
        compare = compare_appl_bp_desc;
      break;
logusg:
      if (order == asc)
        compare = compare_appl_logusg_asc;
      else
        compare = compare_appl_logusg_desc;
      break;
    default:
      if (order == asc)
        compare = compare_appl_cpu_asc;
      else
        compare = compare_appl_cpu_desc;
    } /* appl|agent_id order_by_col*/
    break;
  case stmts:
    switch(order_by_col) {
    default:
      if (order == asc)
        compare = compare_stmt_cpu_asc;
      else
        compare = compare_stmt_cpu_desc;

    }
    break;
  case agent_id_detl:
    switch(order_by_col) {
    case tqr:
      if (order == asc)
        compare = compare_appl_ss_tqr_asc;
      else
        compare = compare_appl_ss_tqr_desc;
      break;
    case tqw:
      if (order == asc)
        compare = compare_appl_ss_tqw_asc;
      else
        compare = compare_appl_ss_tqr_desc;
      break;
    case rr:
      if (order == asc)
        compare = compare_appl_ss_rr_asc;
      else
        compare = compare_appl_ss_rr_desc;
      break;
    case rw:
      if (order == asc)
        compare = compare_appl_ss_rw_asc;
      else
        compare = compare_appl_ss_rw_desc;
      break;
    case node:
      if (order == asc)
        compare = compare_appl_ss_node_asc;
      else
        compare = compare_appl_ss_node_desc;
      break;
    case nagents:
      if (order == asc)
        compare = compare_appl_ss_nagents_asc;
      else
        compare = compare_appl_ss_nagents_desc;
      break;
    default:
      if (order == asc)
        compare = compare_appl_ss_cpu_desc;
      else
        compare = compare_appl_ss_cpu_desc;
    } /*agent_id_detl order_by_col*/
    break;
  case utils_id:
    switch(order_by_col) {
    default:
      if (order == desc)
        compare = compare_utils_id_node_desc;
      else
        compare = compare_utils_id_node_asc;
    } /* utils_id order_by_col */
    break;
  case tbspace:
    switch(order_by_col) {
    case rw:
      if (order == desc)
        compare = compare_tbspace_rw_desc;
      else
        compare = compare_tbspace_rw_asc;
    default:
      if (order == desc)
        compare = compare_tbspace_rr_desc;
      else
        compare = compare_tbspace_rr_asc;
    }
    break;
  case tbspace_id:
    switch(order_by_col) {
    case rw:
      if (order == desc)
        compare = compare_table_rw_desc;
      else
        compare = compare_table_rw_asc;
    default:
      if (order == desc)
        compare = compare_table_rr_desc;
      else
        compare = compare_table_rr_asc;
    }
    break;
  } /*end of snapshot type */

  /* sort */
  if (compare != NULL)
    db2list_sort(list, compare);

  return;

}
/********************************************************************************/
void reclen_table(Header *header, DB2List *list)
{

  char buffer[256];
  int rc =0;
  DB2ListElmt * element;
  Table *table;
  const SnapReq *snapReq;
  GetReclenArgs thread_data;

  snapReq  = peek_snapReq(header);
  strncpy(thread_data.dbAlias, snapReq->dbname, SQL_DBNAME_SZ);
  if (strlen(header->pswd)  > 0)
    strncpy(thread_data.user, header->user_id, USERID_SZ);
  else 
    strncpy(thread_data.user, "",  USERID_SZ);

  strncpy(thread_data.pswd, header->pswd, PSWD_SZ);

  if (snapReq->tbspace_content_type != SQLM_TABLESPACE_CONTENT_SYSTEMP
      && snapReq->tbspace_content_type != SQLM_TABLESPACE_CONTENT_USRTEMP) {
    for (element = db2list_head(list); element != NULL; element = db2list_next(element)) {
      table = db2list_data(element);
      if (table->avg_row_sz == -3)
        table->avg_row_sz = -2;
      else if ((table->rr_delta > 0 || table->rw_delta > 0) && table->avg_row_sz == -2) {
        /* get avg record length from syscat.tables */
        thread_data.table = table;
        rc = getreclen(&thread_data);
      }
    }
  }
  return;
}
/********************************************************************************/
int getreclen(void *p) {

  char buffer[1024];
  GetReclenArgs *thread_args = (GetReclenArgs *) p;
  SQLRETURN cliRC = SQL_SUCCESS;
  int rc = 0;
  SQLHANDLE henv; /* environment handle */
  SQLHANDLE hdbc; /* connection handle */

  rc = CLIAppInit(thread_args->dbAlias,                       
      thread_args->user,                          
      thread_args->pswd,                          
      &henv,                         
      &hdbc,                         
      (SQLPOINTER)SQL_AUTOCOMMIT_ON);

  if (rc != 0) {
    thread_args->table->avg_row_sz = -1;
    return rc;
  }

  rc = getreclen_select(hdbc
      , (thread_args->table)->tabschema
      , (thread_args->table)->tabname
      , &((thread_args->table)->avg_row_sz) );

  if (rc != 0)
    return rc;
  else {
    sprintf(buffer, "%s.%s reclen = %d "
        , (thread_args->table)->tabschema 
        , (thread_args->table)->tabname 
        , (thread_args->table)->avg_row_sz);
    INFO_MSG(buffer);
  }

  rc = CLIAppTerm(&henv, &hdbc, thread_args->dbAlias);
  return rc;

}
/********************************************************************************/
int getreclen_select(SQLHANDLE hdbc,char tabschema[], char tabname[], sqlint16 *reclen)
{
  SQLRETURN cliRC = SQL_SUCCESS;
  int rc = 0;
  SQLHANDLE hstmt; /* statement handle */

  *reclen = -1;
  SQLCHAR *stmt = (SQLCHAR *)
      "SELECT avgrowsize FROM syscat.tables WHERE tabschema = ? and tabname = ?";


  struct
    {
    SQLINTEGER ind;
    SQLSMALLINT val;
  }  avgrowsize; /* variable to be bound to the AVGROWSIZE column */

  /* set AUTOCOMMIT on */
  cliRC = SQLSetConnectAttr(hdbc,
      SQL_ATTR_AUTOCOMMIT,
      (SQLPOINTER)SQL_AUTOCOMMIT_ON,
      SQL_NTS);
  DBC_HANDLE_CHECK(hdbc, cliRC);

  /* allocate a statement handle */
  cliRC = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  DBC_HANDLE_CHECK(hdbc, cliRC);

  /* prepare the statement */
  cliRC = SQLPrepare(hstmt, stmt, SQL_NTS);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  /* bind tabschema and tabname to the statement */
  cliRC = SQLBindParameter(hstmt,
      1,
      SQL_PARAM_INPUT,
      SQL_C_CHAR,
      SQL_CHAR,
      TABSCHEMA_SZ,
      0,
      tabschema,
      TABSCHEMA_SZ,
      NULL);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  cliRC = SQLBindParameter(hstmt,
      2,
      SQL_PARAM_INPUT,
      SQL_C_CHAR,
      SQL_CHAR,
      TABNAME_SZ,
      0,
      tabname,
      TABNAME_SZ,
      NULL);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);


  /* execute the statement */
  cliRC = SQLExecute(hstmt);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  /* bind column 1 to variable */
  cliRC = SQLBindCol(hstmt, 1, SQL_C_SHORT, &avgrowsize.val, 0, &avgrowsize.ind);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  /* fetch each row */
  cliRC = SQLFetch(hstmt);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  while (cliRC != SQL_NO_DATA_FOUND)
  {

    *reclen = avgrowsize.val;

    /* fetch next row */
    cliRC = SQLFetch(hstmt);
    STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);
  }

  /* free the statement handle */
  cliRC = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  STMT_HANDLE_CHECK(hstmt, hdbc, cliRC);

  return rc;
}
