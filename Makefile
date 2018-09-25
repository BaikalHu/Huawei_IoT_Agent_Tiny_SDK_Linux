##########################################################################################################################
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
# ------------------------------------------------

######################################
# target
######################################
TARGET = LiteOS_Atiny
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og





#######################################
# binaries
#######################################
PREFIX    = 
CC        = $(PREFIX)gcc
AS        = $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY   = $(PREFIX)objcopy
OBJDUMP   = $(PREFIX)objdump
AR        = $(PREFIX)ar
SZ        = $(PREFIX)size
LD        = $(PREFIX)ld
HEX       = $(OBJCOPY) -O ihex
BIN       = $(OBJCOPY) -O binary -S


PROJECTBASE = $(PWD)
override PROJECTBASE    := $(abspath $(PROJECTBASE))
TOP_DIR = $(PROJECTBASE)/


#######################################
# paths
#######################################
# firmware library path
PERIFLIB_PATH =

# Build path
BUILD_DIR = build

ER_COAP_SRC = \
        ${wildcard $(TOP_DIR)/lwm2m/core/er-coap-13/er-coap-13.c}
        C_SOURCES += $(ER_COAP_SRC)

LWM2M_SRC = \
        ${wildcard $(TOP_DIR)/lwm2m/core/*.c} \
		${wildcard $(TOP_DIR)//lwm2m/examples/shared/*.c}
        C_SOURCES += $(LWM2M_SRC)

OS_ADAPTER_SRC = \
        ${wildcard $(TOP_DIR)/agent_tiny/osadapter/*.c}
        C_SOURCES += $(OS_ADAPTER_SRC)

ATINY_TINY_SRC = \
        ${wildcard $(TOP_DIR)/agent_tiny/lwm2m_client/*.c}
        C_SOURCES += $(ATINY_TINY_SRC)

AGENT_DEMO_SRC = \
        ${wildcard $(TOP_DIR)/agent_tiny/examples/*.c}
        C_SOURCES += $(AGENT_DEMO_SRC)


USER_SRC =  \
        $(TOP_DIR)/main.c
        C_SOURCES += $(USER_SRC)




######################################
# firmware library
######################################
PERIFLIB_SOURCES =


#######################################
# CFLAGS
#######################################

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
        -D MBEDTLS_CONFIG_FILE=\"los_mbedtls_config.h\" \
        -D LWM2M_LITTLE_ENDIAN \
        -D LWM2M_CLIENT_MODE \
        -D NDEBUG \
		-D LWM2M_BOOTSTRAP \
        -D ATINY_DEBUG \
        #-D LWM2M_WITH_LOGS \

#       WITH_DTLS change the place with LWIP_TIMEVAL_PRIVATE

C_DEFS += -D WITH_LINUX




ER_COAP_INC = \
        -I $(TOP_DIR)/lwm2m/core/er-coap-13
        C_INCLUDES += $(ER_COAP_INC)

LWM2M_INC = \
        -I $(TOP_DIR)/lwm2m/core \
		-I $(TOP_DIR)/lwm2m/examples/shared
        C_INCLUDES += $(LWM2M_INC)

OS_ADAPTER_INC = \
        -I $(TOP_DIR)/agent_tiny/osadapter
        C_INCLUDES += $(OS_ADAPTER_INC)

ATINY_TINY_INC = \
        -I $(TOP_DIR)/agent_tiny/lwm2m_client \
        -I $(TOP_DIR)/agent_tiny/comm/include
        C_INCLUDES += $(ATINY_TINY_INC)

AGENT_DEMO_INC = \
        -I $(TOP_DIR)/agent_tiny/examples
        C_INCLUDES += $(AGENT_DEMO_INC)

		



# compile gcc flags
ASFLAGS = $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS +=  -fno-stack-protector -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"



# libraries
LIBS = -lc -lm -lpthread
LIBDIR =
LDFLAGS =  $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES_s:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES_s)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES_S:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES_S)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile $(LD_FILE)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@


$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
