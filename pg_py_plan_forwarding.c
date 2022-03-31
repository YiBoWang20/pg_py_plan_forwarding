#include "postgres.h"
#include "pg_py_plan_forwarding.h"


PG_MODULE_MAGIC;


#define PLANNER "planner"
#define EXECUTOR_START "executor_start"
#define EXECUTOR_END "executor_end"

void _PG_init(void);
void _PG_fini(void);

PlannedStmt*
default_planner(
        Query *parse,
        const char *query_string,
        int cursorOptions,
        ParamListInfo boundParams);

static void
default_executor_start(
    QueryDesc *query_desc,
    int eflags);

static void
default_executor_end(
        QueryDesc *query_desc);

static PlannedStmt*
pg_py_planner(
    Query *parse,
    const char *query_string,
    int cursorOptions,
    ParamListInfo boundParams);

static void
pg_py_executor_start(
    QueryDesc *query_desc,
    int eflags);

static void
pg_py_executor_end(
    QueryDesc *query_desc);


static planner_hook_type prev_planner_hook = NULL;
static ExecutorStart_hook_type prev_executor_start = NULL;
static ExecutorEnd_hook_type prev_executor_end = NULL;

//static ExplainOneQuery_hook_type prev_ExplainOneQuery = NULL;

static PyObject *pg_py_module = NULL;

void
_PG_init(void)
{

	PyObject *py_name;
    PyObject *py_function;
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

    py_function = PyObject_GetAttrString(pg_py_module, PLANNER);
    if (py_function && PyCallable_Check(py_function)) {
        prev_planner_hook = planner_hook;
        planner_hook = pg_py_planner;
    }

    py_function = PyObject_GetAttrString(pg_py_module, EXECUTOR_START);
    if (py_function && PyCallable_Check(py_function)) {
        prev_executor_start = ExecutorStart_hook;
        ExecutorStart_hook = pg_py_executor_start;
    }

    py_function = PyObject_GetAttrString(pg_py_module, EXECUTOR_END);
    if (py_function && PyCallable_Check(py_function)) {
        prev_executor_end = ExecutorEnd_hook;
        ExecutorEnd_hook = pg_py_executor_end;
    }

//	prev_ExplainOneQuery = ExplainOneQuery_hook;
//	ExplainOneQuery_hook = pg_py_ExplainOneQuery;


}

void _PG_fini(void) {
  elog(LOG, "finished extension");
}


PlannedStmt*
default_planner(
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


static void
default_executor_start(
    QueryDesc *query_desc,
    int eflags)
{
    elog(LOG, "default_executor_start");
    if (prev_executor_start)
        return prev_executor_start(
            query_desc,
            eflags);
/*
    else
        return standard_planner(
            query_desc,
            eflags);
*/
}

static void
default_executor_end(
    QueryDesc *query_desc)
{
    elog(LOG, "default_executor_end");
    if (prev_executor_end)
        return prev_executor_end(
            query_desc);
/*
    else
        return standard_planner(
            query_desc);
*/
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

	planned_stmt = default_planner(
		parse,
		query_string,
		cursorOptions,
		boundParams);

	py_planner = PyObject_GetAttrString(pg_py_module, PLANNER);

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

	// undefined symbol: PLyUnicode_AsString
	planned_stmt_str = PyUnicode_AsUTF8(py_value);
	planned_stmt = stringToNode(planned_stmt_str);

	return planned_stmt;
}

static void
pg_py_executor_start(
        QueryDesc *query_desc,
        int eflags)
{


    elog(LOG, "pg_py_executor_start");

    PyObject *py_planner;
    PyObject *py_arguments;
    PyObject *py_value;

    char *query_desc_str;

    default_executor_start(
            query_desc,
            eflags);

    py_planner = PyObject_GetAttrString(pg_py_module, EXECUTOR_START);

    //TODO: It looks creepy at the moment
    py_arguments = PyTuple_New(1);

    elog(LOG, "nodeToString");
    query_desc_str = nodeToString(query_desc);
    py_value = PyUnicode_FromString(query_desc_str);
    if (!py_value) {
        pfree(query_desc_str);
        Py_DECREF(py_arguments);
        Py_DECREF(py_value);
        elog(WARNING, "cannot convert argument query");
        return;
    }

    elog(LOG, "call");
    PyTuple_SetItem(py_arguments, 0, py_value);
    PyObject_CallObject(py_planner, py_arguments);

    Py_DECREF(py_arguments);
    Py_DECREF(py_value);

    return;
}

static void
pg_py_executor_end(
    QueryDesc *query_desc)
{
    elog(LOG, "pg_py_executor_end");
    PyObject *py_planner;
    PyObject *py_arguments;
    PyObject *py_value;

    char *query_desc_str;

    default_executor_end(
            query_desc);

    py_planner = PyObject_GetAttrString(pg_py_module, EXECUTOR_END);

    //TODO: It looks creepy at the moment
    py_arguments = PyTuple_New(1);

    elog(LOG, "nodeToString");
    query_desc_str = nodeToString(query_desc);
    py_value = PyUnicode_FromString(query_desc_str);
    if (!py_value) {
        pfree(query_desc_str);
        Py_DECREF(py_arguments);
        Py_DECREF(py_value);
        elog(WARNING, "cannot convert argument query");
        return;
    }

    elog(LOG, "call");
    PyTuple_SetItem(py_arguments, 0, py_value);
    PyObject_CallObject(py_planner, py_arguments);

    Py_DECREF(py_arguments);
    Py_DECREF(py_value);

    return;
}