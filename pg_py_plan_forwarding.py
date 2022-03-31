import syslog

def planner(parse, planned_stmt):
	syslog.syslog('planner')

	#file_object = open('/home/vagrant/pg_py_plan_forwarding/planner_output.txt', 'a')

	#file_object.write('parse')
	#file_object.write('\n')
	
	#file_object.write(parse)
	#file_object.write('\n')

	#file_object.write('planned_stmt')
	#file_object.write('\n')

	#file_object.write(planned_stmt)
	#file_object.write('\n')

	#file_object.close()

	return planned_stmt

def init():
	syslog.syslog("init")


def executor_start(query_desc):
	syslog.syslog("executor_start")

	file_object = open('/home/vagrant/pg_py_plan_forwarding/planner_output.txt', 'a')

	file_object.write('parse')
	file_object.write('\n')

	file_object.write(query_desc)
	file_object.write('\n')

	file_object.close()

def executor_end(query_desc):
	syslog.syslog("executor_end")

	file_object = open('/home/vagrant/pg_py_plan_forwarding/planner_output.txt', 'a')

	file_object.write('parse')
	file_object.write('\n')

	file_object.write(query_desc)
	file_object.write('\n')

	file_object.close()

syslog.syslog("main")
