cmake_minimum_required(VERSION 3.10)

project(modbus-utils VERSION 1.0.0 LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MODBUS REQUIRED IMPORTED_TARGET libmodbus)

add_executable(modbus_client "${CMAKE_CURRENT_SOURCE_DIR}/modbus_client/modbus_client.c")
target_link_libraries(modbus_client PkgConfig::MODBUS)
target_include_directories(modbus_client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/common" "${MODBUS_INCLUDE_DIRS}")

add_executable(modbus_server "${CMAKE_CURRENT_SOURCE_DIR}/modbus_server/modbus_server.c")
target_link_libraries(modbus_server PkgConfig::MODBUS)
target_include_directories(modbus_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/common" "${MODBUS_INCLUDE_DIRS}")

install(TARGETS modbus_server modbus_client DESTINATION ${CMAKE_INSTALL_BINDIR}
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
