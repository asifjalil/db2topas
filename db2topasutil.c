#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>   
#include <sys/param.h>
#include <sqlcli1.h>
#include <sqlutil.h>  
#include <db2ApiDf.h> 
#include <sqlenv.h>   
#include <pthread.h>

#include "db2topasutil.h"

/* private functions */
void HandleLocationPrint(db2AdminMsgWriteStruct *msg, SQLRETURN cliRC) ;
void HandleDiagnosticsPrint(db2AdminMsgWriteStruct *msg, SQLSMALLINT htype, SQLHANDLE hndl) ;

void  SqlInfoPrint(char *appMsg, struct sqlca *pSqlca, int line, char *file)
{
  int                    rc = 0;
  char                   sqlInfo[1024];
  char                   sqlInfoToken[1024];
  char                   sqlstateMsg[1024];
  char                   errorMsg[1024];
  db2AdminMsgWriteStruct msg;
  struct                 sqlca msg_sqlca;


  if (pSqlca->sqlcode != 0 && pSqlca->sqlcode != 100) {
    sprintf(sqlInfo, "");

    sprintf(sqlInfoToken, "application message = %s\n", appMsg);
    strcat(sqlInfo, sqlInfoToken);
    sprintf(sqlInfoToken, "line= %d ", line);
    strcat(sqlInfo, sqlInfoToken);
    sprintf(sqlInfoToken, "file= %s ", file);
    strcat(sqlInfo, sqlInfoToken);
    sprintf(sqlInfoToken, "SQLCODE= %d\n", pSqlca->sqlcode);
    strcat(sqlInfo, sqlInfoToken);

    /* get error message */
    rc = sqlaintp(errorMsg, 1024, 80, pSqlca);
    if (rc > 0) /* return code is the length of the errorMsg string */ {
      sprintf(sqlInfoToken, "%s", errorMsg);
      strcat(sqlInfo, sqlInfoToken);
    }

    /* get SQLSTATE message */
    rc = sqlogstt(sqlstateMsg, 1024, 80, pSqlca->sqlstate);
    if (rc > 0) {
      sprintf(sqlInfoToken, "%s\n", sqlstateMsg);
      strcat(sqlInfo, sqlInfoToken);
    }

    if (pSqlca->sqlcode < 0) {
      msg.iError_type = DB2LOG_ERROR; 
    } else
    {
      msg.iError_type = DB2LOG_WARNING; 
      strcat(sqlInfo, sqlInfoToken);

    } /* endif */
    msg.iMsgType = STRING_MSG ;                               
    msg.iComponent = 0;                                       
    msg.iFunction = 0;                                        
    msg.iProbeID = line;                                      
    msg.piData_title = file;                                  
    msg.piData = sqlInfo;                                     
    db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca); 
  } /* endif */

  return;
} /* SqlInfoPrint */


/*****************************************************************************/
int HandleInfoPrint(SQLSMALLINT htype, /* handle type identifier */          
                    SQLHANDLE hndl, /* handle used by the CLI function */    
                    SQLRETURN cliRC, /* return code of the CLI function */   
                    int line,                                                
                    char *file)                                              
{                                                                            
  int rc = 0;                                                                
  char buffer[1024];
  db2AdminMsgWriteStruct msg;

  msg.iMsgType = STRING_MSG ;
  msg.iComponent = 0;
  msg.iFunction = 0;
  msg.iProbeID = line;
  
                                                                             
  switch (cliRC)                                                             
  {                                                                          
    case SQL_SUCCESS:                                                        
      rc = 0;                                                                
      break;                                                                 
    case SQL_INVALID_HANDLE:                                                 
      sprintf(buffer, "CLI Invalid Handle, file %s, line %d", file, line);
      msg.piData_title = buffer;
      msg.iError_type = DB2LOG_ERROR; 
      HandleLocationPrint(&msg, cliRC);                                
      rc = 1;                                                                
      break;                                      
    case SQL_ERROR:                               
      sprintf(buffer, "CLI SQL Error, file %s, line %d", file, line);
      msg.piData_title = buffer;
      msg.iError_type = DB2LOG_ERROR; 
      HandleLocationPrint(&msg, cliRC);                                
      HandleDiagnosticsPrint(&msg, htype, hndl);        
      rc = 2;                                     
      break;                                      
    case SQL_SUCCESS_WITH_INFO:                   
      rc = 0;                                     
      break;                                      
    case SQL_STILL_EXECUTING:                     
      rc = 0;                                     
      break;                                      
    case SQL_NEED_DATA:                           
      rc = 0;                                     
      break;                                      
    case SQL_NO_DATA_FOUND:                       
      rc = 0;                                     
      break;                                      
    default:                                      
      sprintf(buffer, "CLI Default, file %s, line %d", file, line);
      msg.piData_title = buffer;
      msg.iError_type = DB2LOG_ERROR; 
      HandleLocationPrint(&msg, cliRC);                                
      rc = 3;                                     
      break;           
  }                    
                       
  return rc;           
} /* HandleInfoPrint */

/*****************************************************************************/
void HandleLocationPrint(db2AdminMsgWriteStruct *msg, SQLRETURN cliRC)
{                                                              
  char buffer[1024];
  struct                 sqlca msg_sqlca; 

  sprintf(buffer, "cliRC = %d", cliRC);                             
  msg->piData = buffer;                                    
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, msg, &msg_sqlca);
  return;
} /* HandleLocationPrint */                                    
/*****************************************************************************/
void HandleDiagnosticsPrint(db2AdminMsgWriteStruct *msg, SQLSMALLINT htype, SQLHANDLE hndl)                 
{                                                                         
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];                            
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];                                
  SQLINTEGER sqlcode;                                                     
  SQLSMALLINT length, i;                                                  
  struct                 sqlca msg_sqlca; 
  char buffer [ SQL_MAX_MESSAGE_LENGTH + SQL_SQLSTATE_SIZE + 512];
                                                                          
  i = 1;                                                                  
                                                                          
  /* get multiple field settings of diagnostic record */                  
  while (SQLGetDiagRec(htype,                                             
                       hndl,                                              
                       i,                                                 
                       sqlstate,                                          
                       &sqlcode,                                          
                       message,                                           
                       SQL_MAX_MESSAGE_LENGTH + 1,                        
                       &length) == SQL_SUCCESS)                           
  {                                                                       
    sprintf(buffer, "sqlstate = %s, sqlcode = %d\nmessage = %s", sqlstate,sqlcode,message);
    msg->piData = buffer;                                    
    db2AdminMsgWrite(SQLM_CURRENT_VERSION, msg, &msg_sqlca);
    i++;                                            
  }                                                 
                                                    
  return;
} /* HandleDiagnosticsPrint */                      

/*****************************************************************************/
int  InstanceAttach(char nodeName[],
char user[],
char pswd[])
{
  struct sqlca sqlca;
  char  *pSpace;
  char  mesg[1024]; 

  if (strlen(nodeName) > 0) {
    
    sprintf(mesg, "##############  ATTACH TO THE INSTANCE: %s #######",
        nodeName);
    INFO_MSG(mesg);

    /* attach to instance */

    if (strlen(pswd) > 0)
      sqleatin(nodeName, user, pswd, &sqlca);
    else
      sqleatin(nodeName, '\0', '\0', &sqlca);
    sprintf(mesg, "instance attach to %s", nodeName);
    DB2_API_CHECK(mesg);
  } else {
    /* attach to default instance DB2INSTANCE */
    sqlegins(nodeName, &sqlca);
    DB2_API_CHECK("instance -- get");
    /* Trim spaces from instance name if present */
    strrtrim(nodeName);
    sqleatin(nodeName, '\0', '\0', &sqlca);
    sprintf(mesg, "instance attach to %s", nodeName);
    DB2_API_CHECK(mesg);
  }

  return 0;
} /* InstanceAttach */


/*****************************************************************************/
int  InstanceDetach(char *nodeName)
{
  char mesg[1024];
  struct sqlca sqlca;

  if (strlen(nodeName) > 0) {
    /* detach from an instance */
    sqledtin(&sqlca);
    sprintf(mesg, "instance detach from %s", nodeName);
    DB2_API_CHECK(mesg);
  }

  return 0;
} /* InstanceDetach */

/*****************************************************************************/
char  *strtrim(char *str)
{
  return strltrim(strrtrim(str));
}


/*****************************************************************************/

char  *strrtrim(char *str)
{
  char  *end;

  if (!str)
    return '\0';

  /* point right before NULL char */
  end = str + strlen(str) - 1;

  while (end >= str) {
    if (isspace(*end))
      *end-- = '\0';
    else
      return str;
  }
  return str;
}


/***********************************************************/
char  *strltrim(char *str)
{
  if (!str)
    return '\0';

  while (*str) {
    if (isspace(*str))
      str++;
    else
      return str;
  }

  return str;
}


/***********************************************************/
char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}


/*********************************************************************/
char  *basename (const char *name)
{
  const char  *base = name;

  while (*name) {
    if (*name++ == '/') {
      base = name;
    }
  }
  return (char *) base;
}


/**********************************************************************/

Header *init_header() 
{

  int  rc = 0;
  SnapReq snapReq;
  Header * header = malloc(sizeof(Header));
  if (header == NULL) {
    WARN_MSG("Failed to allocate memory for header");
    return header;
  }

  /* initialize stack and add snapshot req for all appls*/
  stack_init(&(header->snap_req_stack), free);
  snapReq.type = appls;
  snapReq.agent_id = 0;
  snapReq.util_id = 0;
  snapReq.tbspace_id = 1000000000;
  snapReq.tbspace_pg_sz = 0;
  strcpy(snapReq.auth_id, "");
  strcpy(snapReq.exec_id, "");
  strcpy(snapReq.prog_nm, "");
  strcpy(snapReq.client_nm, "");
  strcpy(snapReq.dbname, "");
  strcpy(snapReq.tbspace, "");
  strcpy(snapReq.stmt_text, "");
  snapReq.stmt_node_number = 1000;
  snapReq.stmt_section_number = 999999;
  snapReq.node = -1;
  snapReq.order_by_col = cpu;
  snapReq.order = desc;

  rc = push_snapReq(header, &snapReq);
  if (rc  <  0) {
    WARN_MSG("Couldn't add snapshot req to stack");
    stack_destroy(&(header->snap_req_stack));
    free(header);
    return NULL;
  }


  header->show_dbase_list = TRUE;
  header->show_appl_list = TRUE;
  header->show_lock_list = TRUE;
  header->show_stmt_list = TRUE;

  header->snapScr = wstmt_list;
  strcpy(header->myname, "N/A");
  header->spin = 0;
  strcpy(header->pswd, "");
  if (gethostname(header->hostname, MAXHOSTNAMELEN) != 0)  
    strcpy(header->hostname, "unknown");
  strcpy(header->db2node, "000");
  if (getenv("DB2NODE") != NULL) 
    strncpy(header->db2node, getenv("DB2NODE"), 4);
  strcpy(header->user_id, "unknown");
  if (getlogin() != NULL) 
    strncpy(header->user_id, getlogin(), USERID_SZ);
  header->interval = 2;

  /* these options are set based on the assumption
     that appls snapshot will be taken
  */
  header->col1_opt = appl_handle;
  header->col3_opt = auth_id;
  header->col4_opt = appl_status;
  header->col8_opt = bp;
  header->col12_opt = log_usg;

  header->reinit_DB2SnapReq = TRUE;

  header->appls_connected = 0;
  header->appls_executing = 0;
  header->db_assoc_agents = 0;
  header->db_memusg = 0;
  header->db_genheap = 0;
  header->db_sortheap = 0;
  header->db_lockheap =0;
  header->db_utilheap =0;

  (header->t1_snapshot_timestamp).seconds = 0;
  (header->t2_snapshot_timestamp).seconds = 0;
  (header->t1_snapshot_timestamp).microsec = 0;
  (header->t2_snapshot_timestamp).microsec = 0;

  header->db_ucpu_used_delta = 0;
  header->db_scpu_used_delta = 0;

  header->t1_db_bpr_tm = 0;
  header->t1_db_bpw_tm = 0;
  header->t2_db_bpr_tm = header->t1_db_bpr_tm;
  header->t2_db_bpw_tm = header->t1_db_bpw_tm;
  
  header->t2_db_buffered_rio = 0;
  header->t2_db_buffered_wio = 0;
  header->t1_db_buffered_rio = header->t2_db_buffered_rio;
  header->t1_db_buffered_wio = header->t2_db_buffered_wio;

  header->t2_db_direct_io = 0;
  header->t1_db_direct_io = header->t2_db_direct_io;
  header->t2_db_direct_io_reqs = 0;
  header->t1_db_direct_io_reqs = header->t2_db_direct_io_reqs;

  header->t1_db_io_type_read = 0;   
  header->t2_db_io_type_read = 0;   
  header->t1_db_io_type_write = 0;  
  header->t2_db_io_type_write = 0;  
  header->t1_db_io_type_data = 0;   
  header->t2_db_io_type_data = 0;   
  header->t1_db_io_type_idx = 0;    
  header->t2_db_io_type_idx = 0;    
  header->t1_db_io_type_temp = 0;   
  header->t2_db_io_type_temp = 0;   
  header->t1_db_io_type_xml = 0;    
  header->t2_db_io_type_xml = 0;    
  header->t1_db_io_type_dio = 0;    
  header->t2_db_io_type_dio = 0;    

  header->t2_db_log_reads = 0;
  header->t2_db_log_writes = 0;
  header->t1_db_log_reads = 0;
  header->t1_db_log_writes = 0;
  header->db_log_avail = 0;
  header->db_log_used = 0;
  header->smallest_log_avail_node = 1000;
  header->agent_id_oldest_xact = 0;

  header->mark = FALSE;
  header->dump = FALSE;
  header->dbase_scrn_row = 4;


  return header;
} /* init_header */




/********************************************************************************/
void  free_header(Header *header) 
{

  if (header != NULL) {
    stack_destroy(&(header->snap_req_stack));
    free(header);
  }

} /*free_header*/


/********************************************************************************/
const SnapReq *peek_snapReq(Header *header) 
{
  const SnapReq * snapReq;

  snapReq = stack_peek(&(header->snap_req_stack));
  if (snapReq == NULL)
    return NULL;

  return snapReq;

} /*peek_snapReq */



/********************************************************************************/
int num_snapReq(Header *header) {
  return stack_size(&(header->snap_req_stack));
}
/********************************************************************************/
int  pop_snapReq(Header *header) 
{

  SnapReq * snapReq;
  int  rc = 0;
  if ( num_snapReq(header) > 1) {
    rc = stack_pop(&(header->snap_req_stack), (void * *) &snapReq) ;
    if (rc == 0)
      free(snapReq);
    return rc;
  }

  return - 1;
} /*pop_snapReq*/


/**********************************************************************************/
int  push_snapReq(Header *header, const SnapReq *snapReq) 
{

  const SnapReq * oldSnapReq = peek_snapReq(header);

  if (snapReq == NULL)
    return -1;

  /* don't add snapReq if it is the same  as what's on the stack */
  if (oldSnapReq != NULL
      && oldSnapReq->type == snapReq->type
      && oldSnapReq->agent_id == snapReq->agent_id
      && oldSnapReq->util_id == snapReq->util_id
      && strncasecmp(oldSnapReq->exec_id,snapReq->exec_id,USERID_SZ) == 0
      && strncasecmp(oldSnapReq->auth_id,snapReq->auth_id,USERID_SZ) == 0
      && strncasecmp(oldSnapReq->prog_nm,snapReq->prog_nm,USERID_SZ) == 0
      && strncasecmp(oldSnapReq->client_nm,snapReq->client_nm,USERID_SZ) == 0
      && strncasecmp(oldSnapReq->dbname,snapReq->dbname,SQL_DBNAME_SZ) == 0
      && oldSnapReq->node == snapReq->node)
      return 1; 
     
  SnapReq * pSnapReq = malloc(sizeof(SnapReq));
  if (pSnapReq == NULL) {
    return - 1;
  }

  *pSnapReq = *snapReq;
  return stack_push(&(header->snap_req_stack), (void *) pSnapReq);

} /*push_snapReq */

/******************************************************************************************/
char  *time_STRING  ( sqlm_timestamp timestamp, char *timeString)
{   /* Event Monitor returns GMT time, adjust it to local time*/

  sqluint64 seconds = (sqluint64) timestamp.seconds;
  sqluint32 microsec = timestamp.microsec;
  struct tm *pstTm;
  pstTm = localtime((time_t * ) & seconds);
  if (seconds == 0)
    strcpy(timeString, "");
  else  {
    if (microsec < 0) {
      sprintf(timeString, "%02d/%02d/%04d %02d:%02d",                            
          pstTm->tm_mon + 1, pstTm->tm_mday, pstTm->tm_year + 1900,              
          pstTm->tm_hour, pstTm->tm_min);
    } else    {
      sprintf(timeString, "%02d/%02d/%04d %02d:%02d",                            
          pstTm->tm_mon + 1, pstTm->tm_mday, pstTm->tm_year + 1900,              
          pstTm->tm_hour, pstTm->tm_min);
    }
  }

  return timeString;
} /* end of time_STRING*/




/******************************************************************************************/
/* initialize a CLI application by:                                  
     o  allocating an environment handle                             
     o  allocating a connection handle                               
     o  setting AUTOCOMMIT                                           
     o  connecting to the database */                                
int CLIAppInit(char dbAlias[],                                       
               char user[],                                          
               char pswd[],                                          
               SQLHANDLE *pHenv,                                     
               SQLHANDLE *pHdbc,                                     
               SQLPOINTER autocommitValue)                           
{                                                                    
  SQLRETURN              cliRC = SQL_SUCCESS;                                     
  char                   buffer[1024];
  int                    rc = 0;                                                        
  db2AdminMsgWriteStruct msg;
  struct                 sqlca msg_sqlca;
  
  msg.iMsgType = STRING_MSG ;  
  msg.iComponent = 0;          
  msg.iFunction = 0;           
                                                                     
  /* allocate an environment handle */                               
  cliRC = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, pHenv);    
  if (cliRC != SQL_SUCCESS)                                          
  {                                                                  
    msg.iProbeID = __LINE__;         
    msg.iError_type = DB2LOG_ERROR; 
    msg.piData_title = "ERROR while allocating the environment handle ";
    sprintf(buffer, " cliRC = %d, line = %d, file = %d ", cliRC, __LINE__, __FILE__);                     
    msg.piData = buffer;

    db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);
    return 1;                                                               
  }                                                                         
                                                                            
  /* set attribute to enable application to run as ODBC 3.0 application */  
  cliRC = SQLSetEnvAttr(*pHenv,                                             
                     SQL_ATTR_ODBC_VERSION,                                 
                     (void *)SQL_OV_ODBC3,                                  
                     0);                                                    
  ENV_HANDLE_CHECK(*pHenv, cliRC);                                          
                                                                            
  /* allocate a database connection handle */                               
  cliRC = SQLAllocHandle(SQL_HANDLE_DBC, *pHenv, pHdbc);                    
  ENV_HANDLE_CHECK(*pHenv, cliRC);                                          
                                                                            
  /* set AUTOCOMMIT off or on */                                            
  cliRC = SQLSetConnectAttr(*pHdbc,                                         
                            SQL_ATTR_AUTOCOMMIT,                            
                            autocommitValue,                                
                            SQL_NTS);                                       
  DBC_HANDLE_CHECK(*pHdbc, cliRC);                                          
                                                                            
  /* connect to the database */            
  cliRC = SQLConnect(*pHdbc,               
                     (SQLCHAR *)dbAlias,   
                     SQL_NTS,              
                     (SQLCHAR *)user,      
                     SQL_NTS,              
                     (SQLCHAR *)pswd,      
                     SQL_NTS);             

  if (cliRC != SQL_SUCCESS) {
    sprintf(buffer, " Can't connect to dbase %s using id %s", dbAlias, user); 
    msg.iProbeID = __LINE__;         
    msg.piData_title = "ERROR while connecting to database ";
    msg.iError_type = DB2LOG_WARNING; 
    msg.piData = buffer;
    db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);
  } 
  DBC_HANDLE_CHECK(*pHdbc, cliRC);         

  sprintf(buffer, " Connected to %s. ", dbAlias); 
  msg.piData_title = "Success";
  msg.iProbeID = __LINE__;         
  msg.iError_type = DB2LOG_INFORMATION; 
  msg.piData = buffer;
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);
                                           
  return 0;                                
} /* CLIAppInit */                         

/******************************************************************************************/
/* terminate a CLI application by:                                   
     o  disconnecting from the database                              
     o  freeing the connection handle                                
     o  freeing the environment handle */                            
int CLIAppTerm(SQLHANDLE * pHenv, SQLHANDLE * pHdbc, char dbAlias[]) 
{                                                                    
  SQLRETURN cliRC = SQL_SUCCESS;                                     
  int rc = 0;                                                        
  char buffer[1024];
  db2AdminMsgWriteStruct msg;            
  struct                 sqlca msg_sqlca;

  /* disconnect from the database */                                 
  cliRC = SQLDisconnect(*pHdbc);                                     
  DBC_HANDLE_CHECK(*pHdbc, cliRC);                                   
                                                                     
  sprintf(buffer, " Disconnected from dbAlias %s ", dbAlias);                      
  msg.iMsgType = STRING_MSG ;                              
  msg.iComponent = 0;                                      
  msg.iFunction = 0;                                       
  msg.iProbeID = __LINE__;                                     
  msg.piData_title = __FILE__;                                 
  msg.piData = buffer;                                    
  msg.iError_type = DB2LOG_INFORMATION;
  db2AdminMsgWrite(SQLM_CURRENT_VERSION, &msg, &msg_sqlca);
                                                                     
  /* free connection handle */                                       
  cliRC = SQLFreeHandle(SQL_HANDLE_DBC, *pHdbc);                     
  DBC_HANDLE_CHECK(*pHdbc, cliRC);                                   
                                                                     
  /* free environment handle */                                      
  cliRC = SQLFreeHandle(SQL_HANDLE_ENV, *pHenv);                     
  ENV_HANDLE_CHECK(*pHenv, cliRC);  
                                    
  return 0;                         
} /* CLIAppTerm */                  
/******************************************************************************************/
int StmtResourcesFree(SQLHANDLE hstmt)                                     
{                                                                          
  SQLRETURN cliRC = SQL_SUCCESS;                                           
  int rc = 0;                                                              
                                                                           
  /* free the statement handle */                                          
  cliRC = SQLFreeStmt(hstmt, SQL_UNBIND);                                  
  rc = HandleInfoPrint(SQL_HANDLE_STMT, hstmt, cliRC, __LINE__, __FILE__); 
  if (rc != 0)                                                             
  {                                                                        
    return 1;                                                              
  }                                                                        
                                                                           
  /* free the statement handle */                                          
  cliRC = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);                            
  rc = HandleInfoPrint(SQL_HANDLE_STMT, hstmt, cliRC, __LINE__, __FILE__); 
  if (rc != 0)                                                             
  {                                                                        
    return 1;                                                              
  }                                                                        
                                                                           
  /* free the statement handle */                                         
  cliRC = SQLFreeStmt(hstmt, SQL_CLOSE);                                  
  rc = HandleInfoPrint(SQL_HANDLE_STMT, hstmt, cliRC, __LINE__, __FILE__);
  if (rc != 0)                                                            
  {                                                                       
    return 1;                                                             
  }                                                                       
                                                                          
  return 0;                                                               
} /* StmtResourcesFree */                                                 
/******************************************************************************************/
void TransRollback(SQLHANDLE hdbc)                                      
{                                                                       
  SQLRETURN cliRC = SQL_SUCCESS;                                        
  int rc = 0;                                                           
                                                                        
  /* end transactions on the connection */                              
  cliRC = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);               
  rc = HandleInfoPrint(SQL_HANDLE_DBC, hdbc, cliRC, __LINE__, __FILE__);
} /* TransRollback */                                                   
/******************************************************************************************/
void chomp(char *s) {
    while(*s && *s != '\n' && *s != '\r') s++;

    *s = 0;
}
/******************************************************************************************/
sqluint32 timediff(sqlm_time T2, sqlm_time T1)
{

  if ( ((T2.seconds * 1000000) + T2.microsec) >
       ((T1.seconds * 1000000) + T1.microsec))
    return ((T2.seconds * 1000000) + T2.microsec) - 
           ((T1.seconds * 1000000) + T1.microsec);
  else
    return 0;
}
/******************************************************************************************/
sqluint32 timestampdiff(sqlm_timestamp T2, sqlm_timestamp T1)
{
  if ( ((T2.seconds * 1000000) + T2.microsec) >
       ((T1.seconds * 1000000) + T1.microsec))
    return ((T2.seconds * 1000000) + T2.microsec) - 
           ((T1.seconds * 1000000) + T1.microsec);
  else
    return 0;

}
