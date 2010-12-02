/**
 * @file carrierTstDrvr.c
 *
 * @brief Test driver for the Carrier driver API
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 16/02/2009
 *
 * This driver provide some IOCTL that can be used to
 * test the carrier driver interface.
 *
 * @version $Id$
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <linux/list.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/* Standard COHT include */
#include "drvr_dbg_prnt.h"

#include "carrierTstDrvr.h"
#include "carrier.h"

/*============================================================================*/
/* Define section                                                             */
/*============================================================================*/
/** Module name */
#define MODULE_NAME "carrierTst"

/** Module major number */
#define CARRIER_TEST_MAJOR 213

/** IRQ name */
#define IRQ_NAME    "Test IRQ"

/*============================================================================*/
/* Structure section                                                          */
/*============================================================================*/
/**
 * \struct slotIdNode_t
 * \brief Node of slotId_t list structure.
 */
typedef struct {
    struct list_head  node;       /**< Used to manage list */
    slotId_t          *pSlotId;   /**< Slot Id */
    unsigned int      irqCounter; /**< IRQ counter */
} slotIdNode_t;

/*============================================================================*/
/* Function definition                                                        */
/*============================================================================*/
static int __init carrierTstDrvrInitModule(void);
static void __exit carrierTstDrvrExitModule(void);
static int carrierTstOpen(struct inode *inode, struct file *filp);
static int carrierTstRelease(struct inode *inode, struct file *filp);
static int carrierTstIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int slotRead(unsigned int cmd, slot_t *slot);
static int slotWrite(unsigned int cmd, slot_t *slot);
static int irqHandler (void *arg);

/*============================================================================*/
/* Module driver description                                                  */
/*============================================================================*/
MODULE_DESCRIPTION("Carrier board interface test driver");
MODULE_AUTHOR("Nicolas Serafini, EIC2 SA");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

module_init(carrierTstDrvrInitModule);
module_exit(carrierTstDrvrExitModule);

/*============================================================================*/
/* Var section                                                                */
/*============================================================================*/
/** DeBuG info Printout level */
dbgipl_t carrierTstIPL = IPL_OPEN | IPL_CLOSE | IPL_ERROR | IPL_INFO | IPL_DBG ;

/** List of slot registered */
static struct list_head slotListRoot;

/*============================================================================*/
/* File operation                                                               */
/*============================================================================*/
/** @brief File operations definition */
struct file_operations carrierTstFops =
  {
    .owner   = THIS_MODULE,
    .ioctl   = carrierTstIoctl,
    .open    = carrierTstOpen,
    .release = carrierTstRelease
  };

/*============================================================================*/
/* Internal driver functions                                                  */
/*============================================================================*/
/**
 * @brief Module driver initialization.
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 */
static int __init carrierTstDrvrInitModule(void)
{
    PRNT_INSTALL(carrierTstIPL, "Carrier test driver loading ( %s )...", MODULE_NAME);
    /* register service character device */
    if(register_chrdev(CARRIER_TEST_MAJOR, MODULE_NAME, &carrierTstFops)) {
        PRNT_ERR(carrierTstIPL,"Can't register character module %s.", MODULE_NAME);
        return -ENODEV;
    }

    /* Init list */
    INIT_LIST_HEAD(&slotListRoot);

    PRNT_INSTALL(carrierTstIPL, "Carrier test driver loaded ( %s ) with major %d.", MODULE_NAME, CARRIER_TEST_MAJOR);

    return 0;
}

/**
 * @brief Module driver exit point
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 */
static void __exit carrierTstDrvrExitModule(void)
{
    slotIdNode_t *pSlotIdNode;
    slotIdNode_t *temp;

    PRNT_UNINST(carrierTstIPL, "Carrier test driver unloading... ( %s )", MODULE_NAME);
    /* unregister device */
    unregister_chrdev(CARRIER_TEST_MAJOR, MODULE_NAME);

    /* Remove all slots */
    if (!(list_empty(&slotListRoot))){
        list_for_each_entry_safe(pSlotIdNode, temp, &slotListRoot, node) {
            /* Unregister the slot */
            ipSlotUnregister(pSlotIdNode->pSlotId);
            /* Delete entry from list */
            list_del(&pSlotIdNode->node);
            kfree(pSlotIdNode);
        }
    }

    PRNT_UNINST(carrierTstIPL, "Carrier test driver unloaded ( %s )", MODULE_NAME);
}

/**
 * __drvrnm - Driver Debug printout mechanism (drvr_dgb_prnt.h) needs this
 *            function to get driver name.
 *
 * @param none
 *
 * @return driver name
 */
char* __drvrnm(void)
{
    return MODULE_NAME;
}

/**
 * @brief Open entry point.
 *
 * @param inode - inode pointer
 * @param filp  - file pointer
 *
 * @return 0   - if success.
 * @return < 0 - if fails.
 */
static int carrierTstOpen(struct inode *inode, struct file *filp)
{
    return 0;
}

/**
 * @brief Close entry point.
 *
 * @param inode - inode pointer
 * @param filp  - file pointer
 *
 * @return 0   - if success.
 * @return < 0 - if fails.
 */
static int carrierTstRelease(struct inode *inode, struct file *filp)
{
    return 0;
}

/**
 * @brief ioctl entry point
 *
 * @param inode - inode struct pointer
 * @param file  - file struct pointer
 * @param cmd   - IOCTL command number
 * @param arg   - user args
 *
 * @return 0  - in case of success.
 * @return <0 - if fails.
 */
static int carrierTstIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *pArg = (void __user *)arg;
    int res;
    slotIdNode_t *pSlotIdNode;
    slot_t slot;

    switch(cmd){
    case CARRIER_SLOT_REGISTER :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_REGISTER");
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Allocate node structure */
        pSlotIdNode = (slotIdNode_t*) kzalloc(sizeof(slotIdNode_t), GFP_KERNEL);
        if (pSlotIdNode == NULL) {
            PRNT_ERR(carrierTstIPL, "Unable to allocate memory for Slot node.");
            return (-ENOMEM);
        }
        /* Try to register the slot */
        pSlotIdNode->pSlotId = ipSlotRegister(slot.boardName, slot.carrierNumber, slot.slotPosition);
        if (pSlotIdNode->pSlotId == NULL){
            PRNT_ERR(carrierTstIPL, "Unable to register slot %d on carrier %d.", slot.slotPosition, slot.carrierNumber);
            return (-EFAULT);
        }
        /* clear counter */
        pSlotIdNode->irqCounter = 0;
        /* Save the slotId into list */
        list_add_tail(&pSlotIdNode->node, &slotListRoot);
        return 0;
        break;
    case CARRIER_SLOT_UNREGISTER :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_UNREGISTER");
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Unregister the slot */
                res = ipSlotUnregister(pSlotIdNode->pSlotId);
                if (res){
                    PRNT_ERR(carrierTstIPL, "Error during slot %d on carrier %d unregister.", slot.slotPosition, slot.carrierNumber);
                    return (res);
                }
                /* Remove slot from list */
                list_del(&pSlotIdNode->node);
                kfree(pSlotIdNode);
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    case CARRIER_SLOT_MAP_SPACE :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_MAP_SPACE");
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                    (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Map the space */
                res = ipSlotMapSpace(pSlotIdNode->pSlotId, slot.memory, slot.space);
                if (res){
                    PRNT_ERR(carrierTstIPL, "Error during mapping space %d from slot %d on carrier %d.", slot.space, slot.slotPosition, slot.carrierNumber);
                    return (res);
                }
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    case CARRIER_SLOT_UNMAP_SPACE :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_UNMAP_SPACE");
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                    (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Unmap the space */
                res = ipSlotUnmapSpace(pSlotIdNode->pSlotId, slot.space);
                if (res){
                    PRNT_ERR(carrierTstIPL, "Error during slot unmapping space %d from slot %d on carrier %d.", slot.space, slot.slotPosition, slot.carrierNumber);
                    return (res);
                }
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    case CARRIER_SLOT_READ_UCHAR :
    case CARRIER_SLOT_READ_USHORT :
    case CARRIER_SLOT_READ_UINT :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_READ_*");
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Read */
        res = slotRead(cmd, &slot);
        if (res){
            PRNT_ERR(carrierTstIPL, "Read error for space %d from slot %d on carrier %d.", slot.space, slot.slotPosition, slot.carrierNumber);
            return (res);
        }
        /* Copy data to user space */
        if (copy_to_user(pArg, &slot, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy to user space.");
            return (-EFAULT);
        }
        return 0;
        break;
    case CARRIER_SLOT_WRITE_UCHAR :
    case CARRIER_SLOT_WRITE_USHORT :
    case CARRIER_SLOT_WRITE_UINT :
        PRNT_IOCTL(carrierTstIPL, "[IOCTL] CARRIER_SLOT_WRITE_*");
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Write */
        res = slotWrite(cmd, &slot);
        if (res){
            PRNT_ERR(carrierTstIPL, "Write error for space %d from slot %d on carrier %d.", slot.space, slot.slotPosition, slot.carrierNumber);
            return (res);
        }
        /* Copy data to user space */
        if (copy_to_user(pArg, &slot, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy to user space.");
            return (-EFAULT);
        }
        return 0;
        break;
    case CARRIER_SLOT_IRQ_INSTALL :
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                    (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Clear the IRQ counter */
                pSlotIdNode->irqCounter = 0;
                /* Install IRQ */
                res = ipSlotRequestIRQ(pSlotIdNode->pSlotId, slot.vector, irqHandler, (void *) &(pSlotIdNode->irqCounter), IRQ_NAME);
                if (res){
                    PRNT_ERR(carrierTstIPL, "Error during IRQ install for slot %d on carrier %d.", slot.slotPosition, slot.carrierNumber);
                    return (res);
                }
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    case CARRIER_SLOT_IRQ_FREE :
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                    (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Free IRQ */
                res = ipSlotFreeIRQ(pSlotIdNode->pSlotId);
                if (res){
                    PRNT_ERR(carrierTstIPL, "Error during IRQ Free for slot %d on carrier %d.", slot.slotPosition, slot.carrierNumber);
                    return (res);
                }
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    case CARRIER_SLOT_GET_IRQ_COUNTER :
        /* Test if there is slot into list */
        if (list_empty(&slotListRoot)) {
            PRNT_ERR(carrierTstIPL, "No slot are registered.");
            return (-EFAULT);
        }
        /* Copy data from user space */
        if (copy_from_user(&slot, pArg, sizeof(slot_t))){
            PRNT_ERR(carrierTstIPL, "Error during data copy from user space.");
            return (-EFAULT);
        }
        /* Find the slot into list */
        list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
            if ((pSlotIdNode->pSlotId->carrierNumber == slot.carrierNumber) &&
                    (pSlotIdNode->pSlotId->slotPosition == slot.slotPosition)) {
                /* Get counter */
                slot.counter = pSlotIdNode->irqCounter;
                /* Copy data to user space */
                if (copy_to_user(pArg, &slot, sizeof(slot_t))){
                    PRNT_ERR(carrierTstIPL, "Error during data copy to user space.");
                    return (-EFAULT);
                }
                return 0;
                break;
            }
        }
        PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot.slotPosition, slot.carrierNumber);
        return -EINVAL;
        break;
    default :
        PRNT_ERR(carrierTstIPL, "[IOCTL] Unkown IOCTL");
        return(-ENOIOCTLCMD);
        break;
    }

    /* Not possible */
    return -EINVAL;
}

/**
 * @brief Read on a slot space
 *
 * @param cmd  - read format
 * @param slot - read informations
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
*/
static int slotRead(unsigned int cmd, slot_t *slot)
{
    int res = -EINVAL;
    slotIdNode_t *pSlotIdNode;

    /* Find the slot into list */
    list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
        if ((pSlotIdNode->pSlotId->carrierNumber == slot->carrierNumber) &&
                (pSlotIdNode->pSlotId->slotPosition == slot->slotPosition)) {
            /* Read */
            if (cmd == CARRIER_SLOT_READ_UCHAR){
                res = ipSlotReadUchar(&slot->ucharValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            else if (cmd == CARRIER_SLOT_READ_USHORT){
                res = ipSlotReadUshort(&slot->ushortValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            else if (cmd == CARRIER_SLOT_READ_UINT){
                res = ipSlotReadUint(&slot->uintValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            return res;
            break;
        }
    }

    PRNT_ERR(carrierTstIPL, "Slot %d on carrier %d not found.", slot->slotPosition, slot->carrierNumber);
    return -EINVAL;
}

/**
 * @brief Write on a slot space
 *
 * @param cmd  - write format
 * @param slot - write informations
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
*/
static int slotWrite(unsigned int cmd, slot_t *slot)
{
    int res = -EINVAL;
    slotIdNode_t *pSlotIdNode;

    /* Find the slot into list */
    list_for_each_entry(pSlotIdNode, &slotListRoot, node) {
        if ((pSlotIdNode->pSlotId->carrierNumber == slot->carrierNumber) &&
                (pSlotIdNode->pSlotId->slotPosition == slot->slotPosition)) {
            /* Read */
            if (cmd == CARRIER_SLOT_WRITE_UCHAR){
                res = ipSlotWriteUchar(slot->ucharValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            else if (cmd == CARRIER_SLOT_WRITE_USHORT){
                res = ipSlotWriteUshort(slot->ushortValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            else if (cmd == CARRIER_SLOT_WRITE_UINT){
                res = ipSlotWriteUint(slot->uintValue, pSlotIdNode->pSlotId, slot->space, slot->offset);
            }
            return res;
            break;
        }
    }

    PRNT_ERR(carrierTstIPL,"Slot %d on carrier %d not found.", slot->slotPosition, slot->carrierNumber);
    return -EINVAL;
}

/**
 * @brief Write on a slot space
 *
 * @param arg - Interrupd user data pointer
*/
static int irqHandler (void *arg)
{
    /* increment counter */
    *((unsigned int *) arg) = *((unsigned int *) arg) + 1;

    return 0;
}
