#-------------------------------------------------------------------------------
# this GNU-makefile relies on the GCC toolchain

# --- control global project settings
# RELEASE=1 -> enable optimisation, then disable debug
# RELEASE=0 -> disable optimisation, then enable debug
RELEASE=0

# --- project architecture
# program name
EXE_PREFIX=main
# project folders, This Makefile should be in the same folder as the SRC folder
SRC=src
BIN=bin
INC=include
LIB=lib
OBJ=${BIN}/obj
DEP=${BIN}/dep
# code folders, in the SRC folder
SUBFOLDERS=drivers target
# Define linker script file here
LDSCRIPT=${SRC}/target/STM32F103XB.ld

# --- advanced config
# List all user C define here
UDEFS=
# Define ASM defines here
UADEFS=

# --- toolchain configuration
TARGET=arm-none-eabi-
CC=${TARGET}gcc
CPP=${TARGET}g++
OBJCOPY=${TARGET}objcopy
AS=${TARGET}gcc -x assembler-with-cpp -c
SIZE=${TARGET}size
OBJDUMP=${TARGET}objdump

# --- hardware settings
ARCH=armv7-m
FLOAT-ABI=soft #no FPU on stm32f103
CPU=cortex-m3
CPUFLAGS=-mthumb
FPU=fpv4-sp-d16 #"FLOAT-ABI=soft" disable that

#-------------------------------------------------------------------------------
# --- makefile pre-incantation

# List all de:fault C defines here, like -D_DEBUG=1
DDEFS=-march=${ARCH} -mfloat-abi=${FLOAT-ABI} -mcpu=${CPU} -mfpu=${FPU} $\
	  ${CPUFLAGS}
# List all default ASM defines here, like -D_DEBUG=1
DADEFS=-D__ASSEMBLY__

# --- deduce file names
MAIN_C_FILES=${wildcard ${SRC}/${strip ${EXE_PREFIX}}*.c}
MAIN_CPP_FILES=${wildcard ${SRC}/${strip ${EXE_PREFIX}}*.cpp}

COMMON_C_FILES=${filter-out ${MAIN_C_FILES},${wildcard ${SRC}/*.c}}
COMMON_C_FILES+=${foreach dir,${SUBFOLDERS},${wildcard ${SRC}/${dir}/*.c}}

COMMON_CPP_FILES=${filter-out ${MAIN_CPP_FILES},${wildcard ${SRC}/*.cpp}}
COMMON_CPP_FILES+=${foreach dir,${SUBFOLDERS},${wildcard ${SRC}/${dir}/*.cpp}}

COMMON_ASM_FILES=${foreach dir,${SUBFOLDERS},${wildcard ${SRC}/${dir}/*.s}}

MAIN_OBJECT_FILES=${sort ${patsubst ${src}/%.c,${obj}/%.o,${MAIN_C_FILES}} \
                    ${patsubst ${src}/%.cpp,${obj}/%.o,${MAIN_CPP_FILES}}}
COMMON_OBJECT_FILES=${sort \
                      ${patsubst ${SRC}/%.c,${OBJ}/%.o,${COMMON_C_FILES}} \
                      ${patsubst ${SRC}/%.cpp,${OBJ}/%.o,${COMMON_CPP_FILES}} \
                      ${patsubst ${SRC}/%s,${OBJ}/%o,${COMMON_ASM_FILES}}}

LIBRARIES=${foreach dir,${wildcard ${LIB}/*},${wildcard ${LIB}/${dir}/*.a}}

OBJECT_FILES=${MAIN_OBJECT_FILES} ${COMMON_OBJECT_FILES}
DEPEND_FILES=${OBJECT_FILES:%.o,%.d}

#-------------------------------------------------------------------------------
# --- makefile incantation
# down here is black magic, you probably don't want to modify anything

DEFS=${DDEFS} ${UDEFS}
ADEFS=${DADEFS} ${UADEFS}

# --- otpimisation
ifeq (${strip ${RELEASE}},0)
    CFLAGS=-g3 -O0
else
    CFLAGS=-O3
endif

# --- c/cpp
ifneq (${strip ${MAIN_CPP_FILES} ${COMMON_CPP_FILES}},)
    CC:=${CPP}
    CFLAGS+=-std=c++17
else
    CFLAGS+=-std=c17
endif

ASFLAGS=${LIB} $(DEFS) -Wa,--gdwarf2 $(ADEFS)
CFLAGS+=-Wall ${DEFS} -Wextra -Warray-bounds -Wno-unused-parameter $\
		-fomit-frame-pointer
LDFLAGS=-T ${LDSCRIPT} -lc -lgcc -lgcov -lm -Wl,-Map=$@.map,--gc-sections $\
		--specs=nosys.specs
INCLUDE=-I ${INC}

# --- Generate dependency information
CFLAGS+=-MD -MP -MF ${patsubst %o,%d,$@} 
ASFLAGS+=#-MD -MP -MF ${patsubst %o,%d,$@} 

# --- folder tree
DIR_GUARD=@mkdir -p ${@D}

#-------------------------------------------------------------------------------
# --- make rules
all: ${BIN}/${EXE_PREFIX}.elf ${BIN}/${EXE_PREFIX}.hex ${BIN}/${EXE_PREFIX}.bin

rebuild : clean all

.SUFFIXES:
.SECONDARY:
.PHONY: all clean rebuild

# --- compiler command for elf file
${BIN}/%.elf : ${MAIN_OBJECT_FILES} ${COMMON_OBJECT_FILES}
	@echo -e '\e[32;1m ==== linking $@ ====\e[0m'
	@echo ${COMMON_OBJECT_FILES}
	@echo
	${DIR_GUARD}
	${CC} ${INCLUDE} ${CFLAGS} -o $@ $^ ${LIBRARIES} ${LDFLAGS}
	${OBJDUMP} -h $@
	${SIZE} $@
	@echo

# --- compiler commands for uploadable files 
${BIN}/%.hex : ${BIN}/%.elf
	@echo -e '\e[33;1m ==== translating $< ====\e[0m'
	${OBJCOPY} -O ihex $< $@
	@echo

${BIN}/%.bin : ${BIN}/%.elf
	@echo -e '\e[33;1m ==== translating $< ====\e[0m'
	${OBJCOPY} -O binary $< $@
	@echo

# --- compiler commands for every source file 
${OBJ}/%.o : ${SRC}/%.cpp
	@echo -e '\e[36;1m ==== compiling $< ====\e[0m'
	${DIR_GUARD}
	${CPP} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@echo

${BIN}/%.o : ${SRC}/%.cpp
	@echo -e '\e[36;1m ==== compiling $< ====\e[0m'
	${DIR_GUARD}
	${CPP} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@echo

${OBJ}/%.o : ${SRC}/%.c
	@echo -e '\e[34;1m ==== compiling $< ====\e[0m'	
	${DIR_GUARD}
	${CC} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@echo

${BIN}/%.o : ${SRC}/%.c
	@echo -e '\e[34;1m ==== compiling $< ====\e[0m'
	${DIR_GUARD}
	${CC} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@echo

${OBJ}/%.o : ${SRC}/%.s
	@echo -e '\e[35;1m ==== compiling $^ ====\e[0m'
	${DIR_GUARD}
	${AS} -o ${ASFLAGS} $< -o $@ ${LIBRARIES} 
	@echo

${BIN}/%.o : ${SRC}/%.s
	@echo -e '\e[35;1m ==== compiling $^ ====\e[0m'
	${DIR_GUARD}
	${AS} -o ${ASFLAGS} $< -o $@ ${LIBRARIES}
	@echo

# --- remove generated files
clean:
	-rm -rf ${BIN}/*

-include ${DEPEND_FILES}

