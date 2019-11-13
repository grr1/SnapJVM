KEYSTONE_LDFLAGS = -L. -lkeystone -lstdc++ -lm
CC=gcc

all:
	${CC} -o keystone-test -I. keystone-test.c ${KEYSTONE_LDFLAGS}
	${CC}  -g -zexecstack -o keystone-test2 -I. keystone-test2.c ${KEYSTONE_LDFLAGS}
	${CC}  -g -zexecstack -o keystone-test3 -I. keystone-test3.c ${KEYSTONE_LDFLAGS}

clean:
	rm -rf *.o keystone-test
