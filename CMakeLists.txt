cmake_minimum_required(VERSION 2.8.1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CPU "-mcpu=cortex-m3 -mthumb")

set(SOURCES src/main.c src/ksz8721if.c)

# library libopencm3 (stm32f1 only)
set(LIBOPENCM3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libopencm3)
add_custom_target(
    libopencm3
    COMMAND make IRQ_DEFN_FILES=include/libopencm3/stm32/f1/irq.json TARGETS=stm32/f1 all
    WORKING_DIRECTORY ${LIBOPENCM3_DIR}
)
link_directories(${LIBOPENCM3_DIR}/lib)
link_directories(${LIBOPENCM3_DIR}/lib/stm32/f1)
include_directories(${LIBOPENCM3_DIR}/include)

# library lwip
set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lwip)
set(LWIP_CORE ${LWIP_DIR}/src/core/def.c
    ${LWIP_DIR}/src/core/dhcp.c
    ${LWIP_DIR}/src/core/dns.c
    ${LWIP_DIR}/src/core/init.c
    ${LWIP_DIR}/src/core/mem.c
    ${LWIP_DIR}/src/core/memp.c
    ${LWIP_DIR}/src/core/netif.c
    ${LWIP_DIR}/src/core/pbuf.c
    ${LWIP_DIR}/src/core/raw.c
    ${LWIP_DIR}/src/core/stats.c
    ${LWIP_DIR}/src/core/sys.c
    ${LWIP_DIR}/src/core/tcp.c
    ${LWIP_DIR}/src/core/tcp_in.c
    ${LWIP_DIR}/src/core/tcp_out.c
    ${LWIP_DIR}/src/core/timers.c
    ${LWIP_DIR}/src/core/udp.c)
set(LWIP_CORE4FILES ${LWIP_DIR}/src/core/ipv4/autoip.c
    ${LWIP_DIR}/src/core/ipv4/icmp.c
    ${LWIP_DIR}/src/core/ipv4/igmp.c
    ${LWIP_DIR}/src/core/ipv4/inet.c
    ${LWIP_DIR}/src/core/ipv4/inet_chksum.c
    ${LWIP_DIR}/src/core/ipv4/ip.c
    ${LWIP_DIR}/src/core/ipv4/ip_addr.c
    ${LWIP_DIR}/src/core/ipv4/ip_frag.c)
set(LWIP_NETIF ${LWIP_DIR}/src/netif/etharp.c)
set(LWIP_SOURCES ${LWIP_CORE} ${LWIP_CORE4FILES} ${LWIP_NETIF})
include_directories(${LWIP_DIR}/src/include)
include_directories(${LWIP_DIR}/src/include/ipv4)

# Flash via OpenOCD
add_custom_target(
    flash
    COMMAND openocd -f ${CMAKE_CURRENT_SOURCE_DIR}/openocd.cfg -c "program app verify reset exit"
    DEPENDS app
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

# Reset via OpenOCD
add_custom_target(
    reset
    COMMAND openocd -f ${CMAKE_CURRENT_SOURCE_DIR}/ -c "init" -c "reset run" -c "exit"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

add_definitions(-DSTM32F1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_EXE_LINKER_FLAGS "-Tstm32f103xc.ld ${CPU} -nostartfiles -Wl,--gc-sections")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS) # remove -rdynamic

add_executable(app ${SOURCES} ${LWIP_SOURCES})

target_link_libraries(app c_nano)
target_link_libraries(app nosys)
target_link_libraries(app opencm3_stm32f1)

set_target_properties(app PROPERTIES
    COMPILE_FLAGS "${CPU} -g3 -ggdb -Wall -ffunction-sections -fdata-sections -O2")

add_dependencies(app libopencm3)
