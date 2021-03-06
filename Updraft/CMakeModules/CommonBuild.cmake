SET(updraft_src_root ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Collect all source files (cpp, h, ui and qrc) in current directory and
# subdirectories and process them, optionally skipping some of the subdirectories.
# - Sets the variables ${prefix}_sources ${prefix}_headers, ${prefix}_forms,
#   ${prefix}_resources and ${prefix}_all_sources. The last variable has
#   all of the qt stuff wrapped and can be directly used as input to
#   ADD_EXECTUTABLE or ADD_LIBRARY.
# - The directory to be skipped need not exist.
FUNCTION(GATHER_SOURCES prefix)
  CMAKE_PARSE_ARGUMENTS(GATHER_SOURCES "" "" "SKIP_DIRECTORY" ${ARGN})

  FILE(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
  FILE(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
  FILE(GLOB_RECURSE forms ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)
  FILE(GLOB_RECURSE resources ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)
  FILE(GLOB_RECURSE translations ${CMAKE_CURRENT_SOURCE_DIR}/*.ts)

  FOREACH(skipped_dir ${GATHER_SOURCES_SKIP_DIRECTORY})
    SET(skipped_dir_abs ${CMAKE_CURRENT_SOURCE_DIR}/${skipped_dir})

    FILE(GLOB_RECURSE skipped_sources ${skipped_dir_abs}/*.cpp)
    LIST(REMOVE_ITEM sources ${skipped_sources} "")

    FILE(GLOB_RECURSE skipped_headers ${skipped_dir_abs}/*.h)
    LIST(REMOVE_ITEM headers ${skipped_headers} "")

    FILE(GLOB_RECURSE skipped_forms ${skipped_dir_abs}/*.ui)
    LIST(REMOVE_ITEM forms ${skipped_forms} "")

    FILE(GLOB_RECURSE skipped_resources ${skipped_dir_abs}/*.qrc)
    LIST(REMOVE_ITEM resources ${skipped_resources} "")

    FILE(GLOB_RECURSE translations_resources ${skipped_dir_abs}/*.ts)
    LIST(REMOVE_ITEM translations ${skipped_resources} "")
  ENDFOREACH()

  QT4_WRAP_CPP_FILTERED(headers_wrapped ${headers})
  QT4_WRAP_UI(forms_wrapped ${forms})
  QT4_ADD_RESOURCES(resources_wrapped ${resources})

  IF(UPDATE_TRANSLATIONS)
    QT4_CREATE_TRANSLATION(translations_wrapped ${sources} ${headers} ${forms} ${translations})
  ELSE()
    QT4_ADD_TRANSLATION(translations_wrapped ${translations})
  ENDIF()
  
  SET(${prefix}_sources ${sources} PARENT_SCOPE)
  SET(${prefix}_headers ${headers} PARENT_SCOPE)
  SET(${prefix}_forms ${forms} PARENT_SCOPE)
  SET(${prefix}_resources ${resources} PARENT_SCOPE)
  SET(${prefix}_translations ${translations} PARENT_SCOPE)
  SET(${prefix}_translations_wrapped ${translations_wrapped} PARENT_SCOPE)
  SET(${prefix}_all_sources
    ${sources}
    ${headers}
    ${headers_wrapped}
    ${forms_wrapped}
    ${resources_wrapped}
    ${translations_wrapped}
    PARENT_SCOPE)
ENDFUNCTION(GATHER_SOURCES)

MACRO(LIBRARY_BUILD name)
  CMAKE_PARSE_ARGUMENTS(LIBRARY_BUILD "DISABLE_STYLECHECK" "" "" ${ARGN})

  PROJECT(${name})

  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})

  GATHER_SOURCES(${name} SKIP_DIRECTORY tests)

  #SET(LANG
  #  translations/czech.ts
  #)
  #QT4_CREATE_TRANSLATION(TRANSLATIONS ${library_sources} ${CORE_HEADERS} ${CORE_FORMS} ${LANG})

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${updraft_src_root}/libraries)

  if(NOT WIN32)	# If it was set on win32 it also added no_debug to debug conf.
    ADD_DEFINITIONS(-DQT_NO_DEBUG)
  ENDIF()
  ADD_DEFINITIONS(-DQT_SHARED)
  ADD_DEFINITIONS(-DQT_DLL)

  # Following preprocessor definition is used for export/import macro.
  STRING(TOUPPER ${name} name_uc)
  ADD_DEFINITIONS(-DUPDRAFT_${name_uc}_INTERNAL)

  ADD_LIBRARY(${name} SHARED ${${name}_all_sources})

  # Handling of output filenames and directories on Windows
  IF(WIN32)
    # Sets properties for generic no-config case
    SET_TARGET_PROPERTIES(
      ${name}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${WIN_UPDRAFT_BIN_DIR}
      DEBUG_POSTFIX d
    )

    # Sets output path for each config.
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
      SET_TARGET_PROPERTIES(
        ${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${WIN_UPDRAFT_BIN_DIR}
      )
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
  ENDIF(WIN32)

  IF(NOT LIBRARY_BUILD_DISABLE_STYLECHECK)
    STYLE_CHECK(${name} ${${name}_sources} ${${name}_headers})
  ENDIF()

  TARGET_LINK_LIBRARIES(${name} ${QT_LIBRARIES})
  INSTALL(
    TARGETS ${name}
    DESTINATION ${CMAKE_BINARY_DIR}
    OPTIONAL
  )

  IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    ADD_SUBDIRECTORY(tests)
  ENDIF()

ENDMACRO(LIBRARY_BUILD)

MACRO(PLUGIN_BUILD name)
  STRING(TOUPPER ${name} name_uc)
  SET(BUILD_${name_uc} ON CACHE
    BOOL "Build the plugin ${name}?")

  IF(NOT ${BUILD_${name_uc}})
    RETURN()
  ENDIF()

  PROJECT(${name})

  FIND_PACKAGE(Qt4 REQUIRED)
  FIND_PACKAGE(OpenSceneGraph REQUIRED osgUtil osgDB osgGA osgViewer osgText)
  FIND_PACKAGE(OsgEarth REQUIRED)

  INCLUDE(${QT_USE_FILE})

  INCLUDE_DIRECTORIES(SYSTEM
    ${QT_INCLUDE_DIRS}
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OSGEARTH_INCLUDE_DIRS})

  GATHER_SOURCES(${name} SKIP_DIRECTORY tests)

  #SET(LANG
  #  translations/czech.ts
  #)
  #QT4_CREATE_TRANSLATION(TRANSLATIONS ${library_sources} ${CORE_HEADERS} ${CORE_FORMS} ${LANG})

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${updraft_src_root})
  INCLUDE_DIRECTORIES(${updraft_src_root}/libraries)

  if(NOT WIN32)	# If it was set on win32 it also added no_debug to debug conf.
    ADD_DEFINITIONS(-DQT_NO_DEBUG)
  ENDIF()
  ADD_DEFINITIONS(-DQT_SHARED)
  ADD_DEFINITIONS(-DQT_DLL)

  ADD_LIBRARY(${name} SHARED ${${name}_all_sources})

  # Handling of output filenames and directories on Windows
  IF(WIN32)
    # Sets properties for generic no-config case
    SET_TARGET_PROPERTIES(
      ${name}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${WIN_UPDRAFT_BIN_DIR}
      DEBUG_POSTFIX d
    )

    # Sets output path for each config.
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
      SET_TARGET_PROPERTIES(
        ${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${WIN_UPDRAFT_PLUGINS_DIR}/${PLUGIN_NAME}
      )
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
  ENDIF(WIN32)

  STYLE_CHECK(${name} ${${name}_sources} ${${name}_headers})

  TARGET_LINK_LIBRARIES(${name}
    ${QT_LIBRARIES}
    ${OPENSCENEGRAPH_LIBRARIES}
    ${OSGEARTH_LIBRARIES})
  INSTALL(
    TARGETS ${name}
    DESTINATION ${CMAKE_BINARY_DIR}/plugins/${name}
    OPTIONAL
  )
  INSTALL(FILES ${${name}_translations_wrapped}
    DESTINATION ${TRANSLATIONS_DST_DIR}/plugins/${name}
    OPTIONAL
  )

  IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    ADD_SUBDIRECTORY(tests)
  ENDIF()

ENDMACRO(PLUGIN_BUILD)

MACRO(TEST_BUILD name)
  IF(NOT BUILD_TESTS)
    RETURN()
  ENDIF()

  PROJECT(${name})

  FIND_PACKAGE(Qt4 REQUIRED)
  SET(QT_USE_QTTEST TRUE)

  INCLUDE(${QT_USE_FILE})

  GATHER_SOURCES(${name})

  #SET(LANG
  #  translations/czech.ts
  #)
  #QT4_CREATE_TRANSLATION(TRANSLATIONS ${library_sources} ${CORE_HEADERS} ${CORE_FORMS} ${LANG})

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${updraft_src_root}/libraries)

  if(NOT WIN32)	# If it was set on win32 it also added no_debug to debug conf.
    ADD_DEFINITIONS(-DQT_NO_DEBUG)
  ENDIF()
  ADD_DEFINITIONS(-DQT_SHARED)
  ADD_DEFINITIONS(-DQT_DLL)

  ADD_DEFINITIONS(-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

  ADD_EXECUTABLE(${name} ${${name}_all_sources})
  ADD_TEST(NAME ${name} COMMAND ${name})

  STYLE_CHECK(${name} ${${name}_sources} ${${name}_headers})

  TARGET_LINK_LIBRARIES(${name} ${QT_LIBRARIES})
ENDMACRO(TEST_BUILD)
