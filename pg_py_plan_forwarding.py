import syslog

def planner(parse, planned_stmt):
	syslog.syslog('planner')
	syslog.syslog(parse)
	syslog.syslog(planned_stmt)

	return planned_stmt

def init():
	syslog.syslog("init")


def executor_start(query_desc):
	syslog.syslog("executor_start")
	syslog.syslog(query_desc)

def executor_end(query_desc):
	syslog.syslog("executor_end")
	syslog.syslog(query_desc)

syslog.syslog("main")
