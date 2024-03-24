#
# Makefile - plipbox mcu makefile
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# This file is part of plipbox.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

SCRIPT_DIR=../scripts

include $(SCRIPT_DIR)/boards.mk
include ../../version.mk
include $(SCRIPT_DIR)/define.mk
include $(SCRIPT_DIR)/define-$(ARCH).mk

# mainfile/project name
PROJECT = plipbox

# --- arch dependent files ---
SRC_avr = hw_persist.c hw_spi.c hw_system.c hw_timer.c hw_uart.c
ASRC_avr = proto_low_asm.S

# --- common sources
SRC += main.c
SRC += util.c uartutil.c
SRC += proto_atom.c proto_cmd.c req.c
SRC += cmd.c cmdkey_table.c cmd_table.c
SRC += param_def.c param.c param_cmd.c
SRC += pkt_buf.c net.c arp.c
SRC += dump.c stats.c
SRC += mode.c mode_mod.c mode_cmd.c mode_def.c
SRC += mode_loop_buf.c mode_nic.c nic_test.c
SRC += nic.c nic_def.c nic_mod.c nic_loop.c nic_enc28j60.c enc28j60.c
SRC += $(SRC_$(ARCH))

ASRC = $(ASRC_$(ARCH))

PROJDIR = arch arch/$(ARCH)/mach-$(MACH) arch/$(ARCH) arch/common
PROJDIR += base command mode mode_mod net nic nic_mod param proto req

# search paths
VPATH = $(PROJDIR)

# target files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))
OBJ += $(patsubst %.S,$(OBJDIR)/%.o,$(ASRC))

# include paths
FLAGS_COMMON += $(patsubst %,-I%,$(PROJDIR))
FLAGS_COMMON += -I../../shared/include -I.

FLAGS_COMMON += -DVERSION_MIN="$(VERSION_MIN)" -DVERSION_MAJ="$(VERSION_MAJ)"
FLAGS_COMMON += -DVERSION="\"$(VERSION)\"" -DBUILD_DATE="\"$(BUILD_DATE)\""
FLAGS_COMMON += -DHAVE_$(BOARD) -DUART_BAUD=$(UART_BAUD) -DBOARD=$(BOARD)
FLAGS_COMMON += -DF_CPU=$(F_CPU) -DCONFIG_PTR_BITS=16
FLAGS_COMMON += -DCONFIG_BAUD_RATE=$(UART_BAUD) -DCONFIG_SPI_NUM_CS=1

# debug
ifeq "$(BUILD_TYPE)" "DEBUG"
#FLAGS_COMMON += -DDEBUG_PROTO_ATOM
FLAGS_COMMON += -DDEBUG_PROTO_CMD
#FLAGS_COMMON += -DDEBUG_DUMP_FRAMES
#FLAGS_COMMON += -DDEBUG_MODE
#FLAGS_COMMON += -DDEBUG_NIC
FLAGS_COMMON += -DDEBUG_PARAM
FLAGS_COMMON += -DDEBUG_ENC28J60
endif

# ---------- RULES ----------

include $(SCRIPT_DIR)/rule.mk
include $(SCRIPT_DIR)/rule-$(ARCH).mk

# include dependencies
-include $(shell mkdir -p $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)
