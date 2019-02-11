GCC = gcc
SOURCE = controller.c
OUTFILE = controller
LOG_SOURCE = logger/log.c
MSG_HANDLE = messageHandle/messageHandle.c
WIDEST_PATH = widestPath/widestPath.c
MATH_LIB = lm
LOG_FILES = controller_*

build: 
	${GCC} -o ${OUTFILE} ${SOURCE} ${MSG_HANDLE} ${WIDEST_PATH} ${LOG_SOURCE}
 
info:
	@echo "This is the controller for SDN"
	@echo "Built by Malin Prematilake"
	
run:
	./${OUTFILE}
	
clean:
	rm ${OUTFILE} 


cleanLog:
	rm ${LOG_FILES}

.PHONY:	info build_controller run clean
