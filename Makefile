# contrib/pg_py_plan_forwarding/Makefile

EXTENSION = pg_py_plan_forwarding
EXTVERSION = 0.1
PGFILEDESC = "PG to Py plan forwarding"
MODULE_big = pg_py_plan_forwarding
PGFILEDESC = "pg_py_plan_forwarding - template for future extensions"

PG_CPPFLAGS += -I/usr/include/python3.9 -lpython3.9

OBJS = pg_py_plan_forwarding.o

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
