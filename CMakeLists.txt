cmake_minimum_required(VERSION 3.13)
option(USE_ICONV "Use iconv for converting Shift-JIS to UTF-8." ON)

project(run68 C)

#add_definitions(-DFNC_TRACE -DENV_FROM_INI)
add_definitions(-DENV_FROM_INI)

if(USE_ICONV)
    find_package(Iconv REQUIRED)
    add_definitions(-DUSE_ICONV)
else()
endif()

if (EMSCRIPTEN)
set(CMAKE_EXE_LINKER_FLAGS "--embed-file ./fs@ -sFORCE_FILESYSTEM")
endif()


set(CMAKE_C_FLAGS 
    "-O3 -Wno-int-conversion -Wno-return-type -Wno-switch -Wno-implicit-function-declaration -Wno-format -Wno-invalid-source-encoding -Wno-pointer-integer-compare -Wno-int-to-pointer-cast -Wno-pointer-sign -Wno-incompatible-pointer-types")

set(SRC 
    src/ansicolor-w32.c
    src/calc.c
    src/conditions.c
    src/debugger.c
    src/disassemble.c
    src/eaaccess.c
    src/exec.c
    src/getini.c
    src/key.c
    src/line0.c
    src/line2.c
    src/line4.c
    src/line5.c
    src/line6.c
    src/line7.c
    src/line8.c
    src/line9.c
    src/lineb.c
    src/linec.c
    src/lined.c
    src/linee.c
    src/linef.c
    src/load.c
    src/mem.c
    src/run68.c
    src/iocscall.c
    src/doscall.c
    src/run68.h
)

set(CMAKE_CXX_STANDARD, 11)

add_executable(run68 ${SRC})

set(LIBS m) # math lib

if(USE_ICONV)
  include_directories(${Iconv_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${Iconv_LIBRARIES})
endif()

target_link_libraries(run68 PUBLIC ${LIBS})

INSTALL(TARGETS run68 RUNTIME DESTINATION bin)
