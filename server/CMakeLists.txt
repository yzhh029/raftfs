aux_source_directory(. SER_SRC)
list(REMOVE_ITEM SER_SRC ${CMAKE_CURRENT_SOURCE_DIR}                             /server_main.cpp)

aux_source_directory(../protocol PROTOSRC)
aux_source_directory(../utils UTLSRC)
aux_source_directory(../storage STOSRC)
aux_source_directory(../filesystem FSORC)

set(SERVERSRC ${SER_SRC} ${PROTOSRC} ${UTLSRC} ${STOSRC} ${FSORC})

include_directories("/usr/local/include/thrift/")
include_directories("../protocol")
include_directories("../utils")
include_directories("../storage")
include_directories("../filesystem")

find_package(Threads)

find_library(THRIFT_LIB libthrift.a)

add_library(raftserver ${SERVERSRC})
target_link_libraries(raftserver ${THRIFT_LIB} ${CMAKE_THREAD_LIBS_INIT})
