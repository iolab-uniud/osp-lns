project(oven-scheduling)

find_package(Boost 1.70 COMPONENTS system)

file(GLOB sources *.cc)
file(GLOB headers *.hh)
include(easylocal-ide)

#list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/SP_Main.cc)

set(SOURCE_FILES ${sources} ${headers})
add_executable(osp ${SOURCE_FILES})
target_compile_options(osp PUBLIC -Wall -Wpedantic)
target_link_libraries(osp EasyLocal)
set_target_properties(osp PROPERTIES CXX_STANDARD 17)
#install(TARGETS IA_solver RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../bin)
