/**
 * @file carrierDrvr.c
 *
 * @brief Carrier interface driver
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * This driver is going to make a transparent interface
 * for IndustryPack board.
 * The carrier board supported are :
 * @li TEWS TPCI200 -> Four slot PCI Carrier
 * @li TEWS TVME200 -> Four slot VME Carrier (Same as VIPC626)
 * @li SBS  VIPC626 -> Four slot VME Carrier (Same as TVME200)
 * @li SBS  VIPC616 -> Four slot VME Carrier
 * @li SBS  VIPC610 -> Four slot VME Carrier
 *
 * @version $Id: carrierDrvr.c,v 1.2 2009/12/01 17:33:57 lewis Exp dcobas $
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/* Standard COHT include */
#include <drvr_dbg_prnt.h>
#include <general_ioctl.h>
#include <config_data.h>
#include <libinstkernel.h>
#include <general_drvr.h>


#include "carrier.h"
#include "carrierCommon.h"
#include "carrierTPCI200.h"
#include "carrierTVME200.h"
#include "carrierVIPC61x.h"
#include "TPCI200.h"
#include "TVME200.h"
#include "VIPC61x.h"

/*============================================================================*/
/* Define section                                                             */
/*============================================================================*/
/** Driver description */
#define CARRIER_DRIVER_DESCRIPTION "Carrier interface driver (PCI-VME)"
/** Driver version */
#define CARRIER_DRIVER_VERSION "1.0" /* "$Id: carrierDrvr.c,v 1.2 2009/12/01 17:33:57 lewis Exp dcobas $" */
/** Driver Author */
#define CARRIER_DRIVER_AUTHOR "Nicolas Serafini, EIC2 SA"
/** Driver license */
#define CARRIER_DRIVER_LICENSE "GPL"

/** Name of the driver dir entry into /proc */
#define PROC_DIR_ENTRY_NAME "carrier"
/** Name of the info entry into /proc/carrier */
#define PROC_INFO_ENTRY_NAME "info"

/*============================================================================*/
/* Function definition                                                        */
/*============================================================================*/
static int __init carrierDrvrInitModule(void);
static void __exit carrierDrvrExitModule(void);
static int carrierOpen(struct inode *inode, struct file *filp);
static int carrierRelease(struct inode *inode, struct file *filp);
static int carrierIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int carrierInstall(carrierBoard_t *carrier, carrierAPI_t *carrierAPI, void *params, unsigned int carrierNumber);
static int carrierInstallAll(InsLibDrvrDesc *dd);
static void carrierUninstall(carrierBoard_t *carrier);
static carrierBoard_t *checkSlot(slotId_t *pSlotId);

/*============================================================================*/
/* Var section                                                                */
/*============================================================================*/
/** DeBuG info Printout level */
dbgipl_t carrierIPL = IPL_OPEN | IPL_CLOSE | IPL_ERROR | IPL_INFO | IPL_DBG;

/** Driver name */
static char carrierDriverName[NAME_MAX];

/** Major number of the module */
static int carrierMajorNumber = 0;

/** Number of boards installed */
static unsigned int nbInstalledBoards = 0;

/** Array of all the Installed Carrier board */
static carrierBoard_t* carrierBoards = NULL;

/** Proc dir entry root of the carrier */
static struct proc_dir_entry *carrierRoot;

/** All boards known API */
extern carrierAPI_t *carriersAPI[];

/*============================================================================*/
/* Module driver description                                                  */
/*============================================================================*/
MODULE_DESCRIPTION(CARRIER_DRIVER_DESCRIPTION);
MODULE_AUTHOR(CARRIER_DRIVER_AUTHOR);
MODULE_LICENSE(CARRIER_DRIVER_LICENSE);
MODULE_VERSION(CARRIER_DRIVER_VERSION);

module_param_string(dname, carrierDriverName, sizeof(carrierDriverName), 0);
MODULE_PARM_DESC(dname, "Driver name.");

module_init(carrierDrvrInitModule);
module_exit(carrierDrvrExitModule);

/*============================================================================*/
/* File operation                                                               */
/*============================================================================*/
/** @brief File operations definition */
struct file_operations carrierFops =
{
 .owner   = THIS_MODULE,
 .ioctl   = carrierIoctl,
 .open    = carrierOpen,
 .release = carrierRelease
};

/*============================================================================*/
/* PROC fs  interface                                                         */
/*============================================================================*/
/**
 * @brief Show function of info entry
 *
 * @param page  - Page where data are writted
 * @param start - ??
 * @param off   - Offset from page to write (ignored)
 * @param count - Number of bytes to write (ignored)
 * @param eof   - Flag to specify the end of file
 * @param data  - user data (Carrier board pointer)
 *
 * @return Number of bytes writted.
 */
static int carrierInfoProcShow(char *page, char **start, off_t off, int count,
                                int *eof, void *data)
{
    char *p = page;

    /* Header */
    p += sprintf(p,"\n");
    p += sprintf(p,"==================================================\n");
    p += sprintf(p,"%s\n", CARRIER_DRIVER_DESCRIPTION);
    p += sprintf(p,"==================================================\n");
    p += sprintf(p,"Author  : %s\n", CARRIER_DRIVER_AUTHOR);
    p += sprintf(p,"Version : %s\n", CARRIER_DRIVER_VERSION);
    p += sprintf(p,"License : %s\n\n", CARRIER_DRIVER_LICENSE);
    p += sprintf(p,"Carrier supported :\n");
    p += sprintf(p,"  - TEWS TPCI200 -> Four slot PCI Carrier\n");
    p += sprintf(p,"  - TEWS TVME200 -> Four slot VME Carrier\n");
    p += sprintf(p,"  - SBS  VIPC626 -> Four slot VME Carrier\n");
    p += sprintf(p,"  - SBS  VIPC616 -> Four slot VME Carrier\n");
    p += sprintf(p,"  - SBS  VIPC610 -> Four slot VME Carrier\n");
    p += sprintf(p,"==================================================\n");

    /* End of file */
    *eof = 1;

    return p - page;
}

/**
 * @brief Show function of /proc entry
 *
 * @param page  - Page where data are writted
 * @param start - ??
 * @param off   - Offset from page to write (ignored)
 * @param count - Number of bytes to write (ignored)
 * @param eof   - Flag to specify the end of file
 * @param data  - user data (Carrier board pointer)
 *
 * @return Number of bytes writted.
 */
static int carrierBoardProcShow(char *page, char **start, off_t off, int count,
                                int *eof, void *data)
{
    int i;
    char *p = page;
    carrierBoard_t *pCarrier = (carrierBoard_t *) data;
    carrierSlot_t *pSlot = NULL;

    /* Header */
    p += sprintf(p,"\n");
    p += sprintf(p,"==================================================\n");
    p += sprintf(p,"%s\n", pCarrier->carrierAPI->longName);
    p += sprintf(p,"==================================================\n");

    /* Print specific carrier infos */
    p += pCarrier->carrierAPI->carrierProcShow(pCarrier, p);

    /* Print each slots */
    for (i=0; i<pCarrier->carrierAPI->numberOfSlot; i++){
        /* Get slot */
        pSlot = &(pCarrier->slots[i]);

        /* Print slot infos */
        p += sprintf(p,"--------------------------------------------------\n");
        p += sprintf(p,"Slot %c\n", 'A'+i);

        /* Print slot status */
        if (pSlot->slotId != NULL){
            p += sprintf(p, "    Status       Registered\n");
        }
        else{
            p += sprintf(p, "    Status       Not registered\n");
        }

        /* Print slot name */
        if (pSlot->slotId != NULL){
            p += sprintf(p,"    Name         %s\n", pSlot->slotId->boardName);
        }

        /* Print slot IO physical address */
        p += sprintf(p,"    IO space     Phys.=%p, Size=%.8x\n",
                     pSlot->ioPhys.address, pSlot->ioPhys.size);
        /* Print slot IO virtual address */
        if ((pSlot->slotId != NULL) && (pSlot->slotId->ioSpace.address != NULL)) {
            p += sprintf(p,"                 Virt.=%p, Size=%.8x\n",
                         pSlot->slotId->ioSpace.address, pSlot->slotId->ioSpace.size);
        }

        /* Print slot ID physical address */
        p += sprintf(p,"    ID space     Phys.=%p, Size=%.8x\n",
                     pSlot->idPhys.address, pSlot->idPhys.size);
        /* Print slot ID virtual address */
        if ((pSlot->slotId != NULL)  && (pSlot->slotId->idSpace.address != NULL)) {
            p += sprintf(p,"                 Virt.=%p, Size=%.8x\n",
                         pSlot->slotId->idSpace.address, pSlot->slotId->idSpace.size);
        }

        /* Print slot MEM physical address */
        p += sprintf(p,"    MEM space    Phys.=%p, Size=%.8x\n",
                     pSlot->memPhys.address, pSlot->memPhys.size);
        /* Print slot MEM virtual address */
        if ((pSlot->slotId != NULL)  && (pSlot->slotId->memSpace.address != NULL)) {
            p += sprintf(p,"                 Virt.=%p, Size=%.8x\n",
                         pSlot->slotId->memSpace.address, pSlot->slotId->memSpace.size);
        }

        /* Print slot IRQ */
        if (pSlot->irq == NULL){
            p += sprintf(p,"    IRQ          Not registered\n");
        }
        else {
            p += sprintf(p,"    IRQ          Registered\n");
            p += sprintf(p,"                 Name      %s\n", pSlot->irq->name);
            p += sprintf(p,"                 Vector    %d\n", pSlot->irq->vector);
            p += sprintf(p,"                 Handler   %p\n", pSlot->irq->handler);
            p += sprintf(p,"                 Argument  %p\n", pSlot->irq->arg);
        }
    }

    p += sprintf(p,"==================================================\n");

    /* End of file */
    *eof = 1;

    return p - page;
}

/*============================================================================*/
/* Internal driver functions                                                  */
/*============================================================================*/
/**
 * @brief Module driver initialization.
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 */
static int __init carrierDrvrInitModule(void)
{
    struct proc_dir_entry *procEntry = NULL;

    PRNT_INSTALL(carrierIPL, "Carrier driver loading...");

    /* Registering the char device */
    carrierMajorNumber = register_chrdev(0, carrierDriverName, &carrierFops);
    if(carrierMajorNumber <= 0) {
        PRNT_ERR(carrierIPL, "Can't register character module %s.", carrierDriverName);
        return -ENODEV;
    }

    /* Create /proc/carrier root directory */
    carrierRoot = proc_mkdir(PROC_DIR_ENTRY_NAME, NULL);
    if (!carrierRoot) {
        PRNT_ABS_WARN("Failed to create /proc/%s entry !", PROC_DIR_ENTRY_NAME);
    }
    else {
        /* Creat the info entry */
        procEntry = create_proc_read_entry(PROC_INFO_ENTRY_NAME, S_IFREG | S_IRUGO, carrierRoot, carrierInfoProcShow, NULL);
        if (!procEntry) {
            PRNT_ABS_WARN("Failed to create /proc/%s/%s entry !", PROC_DIR_ENTRY_NAME, PROC_INFO_ENTRY_NAME);
        }
    }

    /* Reset the number of installed boards */
    nbInstalledBoards = 0;

    PRNT_INSTALL(carrierIPL, "Carrier driver loaded.");

    return carrierMajorNumber;
}

/**
 * @brief Module driver exit point
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 */
static void __exit carrierDrvrExitModule(void)
{
    int i;

    PRNT_UNINST(carrierIPL, "Carrier driver unloading...");

    /* unregister driver */
    unregister_chrdev(carrierMajorNumber, carrierDriverName);

    /* Free all specific carrier boards and slots */
    for (i=0;i<nbInstalledBoards;i++) {
        /* Uninstall carrier */
        carrierUninstall(&(carrierBoards[i]));
    }

    /* No boards are installed */
    nbInstalledBoards = 0;

    /* Remove the /proc/carrier/info root directory */
    remove_proc_entry(PROC_INFO_ENTRY_NAME, carrierRoot);

    /* Remove the /proc/carrier root directory */
    remove_proc_entry(PROC_DIR_ENTRY_NAME, NULL);

    /* Free Boards */
    kfree(carrierBoards);

    PRNT_UNINST(carrierIPL, "Carrier driver unloaded.");
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
    return carrierDriverName;
}

/**
 * @brief Install a carrier.
 *
 * @param carrier       - Carrier to install into the carrier array
 * @param carrierAPI    - The specific API of the carrier
 * @param params        - Specific parameters of the carrier (PCI or VME)
 * @param carrierNumber - The logical carrier number position
 *
 * @return  0 - Install success
 * @return <0 - if fails
 */
static int carrierInstall(carrierBoard_t *carrier, carrierAPI_t *carrierAPI, void *params, unsigned int carrierNumber)
{
    int res = 0;
    struct proc_dir_entry *procEntry = NULL;

    /* Return if API is not defined */
    if (carrierAPI == NULL){
        PRNT_ERR(carrierIPL, "Carrier %d not installed, API nod defined !", carrierNumber);
        res = -EINVAL;
        goto out_err;
    }

    PRNT_INSTALL(carrierIPL, "Carrier %s number %d installing...", carrierAPI->shortName, carrierNumber);

    /* Set carrier number */
    carrier->carrierNumber = carrierNumber;
    /* Reset specific */
    carrier->carrierSpecific = NULL;
    /* Set the specific API */
    carrier->carrierAPI = carrierAPI;
    /* Init slots */
    carrier->slots = NULL;
    carrier->slots = (carrierSlot_t*) kzalloc(carrier->carrierAPI->numberOfSlot * sizeof(carrierSlot_t), GFP_KERNEL);
    if (carrier->slots == NULL) {
        PRNT_ABS_ERR("Carrier %d, unable to allocate slots memory !", carrier->carrierNumber);
        res = -ENOMEM;
        goto out_err;
    }

    /* call specific init */
    res = carrier->carrierAPI->carrierRegister(carrier, params);
    if (res){
        PRNT_ABS_ERR("Carrier %d, unable to init board !", carrier->carrierNumber);
        goto out_free;
    }

    /* Init Mutex */
    mutex_init(&(carrier->carrierMutex));

    /* Insert proc entry */
    if (carrierRoot) {
        /* Board Name */
        sprintf(carrier->procEntryName, "%s_%d", carrier->carrierAPI->shortName, carrier->carrierNumber);
        PRNT_INFO(carrierIPL, "Creating /proc/%s/%s entry...", PROC_DIR_ENTRY_NAME, carrier->procEntryName);
        /* Create carrier proc entry */
        procEntry = create_proc_read_entry(carrier->procEntryName, S_IFREG | S_IRUGO, carrierRoot, carrierBoardProcShow, (void *) carrier);
        if (!procEntry) {
            PRNT_ABS_WARN("Failed to create /proc/%s/%s entry !", PROC_DIR_ENTRY_NAME, carrier->procEntryName);
            carrier->procEntryName[0] = '\0';
        }
    }
    else {
        carrier->procEntryName[0] = '\0';
    }

    PRNT_INSTALL(carrierIPL,"Carrier %s number %d installed.", carrierAPI->shortName, carrierNumber);

    return 0;

out_free :
    /* Free slot allocation */
    kfree(carrier->slots);
    carrier->slots = NULL;
out_err :
    return res;
}

/**
 * @brief Install all module passed truogh the list
 *
 * @param dd - Driver description
 *
 * @return 0  - if success.
 * @return <0 - if fails.
 */
static int carrierInstallAll(InsLibDrvrDesc *drvrDescr)
{
    InsLibModlDesc *moduleDescr = drvrDescr->Modules;
    InsLibPciModuleAddress *pciModuleAddr;
    InsLibVmeModuleAddress *vmeModuleAddr;
    InsLibVmeAddressSpace *vmeAddrSpace;
    carrierAPI_t *carrierAPI;
    paramsPCI_t paramsPCI;
    paramsVME_t paramsVME;
    int board, i;
    int res = 0;

    /* Count the number of module that need to be installed */
    nbInstalledBoards = 0;
    do {
        switch (moduleDescr->BusType) {
        case InsLibBusTypeVME :
            /* We are on VME carrier */
            vmeModuleAddr = (InsLibVmeModuleAddress *) moduleDescr->ModuleAddress;

//            do {
//                nbInstalledBoards++;
//                if (vmeModuleAddr->Next == NULL) break;
//            } while(1);

	    nbInstalledBoards++;

            break;
        case InsLibBusTypePCI :
        case InsLibBusTypePMC :
            /* We are on PCI carrier */
            pciModuleAddr = (InsLibPciModuleAddress *) moduleDescr->ModuleAddress;

//            do{
//                nbInstalledBoards++;
//                if (pciModuleAddr->Next == NULL) break;
//                pciModuleAddr = pciModuleAddr->Next;
//            } while(1);

	    nbInstalledBoards++;

            break;
        default :
            break;
        }
        if (moduleDescr->Next == NULL) break;
        /* Go to next module */
        moduleDescr = moduleDescr->Next;
    } while(1);

    /* Now we know the number of carrier boards so we can allocate memory */
    carrierBoards = (carrierBoard_t*) kzalloc(nbInstalledBoards * sizeof(carrierBoard_t), GFP_KERNEL);
    if (carrierBoards == NULL) {
        PRNT_ABS_ERR("Unable to allocate carrier boards structure !");
        res = -ENOMEM;
        goto out_err;
    }

    /* Install all modules */
    board = 0;
    moduleDescr = drvrDescr->Modules;
    do {
        /* find the API of the carrier */
        i = 0;
        while (strcmp(carriersAPI[i]->shortName, moduleDescr->Comment)){
            i++;
            if (carriersAPI[i] == NULL){
                PRNT_ERR(carrierIPL, "Carrier API not found for module number %d (%s) !",
                         moduleDescr->ModuleNumber, moduleDescr->Comment);
                res = -EINVAL;
                goto out_free;
            }
        };
        /* Set the current API */
        carrierAPI = carriersAPI[i];
        switch (moduleDescr->BusType) {
        case InsLibBusTypeVME :
            /* We are on VME carrier */
            vmeModuleAddr = (InsLibVmeModuleAddress *) moduleDescr->ModuleAddress;
            do {
                vmeAddrSpace = vmeModuleAddr->VmeAddressSpace;
                /* reset the vme params to 0 */
                memset(&paramsVME, 0, sizeof(paramsVME_t));
                /* Set the params */
                do {
                    printk("A %s", vmeAddrSpace->Comment);
                    /* Find the space */
                    if (strcmp(vmeAddrSpace->Comment, VME_IOID_SPACE) == 0){
                        /* IoId space */
                        paramsVME.ioidSpace.addressModifier = vmeAddrSpace->AddressModifier;
                        paramsVME.ioidSpace.baseAddress = vmeAddrSpace->BaseAddress;
                        paramsVME.ioidSpace.dataWidth = vmeAddrSpace->DataWidth;
                        paramsVME.ioidSpace.windowSize = vmeAddrSpace->WindowSize;
                    }
                    if (strcmp(vmeAddrSpace->Comment, VME_MEM_SPACE) == 0) {
                        /* Mem space */
                        paramsVME.memSpace.addressModifier = vmeAddrSpace->AddressModifier;
                        paramsVME.memSpace.baseAddress = vmeAddrSpace->BaseAddress;
                        paramsVME.memSpace.dataWidth = vmeAddrSpace->DataWidth;
                        paramsVME.memSpace.windowSize = vmeAddrSpace->WindowSize;
                    }
                    if (vmeAddrSpace->Next == NULL) break;
                    vmeAddrSpace = vmeAddrSpace->Next;
                } while (1);
                /* Install the carrier */
                res = carrierInstall(&carrierBoards[board], carrierAPI, &paramsVME, board);
                if (res) {
                    PRNT_ERR(carrierIPL, "Error during carrier install !");
                    goto out_uninst;
                }
                board++;

//                if (vmeModuleAddr->Next == NULL) break;
//                /* Go to next module address */
//                vmeModuleAddr = vmeModuleAddr->Next;

		break;

            } while(1);
            break;
        case InsLibBusTypePCI :
        case InsLibBusTypePMC :
            /* We are on PCI carrier */
            pciModuleAddr = (InsLibPciModuleAddress *) moduleDescr->ModuleAddress;
            do{
                /* Set the params */
                paramsPCI.busNumber = pciModuleAddr->BusNumber;
                paramsPCI.slotNumber = pciModuleAddr->SlotNumber;
                paramsPCI.vendorId = pciModuleAddr->VendorId;
                paramsPCI.deviceId = pciModuleAddr->DeviceId;
                paramsPCI.subVendorId = pciModuleAddr->SubVendorId;
                paramsPCI.subDeviceId = pciModuleAddr->SubDeviceId;
                /* Install the carrier */
                res = carrierInstall(&carrierBoards[board], carrierAPI, &paramsPCI, board);
                if (res) {
                    PRNT_ERR(carrierIPL, "Error during carrier install !");
                    goto out_uninst;
                }
                board++;

//                if (pciModuleAddr->Next == NULL) break;
//                /* Go to next module address */
//                pciModuleAddr = pciModuleAddr->Next;

		break;

            } while(1);
            break;
        default :
            break;
        }
        if (moduleDescr->Next == NULL) break;
        /* Go to next module */
        moduleDescr = moduleDescr->Next;
    } while(1);

    return 0;

out_uninst :
    /* Uninstall already installed boards */
    for (i=0;i<=board;i++){
        carrierUninstall(&carrierBoards[i]);
    }
out_free :
    nbInstalledBoards = 0;
    kfree(carrierBoards);
out_err :
    return res;
}

/**
 * @brief Uninstall a carrier.
 *
 * @param carrier       - Carrier to uninstall
 *
 */
static void carrierUninstall(carrierBoard_t *carrier)
{
    int i;

    /* Test if carrier is installed */
    if (carrier->carrierSpecific != NULL) {
        /* Remove the proc entry */
        if(strlen(carrier->procEntryName) > 0){
            PRNT_INFO(carrierIPL, "Deleting /proc/%s/%s entry...", PROC_DIR_ENTRY_NAME, carrier->procEntryName);
            remove_proc_entry(carrier->procEntryName, carrierRoot);
        }
        /* Release all slots */
        for (i=0;i<carrier->carrierAPI->numberOfSlot;i++){
            ipSlotUnregister(carrier->slots[i].slotId);
        }
        /* Release board specific */
        carrier->carrierAPI->carrierUnregister(carrier);
    }
    if (carrier->slots != NULL) {
        /* Free Slots */
        kfree(carrier->slots);
    }
}

/**
 * @brief Check if a slot exist and is registered.
 *
 * @param carrierNumber - Carrier on which slot to test
 * @param slotPosition  - Position on the carrier
 *
 * @return non null - pointer to the carrier board where slot is registered
 * @return NULL     - slot not registered or doesn't exist
 */
static carrierBoard_t *checkSlot(slotId_t *pSlotId)
{
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot exist */
    if (pSlotId == NULL){
        PRNT_INFO(carrierIPL, "Slot doesn't exist.");
        return NULL;
    }

    /* Verify board position */
    if (pSlotId->carrierNumber >= nbInstalledBoards) {
        PRNT_INFO(carrierIPL, "The Carrier(nb. %d) of this slot doesn't exist ! Last Carrier is nb.%d", pSlotId->carrierNumber, nbInstalledBoards-1);
        return NULL;
    }

    /* Get the carier */
    pCarrier = &(carrierBoards[pSlotId->carrierNumber]);

    /* Test the carrier */
    if (pCarrier->carrierSpecific == NULL) {
        PRNT_INFO(carrierIPL, "Carrier [%d] not registered!", pSlotId->carrierNumber);
    }

    /* Verify Slot position */
    if (pSlotId->slotPosition >= pCarrier->carrierAPI->numberOfSlot) {
        PRNT_INFO(carrierIPL, "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition,
                 pCarrier->carrierAPI->numberOfSlot-1);
        return NULL;
    }

    /* Verify if slot is Installed */
    if (pCarrier->slots[pSlotId->slotPosition].slotId == NULL) {
        PRNT_INFO(carrierIPL, "Slot [%s %d:%d] is not registered !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition);
        return NULL;
    }

    return (pCarrier);
}

/*============================================================================*/
/* Driver File descriptor operations                                          */
/*============================================================================*/
/**
 * @brief Open entry point.
 *
 * @param inode - inode pointer
 * @param filp  - file pointer
 *
 * @return 0   - if success.
 * @return < 0 - if fails.
 */
static int carrierOpen(struct inode *inode, struct file *filp)
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
static int carrierRelease(struct inode *inode, struct file *filp)
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
static int carrierIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *pArg = (void __user *)arg;
    InsLibDrvrDesc *dd = NULL;
    unsigned long *infoAddr = NULL;
    char infoFilePath[128] = { 0 };
    void (*prev)(void*, const void*, int) = NULL;
    int res = 0;

    switch(cmd){
    case _GIOCTL_CDV_INSTALL :
        PRNT_IOCTL(carrierIPL, "[IOCTL] _GIOCTL_CDV_INSTALL");
        /* Test if modules are already installed */
        if (nbInstalledBoards > 0) {
            PRNT_ERR(carrierIPL, "Modules are already installed !");
            return -EPERM;
        }
        /* get info file path */
        if (strncpy_from_user(infoFilePath, (char*)pArg, 128) < 0) {
            PRNT_ERR(carrierIPL, "Unable to copy info file path from user space !");
            res = -EFAULT;
            goto out_drInstall;
        }
        /* get user-space address to copy info table from */
        infoAddr = (unsigned long *) read_info_file(infoFilePath, NULL);
        if (!(infoAddr)) {
            PRNT_ERR(carrierIPL, "Unable get the user space address where copy info table !");
            res = -EFAULT;
            goto out_drInstall;
        }
        /* set copy method */
        prev = InsLibSetCopyRoutine((void(*)(void*, const void*, int n))
                                    copy_from_user);
        /* get info table from the user space */
        dd = InsLibCloneOneDriver((InsLibDrvrDesc*) *infoAddr);
        if (!(dd)) {
            PRNT_ERR(carrierIPL, "Unable get info table from user space !");
            res = -ENOMEM;
            goto out_drInstall;
        }
        /* Install all modules */
        res = carrierInstallAll(dd);
        if (res) {
            goto out_drInstall;
        }

        /* No error */
        res = carrierMajorNumber;

        out_drInstall :
        if (infoAddr) sysfree((char*)infoAddr, 0);
        if (dd) InsLibFreeDriver(dd);
        break;

    default :
        PRNT_ERR(carrierIPL, "[IOCTL] Unkown IOCTL");
        res = -ENOIOCTLCMD;
        break;
    }

    return res;
}

/*============================================================================*/
/* IP drivers API functions                                                   */
/*============================================================================*/
/**
 * @brief Register a new IP Slot,\n
 *
 * @param boardName     - Name of the slot board
 * @param carrierNumber - Carrier board number (0..N)
 * @param slotPosition  - Position on the carrier(0..N)
 *
 * @return slotId_t structure - if success.
 * @return NULL               - if fails.
 */
slotId_t* ipSlotRegister(char* boardName, unsigned int carrierNumber,
                                unsigned int slotPosition)
{
    slotId_t  *pSlotId = NULL;
    carrierBoard_t *pCarrier = NULL;

    PRNT_INFO(carrierIPL, "Register slot [%d:%d] request...", carrierNumber, slotPosition);

    /* Verify board position */
    if (carrierNumber >= nbInstalledBoards) {
        PRNT_ABS_ERR("Carrier nb.%d doesn't exist! Last carrier is %d.", carrierNumber, nbInstalledBoards-1);
        return NULL;
    }

    /* Get the correct carrier */
    pCarrier = &(carrierBoards[carrierNumber]);

    /* Verify Slot position */
    if (slotPosition >= pCarrier->carrierAPI->numberOfSlot){
        PRNT_ABS_ERR("Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.",
                     carrierBoards[carrierNumber].carrierAPI->shortName,
                     carrierNumber, slotPosition,
                     carrierBoards[carrierNumber].carrierAPI->numberOfSlot-1);
        goto out;
    }

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        goto out;
    }

    /* Verify if slot is already Installed */
    if (pCarrier->slots[slotPosition].slotId != NULL) {
        PRNT_ABS_ERR("Slot [%s %d:%d] already installed !",
                     carrierBoards[carrierNumber].carrierAPI->shortName,
                     carrierNumber, slotPosition);
        goto out_unlock;
    }

    /* Allocate the new slot */
    pSlotId = (slotId_t*) kzalloc(sizeof(slotId_t), GFP_KERNEL);
    if (pSlotId == NULL) {
        PRNT_ABS_ERR("Slot [%s %d:%d] Unable to allocate memory for new slot !",
                     carrierBoards[carrierNumber].carrierAPI->shortName,
                     carrierNumber, slotPosition);
        goto out_unlock;
    }

    /* Test name length */
    if (strlen(boardName) > SLOT_BOARD_NAME_SIZE) {
        PRNT_ABS_WARN("Slot [%s %d:%d] name (%s) too long (%d char > %d char MAX). Will be truncated!\n",
                      carrierBoards[carrierNumber].carrierAPI->shortName,
                      carrierNumber, slotPosition,
                      boardName, strlen(boardName), SLOT_BOARD_NAME_SIZE);
    }

    /* Init the slot structure */
    strncpy(pSlotId->boardName, boardName, SLOT_BOARD_NAME_SIZE-1);
    pSlotId->carrierNumber = carrierNumber;
    pSlotId->slotPosition = slotPosition;
    pSlotId->idSpace.address = NULL;
    pSlotId->idSpace.size = 0;
    pSlotId->ioSpace.address = NULL;
    pSlotId->ioSpace.size = 0;
    pSlotId->memSpace.address = NULL;
    pSlotId->memSpace.size = 0;

    /* Set the new slot into array of the board */
    pCarrier->slots[slotPosition].slotId = pSlotId;

    PRNT_INFO(carrierIPL, "Slot registered.");

out_unlock:
    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));
out:
    return pSlotId;
}
EXPORT_SYMBOL(ipSlotRegister);

/**
 * @brief Unregister a IP Slot.
 *
 * @param pSlotId - Slot to unregister
 *
 * @return 0   - success
 * @return < 0 - on error
 */
int ipSlotUnregister(slotId_t *pSlotId)
{
    carrierBoard_t *pCarrier = NULL;

    if (pSlotId != NULL)
        PRNT_INFO(carrierIPL, "Removing slot [%d:%d] request...", pSlotId->carrierNumber, pSlotId->slotPosition);

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Free the slot IRQ */
    ipSlotFreeIRQ(pSlotId);

    /* Unmap all spaces */
    ipSlotUnmapSpace(pSlotId, SLOT_IO_SPACE);
    ipSlotUnmapSpace(pSlotId, SLOT_ID_SPACE);
    ipSlotUnmapSpace(pSlotId, SLOT_MEM_SPACE);

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        return -ERESTARTSYS;
    }

    /* Remove the slot */
    carrierBoards[pSlotId->carrierNumber].slots[pSlotId->slotPosition].slotId = NULL;

    /* Free the memory */
    kfree(pSlotId);

    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));

    PRNT_INFO(carrierIPL, "Slot removed.");

    return 0;
}
EXPORT_SYMBOL(ipSlotUnregister);

/**
 * @brief Map an IP Slot specific space.\n
 *        If space address is NULL after mapping it's because space size is 0 or an error occured.
 *
 * @param pSlotId    - Slot to map space
 * @param memorySize - Memory Size needed by the IP module.\n
 *                     <i>This parameter is only used to map memory space</i>\n
 *                     <i>For other space size is hardware fixed</i>
 * @param space      - specific space to map
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotMapSpace(slotId_t *pSlotId, unsigned int memorySize, slotSpace_e space)
{
    int res = 0;
    unsigned int sizeToMap = 0;
    void *physAddress = NULL;
    slotAddrSpace_t *virtAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    if (pSlotId != NULL)
        PRNT_INFO(carrierIPL, "Mapping space for slot [%d:%d] request...", pSlotId->carrierNumber, pSlotId->slotPosition);

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        res = -EINVAL;
        goto out;
    }

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        res = -ERESTARTSYS;
        goto out;
    }

    /* Verification */
    switch (space){
    case SLOT_IO_SPACE :
        PRNT_INFO(carrierIPL, "IO space mapping...");
        /* Verify if space is not already mapped */
        if (pSlotId->ioSpace.address != NULL) {
            PRNT_ERR(carrierIPL, "Slot [%s %d:%d] IO space already mapped !",
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, pSlotId->slotPosition);
            res = -EINVAL;
            goto out_unlock;
        }
        /* Get the address space to map */
        virtAddrSpace = &(pSlotId->ioSpace);

        /* Set the physical address space we are working with and the size to map */
        physAddress = pCarrier->slots[pSlotId->slotPosition].ioPhys.address;
        sizeToMap = pCarrier->slots[pSlotId->slotPosition].ioPhys.size;
        break;
    case SLOT_ID_SPACE :
        PRNT_INFO(carrierIPL, "ID space mapping...");
        /* Verify if space is not already mapped */
        if (pSlotId->idSpace.address != NULL) {
            PRNT_ERR(carrierIPL, "Slot [%s %d:%d] ID space already mapped !",
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, pSlotId->slotPosition);
            res = -EINVAL;
            goto out_unlock;
        }
        /* Get the address space to map */
        virtAddrSpace = &(pSlotId->idSpace);

        /* Set the physical address space we are working with and the size to map */
        physAddress = pCarrier->slots[pSlotId->slotPosition].idPhys.address;
        sizeToMap = pCarrier->slots[pSlotId->slotPosition].idPhys.size;
        break;
    case SLOT_MEM_SPACE :
        PRNT_INFO(carrierIPL, "MEM space mapping...");
        /* Verify if space is not already mapped */
        if (pSlotId->memSpace.address != NULL) {
            PRNT_ERR(carrierIPL, "Slot [%s %d:%d] MEM space already mapped !",
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, pSlotId->slotPosition);
            res = -EINVAL;
            goto out_unlock;
        }
        /* Get the address space to map */
        virtAddrSpace = &(pSlotId->memSpace);

        /* Verify if memory requested is not too big */
        if (memorySize > pCarrier->slots[pSlotId->slotPosition].memPhys.size){
            PRNT_ABS_ERR("Slot [%s %d:%d] request is 0x%X memory, only 0x%X available !",
                         pCarrier->carrierAPI->shortName,
                         pSlotId->carrierNumber,
                         pSlotId->slotPosition,
                         memorySize,
                         pCarrier->slots[pSlotId->slotPosition].memPhys.size);
            res = -EINVAL;
            goto out_unlock;
        }

        /* Set the physical address space we are working with and the size to map */
        physAddress = pCarrier->slots[pSlotId->slotPosition].memPhys.address;
        sizeToMap = memorySize;
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pCarrier->carrierNumber, pSlotId->slotPosition, space);
        res = -EINVAL;
        goto out_unlock;
        break;
    }

    /* Remap the memory */
    virtAddrSpace->size = sizeToMap;
    virtAddrSpace->address = ioremap((unsigned long)physAddress, sizeToMap);

    PRNT_INFO(carrierIPL, "Size=0x%x mapped to 0x%p.", sizeToMap, virtAddrSpace->address);

out_unlock:
    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));
out:
    return res;
}
EXPORT_SYMBOL(ipSlotMapSpace);

/**
 * @brief Unmap an IP Slot specific space.
 *
 * @param pSlotId - Slot to map space
 * @param space   - specific space to unmap
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotUnmapSpace(slotId_t *pSlotId, slotSpace_e space)
{
    int res = 0;
    slotAddrSpace_t *virtAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    if (pSlotId != NULL)
        PRNT_INFO(carrierIPL, "Unmapping space for slot [%d:%d] request...", pSlotId->carrierNumber, pSlotId->slotPosition);

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        res = -EINVAL;
        goto out;
    }

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        res = -ERESTARTSYS;
        goto out;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        PRNT_INFO(carrierIPL, "IO space unmapping...");
        /* Test if slot space is mapped */
        if (pSlotId->ioSpace.address == NULL){
            PRNT_INFO(carrierIPL, "Slot [%s %d:%d] IO space not mapped !",
                      pCarrier->carrierAPI->shortName,
                      pSlotId->carrierNumber, pSlotId->slotPosition);
            goto out_unlock;
        }
        /* Get the IO space */
        virtAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        PRNT_INFO(carrierIPL, "ID space unmapping...");
        /* Test if slot space is mapped */
        if (pSlotId->idSpace.address == NULL){
            PRNT_INFO(carrierIPL, "Slot [%s %d:%d] ID space not mapped !",
                      pCarrier->carrierAPI->shortName,
                      pSlotId->carrierNumber, pSlotId->slotPosition);
            goto out_unlock;
        }
        /* Get the ID space */
        virtAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        PRNT_INFO(carrierIPL, "MEM space unmapping...");
        /* Test if slot space is mapped */
        if (pSlotId->memSpace.address == NULL){
            PRNT_INFO(carrierIPL, "Slot [%s %d:%d] MEM space not mapped !",
                      pCarrier->carrierAPI->shortName,
                      pSlotId->carrierNumber, pSlotId->slotPosition);
            goto out_unlock;
        }
        /* Get the MEM space */
        virtAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        res = -EINVAL;
        goto out_unlock;
        break;
    }

    /* Unmap space */
    iounmap(virtAddrSpace->address);

    /* Reset space structure */
    virtAddrSpace->address = NULL;
    virtAddrSpace->size = 0;

    PRNT_INFO(carrierIPL, "Space unmapped.");

out_unlock:
    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));
out:
    return res;
}
EXPORT_SYMBOL(ipSlotUnmapSpace);

/**
 * @brief Install the IRQ for the slot
 *
 * @param pSlotId - Slot to install IRQ
 * @param vector  - Interrupt vector
 * @param handler - Interrupt handler
 * @param arg     - Interrupt handler argument
 * @param name    - Interrupt name
 *
 * <b> WARNING </b> : Setup Interrupt Vector in the IndustryPack device before an IRQ request.\n
 * This is necessary for VMEbus carrier interrupt identification.\n
 * Read the User Manual of your IndustryPack device to know where writing the vector in memory.
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotRequestIRQ(slotId_t *pSlotId, int vector, int (*handler)(void *),
                        void *arg, char *name)
{
    int res = 0;
    slotIrq_t *pSlotIRQ = NULL;
    carrierBoard_t *pCarrier = NULL;

    if (pSlotId != NULL)
        PRNT_INFO(carrierIPL, "IRQ request for slot [%d:%d]...", pSlotId->carrierNumber, pSlotId->slotPosition);

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        res = -EINVAL;
        goto out;
    }

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        res = -ERESTARTSYS;
        goto out;
    }

    /* Verify if IRQ is not already registered */
    if (pCarrier->slots[pSlotId->slotPosition].irq != NULL) {
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] IRQ already registered !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition);
        res = -EINVAL;
        goto out_unlock;
    }

    /* Allocate the IRQ struct */
    pSlotIRQ = (slotIrq_t*) kzalloc(sizeof(slotIrq_t), GFP_KERNEL);
    if (pSlotIRQ == NULL) {
        PRNT_ABS_ERR("Slot [%s %d:%d] unable to allocate memory for IRQ !",
                     pCarrier->carrierAPI->shortName,
                     pSlotId->carrierNumber, pSlotId->slotPosition);
        res = -ENOMEM;
        goto out_unlock;
    }

    /* Set data */
    pSlotIRQ->vector = vector;
    pSlotIRQ->handler = handler;
    pSlotIRQ->arg = arg;
    if (name){
        if (strlen(name) > SLOT_IRQ_NAME_SIZE) {
            PRNT_ABS_WARN("Slot [%s %d:%d] IRQ name (%s) too long (%d char > %d char MAX). Will be truncated!\n",
                          carrierBoards[pSlotId->carrierNumber].carrierAPI->shortName,
                          pSlotId->carrierNumber, pSlotId->slotPosition,
                          name, strlen(name), SLOT_IRQ_NAME_SIZE);
        }
        strncpy(pSlotIRQ->name, name, SLOT_IRQ_NAME_SIZE-1);
    }
    else {
        strcpy(pSlotIRQ->name, "Unkown");
    }

    /* Set the slot IRQ */
    pCarrier->slots[pSlotId->slotPosition].irq = pSlotIRQ;

    /* Run the carrier specific IRQ registering return */
    res = pCarrier->carrierAPI->slotRequestIrq(pCarrier, pSlotId);

    PRNT_INFO(carrierIPL, "IRQ registered.");

out_unlock:
    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));
out:
    return res;
}
EXPORT_SYMBOL(ipSlotRequestIRQ);

/**
 * @brief Free the IRQ of the slot
 *
 * @param pSlotId - Slot to uninstall IRQ
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotFreeIRQ(slotId_t *pSlotId)
{
    int res = 0;
    slotIrq_t *pSlotIRQ = NULL;
    carrierBoard_t *pCarrier = NULL;

    if (pSlotId != NULL)
        PRNT_INFO(carrierIPL, "Free IRQ request for slot [%d:%d]...", pSlotId->carrierNumber, pSlotId->slotPosition);

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        res = -EINVAL;
        goto out;
    }

    /* Lock Carrier */
    if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
        res = -ERESTARTSYS;
        goto out;
    }

    /* Verify if IRQ is registered */
    if (pCarrier->slots[pSlotId->slotPosition].irq == NULL) {
        PRNT_INFO(carrierIPL, "Slot [%s %d:%d] unable to free unregistered IRQ :o)!",
                  pCarrier->carrierAPI->shortName,
                  pSlotId->carrierNumber, pSlotId->slotPosition);
        res = -EINVAL;
        goto out_unlock;
    }

    /* Unregister IRQ carrier specific */
    pCarrier->carrierAPI->slotFreeIrq(pCarrier, pSlotId);

    /* Remove the IRQ */
    pSlotIRQ = pCarrier->slots[pSlotId->slotPosition].irq;
    pCarrier->slots[pSlotId->slotPosition].irq = NULL;

    /* Free the memory */
    kfree(pSlotIRQ);

    PRNT_INFO(carrierIPL, "IRQ unregistered.");

out_unlock:
    /* Release mutex */
    mutex_unlock(&(pCarrier->carrierMutex));
out:
    return res;
}
EXPORT_SYMBOL(ipSlotFreeIRQ);

/**
 * @brief Read an unsigned char value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotReadUchar(unsigned char *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if (offset >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Read */
    *value = pCarrier->carrierAPI->slotReadUchar(pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotReadUchar);

/**
 * @brief Read an unsigned short value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotReadUshort(unsigned short *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if ((offset+2) >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Read */
    *value = pCarrier->carrierAPI->slotReadUshort(pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotReadUshort);

/**
 * @brief Read an unsigned int value on space with offset.
 *
 * @param value   - Readed value
 * @param pSlotId - Slot to read
 * @param space   - space of the slot to read
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotReadUint(unsigned int *value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if ((offset+4) >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Read */
    *value = pCarrier->carrierAPI->slotReadUint(pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotReadUint);

/**
 * @brief Write an unsigned char value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotWriteUchar(unsigned char value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if (offset >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Write */
    pCarrier->carrierAPI->slotWriteUchar(value, pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotWriteUchar);

/**
 * @brief Write an unsigned short value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotWriteUshort(unsigned short value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if ((offset+2) >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Write */
    pCarrier->carrierAPI->slotWriteUshort(value, pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotWriteUshort);

/**
 * @brief Write an unsigned int value on space with offset.
 *
 * @param value   - Value to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotWriteUint(unsigned int value, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    slotAddrSpace_t *pSlotAddrSpace = NULL;
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

    /* Get slot space */
    switch (space){
    case SLOT_IO_SPACE :
        /* Get the IO space */
        pSlotAddrSpace = &(pSlotId->ioSpace);
        break;
    case SLOT_ID_SPACE :
        /* Get the ID space */
        pSlotAddrSpace = &(pSlotId->idSpace);
        break;
    case SLOT_MEM_SPACE :
        /* Get the MEM space */
        pSlotAddrSpace = &(pSlotId->memSpace);
        break;
    default :
        PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
                 pCarrier->carrierAPI->shortName,
                 pSlotId->carrierNumber, pSlotId->slotPosition, space);
        return -EINVAL;
        break;
    }

    /* Test slot space mapped */
    if ((pSlotAddrSpace->size == 0) || (pSlotAddrSpace->address == NULL)) {
        PRNT_ERR(carrierIPL, "Error, slot space not mapped !");
        return -EINVAL;
    }

    /* Test slot space offset */
    if ((offset+4) >= pSlotAddrSpace->size) {
        PRNT_ERR(carrierIPL, "Error, slot space offset error !");
        return -EFAULT;
    }

    /* Write */
    pCarrier->carrierAPI->slotWriteUint(value, pSlotAddrSpace->address, offset);

    return 0;
}
EXPORT_SYMBOL(ipSlotWriteUint);

/**
 * @brief Write the vector into the specified offset of memory space.
 *
 * @param vector  - Vector to write
 * @param pSlotId - Slot to write
 * @param space   - space of the slot to write
 * @param offset  - Offset
 *
 * This function is like ipSlotWriteUchar but work only for VME carrier.\n
 * PCI carrier doesn't need vector value but to have a standard interface use this API
 * to write vector.
 *
 * @return  0 - success
 * @return <0 - on error
 */
int ipSlotWriteVector(unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
    carrierBoard_t *pCarrier = NULL;

    /* Verify slot is installed */
    pCarrier = checkSlot(pSlotId);
    if (pCarrier == NULL){
        return -EINVAL;
    }

	printk(KERN_ERR "SIG: vector %d, offset %lx space %d\n", vector, offset, space);
    /* Do the specific carrier write */
    return (pCarrier->carrierAPI->slotWriteVector(vector, pSlotId, space, offset));
}
EXPORT_SYMBOL(ipSlotWriteVector);
