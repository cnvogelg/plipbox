cmake_minimum_required(VERSION 3.12)

include(../scripts/pico_sdk_import.cmake)

project(plipbox C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

string(TOUPPER ${CMAKE_BUILD_TYPE} FLAVOR)

# Initialize the SDK
pico_sdk_init()

set(PB_PICO_LIBS 
  hardware_spi
  hardware_i2c
  hardware_watchdog
  hardware_gpio
  hardware_uart
  hardware_sync
  pico_stdlib
)

# source lists
set(SRCS
  arch/rp2/hw_system.c
  arch/rp2/hw_uart.c
  arch/rp2/hw_spi.c
  arch/rp2/hw_persist.c
  arch/common/proto_low.c

  base/util.c
  base/uartutil.c

  command/cmd.c
  command/cmd_table.c
  command/cmdkey_table.c

  mode/mode.c
  mode/mode_cmd.c
  mode/mode_mod.c

  mode_mod/mode_def.c
  mode_mod/mode_loop_buf.c
  mode_mod/mode_nic.c

  net/arp.c
  net/net.c

  nic/nic.c
  nic/nic_mod.c
  nic/nic_test.c

  nic_mod/enc28j60.c
  nic_mod/nic_def.c
  nic_mod/nic_enc28j60.c
  nic_mod/nic_loop.c

  param/param.c
  param/param_cmd.c
  param/param_def.c

  proto/proto_atom.c
  proto/proto_cmd.c

  req/req.c

  pkt_buf.c
  net/net.c
  net/arp.c

  dump.c
  main.c
  pkt_buf.c
  stats.c
)

add_compile_definitions(
  CONFIG_BAUD_RATE=115200
  CONFIG_SPI_NUM_CS=2
  BUILD_DATE="${BUILD_DATE}"
  VERSION="${VERSION}"
  VERSION_MAJ=${VERSION_MAJ}
  VERSION_MIN=${VERSION_MIN}
)

include_directories(
  arch
  arch/common
  arch/rp2
  arch/rp2/mach-pico
  base
  command
  mode
  mode_mod
  net
  nic
  nic_mod
  param
  proto
  req
  .
  ../../shared/include
)

macro(make_fw name)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} ${PB_PICO_LIBS})
#        pico_enable_stdio_usb(${name} 0)
#        pico_enable_stdio_uart(${name} 1)
        pico_add_extra_outputs(${name})
endmacro()

# ----- firmwares -----
make_fw(plipbox ${SRCS})
