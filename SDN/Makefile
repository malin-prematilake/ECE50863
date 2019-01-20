GCC = gcc
SOURCE = detHelpers2.c
OUTFILE = detHelpers
LOG_SOURCE = log/log.c
GLU_CAL = ../glucoseCalculation/getLastGlucose.c
HELPERS = ../detHelpers.c
COLOR_OPTION = DLOG_USE_COLOR
MATH_LIB = lm
FRAMA = frama-c-gui

build_detHelper: 
	${GCC} -o ${OUTFILE} ${SOURCE} ${LOG_SOURCE} -${COLOR_OPTION} -${MATH_LIB}
 
info:
	@echo "This is the program for determining the basal dose"
	@echo "Built by Malin Prematilake based on the openAPS artificial pancreas system"
	@echo "https://openaps.org/"
run:
	./${OUTFILE}
	
frama:
	 ${FRAMA} -wp  ${SOURCE} &
	
clean:
	rm ${OUTFILE} 

.PHONY:	info build_detHelper run clean
