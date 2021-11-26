#include "postgres.h"
#include "pg_py_plan_forwarding.h"


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

	PyStatus status;
    PyConfig config;

    PyConfig_InitPythonConfig(&config);

    config.isolated = 1;

	status = PyConfig_SetString(&config, &config.pythonpath_env, L"/home/vagrant/pg_py_plan_forwarding");
	if(PyStatus_Exception(status)){
		PyErr_Print();
		elog(WARNING, "failed to set pythonpath env");
		return;
	}

    status = PyWideStringList_Append(&config.module_search_paths, L"/home/vagrant/pg_py_plan_forwarding");
	if(PyStatus_Exception(status)){
		PyErr_Print();
		elog(WARNING, "failed to set module search paths");
		return;
	}

    status = Py_InitializeFromConfig(&config);
	if(PyStatus_Exception(status)){
		PyErr_Print();
		elog(WARNING, "failed to initialize from config");
		return;
	}


	elog(DEBUG1, "import module");
	py_name = PyUnicode_DecodeFSDefault("pg_py_plan_forwarding");
	pg_py_module = PyImport_Import(py_name);
	Py_DECREF(py_name);

	if (!pg_py_module){
		PyErr_Print();
		elog(WARNING, "failed to load pg py plan forwarding module");
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
	PyObject *py_arguments;
	PyObject *py_value;
	
	char *parse_str;
	char *planned_stmt_str;

	planned_stmt = call_default_planner(
		parse,
		query_string,
		cursorOptions,
		boundParams);

	py_planner = PyObject_GetAttrString(pg_py_module, "planner");
	if (!py_planner || !PyCallable_Check(py_planner)) {
		elog(DEBUG1, "method planner not available");
		return planned_stmt;
	}

	//TODO: It looks creepy at the moment
	py_arguments = PyTuple_New(2);

	parse_str = nodeToString(parse);
	py_value = PyUnicode_FromString(parse_str);
	if (!py_value) {
		pfree(parse_str);
		Py_DECREF(py_arguments);
		Py_DECREF(py_value);
		elog(WARNING, "cannot convert argument query");
		return planned_stmt;
	}

	PyTuple_SetItem(py_arguments, 0, py_value);


	planned_stmt_str = nodeToString(planned_stmt);
	py_value = PyUnicode_FromString(planned_stmt_str);
	if (!py_value) {
		pfree(parse_str);
		pfree(planned_stmt_str);

		Py_DECREF(py_arguments);
		Py_DECREF(py_value);
		elog(WARNING, "cannot convert argument planned stmt");
		return planned_stmt;
	}
	PyTuple_SetItem(py_arguments, 1, py_value);	

	py_value = PyObject_CallObject(py_planner, py_arguments);
	
	if (!py_value) {
		Py_DECREF(py_value);
		elog(WARNING, "call planner");
		return planned_stmt;
	}

	Py_DECREF(py_arguments);

	//pfree(planned_stmt_str);
	//pfree(planned_stmt);

	// undefined symbol: PLyUnicode_AsString
	planned_stmt_str = PyUnicode_AsUTF8(py_value);
	planned_stmt = stringToNode(planned_stmt_str);
/*
	elog(DEBUG1, "python result:%s", 
		planned_stmt_str);
*/
	return planned_stmt;

}
