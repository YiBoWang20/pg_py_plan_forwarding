#include "postgres.h"
#include <Python.h>
#include "pg_py_plan_forwarding.h"

//#include "executor/executor.h"
//#include "optimizer/paths.h"



PG_MODULE_MAGIC;

void _PG_init(void);
void _PG_fini(void);


static planner_hook_type prev_planner_hook = NULL;
//static ExecutorStart_hook_type prev_ExecutorStart = NULL;
//static ExecutorEnd_hook_type prev_ExecutorEnd = NULL;
//static ExplainOneQuery_hook_type prev_ExplainOneQuery = NULL;

static PyObject *pg_py_module = NULL;

void
_PG_init(void)
{
	
	PyObject *py_name;

	Py_Initialize();

    py_name = PyUnicode_DecodeFSDefault("pg_py_plan_forwarding");

    pg_py_module = PyImport_Import(py_name);
    Py_DECREF(py_name);

	if (pg_py_module == NULL) {
		elog(WARNING, "Failed to load pg py plan forwarding module");
		return;
	}


	prev_planner_hook = planner_hook;
	planner_hook = pg_py_planner;

//	prev_ExecutorStart = ExecutorStart_hook;
//	ExecutorStart_hook = pg_py_ExecutorStart;

//	prev_ExecutorEnd = ExecutorEnd_hook;
//	ExecutorEnd_hook = pg_py_ExecutorEnd;

//	prev_ExplainOneQuery = ExplainOneQuery_hook;
//	ExplainOneQuery_hook = pg_py_ExplainOneQuery;

/*
	py_executor_start_hook = PyObject_GetAttrString(py_module, "executor_start_hook");
	if (!py_executor_start_hook || !PyCallable_Check(py_executor_start_hook)) {
		py_executor_start_hook = NULL;
	}

	py_executor_end_hook = PyObject_GetAttrString(py_module, "executor_end_hook");
	if (!py_executor_end_hook || !PyCallable_Check(py_executor_end_hook)) {
		py_executor_end_hook = NULL;
	}

	py_explain_one_query_hook = PyObject_GetAttrString(py_module, "explain_one_query_hook");
	if (!py_explain_one_query_hook || !PyCallable_Check(py_explain_one_query_hook)) {
		py_explain_one_query_hook = NULL;
	}
*/
}

void _PG_fini(void) {
  elog(LOG, "finished extension");
}


PlannedStmt*
call_default_planner(
	Query *parse,
	const char *query_string,
	int cursorOptions,
	ParamListInfo boundParams)
{
	if (prev_planner_hook)
		return prev_planner_hook(
			parse,
			query_string,
			cursorOptions,
			boundParams);
	else
		return standard_planner(
			parse,
			query_string,
			cursorOptions,
			boundParams);
}


static PlannedStmt* 
pg_py_planner(
	Query *parse,
	const char *query_string,
	int cursorOptions, 
	ParamListInfo boundParams)
{

	PlannedStmt *planned_stmt;
	PyObject *py_planner;


	planned_stmt = call_default_planner(
		parse,
		query_string,
		cursorOptions,
		boundParams);



	py_planner = PyObject_GetAttrString(pg_py_module, "planner");
	if (!py_planner || !PyCallable_Check(py_planner)) {
		return planned_stmt;
	}


	PyObject *py_arguments;
	PyObject *py_value;	

	//TODO: It looks creepy at the moment
	py_arguments = PyTuple_New(2);

	char *parse_str = nodeToString(parse);
	py_value = PyUnicode_FromString(parse_str);
	if (!py_value) {
		pfree(parse_str);
		Py_DECREF(py_arguments);
		Py_DECREF(py_value);
		elog(WARNING, "Cannot convert argument query");
		return planned_stmt;
	}

	PyTuple_SetItem(py_arguments, 0, py_value);

	
	char *planned_stmt_str = nodeToString(planned_stmt);
	py_value = PyUnicode_FromString(planned_stmt_str);
	if (!py_value) {
		pfree(parse_str);
		pfree(planned_stmt_str);

		Py_DECREF(py_arguments);
		Py_DECREF(py_value);
		elog(WARNING, "Cannot convert argument planned stmt");
		return planned_stmt;
	}
	PyTuple_SetItem(py_arguments, 1, py_value);	
	

	PyObject_CallObject(py_planner, py_arguments);
	Py_DECREF(py_arguments);

	return planned_stmt;
}
