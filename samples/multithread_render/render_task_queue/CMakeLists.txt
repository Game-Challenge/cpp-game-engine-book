﻿cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    add_compile_options(/wd4819)#忽略保存为unicode警告
endif ()

include(CMakeLists.txt.glfw)

#拷贝depends目录
file(COPY "../../template/depends/glfw-3.3-3.4/" DESTINATION "../depends/glfw-3.3-3.4/")
file(COPY "../../template/depends/glm/" DESTINATION "../depends/glm/")
file(COPY "../../template/depends/SPSCQueue-1.1/" DESTINATION "../depends/spscqueue/")

include_directories("depends")

add_executable(untitled ${glfw_sources} main.cpp render_task_queue.cpp render_task_consumer.cpp render_task_producer.cpp)