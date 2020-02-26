#-----------------------------------------------------------------------------
# this GNU-makefile relies on the GCC toolchain

# --- control global project settings
# RELEASE=1 -> enable optimisation, then disable debug
# RELEASE=0 -> disbale optimisation, then enable debug
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
CC=$(TARGET)gcc
OBJCOPY=$(TARGET)objcopy
AS=$(TARGET)gcc -x assembler-with-cpp -c
SIZE=$(TARGET)size
OBJDUMP=$(TARGET)objdump

# --- hardware settings
ARCH=armv7-m
FLOAT-ABI=soft #no FPU on stm32f103
CPU=cortex-m3
CPUFLAGS=-mthumb
FPU=fpv4-sp-d16 #"FLOAT-ABI=soft" disable that

##-----------------------------------------------------------------------------
# --- makefile pre-incantation

# List all default C defines here, like -D_DEBUG=1
DDEFS=-march=$(ARCH) -mfloat-abi=$(FLOAT-ABI) -mcpu=$(CPU) -mfpu=$(FPU) $(CPUFLAGS)
# List all default ASM defines here, like -D_DEBUG=1
DADEFS=-D__ASSEMBLY__

# --- deduce file names
MAIN_C_FILES=${wildcard ${SRC}/${strip ${EXE_PREFIX}}*.c}
COMMON_C_FILES=${filter-out ${MAIN_C_FILES},${wildcard ${SRC}/*.c} \
			   	 ${foreach dir,${SUBFOLDERS},${wildcard ${SRC}/${dir}/*.c}}}
COMMON_ASM_FILES=${filter-out ${MAIN_CXX_FILES},${wildcard *.s} \
				   ${foreach dir,${SUBFOLDERS},${wildcard ${SRC}/${dir}/*.s}}}
MAIN_OBJECT_FILES=${sort ${patsubst ${SRC}/%.c,${OBJ}/%.o,${MAIN_C_FILES}}}
COMMON_OBJECT_FILES=${sort ${patsubst ${SRC}/%.c,${OBJ}/%.o,${COMMON_C_FILES}} \
                      ${patsubst ${SRC}/%.s,${OBJ}/%.o,${COMMON_ASM_FILES}}}
LIBRARIES=${foreach dir,${wildcard ${LIB}/*},${wildcard ${LIB}/${dir}/*.a}}

#-----------------------------------------------------------------------------
# --- makefile incantation
# down here is black magic, you probably don't want to modify anything

DEFS  =$(DDEFS) $(UDEFS)
ADEFS =$(DADEFS) $(UADEFS)

ifeq (${strip ${RELEASE}},0)
    CFLAGS=-g3 -O0
else
    CFLAGS=-O3
endif
 
ASFLAGS = $(LIB) $(DEFS) -Wa,--gdwarf2 $(ADEFS)
CFLAGS+=-std=c17 -Wall $(DEFS) -Wextra -Warray-bounds -Wno-unused-parameter -fomit-frame-pointer
LDFLAGS= -T$(LDSCRIPT) -lc -lgcc -lgcov -lm -Wl,-Map=$@.map,--gc-sections --specs=nosys.specs
INCLUDE=-I${INC}

# --- Generate dependency information
#CFLAGS += -MD -MP -MF ${DEP}/$(@F0).d
#ASFLAGS += -MD -MP -MF ${DEP}/$(@F).d

# --- folder tree
DIR_GUARD=@mkdir -p ${@D}
ifeq (${OS},Windows_NT)
	DIR_GUARD=@md ${@D}
endif

# --- make rules
all: ${BIN}/${EXE_PREFIX}.elf ${BIN}/${EXE_PREFIX}.hex ${BIN}/${EXE_PREFIX}.bin

rebuild : clean all

.SUFFIXES:
.SECONDARY:
.PHONY: all clean rebuild

${BIN}/%.elf : ${MAIN_OBJECT_FILES} ${COMMON_OBJECT_FILES}
	@echo
	@echo ==== linking $@ ====
	@echo ${COMMON_OBJECT_FILES}
	@echo
	${DIR_GUARD}
	${CC} ${CFLAGS} --verbose -o $@ ${filter-out %ld, $^} ${LIBRARIES} ${LDFLAGS}
	${OBJDUMP} -h $@
	${SIZE} $@
	@${SKIP_LINE}

${BIN}/%.hex : ${BIN}/%.elf
	@echo
	@echo ==== traducing [opt=${opt}] $< ====
	@echo
	${OBJCOPY} -O ihex $< $@

${BIN}/%.bin : ${BIN}/%.elf
	@echo
	@echo ==== traducing [opt=${opt}] $< ====
	@echo
	${OBJCOPY} -O binary $< $@

${OBJ}/%.o : ${SRC}/%.c
	@echo
	@echo ==== compiling [opt=${opt}] $< ====
	@echo
	${DIR_GUARD}
	${CC} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@${SKIP_LINE}

${BIN}/%.o : ${SRC}/%.c
	@echo
	@echo ==== compiling [opt=${opt}] $< ====
	@echo
	${DIR_GUARD}
	${CC} ${INCLUDE} -c ${CFLAGS} $< -o $@ ${LIBRARIES}
	@${SKIP_LINE}

${OBJ}/%.o : ${SRC}/%.s
	@echo
	@echo ==== compiling [opt=${opt}] $^ $@ ${LIBRARIES}====
	@echo
	${DIR_GUARD}
	${AS} -o ${ASFLAGS} $< -o $@ ${LIBRARIES} 
	@${SKIP_LINE}

${BIN}/%.o : ${SRC}/%.s
	@echo
	@echo ==== compiling [opt=${opt}] $^ $@ ${LIBRARIES}====
	@echo
	${DIR_GUARD}
	${AS} -o ${ASFLAGS} $< -o $@ ${LIBRARIES}
	@${SKIP_LINE}

# --- remove generated files
clean:
	-rm -rf ${BIN}/*

# Include the dependency files, should be the last of the makefile
#
#-include $(shell mkdir ${DEP}/ 2>/dev/null) $(wildcard ${DEP}/*)
-include ${DEPEND_FILES}
