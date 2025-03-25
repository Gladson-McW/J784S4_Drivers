/**
 * @file lan8720.c
 * @brief Ethernet Driver Implementation for LAN8720 on J784S4
 *
 * This file implements an Ethernet driver for the LAN8720 PHY.
 * It performs PHY initialization, configuration, and provides transmit and receive
 * functions for arbitrary data packets. Additionally, it implements the EnetPhy_Drv interface
 * for seamless integration with the upper-layer Ethernet PHY framework.
 *
 *
 * @date March 24, 2025
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#define ENETTRACE_MOD_ID 0x506   /* Unique ID for tracing */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ti/drv/enet/include/phy/lan8720.h>         /* Public LAN8720 definitions */
#include "lan8720_priv.h"                            /* LAN8720 register definitions and macros */
#include "enetphy_priv.h"                            /* Common PHY definitions */
#include "generic_phy.h"                             /* Generic PHY access helpers */
#include <ti/drv/enet/enet.h>
#include <ti/drv/enet/enet_cfg.h>
#include <ti/drv/enet/include/core/enet_ioctl.h>
#include <ti/drv/enet/include/phy/enetphy.h>
#include <ti/csl/cslr_mdio.h>
#include <ti/drv/enet/priv/core/enet_trace_priv.h>


/* ========================================================================== */
/*                           Macro Definitions                                */
/* ========================================================================== */
#define ENET_MAC_PORT          ENET_MAC_PORT_1
#define ENET_PHY_ADDR          0x01
#define ENET_TX_PKT_SIZE       1500
#define ENET_RX_PKT_SIZE       1500

/* LAN8720 version identification */
#define LAN8720_OUI      (0x000001C1U)
#define LAN8720_MODEL    (0x27U)
#define LAN8720_REV      (0x00U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
Enet_Handle hEnet;
Enet_IoctlPrms prms;
Enet_MacPort macPort = ENET_MAC_PORT;
EnetPhy_Cfg phyCfg = { .phyAddr = ENET_PHY_ADDR };

/* Transmit and receive buffers */
static uint8_t txBuffer[ENET_TX_PKT_SIZE];
static uint8_t rxBuffer[ENET_RX_PKT_SIZE];

/* ========================================================================== */
/*                   PHY Driver Interface Function Prototypes                 */
/* ========================================================================== */
static bool Lan8720_isPhyDevSupported(EnetPhy_Handle hPhy, const EnetPhy_Version *version);
static bool Lan8720_isMacModeSupported(EnetPhy_Handle hPhy, EnetPhy_Mii mii);
static int32_t Lan8720_config(EnetPhy_Handle hPhy, const EnetPhy_Cfg *cfg, EnetPhy_Mii mii);
static void Lan8720_reset(EnetPhy_Handle hPhy);
static bool Lan8720_isResetComplete(EnetPhy_Handle hPhy);
#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_INFO)
static void Lan8720_printRegs(EnetPhy_Handle hPhy);
#endif

/* Extended internal helper functions */
static void Lan8720_setMiiMode(EnetPhy_Handle hPhy, EnetPhy_Mii mii);
static void Lan8720_setVtmIdleThresh(EnetPhy_Handle hPhy, uint32_t idleThresh);
static void Lan8720_setDspFFE(EnetPhy_Handle hPhy);
static void Lan8720_fixFldStrap(EnetPhy_Handle hPhy);
static void Lan8720_setLoopbackCfg(EnetPhy_Handle hPhy, bool enable);
static void Lan8720_enableAutoMdix(EnetPhy_Handle hPhy, bool enable);
static void Lan8720_setClkShift(EnetPhy_Handle hPhy, bool txShiftEn, bool rxShiftEn);
static int32_t Lan8720_setTxFifoDepth(EnetPhy_Handle hPhy, uint8_t depth);
static int32_t Lan8720_setClkDelay(EnetPhy_Handle hPhy, uint32_t txDelay, uint32_t rxDelay);
static int32_t Lan8720_setOutputImpedance(EnetPhy_Handle hPhy, uint32_t impedance);
static void Lan8720_setGpioMux(EnetPhy_Handle hPhy, LAN8720_Gpio0Mode gpio0Mode, LAN8720_Gpio1Mode gpio1Mode);
static void Lan8720_setLedMode(EnetPhy_Handle hPhy, const LAN8720_LedMode *ledMode);
static void Lan8720_restart(EnetPhy_Handle hPhy);
static void Lan8720_rmwExtReg(EnetPhy_Handle hPhy, uint32_t reg, uint16_t mask, uint16_t val);

/* ========================================================================== */
/*                     Ethernet Driver Public Functions                       */
/* ========================================================================== */

/**
 *  \brief Initializes the Ethernet driver and LAN8720 PHY.
 */
void Ethernet_init(void)
{
    Enet_init();
    Enet_open(hEnet, &prms);
    Enet_ioctl(hEnet, ENET_IOCTL_SET_MAC_PORT_STATE, &macPort, &prms);
    EnetPhy_open(hEnet, ENET_MAC_PORT, &phyCfg);
    Ethernet_config();
    printf("Ethernet Initialized Successfully\n");
}

/**
 *  \brief Configures the LAN8720 PHY.
 *
 *  Reads PHY ID registers, prints them, and enables auto-negotiation.
 */
void Ethernet_config(void)
{
    uint16_t phyId1 = 0, phyId2 = 0;
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_PHYID1, &phyId1);
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_PHYID2, &phyId2);
    printf("LAN8720 PHY ID1: 0x%x, PHY ID2: 0x%x\n", phyId1, phyId2);

    /* Enable auto-negotiation */
    uint16_t ctrlReg = BMCR_AUTO_NEG_ENABLE | BMCR_RESTART_AUTO_NEG;
    lan8720_write_reg(ENET_PHY_ADDR, LAN8720_BMCR, ctrlReg);
}

/**
 *  \brief Transmits an Ethernet packet.
 *
 *  This function accepts a pointer to arbitrary data and its length.
 *  It copies the data into a DMA packet and submits it for transmission.
 *
 *  \param data Pointer to the data to be transmitted.
 *  \param len  Length of the data in bytes.
 */
int Ethernet_sendPacket(const void *data, size_t len)
{
    if (len > ENET_TX_PKT_SIZE)
    {
        len = ENET_TX_PKT_SIZE;
    }
    memcpy(txBuffer, data, len);

    EnetDma_PktQ txQueue;
    EnetDma_Pkt *pTxPkt = EnetDma_allocPkt(hEnet, ENET_DMA_DIR_TX);
    if (pTxPkt == NULL)
    {
        printf("Failed to allocate TX packet\n");
        return -1;
    }
    memcpy(pTxPkt->bufPtr, txBuffer, len);
    EnetDma_submitTxPktQ(hEnet, ENET_MAC_PORT, &txQueue);
    printf("Packet transmitted (%u bytes)\n", (unsigned)len);
    return 0;
}

/**
 *  \brief Receives an Ethernet packet.
 *
 *  This function attempts to retrieve a received packet and copies the data
 *  into the provided buffer up to maxLen bytes.
 *
 *  \param buffer Pointer to a buffer where the received data will be stored.
 *  \param maxLen Maximum number of bytes to copy.
 *  \return Number of bytes received, or -1 if no packet was available.
 */
int Ethernet_receivePacket(void *buffer, size_t maxLen)
{
    EnetDma_PktQ rxQueue;
    EnetDma_Pkt *pRxPkt;
    EnetDma_retrieveRxPktQ(hEnet, ENET_MAC_PORT, &rxQueue);
    if (EnetDma_isPktQEmpty(&rxQueue))
    {
        return -1;  /* No packet available */
    }
    pRxPkt = EnetDma_getNextPkt(&rxQueue);
    if (pRxPkt == NULL)
    {
        return -1;
    }
    size_t rxLen = strlen((char *)pRxPkt->bufPtr);
    if (rxLen > maxLen)
    {
        rxLen = maxLen;
    }
    memcpy(buffer, pRxPkt->bufPtr, rxLen);
    EnetDma_freePkt(hEnet, pRxPkt);
    printf("Packet received (%u bytes)\n", (unsigned)rxLen);
    return (int)rxLen;
}

/**
 *  \brief Retrieves the Ethernet link status.
 *
 *  \return 1 if the link is up, 0 otherwise.
 */
uint8_t Ethernet_getStatus(void)
{
    uint16_t statusReg = 0;
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMSR, &statusReg);
    return (statusReg & BMSR_LINK_STATUS) ? 1 : 0;
}

/**
 *  \brief Main device function for managing Ethernet tasks.
 *
 *  This function initializes the driver and then continuously polls for received packets.
 *  In a real system, this may be integrated with an RTOS task or interrupt service routine.
 */
void Ethernet_deviceMain(void)
{
    Ethernet_init();
    while (1)
    {
        if (Ethernet_getStatus())
        {
            uint8_t rxData[ENET_RX_PKT_SIZE] = {0};
            int ret = Ethernet_receivePacket(rxData, ENET_RX_PKT_SIZE);
            if (ret > 0)
            {
                printf("Received Data: %s\n", rxData);
            }
        }
        /* Add delay or yield to RTOS scheduler as needed */
        EnetOsal_sleep(1000U); /* Sleep for 1000ms; replace with your system's delay function */
    }
}

/* ========================================================================== */
/*                    PHY Driver Interface Implementations                    */
/* ========================================================================== */

/**
 *  \brief Checks if the PHY device is supported.
 */
static bool Lan8720_isPhyDevSupported(EnetPhy_Handle hPhy, const EnetPhy_Version *version)
{
    bool supported = false;
    if ((version->oui == LAN8720_OUI) &&
        (version->model == LAN8720_MODEL) &&
        (version->revision == LAN8720_REV))
    {
        supported = true;
    }
    return supported;
}

/**
 *  \brief Checks if the MAC mode is supported.
 */
static bool Lan8720_isMacModeSupported(EnetPhy_Handle hPhy, EnetPhy_Mii mii)
{
    bool supported;
    switch (mii)
    {
        case ENETPHY_MAC_MII_MII:
        case ENETPHY_MAC_MII_RMII:
            supported = true;
            return true;
        /* LAN8720 doesn't support RGMII Interface*/
        case ENETPHY_MAC_MII_RGMII:
        default:
            supported=false;
            return false;
    }
}

/**
 *  \brief Minimal PHY configuration using Ethernet_config().
 */
static int32_t Lan8720_config(EnetPhy_Handle hPhy, const EnetPhy_Cfg *cfg, EnetPhy_Mii mii)
{
    Ethernet_config();
    return ENETPHY_SOK;
}

/**
 *  \brief Performs a global software reset.
 */
static void Lan8720_reset(EnetPhy_Handle hPhy)
{
    lan8720_write_reg(ENET_PHY_ADDR, LAN8720_BMCR, BMCR_RESET);
}

/**
 *  \brief Checks if the global reset is complete.
 */
static bool Lan8720_isResetComplete(EnetPhy_Handle hPhy)
{
    uint16_t reg = 0;
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMCR, &reg);
    return ((reg & BMCR_RESET) == 0);
}

#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_INFO)
/**
 *  \brief Prints key PHY registers for debugging.
 */
static void Lan8720_printRegs(EnetPhy_Handle hPhy)
{
    uint16_t reg;
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMCR, &reg);
    ENETTRACE_INFO("LAN8720 BMCR = 0x%04x", reg);
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMSR, &reg);
    ENETTRACE_INFO("LAN8720 BMSR = 0x%04x", reg);
}
#endif

/**
 *  \brief Extended helper: Sets the MII mode.
 */
static void Lan8720_setMiiMode(EnetPhy_Handle hPhy, EnetPhy_Mii mii)
{
    uint16_t val = 0U;
    ENETTRACE_DBG("PHY %u: MII mode: %u", hPhy->addr, mii);
    if (mii == ENETPHY_MAC_MII_RMII)
    {
        val = RMIICTL_RMIIEN; 
    }
    Lan8720_rmwExtReg(hPhy, LAN8720_RMIICTL, val, RMIICTL_RMIIEN);
}

/**
 *  \brief Extended helper: Sets the Viterbi idle count threshold.
 */
static void Lan8720_setVtmIdleThresh(EnetPhy_Handle hPhy, uint32_t idleThresh)
{
    ENETTRACE_DBG("PHY %u: Viterbi idle threshold: %u", hPhy->addr, idleThresh);
    Lan8720_rmwExtReg(hPhy, LAN8720_VTMCFG, VTMCFG_IDLETHR_MASK, idleThresh);
}

/**
 *  \brief Extended helper: Configures the DSP FFE equalizer.
 */
static void Lan8720_setDspFFE(EnetPhy_Handle hPhy)
{
    ENETTRACE_DBG("PHY %u: DSP FFE equalizer: %u", hPhy->addr, DSPFFECFG_FFEEQ_SHORTCABLE);
    Lan8720_rmwExtReg(hPhy, LAN8720_DSPFFECFG, DSPFFECFG_FFEEQ_MASK, DSPFFECFG_FFEEQ_SHORTCABLE);
}

/**
 *  \brief Extended helper: Applies workaround for FLD threshold.
 */
static void Lan8720_fixFldStrap(EnetPhy_Handle hPhy)
{
    uint16_t val;
    EnetPhy_readExtReg(hPhy, LAN8720_STRAPSTS2, &val);
    if ((val & STRAPSTS2_FLD_MASK) != 0U)
    {
        ENETTRACE_DBG("PHY %u: Applying FLD threshold workaround", hPhy->addr);
        Lan8720_rmwExtReg(hPhy, LAN8720_FLDTHRCFG, FLDTHRCFG_FLDTHR_MASK, 1U);
    }
}

/**
 *  \brief Extended helper: Configures loopback mode.
 */
static void Lan8720_setLoopbackCfg(EnetPhy_Handle hPhy, bool enable)
{
    uint16_t val = enable ? LOOPCR_CFG_LOOPBACK : LOOPCR_CFG_NORMAL;
    ENETTRACE_DBG("PHY %u: %s loopback", hPhy->addr, enable ? "Enabling" : "Disabling");
    GenericPhy_writeExtReg(hPhy, LAN8720_LOOPCR, val);
}

/**
 *  \brief Extended helper: Enables or disables Auto-MDIX.
 */
static void Lan8720_enableAutoMdix(EnetPhy_Handle hPhy, bool enable)
{
    uint16_t val = enable ? PHYCR_MDICROSSOVER_AUTO : PHYCR_MDICROSSOVER_MDI;
    ENETTRACE_DBG("PHY %u: %s Auto-MDIX", hPhy->addr, enable ? "Enabling" : "Disabling");
    EnetPhy_rmwReg(hPhy, LAN8720_PHYCR, PHYCR_MDICROSSOVER_MASK, val);
    if (enable)
    {
        ENETTRACE_DBG("PHY %u: Enabling Robust Auto-MDIX", hPhy->addr);
        EnetPhy_rmwReg(hPhy, LAN8720_CFG3, CFG3_ROBUSTAUTOMDIX, CFG3_ROBUSTAUTOMDIX);
    }
}

/**
 *  \brief Extended helper: Sets clock shift configuration.
 */
static void Lan8720_setClkShift(EnetPhy_Handle hPhy, bool txShiftEn, bool rxShiftEn)
{
    uint16_t val = (txShiftEn ? RMIICTL_TXCLKDLY : 0U) | (rxShiftEn ? RMIICTL_RXCLKDLY : 0U);
    ENETTRACE_DBG("PHY %u: Clock shift TX: %s, RX: %s", hPhy->addr,
                   txShiftEn ? "enabled" : "disabled",
                   rxShiftEn ? "enabled" : "disabled");
    Lan8720_rmwExtReg(hPhy, LAN8720_RMIICTL, RMIICTL_TXCLKDLY | RMIICTL_RXCLKDLY, val);
}

/**
 *  \brief Extended helper: Sets TX FIFO depth.
 */
static int32_t Lan8720_setTxFifoDepth(EnetPhy_Handle hPhy, uint8_t depth)
{
    uint16_t val = 0U;
    int32_t status = ENETPHY_SOK;
    switch (depth)
    {
        case 3U:
            val = PHYCR_TXFIFODEPTH_3B;
            break;
        case 4U:
            val = PHYCR_TXFIFODEPTH_4B;
            break;
        case 6U:
            val = PHYCR_TXFIFODEPTH_6B;
            break;
        case 8U:
            val = PHYCR_TXFIFODEPTH_8B;
            break;
        default:
            status = ENETPHY_EINVALIDPARAMS;
            break;
    }
    if (status == ENETPHY_SOK)
    {
        ENETTRACE_DBG("PHY %u: Setting TX FIFO depth to %u", hPhy->addr, depth);
        EnetPhy_rmwReg(hPhy, LAN8720_PHYCR, PHYCR_TXFIFODEPTH_MASK, val);
    }
    else
    {
        ENETTRACE_ERR(status, "PHY %u: Invalid TX FIFO depth %u", hPhy->addr, depth);
    }
    return status;
}

/**
 *  \brief Extended helper: Sets clock delay for RGMII.
 */
static int32_t Lan8720_setClkDelay(EnetPhy_Handle hPhy, uint32_t txDelay, uint32_t rxDelay)
{
    uint16_t val;
    uint32_t delay, delayCtrl;
    int32_t status = ENETPHY_SOK;
    if ((txDelay <= RMIIDCTL_DELAY_MAX) && (rxDelay <= RMIIDCTL_DELAY_MAX))
    {
        ENETTRACE_DBG("PHY %u: Setting TX delay %u ps, RX delay %u ps", hPhy->addr, txDelay, rxDelay);
        delay = (txDelay > 0U) ? txDelay : 1U;
        delayCtrl = ENETPHY_DIV_ROUNDUP(delay, RMIIDCTL_DELAY_STEP) - 1U;
        val = (uint16_t)((delayCtrl << RMIIDCTL_TXDLYCTRL_OFFSET) & RMIIDCTL_TXDLYCTRL_MASK);
        delay = (rxDelay > 0U) ? rxDelay : 1U;
        delayCtrl = ENETPHY_DIV_ROUNDUP(delay, RMIIDCTL_DELAY_STEP) - 1U;
        val |= (uint16_t)((delayCtrl << RMIIDCTL_RXDLYCTRL_OFFSET) & RMIIDCTL_RXDLYCTRL_MASK);
        GenericPhy_writeExtReg(hPhy, LAN8720_RMIIDCTL, val);
    }
    else
    {
        status = ENETPHY_EINVALIDPARAMS;
        ENETTRACE_ERR(status, "PHY %u: Invalid delay (TX=%u, RX=%u)", hPhy->addr, txDelay, rxDelay);
    }
    return status;
}

/**
 *  \brief Extended helper: Sets output impedance.
 */
static int32_t Lan8720_setOutputImpedance(EnetPhy_Handle hPhy, uint32_t impedance)
{
    int32_t status = ENETPHY_SOK;
    uint32_t val;
    if ((impedance >= IOMUXCFG_IOIMPEDANCE_MIN) && (impedance <= IOMUXCFG_IOIMPEDANCE_MAX))
    {
        ENETTRACE_DBG("PHY %u: Setting output impedance to %u milli-ohms", hPhy->addr, impedance);
        val = (IOMUXCFG_IOIMPEDANCE_MAX - impedance) * IOMUXCFG_IOIMPEDANCE_MASK;
        val = (val + IOMUXCFG_IOIMPEDANCE_RANGE / 2) / IOMUXCFG_IOIMPEDANCE_RANGE;
        Lan8720_rmwExtReg(hPhy, LAN8720_IOMUXCFG, IOMUXCFG_IOIMPEDANCE_MASK, val);
    }
    else
    {
        status = ENETPHY_EINVALIDPARAMS;
        ENETTRACE_ERR(status, "PHY %u: Out-of-range impedance %u", hPhy->addr, impedance);
    }
    return status;
}

/**
 *  \brief Extended helper: Sets GPIO mux control.
 */
static void Lan8720_setGpioMux(EnetPhy_Handle hPhy, LAN8720_Gpio0Mode gpio0Mode, LAN8720_Gpio1Mode gpio1Mode)
{
    int16_t gpio0 = ((uint16_t)gpio0Mode << GPIOMUXCTRL_GPIO0_OFFSET) & GPIOMUXCTRL_GPIO0_MASK;
    int16_t gpio1 = ((uint16_t)gpio1Mode << GPIOMUXCTRL_GPIO1_OFFSET) & GPIOMUXCTRL_GPIO1_MASK;
    ENETTRACE_DBG("PHY %u: Setting GPIO0 mode %u, GPIO1 mode %u", hPhy->addr, gpio0Mode, gpio1Mode);
    Lan8720_rmwExtReg(hPhy, LAN8720_GPIOMUXCTRL, GPIOMUXCTRL_GPIO0_MASK | GPIOMUXCTRL_GPIO1_MASK, gpio0 | gpio1);
}

/**
 *  \brief Extended helper: Sets LED mode.
 */
static void Lan8720_setLedMode(EnetPhy_Handle hPhy, const LAN8720_LedMode *ledMode)
{
    uint16_t val = (((uint16_t)ledMode[0] << LEDCR1_LED0SEL_OFFSET) & LEDCR1_LED0SEL_MASK) |
                   (((uint16_t)ledMode[1] << LEDCR1_LED1SEL_OFFSET) & LEDCR1_LED1SEL_MASK) |
                   (((uint16_t)ledMode[2] << LEDCR1_LED2SEL_OFFSET) & LEDCR1_LED2SEL_MASK) |
                   (((uint16_t)ledMode[3] << LEDCR1_LED3SEL_OFFSET) & LEDCR1_LED3SEL_MASK);
    ENETTRACE_DBG("PHY %u: Setting LED modes: %u, %u, %u, %u", hPhy->addr, ledMode[0], ledMode[1], ledMode[2], ledMode[3]);
    EnetPhy_writeReg(hPhy, LAN8720_LEDCR1, val);
}

/**
 *  \brief Extended helper: Performs a software restart.
 */
static void Lan8720_restart(EnetPhy_Handle hPhy)
{
    ENETTRACE_DBG("PHY %u: Soft restart", hPhy->addr);
    EnetPhy_rmwReg(hPhy, LAN8720_CTRL, CTRL_SWRESTART, CTRL_SWRESTART);
}

/**
 *  \brief Extended helper: Performs a global software reset.
 */
static void Lan8720_reset(EnetPhy_Handle hPhy)
{
    ENETTRACE_DBG("PHY %u: Global soft-reset", hPhy->addr);
    EnetPhy_rmwReg(hPhy, LAN8720_CTRL, CTRL_SWRESET, CTRL_SWRESET);
}

/**
 *  \brief Extended helper: Checks if reset is complete.
 */
static bool Lan8720_isResetComplete(EnetPhy_Handle hPhy)
{
    int32_t status;
    uint16_t reg;
    bool complete = false;
    status = EnetPhy_readReg(hPhy, LAN8720_CTRL, &reg);
    if (status == ENETPHY_SOK)
    {
        complete = ((reg & CTRL_SWRESET) == 0U);
    }
    ENETTRACE_DBG("PHY %u: Global soft-reset is %scomplete", hPhy->addr, complete ? "" : "not ");
    return complete;
}

/**
 *  \brief Extended helper: Performs a read-modify-write on an extended register.
 */
static void Lan8720_rmwExtReg(EnetPhy_Handle hPhy, uint32_t reg, uint16_t mask, uint16_t val)
{
    uint16_t devad = MMD_CR_DEVADDR;
    uint16_t data;
    int32_t status;
    ENETTRACE_VERBOSE("PHY %u: Writing reg %u mask 0x%04x val 0x%04x", hPhy->addr, reg, mask, val);
    EnetPhy_writeReg(hPhy, PHY_MMD_CR, devad | MMD_CR_ADDR);
    EnetPhy_writeReg(hPhy, PHY_MMD_DR, reg);
    EnetPhy_writeReg(hPhy, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
    status = EnetPhy_readReg(hPhy, PHY_MMD_DR, &data);
    if (status == ENETPHY_SOK)
    {
        data = (data & ~mask) | (val & mask);
        EnetPhy_writeReg(hPhy, PHY_MMD_CR, devad | MMD_CR_DATA_NOPOSTINC);
        EnetPhy_writeReg(hPhy, PHY_MMD_DR, data);
    }
}

#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_INFO)
/**
 *  \brief Extended helper: Prints key PHY registers for debugging.
 */
static void Lan8720_printRegs(EnetPhy_Handle hPhy)
{
    uint16_t reg;
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMCR, &reg);
    ENETTRACE_INFO("LAN8720 BMCR = 0x%04x", reg);
    lan8720_read_reg(ENET_PHY_ADDR, LAN8720_BMSR, &reg);
    ENETTRACE_INFO("LAN8720 BMSR = 0x%04x", reg);
}
#endif

/* ========================================================================== */
/*                 PHY Driver Interface Instance for Upper Layers             */
/* ========================================================================== */

EnetPhy_Drv gEnetPhyDrvLan8720 =
{
    .name               = "Lan8720",
    .isPhyDevSupported  = Lan8720_isPhyDevSupported,
    .isMacModeSupported = Lan8720_isMacModeSupported,
    .config             = Lan8720_config,
    .reset              = Lan8720_reset,
    .isResetComplete    = Lan8720_isResetComplete,
    .readExtReg         = GenericPhy_readExtReg,            //Points to Generic PHY layer read and write external register value
    .writeExtReg        = GenericPhy_writeExtReg,
#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_INFO)
    .printRegs          = Lan8720_printRegs,
#endif
};



