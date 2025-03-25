#!/bin/bash

# Variables
HOME_PATH="/home/dockeruser/mcw"
#HOME_PATH="/mnt/d/"
SDK_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/src/phy"
#SDK_PATH="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/src/phy"
SDK_PHY_H="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/include/phy"
#SDK_PHY_H="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/include/phy"

#DRV_PATH="/mnt/d/TI_J784S4/TDA4AP/lan8720_j784s4"

DRV_PATH= "$(HOME_PATH)/lan8720_j784s4"
OUTPUT_PATH="${HOME_PATH}/Output"
#OUTPUT_PATH="/mnt/d/Output"

BUILD_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/build"
#BUILD_PATH="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/build"

#PINMUX_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/board/src/j784s4_evm"
#BUILD_OS_TYPE="FreeRTOS"


# For the required tools
cd ${HOME_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
#cd /mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04

#./sdk_builder/scripts/setup_psdk_rtos.sh

# Build pdk libs
cd $BUILD_PATH
#make -sj BOARD=j784s4_evm CORE=mcu2_0      #RF50 is the main core of J784S4 Board which is mcu2_0 

#Reference: https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-j784s4/09_02_00_05/exports/docs/pdk_j784s4_09_02_00_30/docs/apiguide/j784s4/html/enetphy_guide_top.html#enetphy_guide_implementing

# Move the driver and makefile into the sdk
if [ -d "$DRV_PATH" ]; then
    echo "Moving driver to sdk"
    cp "${DRV_PATH}/driver_j784s4/src/lan8720.c" "${SDK_PATH}"
    #mv "${DRV_PATH}\driver_j784s4\src\lan8720.c" "${SDK_PATH}"
    cp "${DRV_PATH}/driver_j784s4/inc/lan8720_priv.h" "${SDK_PATH}"
    #mv "${DRV_PATH}\driver_j784s4\inc\lan8720_priv.h" "${SDK_PATH}"
    cp "${DRV_PATH}/driver_j784s4/inc/lan8720.h" "${SDK_PHY_H}"
    #mv "${DRV_PATH}\driver_j784s4\inc\lan8720.h" "${SDK_PHY_H}"

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


# In enetphy.c, insert header if not already present at line 55:
grep -q '^#include "lan8720_priv.h"' "$SDK_PATH/enetphy.c" || sed -i '55i #include "lan8720_priv.h"' "$SDK_PATH/enetphy.c"

# Insert external driver declaration at line 191 if not already present:
grep -q '^extern EnetPhy_Drv gEnetPhyDrvLan8720;' "$SDK_PATH/enetphy.c" || sed -i '191i extern EnetPhy_Drv gEnetPhyDrvLan8720;' "$SDK_PATH/enetphy.c"

# Insert LAN8720 driver into the PHY driver lookup table (ensure it’s added only once):
grep -q '&gEnetPhyDrvLan8720,.*LAN8720' "$SDK_PATH/enetphy.c" || sed -i '/&gEnetPhyDrvGeneric,.*Generic PHY/ i \    &gEnetPhyDrvLan8720,   /* LAN8720 */' "$SDK_PATH/enetphy.c"

# For the makefile: Append "lan8720.c" at the end only if not present:
# Define the path to your makefile
MAKEFILE="$SDK_PATH/makefile"

# Check if 'lan8720.c' is already in the SRCS_COMMON line to avoid duplication
grep -q 'lan8720\.c' "$MAKEFILE" || sed -i '22s/$/lan8720.c/' "$MAKEFILE"

# Appending in component.mk
#sed -i '/^export PDK_COMMON_BAREMETAL_COMP/i -include $(EXTERNAL_DRIVERS_COMP_PATH)/external_drivers_component.mk\nifneq ($(ext_drv_LIB_LIST),)\n\tpdk_LIB_LIST += $(ext_drv_LIB_LIST)\nendif\n' "${BUILD_PATH}/makerules/component.mk"

# Appending in comp_paths.mk
#sed -i '/export PDK_BOARD_DIAG_COMP_PATH = $(PDK_BOARD_DIAG_ROOT_PATH)\/ti\/board\/diag/a \
#\nEXTERNAL_DRIVERS_ROOT_PATH ?= $(PDK_PATH)\nexport EXTERNAL_DRIVERS_COMP_PATH = $(EXTERNAL_DRIVERS_ROOT_PATH)\/ti\/drv\/external_drivers' "${BUILD_PATH}/comp_paths.mk"


# Build pdk lib
cd $BUILD_PATH
make -sj  BOARD=j784s4_evm CORE=mcu2_0


cd $HOME_PATH
#mkdir -p /mnt/d/Output
mkdir -p $(OUTPUT_PATH)


tar -czvf ${OUTPUT_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04_MOFIFIED.tar.gz ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
 
exit 0
