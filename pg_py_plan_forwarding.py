import syslog

def planner(parse, planned_stmt):
	syslog.syslog(parse)
	syslog.syslog(planned_stmt)
	return planned_stmt

def init():
	syslog.syslog("init")


syslog.syslog("main")
