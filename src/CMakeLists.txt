# Add subdirectories
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(redsvd)
ADD_SUBDIRECTORY(io)

# Find dependencies libraries for Math
IF(UNIX)
  SET(MATH_LIB m)
ELSE(UNIX)
  SET(MATH_LIB)
ENDIF(UNIX)

# executable declaration
ADD_EXECUTABLE(preprocess preprocess.cpp)
ADD_EXECUTABLE(vocab vocab.cpp)
ADD_EXECUTABLE(cooccurrence cooccurrence.cpp)
ADD_EXECUTABLE(pca pca.cpp)
ADD_EXECUTABLE(embeddings embeddings.cpp)
ADD_EXECUTABLE(inference inference.cpp)
ADD_EXECUTABLE(eval eval.cpp)
ADD_EXECUTABLE(neighbors neighbors.cpp)
ADD_EXECUTABLE(stats stats.cpp)

# Linking
TARGET_LINK_LIBRARIES( preprocess
                       util
                       ${CMAKE_THREAD_LIBS_INIT}
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( vocab
                       util
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( cooccurrence
                       util
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( pca
                       util
                       redsvd
                       io
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( embeddings
                       util
                       redsvd
                       io
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( inference
                       util
                       redsvd
                       io
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( eval
                       util
                       redsvd
                       io
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES} )
TARGET_LINK_LIBRARIES( neighbors
                       util
                       redsvd
                       io
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES}
                       )
TARGET_LINK_LIBRARIES( stats
                       util
                       ${ZLIB_LIBRARIES}
                       )

# Installing
INSTALL(TARGETS preprocess DESTINATION bin)
INSTALL(TARGETS vocab DESTINATION bin)
INSTALL(TARGETS cooccurrence DESTINATION bin)
INSTALL(TARGETS pca DESTINATION bin)
INSTALL(TARGETS embeddings DESTINATION bin)
INSTALL(TARGETS inference DESTINATION bin)
INSTALL(TARGETS eval DESTINATION bin)
INSTALL(TARGETS neighbors DESTINATION bin)
INSTALL(TARGETS stats DESTINATION bin)
