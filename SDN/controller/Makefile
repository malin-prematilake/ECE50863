GCC = gcc
SOURCE = controller.c
OUTFILE = controller
LOG_SOURCE = log/log.c
MSG_HANDLE = messageHandle/messageHandle.c
MATH_LIB = lm

build_controller: 
	${GCC} -o ${OUTFILE} ${SOURCE} ${MSG_HANDLE}
 
info:
	@echo "This is the controller for SDN"
	@echo "Built by Malin Prematilake"
	
run:
	./${OUTFILE}
	
clean:
	rm ${OUTFILE} 

.PHONY:	info build_controller run clean
