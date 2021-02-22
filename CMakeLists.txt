cmake_minimum_required(VERSION 3.12)

project(SimpleBezierSurface)

find_package(VTK REQUIRED)
find_package(ITK REQUIRED)
find_package(RapidJSON REQUIRED)

include_directories(
  ${VTK_INCLUDE_DIRS}
  )

add_executable(${PROJECT_NAME}
  main.cxx
  vtkBezierSurfaceSource.cxx
  vtkBezierSurfaceSource.h
  )

target_link_libraries(${PROJECT_NAME}
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  ${RapidJSON_LIBRARIES}
  )
