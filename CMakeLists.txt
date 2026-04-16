cmake_minimum_required(VERSION 3.18.4)
project(staff-control
        VERSION 0.2.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/khromenokroman/staff-control")

set(CMAKE_CXX_STANDARD 20)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

configure_file(Version.in Version @ONLY)
configure_file(packaging/postinst.in postinst @ONLY)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/staff_control.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES configuration/cfg.json
        DESTINATION /etc/staff-control/
)
install(FILES db/staff-control.json
        DESTINATION /tmp/staff-control/
)
install(FILES systemd/staff-control.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system
)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "khromenokroman")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "khromenokroman")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Staff control")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "khromenokroman")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/postinst")


include(CPack)
