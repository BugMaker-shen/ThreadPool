# add_subdirectory(server)
# add_subdirectory(client)

aux_source_directory(. SRC_LIST)

# 编译生成threadpool动态库，最终会生成libthreadpool_v1.so
add_library(threadpool_v1 SHARED ${SRC_LIST})

# 执行库文件输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)