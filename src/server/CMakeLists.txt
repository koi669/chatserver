# 定义了一个SRC_LIST变量，包含了当前目录下所有的源文件
aux_source_directory(. SRC_LIST)

# 定义了一个DB_LIST变量，包含了db目录下所有的源文件，并添加到SRC_LIST变量
aux_source_directory(./db DB_LIST)
list(APPEND SRC_LIST ${DB_LIST})

# 定义一个MODEL_LIST变量，包含了model目录下的所有源文件，并添加到SRC_LIST变量
aux_source_directory(./model MODEL_LIST)
list(APPEND SRC_LIST ${MODEL_LIST})

# 定义一个REDIS_LIST变量，包含了redis目录下的所有源文件，并添加到SRC_LIST变量
aux_source_directory(./redis REDIS_LIST)
list(APPEND SRC_LIST ${REDIS_LIST})

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST})

# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)

# 指定输出目录
set_target_properties(ChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)