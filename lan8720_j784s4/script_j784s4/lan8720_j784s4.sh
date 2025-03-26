#!/bin/bash

# Variables
#HOME_PATH="/home/dockeruser/mcw"
HOME_PATH="/mnt/d"
#SDK_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/src/phy"
SDK_PATH="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/src/phy"
#SDK_PHY_H="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/include/phy"
SDK_PHY_H="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/drv/enet/include/phy"

DRV_PATH="/mnt/d/TI_J784S4/TDA4AP/lan8720_j784s4"

#DRV_PATH= "$(HOME_PATH)/lan8720_j784s4"
#OUTPUT_PATH="${HOME_PATH}/Output"
OUTPUT_PATH="/mnt/d/Output"

#BUILD_PATH="$(HOME_PATH)/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/build"
BUILD_PATH="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/build"
#PINMUX_PATH="${HOME_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/board/src/j784s4_evm"
PINMUX_PATH="/mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04/pdk_j784s4_10_01_00_25/packages/ti/board/src/j784s4_evm"

PINMUX_C_FILE="${PINMUX_PATH}/J784S4_pinmux_data.c"
#PINMUX_C_FILE="/mnt/d/J784S4_pinmux_data.c"


# For the required tools
#cd ${HOME_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
cd /mnt/d/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04

./sdk_builder/scripts/setup_psdk_rtos.sh

# Build pdk libs
cd $BUILD_PATH
make -sj BOARD=j784s4_evm CORE=mcu2_0      #RF50 is the main core of J784S4 Board which is mcu2_0 

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

# NEW_PINMUX_CFG holds the RMII configuration for LAN8720
read -r -d '' NEW_PINMUX_CFG << 'EOF'
static pinmuxPerCfg_t gLan8720RmiiPinCfg[] =
{
    /* LAN8720 -> MDIO (Management Data I/O) */
    {
        PIN_MCASP1_AFSX, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> MDC (Management Data Clock) */
    {
        PIN_MCASP1_AXR0, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII RXD0 */
    {
        PIN_MCASP1_ACLKX, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII RXD1 */
    {
        PIN_MCASP0_AXR12, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII CRS_DV */
    {
        PIN_MCASP0_AXR13, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII TXD0 */
    {
        PIN_MCASP0_AXR14, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII TXD1 */
    {
        PIN_MCASP1_AXR3, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII TX_EN */
    {
        PIN_MCASP0_AXR15, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* LAN8720 -> RMII REF_CLK */
    {
        PIN_MCASP0_AXR7, PIN_MODE(6) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

EOF

# NEW_PINMUX_DATA_CFG holds the new module configuration entry for the RMII config.
NEW_PINMUX_DATA_CFG="{6, TRUE, gLan8720RmiiPinCfg},"


###############################################################################
# Step 0: (Optional) Backup the original pinmux file
cp "$PINMUX_C_FILE" "${PINMUX_C_FILE}.bak"
echo "Backup created at ${PINMUX_C_FILE}.bak"



###############################################################################
# Step 2: Insert the new gLan8720RmiiPinCfg structure after line 497.
# This uses the "read from stdin" (r command) of sed.
sed -i '497r /dev/stdin' "$PINMUX_C_FILE" <<< "$NEW_PINMUX_CFG"
echo "Inserted new gLan8720RmiiPinCfg structure after line 497 in $PINMUX_C_FILE."

###############################################################################
# Step 3: Update the module configuration array (gMcu_cpsw2gPinCfg) by adding a new entry.
# We search for the start of the gMcu_cpsw2gPinCfg array and then, within that block,
# we insert the new entry before the {PINMUX_END} marker.
sed -i '/static pinmuxModuleCfg_t gcpsw2gPinCfg\[] =/ {
    :a
    N
    /{PINMUX_END}/!ba
    s/{PINMUX_END}/'"$NEW_PINMUX_DATA_CFG"'\n    {PINMUX_END}/
}' "$PINMUX_C_FILE"

echo "Updated gLan8720RmiiPinCfg array with the new LAN8720 RMII entry."

echo "LAN8720 RMII pinmux configuration update completed."

# Build pdk lib
cd $BUILD_PATH
make -sj  BOARD=j784s4_evm CORE=mcu2_0

cd $HOME_PATH
mkdir -p /mnt/d/Output
#mkdir -p $(OUTPUT_PATH)/Output


tar -czvf ${OUTPUT_PATH}/ti-processor-sdk-rtos-j784s4-evm-10_01_00_04_MOFIFIED.tar.gz ti-processor-sdk-rtos-j784s4-evm-10_01_00_04
 
exit 0
