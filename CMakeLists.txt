cmake_minimum_required(VERSION 3.26)
project(WEIGHT)

set(CMAKE_CXX_STANDARD 11)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")


set(MYSQL_INCLUDE_DIR "D:/2_software_environment/6_Mysql/mysql-5.7.43-winx64/include")
set(MYSQL_LIB "D:/2_software_environment/6_Mysql/mysql-5.7.43-winx64/lib/libmysql.lib")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(WEIGHT src/GXMain.cpp
        src/Serial.cpp
        include/Serial.h
        include/RFID.h
        include/WEIGHT.h
        src/UtilityClass.cpp
        include/UtilityClass.h
        include/Defines.h
        src/Logger.cpp
        include/Logger.h
        src/SQL_Conn.cpp
        include/SQL_Conn.h
        src/SQL_Conn.cpp
        src/WeightReckon.cpp
)
target_include_directories(WEIGHT PRIVATE ${MYSQL_INCLUDE_DIR})
target_link_libraries(WEIGHT PRIVATE ${MYSQL_LIB})