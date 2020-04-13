# cmake macro to test if we use sane
#
#  TESSERACT_FOUND - system has Sane
#  TESSERACT_INCLUDE_DIR - the Sane include directory
#  TESSERACT_LIBRARIES - The libraries needed to use Sane

FIND_PATH(TESSERACT_INCLUDE_DIR tesseract/baseapi.h
   /usr/include
   /usr/local/include
   /opt/local/incude
)

FIND_LIBRARY(TESSERACT_LIBRARY NAMES  tesseract libtesseract
   PATHS
   /usr/lib
   /usr/local/lib
   /opt/local/lib
)

if(TESSERACT_INCLUDE_DIR AND TESSERACT_LIBRARY)
   set(TESSERACT_FOUND TRUE)
   set(TESSERACT_LIBRARIES ${TESSERACT_LIBRARY})
else()
   set(TESSERACT_FOUND FALSE)
endif()

if (TESSERACT_FOUND)
    if (NOT Tesseract_FIND_QUIETLY)
        message(STATUS "Found Tesseract: ${TESSERACT_LIBRARIES}")
    endif (NOT Tesseract_FIND_QUIETLY)
else()
    if (NOT Tesseract_FIND_QUIETLY)
        message(STATUS "don't find Tesseract")
    endif (NOT Tesseract_FIND_QUIETLY)
endif (TESSERACT_FOUND)



MARK_AS_ADVANCED(TESSERACT_INCLUDE_DIR TESSERACT_LIBRARIES TESSERACT_LIBRARY)