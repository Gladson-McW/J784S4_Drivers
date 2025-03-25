#ifndef LAN8720_PRIV_H_
#define LAN8720_PRIV_H_

/*!
 * \file lan8720_priv.h
 *
 * \brief Private header file for the LAN8720 Ethernet PHY.
 *
 * This file contains private register definitions and helper macros for the
 * SMSC LAN8720/LAN8720i Ethernet transceiver. The register addresses and bit
 * masks below are derived from the LAN8720 datasheet (Revision 1.0, May 28, 2009).
 * Only those registers required to support basic PHY control, auto-negotiation,
 * and status checking in the driver are defined.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "enetphy_priv.h"  /* Common PHY definitions, if available */

/* --------------------------------------------------------------------------
 * Standard IEEE 802.3 Clause 22 Registers
 * ------------------------------------------------------------------------- */

/* Basic Control Register (BMCR) */
#define LAN8720_BMCR           (0x00U)    /*!< Basic Control Register */
#define BMCR_RESET             (1U << 15) /*!< Reset the PHY */
#define BMCR_LOOPBACK          (1U << 14) /*!< Enable loopback mode */
#define BMCR_SPEED_SEL         (1U << 13) /*!< Speed selection: 0 = 10Mbps, 1 = 100Mbps */
#define BMCR_AUTO_NEG_ENABLE   (1U << 12) /*!< Enable auto-negotiation */
#define BMCR_POWER_DOWN        (1U << 11) /*!< Power down the PHY */
#define BMCR_ISOLATE           (1U << 10) /*!< Isolate the PHY */
#define BMCR_RESTART_AUTO_NEG  (1U << 9)  /*!< Restart auto-negotiation */
#define BMCR_DUPLEX_MODE       (1U << 8)  /*!< Duplex mode: 0 = Half, 1 = Full */
#define BMCR_COLLISION_TEST    (1U << 7)  /*!< Collision Test */

/* Basic Status Register (BMSR) */
#define LAN8720_BMSR           (0x01U)    /*!< Basic Status Register */
#define BMSR_100BASE_TX_FD     (1U << 14) /*!< 100BASE-TX Full Duplex capability */
#define BMSR_100BASE_TX_HD     (1U << 13) /*!< 100BASE-TX Half Duplex capability */
#define BMSR_10BASE_T_FD       (1U << 12) /*!< 10BASE-T Full Duplex capability */
#define BMSR_10BASE_T_HD       (1U << 11) /*!< 10BASE-T Half Duplex capability */
#define BMSR_AUTO_NEG_COMPLETE (1U << 5)  /*!< Auto-negotiation complete */
#define BMSR_REMOTE_FAULT      (1U << 4)  /*!< Remote Fault */
#define BMSR_AUTO_NEG_ABILITY  (1U << 3)  /*!< Auto-negotiation Ability Check */
#define BMSR_LINK_STATUS       (1U << 2)  /*!< Link status: 1 = Link up */
#define BMSR_JABBAR_DETECT     (1U << 1)  /*!< Jabber Detect */
#define BMSR_EXTEND_CAPABLE    (1U)       /*!< Extend Capability */

/* PHY Identifier Registers */
#define LAN8720_PHYID1         (0x02U)    /*!< PHY Identifier 1 */
#define LAN8720_PHYID2         (0x03U)    /*!< PHY Identifier 2 */

/* Auto-Negotiation Advertisement Register */
#define LAN8720_ANAR           (0x04U)    /*!< Auto-Negotiation Advertisement Register */
#define ANAR_SELECTOR_FIELD    (0x001FU)  /*!< Selector field (should be 0x01 for IEEE 802.3) */
#define ANAR_10BASE_T          (1U << 5)  /*!< 10BASE-T capability */
#define ANAR_10BASE_T_FD       (1U << 6)  /*!< 10BASE-T Full Duplex capability */
#define ANAR_100BASE_TX        (1U << 7)  /*!< 100BASE-TX capability */
#define ANAR_100BASE_TX_FD     (1U << 8)  /*!< 100BASE-TX Full Duplex capability */
#define ANAR_PAUSE_OPERATION   ((1U << 10) | (1U << 11)) /*!< Pause Operation */
#define ANAR_REMOTE_FAULT      (1U << 13) /*!< Remote Fault Detection */
#define ANAR_NEXT_PAGE         (1U << 15) /*!< Next Page */

/* Auto-Negotiation Link Partner Ability Register */
#define LAN8720_ANLPAR         (0x05U)      /*!< Auto-Negotiation Link Partner Ability Register */
#define ANLPAR_10BASE_T        (1U << 5)    /*!< 10 Base T */
#define ANLPAR_10BASE_T_FD     (1U << 6)    /*!< 10 Base T Full Duplex */
#define ANLPAR_100BASE_TX      (1U << 7)    /*!< 100BASE-TX capability */
#define ANLPAR_100BASE_TX_FD   (1U << 8)    /*!< 100BASE-TX Full Duplex capability */
#define ANLPAR_100BASE_T4      (1U << 9)    /*!< 100BASE-T4 */
#define ANLPAR_PAUSE           (1U << 10)   /*!< Pause */
#define ANLPAR_REMOTE_FAULT    (1U << 13)   /*!< Remote Fault */
#define ANLPAR_ACKNOWLEDGE     (1U << 14)   /*!< Acknowledgement */
#define ANLPAR_NEXT_PAGE       (1U << 15)   /*!< Next Page */
#define ANLPAR_SELECTOR_FIELD  (0x001FU)    /*!< IEEE Address Selector */

/* Auto-Negotiation Expansion Register */
#define LAN8720_ANER                      (0x06U)      /*!< Auto-Negotiation Expansion Register */
#define ANER_LINK_AUTO_NEG_ABLE            (1U)        /*!< Link Partner Auto-negotiation Able */
#define ANER_PAGE_RECEIVED               (1U << 1)     /*!< Page Received */
#define ANER_NEXT_PAGE_ABLE              (1U << 2)     /*!< Next Page Ability */
#define ANER_LINK_PARTNER_NEXT_PAGE_ABLE (1U << 3)     /*!< Link Partner Next Page Ability */
#define ANER_PARALLEL_DETECTION_FAULT    (1U << 4)     /*!< Parallel Detection Fault */

/* Silicon Revision Register */
#define LAN8720_SILIREV                    (0x10U)
#define SILIREV_SILICON_REV                ((1U << 6) | (1U << 7) | (1U << 8) | (1U << 9))

/* --------------------------------------------------------------------------
 * Vendor-Specific / Extended Registers (Basic Usage)
 * ------------------------------------------------------------------------- */

/* Mode Control/Status Register */
#define LAN8720_MODE_CTRL_STATUS      (0x11U)  /*!< Mode Control/Status Register */
#define MODE_CTRL_STATUS_EDPWRDOWN    (1U << 13) /*!< Enable Energy Detect Power Down */
#define MODE_CTRL_STATUS_LOWSQEN      (1U << 11) /*!< Low Squelch Signal */
#define MODE_CTRL_STATUS_MDPREBP      (1U << 10) /*!< Management Data Preamble Bypass */
#define MODE_CTRL_STATUS_FARLOOPBACK  (1U << 9)  /*!< Force FAR Loopback */
#define MODE_CTRL_STATUS_ALTINT       (1U << 6)  /*!< Alternate Interrupt Mode */
#define MODE_CTRL_STATUS_PHYADBP      (1U << 3)  /*!< Physical Address Bus */ 
#define MODE_CTRL_STATUS_FORCE_GD     (1U << 2)  /*!< Force Good Link Status */
#define MODE_CTRL_STATUS_ENERGYON     (1U << 1)  /*!< Energy On */

/* Special Modes Register */
#define LAN8720_SPECIAL_MODES         (0x12U)  /*!< Special Modes Register */
#define SPECIAL_MODES_MODE            (0x07U)   /*!< Mode selection mask */
/* SPECIAL_MODES_PHYAD not defined */

/* Symbol Error Counter Register */
#define LAN8720_SYMBOL_ERROR_COUNTER  (0x1AU)      /*!< Symbol Error Counter Register */

/* Special Control/Status Register */
#define LAN8720_CONTROL_STATUS        (0x1BU)      /*!< Control Status Register */
#define SCS_AMDIXCTRL                 (1U << 15)   /*!< HP Auto-MDIX Control */
#define SCS_CH_SELECT                 (1U << 13)   /*!< Manual Channel Select */
#define SCS_SQEOFF                    (1U << 11)   /*!< Disable Signal Quality Error */
#define SCS_XPOL                      (1U << 4)    /*!< Polarity state of 10BASE-T */

/* Interrupt Source Register */
#define LAN8720_INTERRUPT_SOURCE      (0x1DU)      /*!< Interrupt Source Register */
#define INTERRUPT_SOURCE_INT7         (1U << 7)    /*!< ENERGYON generated */ 
#define INTERRUPT_SOURCE_INT6         (1U << 6)    /*!< Auto-negotiation completed */ 
#define INTERRUPT_SOURCE_INT5         (1U << 5)    /*!< Remote fault detected */ 
#define INTERRUPT_SOURCE_INT4         (1U << 4)    /*!< Link down */ 
#define INTERRUPT_SOURCE_INT3         (1U << 3)    /*!< Auto-negotiation LP Acknowledge */ 
#define INTERRUPT_SOURCE_INT2         (1U << 2)    /*!< Parallel detection fault */ 
#define INTERRUPT_SOURCE_INT1         (1U << 1)    /*!< Auto-negotiation page received */

/* Interrupt Mask Register */
#define LAN8720_INTERRUPT_MASK        (0x1EU)      /*!< Interrupt Mask Register */

/* Physical Special Control/Status Register */
#define LAN8720_SPECIAL_CTRL_STATUS   (0x1FU)      /*!< Special Control/Status Register */
#define PHY_SCS_AUTODONE              (1U << 12)   /*!< Auto-negotiation done indication */
#define PHY_SCS_ENABLE_4B5B           (1U << 6)    /*!< Enable 4B/5B encoding/decoding */
#define PHY_SCS_SPEED_INDI            ((1U << 4) | (1U << 3))
#define PHY_SCS_SCRAMBLE_DISABLE      (0U)         /*!< Enable data scrambling*/

/* LED Control Register */
#define LAN8720_LED_CONTROL           (0x18U)      /*!< LED Control Register */

/* --------------------------------------------------------------------------
 * Additional Registers for Extended Functionality
 * ------------------------------------------------------------------------- */

/* RMII Control Register for enabling RMII mode and clock delay adjustments */ 
#define LAN8720_RMIICTL          (0x32U)  /*!< RMII Control Register */  // Note: Renamed to RMIICTL to reflect RMII usage in code\n#define RMIICTL_RMIIEN          (1U << 7) /*!< Enable RMII mode */\n#define RMIICTL_TXCLKDLY        (1U << 1) /*!< Enable TX clock delay */\n#define RMIICTL_RXCLKDLY        (1U << 0) /*!< Enable RX clock delay */

/* Viterbi Module Configuration Register (for idle threshold settings) */
#define LAN8720_VTMCFG           (0x53U)  /*!< VTM Configuration Register */
#define VTMCFG_IDLETHR_MASK      (0x000FU) /*!< Idle threshold mask */

/* DSP Feedforward Equalizer Configuration Register */
#define LAN8720_DSPFFECFG        (0x12CU) /*!< DSP FFE Configuration Register */
#define DSPFFECFG_FFEEQ_MASK     (0x03FFU) /*!< DSP FFE equalizer mask */
#define DSPFFECFG_FFEEQ_SHORTCABLE (0x281U) /*!< Value for short cable equalization */

/* Strap Status Register 2 (for FLD configuration) */
#define LAN8720_STRAPSTS2        (0x6FU)  /*!< Strap Status Register 2 */
#define STRAPSTS2_FLD_MASK       (0x0400U) /*!< FLD threshold mask */

/* FLD Threshold Configuration Register */ 
#define LAN8720_FLDTHRCFG        (0x70U)  /*!< FLD Threshold Configuration Register */

/* I/O Mux Configuration Register for output impedance settings */
#define LAN8720_IOMUXCFG         (0x170U) /*!< I/O Mux Configuration Register */
#define IOMUXCFG_IOIMPEDANCE_MASK (0x001FU)
#define IOMUXCFG_IOIMPEDANCE_MIN  (35000U)  /* 35 ohms in milli-ohms */
#define IOMUXCFG_IOIMPEDANCE_MAX  (70000U)  /* 70 ohms in milli-ohms */
#define IOMUXCFG_IOIMPEDANCE_RANGE (IOMUXCFG_IOIMPEDANCE_MAX - IOMUXCFG_IOIMPEDANCE_MIN)

/* GPIO Mux Control Register */
#define LAN8720_GPIOMUXCTRL      (0x172U) /*!< GPIO Mux Control Register */
#define GPIOMUXCTRL_GPIO0_MASK    (0x000FU)
#define GPIOMUXCTRL_GPIO0_OFFSET  (0U)
#define GPIOMUXCTRL_GPIO1_MASK    (0x00F0U)
#define GPIOMUXCTRL_GPIO1_OFFSET  (4U)

/* LED Control Register 1 (For individual LED selection) */
#define LAN8720_LEDCR1           (0x18U)  /*!< LED Control Register 1 */
#define LEDCR1_LED0SEL_OFFSET    (0U)      /*!< LED0 selection starts at bit 0 */
#define LEDCR1_LED0SEL_MASK      (0x000FU) /*!< LED0 selection mask (4 bits) */
#define LEDCR1_LED1SEL_OFFSET    (4U)      /*!< LED1 selection starts at bit 4 */
#define LEDCR1_LED1SEL_MASK      (0x00F0U) /*!< LED1 selection mask (4 bits) */
#define LEDCR1_LED2SEL_OFFSET    (8U)      /*!< LED2 selection starts at bit 8 */
#define LEDCR1_LED2SEL_MASK      (0x0F00U) /*!< LED2 selection mask (4 bits) */
#define LEDCR1_LED3SEL_OFFSET    (12U)     /*!< LED3 selection starts at bit 12 */
#define LEDCR1_LED3SEL_MASK      (0xF000U) /*!< LED3 selection mask (4 bits) */

/* Global Control Register for reset and restart operations */
#define LAN8720_CTRL             (0x1FU)  /*!< Global Control Register */
#define CTRL_SWRESET            (1U << 15) /*!< Software reset */
#define CTRL_SWRESTART          (1U << 14) /*!< Software restart */

/* Configuration Register 3 for robust Auto-MDIX and other settings */
#define LAN8720_CFG3             (0x1EU)  /*!< Configuration Register 3 */
#define CFG3_ROBUSTAUTOMDIX     (1U << 9)  /*!< Robust Auto-MDIX Enable */

/* RGMII (RMII in our case) Delay Control Register for clock delay adjustments */
#define LAN8720_RMIIDCTL        (0x86U)  /*!< RGMII Delay Control Register */
#define RMIIDCTL_TXDLYCTRL_MASK (0x00F0U)
#define RMIIDCTL_TXDLYCTRL_OFFSET (4U)
#define RMIIDCTL_RXDLYCTRL_MASK (0x000FU)
#define RMIIDCTL_RXDLYCTRL_OFFSET (0U)
#define RMIIDCTL_DELAY_MAX      (4000U) /* 4.00 ns */
#define RMIIDCTL_DELAY_STEP     (250U)  /* 0.25 ns */

/* Header Definitions Needed to be Considered - Not Configured */
/* Clock delay definitions (in arbitrary units, e.g., picoseconds) */
#define RMIICTL_TXCLKDLY                (1000U)
#define RMIICTL_RXCLKDLY                (1000U)

/* RGMII/RMII Delay Control Register */
#define LAN8720_RMIIDCTL                (0x86U)

/* TX FIFO depth definitions (using a 16-bit field in PHYCR) */
#define PHYCR_TXFIFODEPTH_MASK          (0xC000U)
#define PHYCR_TXFIFODEPTH_3B            (0x0000U)  /* 3 bytes/nibbles depth */
#define PHYCR_TXFIFODEPTH_4B            (0x4000U)  /* 4 bytes/nibbles depth */
#define PHYCR_TXFIFODEPTH_6B            (0x8000U)  /* 6 bytes/nibbles depth */
#define PHYCR_TXFIFODEPTH_8B            (0xC000U)  /* 8 bytes/nibbles depth */

/* PHY MDIO crossover settings */
#define PHYCR_MDICROSSOVER_AUTO         (0x0040U)
#define PHYCR_MDICROSSOVER_MDI          (0x0020U)
#define PHYCR_MDICROSSOVER_MASK         (0x0060U)

/* PHY Control Register */
#define LAN8720_PHYCR                   (0x1BU)

/* Loopback configuration definitions */
#define LAN8720_LOOPCR                  (0xFEU)
#define LOOPCR_CFG_LOOPBACK             (0xE720U)
#define LOOPCR_CFG_NORMAL               (0xE721U)

/* FLD Threshold Configuration */
#define FLDTHRCFG_FLDTHR_MASK           (0x0007U)

/* RGMII (RMII) mode enabling macro */
#define RMIICTL_RMIIEN                   (1U << 7)

/* Example IOCTL command for setting MAC port state */
#define ENET_IOCTL_SET_MAC_PORT_STATE    (0x1000U)

#define ENET_DMA_DIR_TX                  (0x1000U)

#ifdef __cplusplus\n
}
#endif
#endif /* LAN8720_PRIV_H_ */
