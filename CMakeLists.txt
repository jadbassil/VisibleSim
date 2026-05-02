cmake_minimum_required(VERSION 3.16)
project(VisibleSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long -Wpedantic")

# Set the following environment variables before running CMake:
# bash
# export CC=/opt/homebrew/opt/llvm/bin/clang
# export CXX=/opt/homebrew/opt/llvm/bin/clang++
# export LDFLAGS="-L/opt/homebrew/opt/libomp/lib"
# export CPPFLAGS="-I/opt/homebrew/opt/libomp/include"
# export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
# Optionally, you can set OpenMP_ROOT to help CMake find libomp:
# bash
# export OpenMP_ROOT=/opt/homebrew/opt/libomp
# 4. Run CMake with the Correct Compiler
# Now, run CMake as usual:
# bash
# cmake -B build .

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(WIN32)
    message(STATUS ">>> Win32")
    add_compile_definitions(USE_FREEGLUT)
    add_compile_definitions("ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

    # message(STATUS "Installation dir:" ${CMAKE_CURRENT_SOURCE_DIR})
    # #[[add_compile_definitions(DEBUG_GRAPHICS)]]

    # set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../simulatorCore/lib")
    IF(MSVC)
        message(STATUS "+Visual studio")
        set(GLUT_DIR F:/freeglut-3.0.0)
        set(CMAKE_PREFIX_PATH FREEGLUT)

        set(FREEGLUT_INCLUDE_DIRS F:/freeglut-3.0.0/include)
        set(FREEGLUT_LIBRARY_DIRS F:/freeglut-3.0.0/cmake-build-debug/lib)
        set(FREEGLUT_LIBRARIES libfreeglut_static.a)

        find_package(OPENGL REQUIRED)

        include_directories(${FREEGLUT_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLUT_INTERFACE_DIRS})
        link_directories(${FREEGLUT_LIBRARY_DIRS} ${OPENGL_LIBRARY_DIRS} ${GLUT_INTERFACE_LIB_DIRS})
        add_definitions(${FREEGLUT_DEFINITIONS} ${OPENGL_DEFINITIONS})
        add_executable(${PROJECT_NAME} ${PROJECT_FILES})
        target_link_libraries(${PROJECT_NAME} ${FREEGLUT_LIBRARIES} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} glutInterface.lib)
    ENDIF(MSVC)

    IF(MINGW)
        message(STATUS "+MinGW")

        # Include CPM.cmake for dependency management
        set(CPM_DOWNLOAD_VERSION 0.40.8)
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

        if(NOT(EXISTS ${CPM_DOWNLOAD_LOCATION}))
            message(STATUS "Downloading CPM.cmake")
            file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
        endif()

        include(${CPM_DOWNLOAD_LOCATION})
        set(BUILD_SHARED_LIBS OFF)

        # Fetch dependencies
        CPMAddPackage(
            NAME glew
            GITHUB_REPOSITORY Perlmint/glew-cmake
            GIT_TAG glew-cmake-2.2.0
        )
        CPMAddPackage(
            NAME freeglut
            GITHUB_REPOSITORY freeglut/freeglut
            VERSION 3.6.0
        )
        CPMAddPackage(
            NAME muparser
            GITHUB_REPOSITORY beltoforion/muparser
            VERSION 2.3.5
            OPTIONS "BUILD_SHARED_LIBS=OFF"
        )

        # #########################################################
        # #####Change Paths to match your library installation#####
        # #########################################################
        set(FREEGLUT_INCLUDE_DIRS build/_deps/freeglut-src/include)
        set(FREEGLUT_LIBRARY_DIRS build/_deps/freeglut-build/lib)
        add_compile_definitions(GLEW_STATIC)
        set(GLEW_INCLUDE_DIRS build/_deps/glew-src/include)
        set(GLEW_LIBRARY_DIRS build/_deps/glew-build/lib)
        set(muparser_INCLUDE_DIRS build/_deps/muparser-src/include)
        set(muparser_LIBRARY_DIRS build/_deps/muparser-build)

        # #########################################################
        # #########################################################
        # #########################################################
        set(ROBOTS_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/lib)

        link_directories(${ROBOTS_LIBRARY_DIRS} ${FREEGLUT_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS} ${muparser_LIBRARY_DIRS})
        include_directories(${FREEGLUT_INCLUDE_DIRS} ${GLUT_LIBRARIES} ${GLEW_INCLUDE_DIRS} ${muparser_INCLUDE_DIRS})
    ENDIF(MINGW)
ENDIF(WIN32)

IF (LINUX)
    message(STATUS ">>> Linux")

    find_package(GLUT REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(GLU REQUIRED)
    include_directories( ${OPENGL_INCLUDE_DIRS} ${GLU_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLU_LIBRARIES} -lmuparser -lpthread -ldl)
    if (NOT GLUT_FOUND)
        message(ERROR " GLUT not found!")
    endif (NOT GLUT_FOUND)

    if (NOT OpenGL_FOUND)
        message(ERROR " openGL not found!")
    endif (NOT OpenGL_FOUND)

    include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} )

ENDIF (LINUX)

IF(APPLE)
    message(STATUS ">>> APPLE SILICON / macOS")
    
    # Find required packages
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(GLUT REQUIRED)
    
    # Set architecture
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    
    # Set minimum macOS version for compatibility
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
    
    # Homebrew paths for Apple Silicon
    set(HOMEBREW_PREFIX "/opt/homebrew")
    
    # GLEW configuration - use Homebrew GLEW with proper includes
    set(GLEW_INCLUDE_DIRS ${HOMEBREW_PREFIX}/opt/glew/include)
    set(GLEW_LIBRARY_DIRS ${HOMEBREW_PREFIX}/lib)
    
    # muparser configuration  
    set(muparser_LIBRARY_DIRS ${HOMEBREW_PREFIX}/lib)
    set(muparser_INCLUDE_DIRS ${HOMEBREW_PREFIX}/include)
    
    # Set library directories
    link_directories(${GLEW_LIBRARY_DIRS} ${muparser_LIBRARY_DIRS})
    
    # Set include directories - IMPORTANT: System GLUT before Homebrew to avoid FreeGLUT headers
    include_directories(BEFORE SYSTEM
        ${GLUT_INCLUDE_DIRS}
    )
    
    include_directories(
        ${OPENGL_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${muparser_INCLUDE_DIRS}
    )
    
    # Add framework search paths for macOS - Use native macOS GLUT Framework
    link_libraries(
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
        GLEW::GLEW
    )
    
    # Add compile definitions for macOS
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    
ENDIF(APPLE)

include_directories(simulatorCore/src)
ADD_SUBDIRECTORY(simulatorCore)

# cpm_export_variables("-DGLEW_STATIC")
# add_definitions(-DGLEW_STATIC)
# set(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glew/include")

add_executable(simpleColorBB applicationsSrc/simpleColorBB/simpleColorBB.cpp applicationsSrc/simpleColorBB/simpleColorCodeBB.cpp)
target_link_libraries(simpleColorBB BlinkyBlocks )
set_target_properties(simpleColorBB PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/simpleColorBB)


add_executable(reconf2D applicationsSrc/reconf2D/reconf2D.cpp applicationsSrc/reconf2D/reconf2DBlockCode.cpp)
target_link_libraries(reconf2D Hexanodes)
set_target_properties(reconf2D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/reconf2D)

add_executable(reconf2DDemo applicationsSrc/reconf2DDemo/reconf2DDemo.cpp applicationsSrc/reconf2DDemo/reconf2DDemoBlockCode.cpp)
target_link_libraries(reconf2DDemo Hexanodes)
set_target_properties(reconf2DDemo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/reconf2DDemo)

add_executable(shapeDecomposition applicationsSrc/shapeDecomposition/shapeDecomposition.cpp applicationsSrc/shapeDecomposition/shapeDecompositionBlockCode.cpp applicationsSrc/shapeDecomposition/polygon.cpp)
target_link_libraries(shapeDecomposition BlinkyBlocks)
set_target_properties(shapeDecomposition PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/shapeDecomposition)


add_executable(demoReconfC3D applicationsSrc/demoReconfC3D/demoReconfC3D.cpp applicationsSrc/demoReconfC3D/demoReconfC3DCode.cpp)
target_link_libraries(demoReconfC3D Catoms3D)
set_target_properties(demoReconfC3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/demoReconfC3D)

# add_executable(slidingCubesExample applicationsSrc/slidingCubesExample/slidingCubesExample.cpp applicationsSrc/slidingCubesExample/slidingCubesExampleBlockCode.cpp)
# target_link_libraries(slidingCubesExample SlidingCubes)
# set_target_properties(slidingCubesExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/slidingCubesExample)

add_executable(gnnLocomotion
    applicationsSrc/gnnLocomotion/gnnLocomotion.cpp
    applicationsSrc/gnnLocomotion/gnnLocomotionBlockCode.cpp
    applicationsSrc/gnnLocomotion/gymServer.cpp
    applicationsSrc/gnnLocomotion/gcnPolicy.cpp)
target_include_directories(gnnLocomotion PRIVATE simulatorCore/src/deps)
target_link_libraries(gnnLocomotion SlidingCubes)
set_target_properties(gnnLocomotion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/applicationsBin/gnnLocomotion)
