cmake_minimum_required(VERSION 2.8.3)
project(squirrel_person_tracker)

# check c++11 / c++0x
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "-std=c++11")
   message(COMPILER_SUPPORTS_CXX11)
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
    message(COMPILER_SUPPORTS_CXX0X)
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
  roscpp
  roslib
  tf
  pcl_ros
  squirrel_person_tracker_msgs
  libnite2
  nodelet
)

# Find OpenNI2
find_path(OpenNI2_INCLUDEDIR
	  NAMES OpenNI.h
	  HINTS /usr/include/openni2)
message(STATUS ${OpenNI2_INCLUDEDIR}) 
find_library(OpenNI2_LIBRARIES
	     NAMES OpenNI2 DummyDevice OniFile PS1090
	     HINTS /usr/lib/ /usr/lib/OpenNI2/Drivers
	     PATH_SUFFIXES lib)
message(STATUS ${OpenNI2_LIBRARIES})
# Find Nite2
#message(STATUS $ENV{NITE2_INCLUDE})
#message(STATUS $ENV{NITE2_REDIST64})
#find_path(Nite2_INCLUDEDIR
#	  NAMES NiTE.h
#	  HINTS $ENV{NITE2_INCLUDE})
#find_library(Nite2_LIBRARY
#	     NAMES NiTE2
#	     HINTS $ENV{NITE2_REDIST64}
#	     PATH_SUFFIXES lib) 



###################################
## catkin specific configuration ##
###################################

catkin_package(
  INCLUDE_DIRS include
#  LIBRARIES tracker_bonn
  CATKIN_DEPENDS 
	roscpp 
	roslib
	tf
	pcl_ros
	squirrel_person_tracker_msgs
	libnite2
	nodelet
#  DEPENDS system_lib
)

###########
## Build ##
###########


include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenNI2_INCLUDEDIR}
  ${Nite2_INCLUDEDIR}
)


add_executable(squirrel_person_tracker src/squirrel_person_tracker_main.cpp src/floor_pointer.cpp src/squirrel_person_tracker.cpp)
target_link_libraries(squirrel_person_tracker ${catkin_LIBRARIES} ${OpenNI2_LIBRARIES} )

add_library(squirrel_person_tracker_nodelet src/squirrel_person_tracker_nodelet.cpp src/floor_pointer.cpp src/squirrel_person_tracker.cpp)
target_link_libraries(squirrel_person_tracker_nodelet ${catkin_LIBRARIES} ${OpenNI2_LIBRARIES} )


catkin_add_gtest(test_floorpointer test/squirrel_person_tracker_floor_point_test.cpp src/floor_pointer.cpp)
target_link_libraries(test_floorpointer ${catkin_LIBRARIES} ${OpenNI2_LIBRARIES} )

add_custom_command(
TARGET squirrel_person_tracker
POST_BUILD
COMMAND ${PROJECT_SOURCE_DIR}/create_nite2_link.bash
)

# for ros hydro
set_target_properties(squirrel_person_tracker PROPERTIES COMPILE_FLAGS -Dlinux) # for comipling with openni2 with linux 12.04
set_target_properties(squirrel_person_tracker_nodelet PROPERTIES COMPILE_FLAGS -Dlinux)