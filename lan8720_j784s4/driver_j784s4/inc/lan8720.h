/*!
 * \file  LAN8720.h
 *
 * \brief This file contains type definitions and helper macros for the
 *        LAN8720 Ethernet PHY.
 *
 * The register definitions are assumed to be provided in a separate file.
 *
 * \ingroup DRV_ENETPHY
 * \defgroup ENETPHY_LAN8720 TI LAN8720 PHY
 *
 * TI LAN8720 RMII Ethernet PHY.
 *
 * @{
 */


#ifndef LAN8720_H_
#define LAN8720_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief Number of LEDs available in the LAN8720 PHY. */
#define LAN8720_LED_NUM       (4U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief GPIO_0 mux modes.
 */
typedef enum LAN8720_Gpio0Mode_e
{
    LAN8720_GPIO0_RXERR     = 0x0U,  /*!< RX_ER */
    LAN8720_GPIO0_1588TXSFD = 0x1U,  /*!< 1588 TX Start of Frame Delimiter */
    LAN8720_GPIO0_1588RXSFD = 0x2U,  /*!< 1588 RX Start of Frame Delimiter */
    LAN8720_GPIO0_WOL       = 0x3U,  /*!< Wake-On-LAN */
    LAN8720_GPIO0_ENERGYDET = 0x4U,  /*!< Energy Detection (for 1000Base-T/100Base-TX) */
    LAN8720_GPIO0_LED3      = 0x6U,  /*!< LED indicator (LED3) */
    LAN8720_GPIO0_PRBSERR   = 0x7U,  /*!< PRBS Errors / Loss of Sync */
    LAN8720_GPIO0_CONSTANT0 = 0x8U,  /*!< Constant logic 0 */
    LAN8720_GPIO0_CONSTANT1 = 0x9U   /*!< Constant logic 1 */
} LAN8720_Gpio0Mode;

/*!
 * \brief GPIO_1 mux modes.
 */
typedef enum LAN8720_Gpio1Mode_e
{
    LAN8720_GPIO1_COL       = 0x0U,  /*!< Collision detection */
    LAN8720_GPIO1_1588TXSFD = 0x1U,  /*!< 1588 TX Start of Frame Delimiter */
    LAN8720_GPIO1_1588RXSFD = 0x2U,  /*!< 1588 RX Start of Frame Delimiter */
    LAN8720_GPIO1_WOL       = 0x3U,  /*!< Wake-On-LAN */
    LAN8720_GPIO1_ENERGYDET = 0x4U,  /*!< Energy Detection (for 1000Base-T/100Base-TX) */
    LAN8720_GPIO1_LED3      = 0x6U,  /*!< LED indicator (LED3) */
    LAN8720_GPIO1_PRBSERR   = 0x7U,  /*!< PRBS Errors / Loss of Sync */
    LAN8720_GPIO1_CONSTANT0 = 0x8U,  /*!< Constant logic 0 */
    LAN8720_GPIO1_CONSTANT1 = 0x9U   /*!< Constant logic 1 */
} LAN8720_Gpio1Mode;

/*!
 * \brief LED modes (sources) for the LAN8720 PHY.
 */
typedef enum LAN8720_LedMode_e
{
    LAN8720_LED_LINKED           = 0x0U,  /*!< Link established */
    LAN8720_LED_RXTXACT          = 0x1U,  /*!< RX or TX activity */
    LAN8720_LED_TXACT            = 0x2U,  /*!< Transmit activity */
    LAN8720_LED_RXACT            = 0x3U,  /*!< Receive activity */
    LAN8720_LED_COLLDET          = 0x4U,  /*!< Collision detected */
    LAN8720_LED_LINKED_1000BT    = 0x5U,  /*!< 1000Base-T link established */
    LAN8720_LED_LINKED_100BTX    = 0x6U,  /*!< 100Base-TX link established */
    LAN8720_LED_LINKED_10BT      = 0x7U,  /*!< 10Base-T link established */
    LAN8720_LED_LINKED_10100BT   = 0x8U,  /*!< 10/100Base-T link established */
    LAN8720_LED_LINKED_1001000BT = 0x9U,  /*!< 100/1000Base-T link established */
    LAN8720_LED_FULLDUPLEX       = 0xAU,  /*!< Full duplex operation */
    LAN8720_LED_LINKED_BLINKACT  = 0xBU,  /*!< Blink for TX or RX activity */
    LAN8720_LED_RXTXERR          = 0xDU,  /*!< RX/TX error indication */
    LAN8720_LED_RXERR            = 0xEU   /*!< RX error indication */
} LAN8720_LedMode;

/*!
 * \brief LAN8720 PHY configuration parameters.
 *
 * This structure contains configuration parameters specific to the LAN8720 PHY.
 */
typedef struct LAN8720_Cfg_s
{
    /*! Enable TX clock shift */
    bool txClkShiftEn;
 
    /*! Enable RX clock shift */
    bool rxClkShiftEn;
 
    /*! TX delay value in picoseconds */
    uint32_t txDelayInPs;
 
    /*! RX delay value in picoseconds */
    uint32_t rxDelayInPs;
 
    /*! TX FIFO depth (in number of bytes or nibbles, per design requirements) */
    uint8_t txFifoDepth;
 
    /*! Idle count threshold for the Viterbi detector */
    uint32_t idleCntThresh;
 
    /*! Output impedance in milli-ohms (typically from 35000 to 70000 milli-ohms) */
    uint32_t impedanceInMilliOhms;
 
    /*! Mux mode for GPIO_0 */
    LAN8720_Gpio0Mode gpio0Mode;
 
    /*! Mux mode for GPIO_1 */
    LAN8720_Gpio1Mode gpio1Mode;
 
    /*! LED mode configuration for each available LED */
    LAN8720_LedMode ledMode[LAN8720_LED_NUM];
} LAN8720_Cfg;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize LAN8720 PHY configuration parameters.
 *
 * This function initializes the LAN8720-specific configuration parameters in the
 * provided configuration structure.
 *
 * \param cfg   Pointer to a LAN8720_Cfg structure.
 */
void Lan8720_initCfg(LAN8720_Cfg *cfg);

#ifdef __cplusplus
}
#endif

#endif /* LAN8720_H_ */

/*! @} */
