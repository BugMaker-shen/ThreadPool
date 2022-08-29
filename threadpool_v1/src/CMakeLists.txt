# add_subdirectory(server)
# add_subdirectory(client)

aux_source_directory(. SRC_LIST)

add_executable(threadpool_v1 ${SRC_LIST})

# 指定可执行文件链接需要的库文件
target_link_libraries(threadpool_v1 pthread)