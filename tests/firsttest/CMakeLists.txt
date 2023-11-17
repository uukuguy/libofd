PROJECT(libofd)

AUX_SOURCE_DIRECTORY(. SRC_LIST)
ADD_EXECUTABLE(firsttest ${SRC_LIST})

TARGET_LINK_LIBRARIES(firsttest utils_static ${FREETYPE_LIBRARIES} ${HARFBUZZ_LIBRARIES} harfbuzz-icu cairo tinyxml2 zip stdc++)
