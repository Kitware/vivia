project(KmlWriter)

set(SRCS KmlWriter.cxx)

# KML is required.
find_package(KML REQUIRED)

include_directories(SYSTEM
  ${KML_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} ${KML_LIBRARY})

install_executable_target(${PROJECT_NAME} Tools)
