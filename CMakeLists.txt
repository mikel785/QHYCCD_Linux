cmake_minimum_required(VERSION 2.8)
PROJECT(libqhy C CXX)

set(LIBQHY_VERSION "0.1.8")
set(LIBQHY_SOVERSION "0")

IF (WIN32)

ENDIF (WIN32)


IF (APPLE)

install(FILES qhyccd.h qhyccdcamdef.h qhyccderr.h qhyccdstruct.h DESTINATION include/libqhy)
install(FILES libqhy.dylib DESTINATION lib)

file(GLOB FIRMWARE "firmware/*.HEX")
install(FILES ${FIRMWARE} DESTINATION "/usr/local/firmware")

ELSE()

set(FIRMWARE_INSTALL_DIR "/lib/firmware/qhy")
set(UDEVRULES_INSTALL_DIR "/lib/udev/rules.d" CACHE STRING "Base directory for udev rules")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(LIBQHY_TARGET_FILENAME "")
set(LIBQHY_SOURCE_FILENAME "")

IF(UNIX AND NOT WIN32)
 message("Target arch: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
 IF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")
  set(LIBQHY_SOURCE_FILENAME libqhy_arm_v6.bin)
  set(LIBQHY_TARGER_FILENAME libqhy_arm6.so.${LIBQHY_VERSION})
 ELSEIF(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv7l")
  set(LIBQHY_SOURCE_FILENAME libqhy_arm_v7.bin)
  set(LIBQHY_TARGER_FILENAME libqhy_arm7.so.${LIBQHY_VERSION})
 ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(LIBQHY_SOURCE_FILENAME libqhy_64.bin)
  set(LIBQHY_TARGET_FILENAME libqhy_64.so.${LIBQHY_VERSION})
 ELSE()
  set(LIBQHY_SOURCE_FILENAME libqhy_32.bin)
  set(LIBQHY_TARGET_FILENAME libqhy_32.so.${LIBQHY_VERSION})
 ENDIF (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv6l")

 exec_program(cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${LIBQHY_SOURCE_FILENAME} ${CMAKE_BINARY_DIR}/${LIBQHY_TARGET_FILENAME})

ENDIF(UNIX AND NOT WIN32)

# Make sure symbolic links are installed
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_libqhy_symlink.cmake "
exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${LIB_INSTALL_DIR}${LIB_POSTFIX}/${LIBQHY_TARGET_FILENAME} ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libqhy.so.${LIBQHY_SOVERSION})\n
exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libqhy.so.${LIBQHY_SOVERSION} ${LIB_INSTALL_DIR}${LIB_POSTFIX}/libqhy.so)\n
")

install(FILES qhyccd.h qhyccdcamdef.h qhyccderr.h qhyccdstruct.h DESTINATION include/libqhy)
install(FILES ${CMAKE_BINARY_DIR}/${LIBQHY_TARGET_FILENAME} DESTINATION ${LIB_INSTALL_DIR}${LIB_POSTFIX})
install(FILES 85-qhy.rules DESTINATION ${UDEVRULES_INSTALL_DIR})
install(
   CODE "
   file(GLOB QHY_FIRMWARE ${CMAKE_CURRENT_SOURCE_DIR}/firmware/*) \n
   file(INSTALL DESTINATION ${FIRMWARE_INSTALL_DIR} TYPE FILE FILES \${QHY_FIRMWARE})"
 )

install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_libqhy_symlink.cmake)

ENDIF (APPLE)

