
aux_source_directory(. CLISRC)
list(REMOVE_ITEM CLISRC ${CMAKE_CURRENT_SOURCE_DIR}/client_main.cpp)
aux_source_directory(../protocol PROTOSRC)
aux_source_directory(../utils UTLSRC)

set(CLI_SRC ${CLISRC} ${PROTOSRC} ${UTLSRC})
INCLUDE_DIRECTORIES("../protocol")
include_directories("../utils")
find_package(Threads)
find_library(THRIFT_LIB libthrift.a)

add_library(raftclient ${CLI_SRC})
TARGET_LINK_LIBRARIES(raftclient ${THRIFT_LIB} ${CMAKE_THREAD_LIBS_INIT})
