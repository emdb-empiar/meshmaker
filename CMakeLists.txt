cmake_minimum_required(VERSION 2.8)

project(meshmaker)

find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

add_executable(meshmaker MACOSX_BUNDLE meshmaker)

# ensure we use C++11 features
# avoid this warning: "in-class initialization of non-static data member is a C++11 extension [-Wc++11-extensions]"
target_compile_features(meshmaker PRIVATE cxx_nonstatic_member_init)

if (VTK_LIBRARIES)
	target_link_libraries(meshmaker ${VTK_LIBRARIES})
else()
	target_link_libraries(meshmaker vtkHybrid vtkWidgets)
endif()

install(TARGETS meshmaker
	RUNTIME DESTINATION bin
	BUNDLE DESTINATION bin)
