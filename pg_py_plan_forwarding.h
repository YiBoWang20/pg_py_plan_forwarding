#ifndef __ML_CARD_H__
#define __ML_CARD_H__

#include "access/hash.h"
#include "access/htup_details.h"
#include "access/xact.h"
#include "catalog/catalog.h"
#include "catalog/namespace.h"
#include "catalog/index.h"
#include "catalog/indexing.h"
#include "catalog/pg_type.h"
#include "catalog/pg_operator.h"
#include "commands/explain.h"
#include "executor/executor.h"
#include "executor/execdesc.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "optimizer/pathnode.h"
#include "optimizer/planner.h"
#include "optimizer/cost.h"
#include "parser/analyze.h"
#include "parser/parsetree.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"
#include "utils/rel.h"
#include "utils/fmgroids.h"
#include "utils/snapmgr.h"

#include "plpy_util.h"
#include "plpython.h"


PlannedStmt*
default_planner(
	Query *parse,
	const char *query_string,
	int cursorOptions,
	ParamListInfo boundParams);


///home/sk/tmp/postgres/src/pl/plpython/plpy_util.c
static PlannedStmt* 
pg_py_planner(
	Query *parse,
	const char *query_string,
	int cursorOptions, 
	ParamListInfo boundParams);

/*
static void 
pg_py_ExecutorStart(
	QueryDesc *queryDesc, 
	int eflags);

static void 
pg_py_ExecutorEnd(
	QueryDesc *queryDesc);

static void 
pg_py_ExplainOneQuery(
	Query* query, 
	int cursorOptions, 
	IntoClause* into, 
	ExplainState* es, 
	const char* queryString, 
	ParamListInfo params, 
	QueryEnvironment *queryEnv);
*/


#endif