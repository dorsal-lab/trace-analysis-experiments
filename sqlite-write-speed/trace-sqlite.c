// This library is called upon each function entry and exit.
// It inserts into an SQLite database the associated trace records

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sched.h>
#include "sqlite3.h"

static sqlite3 *db;
static sqlite3_stmt *log_function_entry, *log_function_exit;
static unsigned transaction_insertions;
static unsigned cpuid, vpid, vtid;

void __attribute__((destructor)) trace_database_finalize();
void __attribute__((constructor)) trace_database_init();

static void Check(int ret, unsigned lineno, char *msg)
{
  if(ret != SQLITE_OK && ret != SQLITE_ROW && ret != SQLITE_DONE) fprintf(stderr, "Error %d returned on line %d, %s\n", ret, lineno, msg);
}

// Initialise the database and create one table for function entries and one for exits.
// Prepare the insert statements used thereafter.
// Each event is timestamp, event type (entry / exit), cpuid, vpid, vtid, adr, call site

static int init_done = 0;

void trace_database_init()
{
  char *msg = 0;
  int ret;
  
  if(init_done != 0) return;
  init_done = 1;

  ret = sqlite3_open("TestTrace.sql", &db); Check(ret, 1, "");
  ret = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS FunctionEntry (timestamp INTEGER, cpuid INTEGER, vpid INTEGER, vtid INTEGER, addr INTEGER, caller INTEGER)", NULL, NULL, &msg); Check(ret, 2, msg);
  ret = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS FunctionExit (timestamp INTEGER, cpuid INTEGER, vpid INTEGER, vtid INTEGER, addr INTEGER, caller INTEGER)", NULL, NULL, &msg); Check(ret, 3, msg);

  ret = sqlite3_exec(db, "PRAGMA journal_mode = off;", NULL, NULL, &msg); Check(ret, 4, msg);
  ret = sqlite3_exec(db, "PRAGMA synchronous = 0;", NULL, NULL, &msg); Check(ret, 5, msg);
  ret = sqlite3_exec(db, "PRAGMA page_size = 16384;", NULL, NULL, &msg); Check(ret, 6, msg);
  ret = sqlite3_exec(db, "PRAGMA cache_size = 300000;", NULL, NULL, &msg); Check(ret, 7, msg);
  ret = sqlite3_prepare_v2(db, "INSERT INTO FunctionEntry VALUES(?, ?, ?, ?, ?, ?)", -1, &log_function_entry, NULL); Check(ret, 8, "");
  ret = sqlite3_prepare_v2(db, "INSERT INTO FunctionExit VALUES(?, ?, ?, ?, ?, ?)", -1, &log_function_exit, NULL); Check(ret, 9, "");
  ret = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &msg); Check(ret, 10, msg);
  transaction_insertions = 0;
  cpuid = (unsigned)sched_getcpu();
  vpid = (unsigned)getpid();
  vtid = (unsigned)gettid();
}

// Finalize the database by ending the ongoing transactions and closing the files
void trace_database_finalize()
{
  char *msg = 0;
  int ret;
  
  ret = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &msg); Check(ret, __LINE__, msg);
  ret = sqlite3_finalize(log_function_entry); Check(ret, __LINE__, "");
  ret = sqlite3_finalize(log_function_exit); Check(ret, __LINE__, "");
  ret = sqlite3_close(db); Check(ret, __LINE__, "");
}

// This should be the fastest way to read a timestamp
static inline uint64_t RDTSC()
{
  unsigned int hi, lo;
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

void __cyg_profile_func_enter(void *this_fn, void *call_site) __attribute__((no_instrument_function));

void __cyg_profile_func_exit(void *this_fn, void *call_site) __attribute__((no_instrument_function));

void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
  char *msg = 0;
  int ret;
  unsigned long timestamp = RDTSC();
  
  // timestamp, cpuid, vpid, vtid, addr, caller
  ret = sqlite3_bind_int64(log_function_entry, 1, timestamp); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 2, cpuid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 3, vpid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 4, vtid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int64(log_function_entry, 5, (int64_t)this_fn); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int64(log_function_entry, 6, (int64_t)call_site); Check(ret, __LINE__, "");
  ret = sqlite3_step(log_function_entry); Check(ret, __LINE__, "");
  ret = sqlite3_reset(log_function_entry); Check(ret, __LINE__, "");
  transaction_insertions++;
  if(transaction_insertions < 10000) return;
  ret = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &msg); Check(ret, __LINE__, msg);
  ret = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &msg); Check(ret, __LINE__, msg);
  transaction_insertions = 0;
}

void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
  char *msg = 0;
  int ret;
  unsigned long timestamp = RDTSC();
  
  // timestamp, cpuid, vpid, vtid, addr, caller
  ret = sqlite3_bind_int64(log_function_entry, 1, timestamp); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 2, cpuid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 3, vpid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int(log_function_entry, 4, vtid); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int64(log_function_entry, 5, (int64_t)this_fn); Check(ret, __LINE__, "");
  ret = sqlite3_bind_int64(log_function_entry, 6, (int64_t)call_site); Check(ret, __LINE__, "");
  ret = sqlite3_step(log_function_exit); Check(ret, __LINE__, "");
  ret = sqlite3_reset(log_function_exit); Check(ret, __LINE__, "");
  transaction_insertions++;
  if(transaction_insertions < 10000) return;
  ret = sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &msg); Check(ret, __LINE__, msg);
  ret = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &msg); Check(ret, __LINE__, msg);
  transaction_insertions = 0;
}

