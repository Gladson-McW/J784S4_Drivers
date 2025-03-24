#!/bin/bash

# Variables
HOME_PATH="/home/dockeruser/mcw"
SDK_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/src/phy"
SDK_PHY_H="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/include/phy"
DRV_PATH="$(HOME_PATH)/lan8720_j784s4"
OUTPUT_PATH="${HOME_PATH}/Output"
BUILD_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25\packages\ti\build"
PINMUX_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/board/src/j784s4_evm"
BUILD_OS_TYPE="FreeRTOS"

#if grep -q 'PATTERN_TO_MATCH' "$SDK_PATH/enetphy.c"; then
#    sed -i '/PATTERN_TO_MATCH/c\Your new code here' "$SDK_PATH/enetphy.c"
#else
#    echo 'Your new code here' >> "$SDK_PATH/enetphy.c"
#fi

# For the required tools
cd ${HOME_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
./sdk_builder/scripts/setup_psdk_rtos.sh

# Build pdk libs
cd $BUILD_PATH
make -sj  BOARD=j784s4_evm CORE=mcu2_0      #RF50 is the main core of J784S4 Board which is mcu2_0 

#Reference: https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-j784s4/09_02_00_05/exports/docs/pdk_j784s4_09_02_00_30/docs/apiguide/j784s4/html/enetphy_guide_top.html#enetphy_guide_implementing

# Move the driver and makefile into the sdk
if [ -d "$DRV_PATH" ]; then
    echo "Moving driver to sdk"
    mv "${DRV_PATH}/driver_j784s4/src/lan8720.c" "${SDK_PATH}"
    mv "${DRV_PATH}/driver_j784s4/inc/lan8720_priv.h" "${SDK_PATH}"
    mv "${DRV_PATH}/driver_j784s4/inc/lan8720.h" "${SDK_PHY_H}"
    #mv "${DRV_PATH}/" "$SDK_PATH"
    #mv "${DRV_PATH}/driver_j784s4/makefile_temp_bmp280.mk" "${SDK_PATH}/build"    # makefile_temp_bmp280.mk
    echo "Folders moved successfully."
else
    echo "Source folder does not exist."
    exit 1
fi

# Append makefile statements
# Copy paste external_drivers_component.mk content 
#touch ${SDK_PATH}/external_drivers_component.mk 
#cp ${DRV_PATH}/driver_j784s4/ext_component_content.txt ${SDK_PATH}/external_drivers_component.mk

sed -i '55i #include "lan8720_priv.h" ' "$SDK_PATH/enetphy.c"
sed -i '191i extern EnetPhy_Drv gEnetPhyDrvLan8720;' "$SDK_PATH/enetphy.c"
sed -i '/&gEnetPhyDrvGeneric,.*Generic PHY/ i \    &gEnetPhyDrvLan8720,   /* LAN8720 */' "$SDK_PATH/enetphy.c"
#sed -i '200i };' "$SDK_PATH/enetphy.c"

sed -i '22i $a lan8720.c' "$SDK_PATH/makefile"

# Appending in component.mk
#sed -i '/^export PDK_COMMON_BAREMETAL_COMP/i -include $(EXTERNAL_DRIVERS_COMP_PATH)/external_drivers_component.mk\nifneq ($(ext_drv_LIB_LIST),)\n\tpdk_LIB_LIST += $(ext_drv_LIB_LIST)\nendif\n' "${BUILD_PATH}/makerules/component.mk"

# Appending in comp_paths.mk
#sed -i '/export PDK_BOARD_DIAG_COMP_PATH = $(PDK_BOARD_DIAG_ROOT_PATH)\/ti\/board\/diag/a \
#\nEXTERNAL_DRIVERS_ROOT_PATH ?= $(PDK_PATH)\nexport EXTERNAL_DRIVERS_COMP_PATH = $(EXTERNAL_DRIVERS_ROOT_PATH)\/ti\/drv\/external_drivers' "${BUILD_PATH}/comp_paths.mk"


# Build pdk lib
cd $BUILD_PATH
make -sj  BOARD=j784s4_evm CORE=mcu2_0
#make -s pdk_libs BOARD=j784s4_evm CORE=mcu2_0


cd $HOME_PATH
tar -czvf ${OUTPUT_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04.tar.gz ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
 
exit 0
