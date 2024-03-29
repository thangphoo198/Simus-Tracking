# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

cmake_minimum_required(VERSION 3.13)

set(SOURCE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_TOP_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)
set(WITH_WERROR OFF)
include(cmake/extension.cmake)

# Use a global property to collect all application libraries
define_property(GLOBAL PROPERTY app_libraries
    BRIEF_DOCS "app libraries"
    FULL_DOCS "app libraries"
)

set(out_hex_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_hex_example_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_hex_unittest_dir ${CMAKE_CURRENT_BINARY_DIR}/hex/unittests)
set(out_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_quec_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_app_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_rpc_dir ${CMAKE_CURRENT_BINARY_DIR}/rpcgen)
set(out_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
set(tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(ql_app_dir ${CMAKE_CURRENT_SOURCE_DIR}/components/ql-application)

set (QL_CCSDK_QUEC_PROJECT $ENV{ql_buildproj})
set(ql_dm_dir ${SOURCE_TOP_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/dm_libs)
set(build_target_dir ${CMAKE_CURRENT_SOURCE_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/$ENV{BUILD_TARGET})
message("\n target dir: ${CMAKE_CURRENT_SOURCE_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/$ENV{BUILD_TARGET}\n")

set(elf2symld_py ${tools_dir}/elf2symld.py)
set(elf2incld_py ${tools_dir}/elf2incld.py)
set(pacgen_py ${tools_dir}/pacgen.py)
set(signature_py ${tools_dir}/signature/signature.py)
set(iomuxgen_py ${tools_dir}/iomuxgen.py)
set(keypadgen_py ${tools_dir}/keypadgen.py)
set(partinfogen_py ${tools_dir}/partinfogen.py)
set(modemgen_py ${tools_dir}/modemgen.py)
set(romtablegen_py ${tools_dir}/romtablegen.py)
set(symdefgen_py ${tools_dir}/symdefgen.py)
set(groupgen_py ${tools_dir}/groupgen.py)
set(norpropgen_py ${tools_dir}/norpropgen.py)
set(corestubgen dtools expgen)
set(cmd_mkuimage dtools mkuimage)
set(cmd_mksimage dtools mksimage)
set(cmd_mkappimg dtools mkappimg)
set(cmd_elf2lod dtools elf2lod)
set(cmd_elf2bin dtools elf2bin)
set(cmd_elf2romhex dtools elf2romhex)
set(cmd_lzmare2 dtools lzmare2)
set(cmd_re2c re2c)
set(partinfo_bin ${out_hex_dir}/partinfo.bin)
set(core_stub_o ${out_lib_dir}/core_stub.o)
set(dummy_c_file ${SOURCE_TOP_DIR}/cmake/dummy.c)
set(dummy_cxx_file ${SOURCE_TOP_DIR}/cmake/dummy.cpp)

set(AUTO_GENERATION_NOTICE "Auto generated. Don't edit it manually!")

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --abbrev=8 --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE BUILD_GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# BUILD_REVISION: A string to identify the build. When not specified, which
#       is common in development, it will be "DEVEL". CI will define it in
#       command line, such as "8915_W19.32.2"
set (BUILD_REVISION "1_4_W21.31.2")
if(NOT BUILD_REVISION)
    set(BUILD_REVISION $ENV{BUILD_REVISION})
    if(NOT BUILD_REVISION)
        set(BUILD_REVISION DEVEL)
    endif()
endif()

# BUILD_AUTO_REVISION: A string to identify the source codes revision.
#       When it is built under git working directory, it will be "g1234abcd"
#       or "g1234abcd-dirty" if the working directory is dirty. When not in
#       git working directory, it will be "unknown". And it can be overrided
#       in command line.
if(NOT BUILD_AUTO_REVISION)
    if(BUILD_GIT_REVISION)
        set(BUILD_AUTO_REVISION "g${BUILD_GIT_REVISION}")
    endif()
endif()

if(NOT BUILD_AUTO_REVISION)
    set(BUILD_AUTO_REVISION "unknown")
endif()

# BUILD_REVISION_NUMBER: A number to identify the source codes revision.
#       When it is built under git working directory, and the working
#       directory is clean, it will be "0x1234abcd". Otherwise, it is "0".
#       And it can be overrided in command line.
if(NOT BUILD_REVISION_NUMBER)
    if(BUILD_GIT_REVISION)
        set(BUILD_REVISION_NUMBER "0x${BUILD_GIT_REVISION}")
    endif()
    if(BUILD_REVISION_NUMBER MATCHES "-dirty")
        set(BUILD_REVISION_NUMBER "0")
    endif()
endif()

if(NOT BUILD_REVISION_NUMBER)
    set(BUILD_REVISION_NUMBER "0")
endif()

# BUILD_TARGET
if(NOT BUILD_TARGET)
    set(BUILD_TARGET $ENV{BUILD_TARGET})
endif()

# BUILD_RELEASE_TYPE
if (NOT BUILD_RELEASE_TYPE)
    set (BUILD_RELEASE_TYPE $ENV{BUILD_RELEASE_TYPE})
endif()

if("${BUILD_RELEASE_TYPE}" STREQUAL "release")
    set(CONFIG_WDT_ENABLE ON)
endif()

if(NOT BUILD_TARGET)
    message(FATAL_ERROR "BUILD_TARGET is empty")
endif()

#include(components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/ql_target.cmake)
#if (NOT CONFIG_QUEC_PROJECT_FEATURE_VOLTE)
#	if ($ENV{quec_ims_feature})
#		message(FATAL_ERROR "\nVOLTE is disabled at CCSDK!!!")
#	endif()
#endif()
set (ql_dsim_enable "n")
if ($ENV{ql_dsim_cfg})
	if(EXISTS ${SOURCE_TOP_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/ql_diff_ctx.cmake)
		include(${SOURCE_TOP_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/ql_diff_ctx.cmake)
	endif()
	if (CONFIG_QUEC_DIFF_CTX_DSIM)
		set (ql_dsim_enable "y")
	else()
		set (ql_dsim_enable "n")
		message("\n")
		#FATAL_ERROR
		message(WARNING "*********************************************\nDOUBLESIM is not supported for your project ${QL_CCSDK_QUEC_PROJECT}, ignore...\n*********************************************\n")
	endif()
endif()

set (QL_CCSDK_BUILD $ENV{_ccsdk_build})
set (QL_CORE_BUILD_VER $ENV{buildver})
set (QL_APP_BUILD_VER $ENV{ql_app_ver})
# Process and include target config
set(CONFIG_BUILD_IDENTIFY "${BUILD_REVISION}")

set(SRC_CONFIG ${SOURCE_TOP_DIR}/components/ql-config/build/${QL_CCSDK_QUEC_PROJECT}/${BUILD_TARGET}/target.config)
set(TARGET_CONFIG ${BINARY_TOP_DIR}/target.config)
file(COPY ${SRC_CONFIG} DESTINATION  ${BINARY_TOP_DIR})

file(APPEND ${TARGET_CONFIG} "\nCONFIG_APP_RAM_OFFSET=$ENV{ap_ram_offset}\n")
file(APPEND ${TARGET_CONFIG} "CONFIG_APP_TOTAL_RAM_SIZE=$ENV{ap_ram_size}\n")
file(APPEND ${TARGET_CONFIG} "CONFIG_SUPPORT_IMS_DELTA_NV=$ENV{ims_delta_nv}\n")
file(APPEND ${TARGET_CONFIG} "CONFIG_DUAL_SIM_SUPPORT=${ql_dsim_enable}\n")
file(APPEND ${TARGET_CONFIG} "CONFIG_QUEC_PROJECT_FEATURE_QDSIM=${ql_dsim_enable}\n")
file(APPEND ${TARGET_CONFIG} "CONFIG_QUEC_PROJECT_FEATURE_VOLTE=$ENV{quec_ims_feature}\n")

set(TARGET_CMAKE ${BINARY_TOP_DIR}/target.cmake)
execute_process(
    COMMAND python3 ${tools_dir}/cmakeconfig.py ${TARGET_CONFIG} ${TARGET_CMAKE}
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)
include(${TARGET_CMAKE})
message("\nCurr Proj: ${CONFIG_QL_PROJECT_DEF}${CONFIG_QL_PROJECT_DEF_SUB}, QL_CCSDK_BUILD ${QL_CCSDK_BUILD}\n")
message("\n${CONFIG_NVM_VARIANTS}, ${CONFIG_PARTINFO_JSON_PATH}, ${CONFIG_APP_RAM_OFFSET}, ${CONFIG_APP_TOTAL_RAM_SIZE}, ${CONFIG_SUPPORT_IMS_DELTA_NV}, ${CONFIG_DUAL_SIM_SUPPORT}, ${CONFIG_BOARD_WITH_EXT_FLASH}\n")

configure_file(${SOURCE_TOP_DIR}/components/ql-kernel/inc/quec_proj_config.h.in ${out_inc_dir}/quec_proj_config.h)

# Process and include partition config
execute_process(
    COMMAND python3 ${partinfogen_py} ${CONFIG_PARTINFO_JSON_PATH} ${partinfo_bin} ${BINARY_TOP_DIR}/partinfo.cmake
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)
include(${BINARY_TOP_DIR}/partinfo.cmake)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${partinfogen_py} ${CONFIG_PARTINFO_JSON_PATH})

# Include chip config
include(components/chip/chip.cmake)

# Process and include Kconfig dependency
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TARGET_CONFIG})
set(KCONFIG_DEP ${BINARY_TOP_DIR}/kconfig_dep.cmake)
execute_process(
    COMMAND python3 ${tools_dir}/cmakedep.py ${KCONFIG_DEP}
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)
include(${KCONFIG_DEP})

# Include toolchain config
include(cmake/toolchain-gcc.cmake)
include(components/ql-application/ql_app_feature_config.cmake)
set(BUILD_SHARED_LIBS OFF)

project(${BUILD_TARGET} C CXX ASM)
include_directories(${out_inc_dir})
include_directories(${SOURCE_TOP_DIR}/components/include)
include_directories(${SOURCE_TOP_DIR}/components/newlib/include)
include_directories(${SOURCE_TOP_DIR}/components/hal/include)
include_directories(${SOURCE_TOP_DIR}/components/fs/include)
include_directories(${SOURCE_TOP_DIR}/components/fs/fsmount/include)
include_directories(${SOURCE_TOP_DIR}/components/cfw/include)
include_directories(${SOURCE_TOP_DIR}/components/net/lwip/include)
include_directories(${SOURCE_TOP_DIR}/components/net/lwip/src/include)
# CSDK without lwip source
include_directories(${SOURCE_TOP_DIR}/components/net/lwip/include/lwip)
include_directories(${SOURCE_TOP_DIR}/components/net/lwip/include/posix)
include_directories(${SOURCE_TOP_DIR}/components/net/mbedtls/include)
include_directories(${SOURCE_TOP_DIR}/components/net/include)
include_directories(${SOURCE_TOP_DIR}/components/kernel/include)
include_directories(${SOURCE_TOP_DIR}/components/driver/src/usb)
include_directories(${SOURCE_TOP_DIR}/components/ql-kernel/inc)



######### Customer ############
# Add include folder here
include_directories(Include)

################################

#add_custom_target(quectel)
add_custom_target(beautify)
add_custom_target(unittests)
#add_custom_target(examples ALL)


    set(ldscript ${SOURCE_TOP_DIR}/components/apploader/pack/app_flashimg.ld)
    set(target ${QL_APP_BUILD_VER})
    set(MainPath ${SOURCE_TOP_DIR}/main.c )
    set(SourcePath
    ${SOURCE_TOP_DIR}/Source/Driver_UART.c
    ${SOURCE_TOP_DIR}/Source/MQTT.c
	${SOURCE_TOP_DIR}/Source/sms_demo.c
	${SOURCE_TOP_DIR}/Source/I2C_demo.c
	${SOURCE_TOP_DIR}/Source/fota_http_demo.c
	${SOURCE_TOP_DIR}/Source/gnss_demo.c
	${SOURCE_TOP_DIR}/Source/cJSON.c
	${SOURCE_TOP_DIR}/Source/wifi_scan_demo.c
	${SOURCE_TOP_DIR}/Source/power_demo.c
	${SOURCE_TOP_DIR}/Source/nw_demo.c
    ${SOURCE_TOP_DIR}/Source/get_stt.c
    ${SOURCE_TOP_DIR}/Source/get_gps.c
    ${SOURCE_TOP_DIR}/Source/remote.c
	${SOURCE_TOP_DIR}/Source/setting.c


    )



    set(flash_ldscript ${SOURCE_TOP_DIR}/components/apploader/pack/app_flashimg.ld)
    add_appimg(${target} ${flash_ldscript} 
        ${MainPath} 
        ${SourcePath}
    
    )
    # add external library ex: libc, libm
	target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})

	 set(prepack_cpio ${out_hex_dir}/${target}_prepack.cpio)

    set(pac_file ${out_hex_dir}/${target}.pac)
    set(target_img_file ${out_hex_dir}/${target}.img)
    set(target_sign_img_file ${out_hex_dir}/${target}.sign.img) 
   pac_init_fdl(init_fdl)
    add_custom_command(OUTPUT ${prepack_cpio}
            COMMAND python3 ${modemgen_py} prepackgen
                --source-top ${SOURCE_TOP_DIR}
                --binary-top ${BINARY_TOP_DIR}
                --prepackfile ${CONFIG_PACKAGE_FILE_APPIMG_JSON_PATH}
                ${prepack_cpio}
            DEPENDS ${modemgen_py}
            WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    if(CONFIG_PACKAGE_FILE_APPIMG_JSON_PATH)
        set(cfg_pack_gpio cfg-pack-cpio -i PREPACK -p ${prepack_cpio})
        set(prepack_file ${prepack_cpio})
    endif()
 #  sign_image(${target_img_file} ${target_sign_img_file}) 
    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-image -i APPIMG -a ${CONFIG_APPIMG_FLASH_ADDRESS}
                -s ${CONFIG_APPIMG_FLASH_SIZE}
               -p ${target_sign_img_file}
            ${cfg_pack_gpio}
            pac-gen ${pac_file}
        DEPENDS ${pacgen_py} ${target_sign_img_file} ${prepack_file}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})


add_custom_target(rpcgen)
add_appimg_delete()

add_subdirectory_if_exist(components)
#Customer add_subdirectory_if_exist(unittest)
#add_subdirectory_if_exist(examples)

pacvariant_gen()

#file(GLOB QL_LIBS_PATH ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libq*.a ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboq*.a)
#message("\nQL_LIBS_PATH ${QL_LIBS_PATH}\n")

# "all_libs" is a target to link all app libraries
set(target all_libs)
set(all_libs_out ${out_lib_dir}/all_libs.a)
get_property(app_libraries GLOBAL PROPERTY app_libraries)

list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_dev.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_adc.a)
if(EXISTS ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_pin_init.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_pin_init.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_gpio.a)
if(EXISTS ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_api_power.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_api_power.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_power.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_bsp.a)

if(CONFIG_QUEC_PROJECT_FEATURE_FILE_AT)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_file_at.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_at.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_urc.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libquectel.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_common.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_utils.a)
#list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_rtos.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_osi.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libq_modem.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_sim.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_nw.a)
if(CONFIG_QUEC_PROJECT_FEATURE_USBNET)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libq_usbnet.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_usbnet.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_net.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_datacall.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_fs.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_file.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_fatfs_ex.a)

if(CONFIG_QUEC_PROJECT_FEATURE_FILE_ZIP)
# FILE_ZIP depends on FILE
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_zip.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_zip.a)
endif()

if((CONFIG_QUEC_PROJECT_FEATURE_FOTA) AND (CONFIG_QUEC_PROJECT_FEATURE_HTTP))
if(CONFIG_QUEC_PROJECT_FEATURE_CLOUDOTA)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_cloudota.a)
endif()
endif()

if(CONFIG_QUEC_PROJECT_FEATURE_RTC)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_rtc.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_VIRT_AT)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_virt_at.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_FOTA)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_fota.a)
endif()

if (CONFIG_QUEC_PROJECT_FEATURE_SMS)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_sms.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_PBK)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_pbk.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_VOICE_CALL)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_voice_call.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_VOLTE)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_volte.a)
endif()

list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_apptcpip.a)
if (CONFIG_QUEC_PROJECT_FEATURE_FTP)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_curl.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_ftp.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_ftp.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_SSH2)
# SSH2 depends on ssl 
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_ssh2.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_HTTP)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_http.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_http.a)
if (CONFIG_QUEC_PROJECT_FEATURE_MMS)
# MMS depends on HTTP
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_mms.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_LBS)
# LBS depends on HTTP
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_tlv.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_lbs.a)
endif()
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_MQTT)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_mqtt.a)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_mqtt.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_PING)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_ping.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_NTP)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_ntp.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_QTHSDK)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_qthsdk.a)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_qth_mqtt.a)
endif()

if(CONFIG_QUEC_PROJECT_FEATURE_AUDIO)
if(EXISTS ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_api_audio.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_api_audio.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_ext_codec.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_TTS_IN_KERNEL)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/newlib/armca5/libql_api_tts.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_UART)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_uart.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_USB)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_usb.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_LEDCFG)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_pwm.a)
endif()
if(CONFIG_QUEC_PROJECT_FEATURE_I2C)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_i2c.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_KEYPAD)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_keypad.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_LCD)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_lcd.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_WIFISCAN)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_wifiscan.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_BT)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_bt.a)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_ble.a)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_bt_ble.a)
if (CONFIG_QUEC_PROJECT_FEATURE_BT_HFP)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_bt_hfp.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_BT_A2DP_AVRCP)
# Currently, BT A2DP & AVRCP depends on AUDIO to support AVRCP
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_bt_a2dp_avrcp.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_BLE_GATT)
 list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_ble_gatt.a)
endif()
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_GNSS)
# GNSS depends on UART/GPIO
# GNSS depends on HTTP to support AGPS.
if(EXISTS ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_gnss.a)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/liboql_gnss.a)
endif()
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_gnss_aux.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_CAMERA)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_camera.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_SPI)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_spi.a)
if (CONFIG_QUEC_PROJECT_FEATURE_SPI_FLASH)
# SPI_FLASH depends on SPI
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_spi_flash.a)
if (CONFIG_QUEC_PROJECT_FEATURE_SPI_NOR_FLASH)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_spi_nor_flash.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_SPI_NAND_FLASH)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_spi_nand_flash.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_FS_NAND_FLASH)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_fs_nand_flash.a)
endif()
endif()
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_spi6_ext_nor_flash.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_SDMMC)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_api_sdmmc.a)
endif()
if (CONFIG_QUEC_PROJECT_FEATURE_GPRS_DATA_TRANSFER)
list(APPEND app_libraries ${SOURCE_TOP_DIR}/components/ql-kernel/libs/libql_gprs_data_transfer.a)
endif()

list(APPEND app_libraries ${libc_file_name} ${libm_file_name} ${libgcc_file_name} ${QL_LIBS_PATH})
add_custom_command(OUTPUT ${all_libs_out}
    COMMAND python3 ${groupgen_py} --base ${BINARY_TOP_DIR} ${all_libs_out} ${app_libraries}
    DEPENDS ${groupgen_py} ${app_libraries}
    WORKING_DIRECTORY ${BINARY_TOP_DIR}
)
add_custom_target(build_${target} DEPENDS ${all_libs_out})
add_library(${target} STATIC IMPORTED GLOBAL)
set_target_properties(${target} PROPERTIES IMPORTED_LOCATION ${all_libs_out})
add_dependencies(${target} build_${target})
