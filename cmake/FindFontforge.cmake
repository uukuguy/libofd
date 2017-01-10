#
#  Copyright (c) 2006-2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

find_path(FONTFORGE_INCLUDE_DIR fontforge/fontforge.h
/usr/local/include
/usr/include
)

find_library(FONTFORGE_LIBRARY
  NAMES fontforge
  PATHS /usr/local/lib /usr/lib
  )

if (FONTFORGE_LIBRARY AND FONTFORGE_INCLUDE_DIR)
    set(FONTFORGE_LIBRARIES ${FONTFORGE_LIBRARY})
    set(FONTFORGE_INCLUDE_DIRS ${FONTFORGE_INCLUDE_DIR} ${FONTFORGE_INCLUDE_DIR}/fontforge)
    set(FONTFORGE_FOUND "YES")
else ()
  set(FONTFORGE_FOUND "NO")
endif ()

if (FONTFORGE_FOUND)
    if (NOT Fontforge_FIND_QUIETLY)
      message(STATUS "Found FONTFORGE: ${FONTFORGE_LIBRARIES}")
   endif ()
else ()
    if (Fontforge_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FONTFORGE library")
   endif ()
endif ()

mark_as_advanced(
  FONTFORGE_LIBRARY
  FONTFORGE_INCLUDE_DIR
  )
