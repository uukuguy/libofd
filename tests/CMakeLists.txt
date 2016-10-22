AUX_SOURCE_DIRECTORY(. SRC_LIST)
ADD_EXECUTABLE(ofdtest ${SRC_LIST})
TARGET_LINK_LIBRARIES(ofdtest ofd cairo tinyxml2 zip stdc++)
