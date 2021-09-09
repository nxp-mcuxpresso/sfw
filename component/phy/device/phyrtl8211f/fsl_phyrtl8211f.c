/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_phyrtl8211f.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines the PHY RTL8211F vendor defined registers. */
#define PHY_SPECIFIC_STATUS_REG 0x1AU /*!< The PHY specific status register. */
#define PHY_PAGE_SELECT_REG     0x1FU /*!< The PHY page select register. */

/*! @brief Defines the PHY RTL8211F ID number. */
#define PHY_CONTROL_ID1 0x001CU /*!< The PHY ID1 is 0x001C. */

/*! @brief Defines the mask flag in specific status register. */
#define PHY_SSTATUS_LINKSTATUS_MASK 0x04U /*!< The PHY link status mask. */
#define PHY_SSTATUS_LINKSPEED_MASK  0x30U /*!< The PHY link speed mask. */
#define PHY_SSTATUS_LINKDUPLEX_MASK 0x08U /*!< The PHY link duplex mask. */
#define PHY_SSTATUS_LINKSPEED_SHIFT 4     /*!< The link speed shift */

/*! @brief Defines the mask flag in the 1000BASE-T control register. */
#define PHY_1000BASET_FULLDUPLEX_MASK 0x200U /*!< The PHY has the 1000M full duplex ability.*/

/*! @brief Defines the timeout macro. */
#define PHY_READID_TIMEOUT_COUNT 1000U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

const phy_operations_t phyrtl8211f_ops = {.phyInit            = PHY_RTL8211F_Init,
                                          .phyWrite           = PHY_RTL8211F_Write,
                                          .phyRead            = PHY_RTL8211F_Read,
                                          .getLinkStatus      = PHY_RTL8211F_GetLinkStatus,
                                          .getLinkSpeedDuplex = PHY_RTL8211F_GetLinkSpeedDuplex,
                                          .getAutoNegoStatus  = PHY_RTL8211F_GetAutoNegotiationStatus,
                                          .enableLoopback     = PHY_RTL8211F_EnableLoopback};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t PHY_RTL8211F_Init(phy_handle_t *handle, const phy_config_t *config)
{
    uint32_t reg     = 0;
    uint32_t idReg   = 0;
    uint32_t counter = PHY_READID_TIMEOUT_COUNT;
    status_t result  = kStatus_Success;

    /* Init MDIO interface. */
    MDIO_Init(handle->mdioHandle);

    /* Assign phy address. */
    handle->phyAddr = config->phyAddr;

    /* Initialization after PHY starting to work. */
    while ((idReg != PHY_CONTROL_ID1) && (counter != 0U))
    {
        MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_ID1_REG, &idReg);
        counter--;
    }

    if (!counter)
    {
        return kStatus_Fail;
    }

    /* Reset PHY. */
    result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG, PHY_BCTL_RESET_MASK);
    if (result == kStatus_Success)
    {
        /* The RGMII specifies output TXC/RXC and TXD/RXD without any clock skew. Need to add skew on clock line
           to make sure the other side sample right data. This can also be done in PCB traces. */
        result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_PAGE_SELECT_REG, 0xd08);
        if (result == kStatus_Success)
        {
            /* Set Tx Delay. */
            result = MDIO_Read(handle->mdioHandle, handle->phyAddr, 0x11, &reg);
            if (result == kStatus_Success)
            {
                reg |= 0x100U;
                MDIO_Write(handle->mdioHandle, handle->phyAddr, 0x11, reg);
            }
            /* Set Rx Delay. */
            result = MDIO_Read(handle->mdioHandle, handle->phyAddr, 0x15, &reg);
            if (result == kStatus_Success)
            {
                reg |= 0x8U;
                MDIO_Write(handle->mdioHandle, handle->phyAddr, 0x15, reg);
            }
            /* Restore to default page 0 */
            MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_PAGE_SELECT_REG, 0x0);
        }
    }
    /* Set the negotiation. */
    result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_AUTONEG_ADVERTISE_REG,
                        (PHY_100BASETX_FULLDUPLEX_MASK | PHY_100BASETX_HALFDUPLEX_MASK | PHY_10BASETX_FULLDUPLEX_MASK |
                         PHY_10BASETX_HALFDUPLEX_MASK));
    if (result == kStatus_Success)
    {
        result =
            MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_1000BASET_CONTROL_REG, PHY_1000BASET_FULLDUPLEX_MASK);
        if (result == kStatus_Success)
        {
            result = MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG, &reg);
            result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG,
                                (reg | PHY_BCTL_AUTONEG_MASK | PHY_BCTL_RESTART_AUTONEG_MASK));
        }
    }
    return result;
}

status_t PHY_RTL8211F_Write(phy_handle_t *handle, uint32_t phyReg, uint32_t data)
{
    return MDIO_Write(handle->mdioHandle, handle->phyAddr, phyReg, data);
}

status_t PHY_RTL8211F_Read(phy_handle_t *handle, uint32_t phyReg, uint32_t *dataPtr)
{
    return MDIO_Read(handle->mdioHandle, handle->phyAddr, phyReg, dataPtr);
}

status_t PHY_RTL8211F_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result = kStatus_Success;
    uint32_t data;

    /* Read the basic status register. */
    result = MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_SPECIFIC_STATUS_REG, &data);
    if (result == kStatus_Success)
    {
        if (!(PHY_SSTATUS_LINKSTATUS_MASK & data))
        {
            /* link down. */
            *status = false;
        }
        else
        {
            /* link up. */
            *status = true;
        }
    }
    return result;
}

status_t PHY_RTL8211F_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    assert(duplex);

    status_t result = kStatus_Success;
    uint32_t statReg;

    /* Read the control 2 register. */
    result = MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_SPECIFIC_STATUS_REG, &statReg);
    if (result == kStatus_Success)
    {
        if ((statReg & PHY_SSTATUS_LINKDUPLEX_MASK))
        {
            /* Full duplex. */
            *duplex = kPHY_FullDuplex;
        }
        else
        {
            /* Half duplex. */
            *duplex = kPHY_HalfDuplex;
        }

        switch ((statReg & PHY_SSTATUS_LINKSPEED_MASK) >> PHY_SSTATUS_LINKSPEED_SHIFT)
        {
            case kPHY_Speed10M:
                *speed = kPHY_Speed10M;
                break;
            case kPHY_Speed100M:
                *speed = kPHY_Speed100M;
                break;
            case kPHY_Speed1000M:
                *speed = kPHY_Speed1000M;
                break;
            default:
                *speed = kPHY_Speed10M;
                break;
        }
    }
    return result;
}

status_t PHY_RTL8211F_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    status_t result;
    uint32_t bssReg;

    *status = false;

    /* Check auto negotiation complete. */
    result = MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_BASICSTATUS_REG, &bssReg);
    if (result == kStatus_Success)
    {
        if ((bssReg & PHY_BSTATUS_AUTONEGCOMP_MASK) != 0)
        {
            *status = true;
        }
    }
    return result;
}

status_t PHY_RTL8211F_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    status_t result;
    uint32_t data = 0;

    /* Set the loop mode. */
    if (enable)
    {
        if (speed == kPHY_Speed1000M)
        {
            data = PHY_BCTL_SPEED1_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        else if (speed == kPHY_Speed100M)
        {
            data = PHY_BCTL_SPEED0_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        else
        {
            data = PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
        }
        result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG, data);
    }
    else
    {
        /* First read the current status in control register. */
        result = MDIO_Read(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG, &data);
        if (result == kStatus_Success)
        {
            data &= ~PHY_BCTL_LOOP_MASK;
            result = MDIO_Write(handle->mdioHandle, handle->phyAddr, PHY_BASICCONTROL_REG,
                                (data | PHY_BCTL_RESTART_AUTONEG_MASK));
        }
    }
    return result;
}
