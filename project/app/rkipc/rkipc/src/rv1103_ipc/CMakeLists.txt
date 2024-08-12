cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/./include
					${CMAKE_SYSROOT}/usr/include/rkaiq/
					${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI
					${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI2
					${CMAKE_SYSROOT}/usr/include/rkaiq/algos
					${CMAKE_SYSROOT}/usr/include/rkaiq/common
					${CMAKE_SYSROOT}/usr/include/rkaiq/xcore
					${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser
					${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser_v2
					${CMAKE_SYSROOT}/usr/include/libdrm
					${WPA_SUPPLICANT_INCLUDE_DIRS}
					)

# include_directories(/home/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/arm-rockchip830-linux-uclibcgnueabihf/include/c++/8.3.0)
# aux_source_directory(/home/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/arm-rockchip830-linux-uclibcgnueabihf/include/c++/8.3.0)

aux_source_directory(./ SRCS)
aux_source_directory(audio SRCS)
aux_source_directory(video SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/isp/rv1106 SRCS)
aux_source_directory(server SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/ SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/socket_server SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/param SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/system SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/rtmp SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/osd SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/roi SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/network SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/storage SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/rockiva SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/event SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/common/region_clip SRCS)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc/ak SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc/common SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc/sys SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc/WebRTC SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/rv1103_ipc/MQTT SRCS)
include_directories(
					audio
					${PROJECT_SOURCE_DIR}/common/isp/rv1106
					video
					server
					${PROJECT_SOURCE_DIR}/common
					${PROJECT_SOURCE_DIR}/common/socket_server
					${PROJECT_SOURCE_DIR}/common/rtsp
					${PROJECT_SOURCE_DIR}/common/rtmp
					${PROJECT_SOURCE_DIR}/common/param
					${PROJECT_SOURCE_DIR}/common/system
					${PROJECT_SOURCE_DIR}/common/osd
					${PROJECT_SOURCE_DIR}/common/osd/freetype2
					${PROJECT_SOURCE_DIR}/common/roi
					${PROJECT_SOURCE_DIR}/common/network
					${PROJECT_SOURCE_DIR}/common/storage
					${PROJECT_SOURCE_DIR}/common/rockiva
					${PROJECT_SOURCE_DIR}/common/event
					${PROJECT_SOURCE_DIR}/common/region_clip
					)


include_directories(
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc/ak
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc/common
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc/sys
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc/WebRTC
						${PROJECT_SOURCE_DIR}/src/rv1103_ipc/MQTT
						)

include_directories(
					${PROJECT_SOURCE_DIR}/lib/libdatachannel/include
					${PROJECT_SOURCE_DIR}/lib/libdatachannel/include/impl
					${PROJECT_SOURCE_DIR}/lib/libdatachannel/include/rtc
					${PROJECT_SOURCE_DIR}/lib/libdatachannel/include/internal
					${PROJECT_SOURCE_DIR}/lib/libdatachannel/include/plog
)

include_directories(
					${PROJECT_SOURCE_DIR}/lib/mqtt/include
)
# OpenSSL include 
include_directories(/home/sysdrv/tools/board/toolkits/openssl/openssl-1.1.1h/include)
# include_directories(/home/project/app/rkipc/rkipc/lib/libdatachannel/include/crypto/include)

link_directories(${PROJECT_SOURCE_DIR}/lib/arm-rockchip830-linux-uclibcgnueabihf)
link_directories(/home/sysdrv/tools/board/toolkits/openssl/openssl-1.1.1h)	# OpenSSL
link_directories(${PROJECT_SOURCE_DIR}/lib/libdatachannel/lib)	# libdatachannel
link_directories(${PROJECT_SOURCE_DIR}/lib/mqtt/lib)

set(CMAKE_PREFIX_PATH "/home/sysdrv/tools/board/toolkits/openssl/openssl-1.1.1h")
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL "Cannot find OpenSSL, please check environment")
endif()

add_definitions(-DISP_HW_V32)

#add_definitions(-g -ggdb)
# add_definitions(-Os)

# add_definitions(-fsanitize=address -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
# add_definitions(-fno-stack-protector -fno-omit-frame-pointer -fsanitize-recover=address)
# link_libraries(asan)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} pthread rockit rockchip_mpp rkaiq rtsp rkaudio_detect aec_bf_process wpa_client m rkmuxer freetype rockiva iconv rknnmrt rga stdc++ rksysutils)
# target_link_libraries(${PROJECT_NAME} thread mutex condition_variable queue functional string)
target_link_libraries(${PROJECT_NAME}  ssl crypto)
target_link_libraries(${PROJECT_NAME} datachannel juice srtp2 usrsctp)		# libdatachannel
target_link_libraries(${PROJECT_NAME} paho-mqtt3a paho-mqtt3as paho-mqtt3c paho-mqtt3cs)		# lib static mqtt

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES rkipc-300w.ini DESTINATION share)
install(FILES rkipc-400w.ini DESTINATION share)
install(FILES ../../common/speaker_test.wav DESTINATION share)
install(PROGRAMS RkLunch.sh DESTINATION bin)
install(PROGRAMS RkLunch-stop.sh DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/common/osd/image.bmp DESTINATION share)
install(FILES ${PROJECT_SOURCE_DIR}/common/osd/simsun_en.ttf DESTINATION share)
install(FILES ${PROJECT_SOURCE_DIR}/lib/arm-rockchip830-linux-uclibcgnueabihf/libwpa_client.so DESTINATION lib)
install(FILES ${PROJECT_SOURCE_DIR}/lib/libdatachannel/lib/libdatachannel.so DESTINATION lib)
