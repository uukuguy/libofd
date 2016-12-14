# - Try to find Mupdf
#
# The following variables are optionally searched for defaults
#  MUPDF_ROOT_DIR:            Base directory where all MUPDF components are found
#
# The following are set after configuration is done:
#  MUPDF_FOUND
#  MUPDF_INCLUDE_DIRS
#  MUPDF_LIBRARIES
#  MUPDF_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(MUPDF_ROOT_DIR "" CACHE PATH "Folder contains Mupdf")

find_path(MUPDF_INCLUDE_DIR mupdf/pdf.h
    PATHS ${MUPDF_ROOT_DIR})

find_library(MUPDF_LIBRARY mupdf)

find_package_handle_standard_args(MUPDF DEFAULT_MSG
    MUPDF_INCLUDE_DIR MUPDF_LIBRARY)


if(MUPDF_FOUND)
    set(MUPDF_INCLUDE_DIRS ${MUPDF_INCLUDE_DIR})
    set(MUPDF_LIBRARIES ${MUPDF_LIBRARY})
endif()
