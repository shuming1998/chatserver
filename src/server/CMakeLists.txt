
aux_source_directory(. SRC) # 存储当前目录下所有源文件
aux_source_directory(./db DB)
aux_source_directory(./model MODEL)
aux_source_directory(./redis REDIS)

add_executable(ChatServer ${SRC} ${DB} ${MODEL} ${REDIS})

target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)
