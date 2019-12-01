set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
cmake_minimum_required(VERSION 3.14)

# Set the title of your project and language
project(Pex2_Client)
#project(Pex1_Server)

# Set the version of the C language standard you will be using
set(CMAKE_C_STANDARD 99)

# Turn on helpful compiler warnings
set(CMAKE_C_FLAGS "-Wall -Werror") # all warnings are on and treated as errors

# Set the source directory, the place where your main.c and
# other files are located
set(SDIR)
#set(SDIR PEX1Server/)


# List all files that are required to compile your project
set(SOURCE_FILES
        #${SDIR}pex3.c
        ${SDIR}PEX2Client.c)
#        ${SDIR}PEX1Server.c)


#        ${SDIR}pa3functs.c
#        ${SDIR}pa3main.c)
#        ${SDIR}pex2.c)
#        ${SDIR}finalProj.c
#        ${SDIR}finalProjFunctions.c)

set(CMAKE_C_FLAGS -mwindows)

# Tell cmake the name of your executable and what source files to
# build from
add_executable(Pex1_Client ${SOURCE_FILES})
#add_executable(Pex1_Server ${SOURCE_FILES})

