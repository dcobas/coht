/**
 * @file carrierTVME200.c
 *
 * @brief TVME200 Carrier interface header file
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * Specific carrier interface for the TEWS TVME200 and SBS VIPC626.
 * <b>Warning :</b>\n
 * The librairie driver doesn't use the "IP IRQ and Control Register" of the Carrier.\n
 * The S3 jumper must be set between 0x8 and 0xC.
 *
 * @version $Id: carrierTVME200.c,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <asm/io.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>

#include "vmebus.h"
#include "drvr_dbg_prnt.h"

#include "carrier.h"
#include "carrierCommon.h"
#include "carrierTVME200.h"
#include "TVME200.h"

/*============================================================================*/
/* Define section                                                             */
/*============================================================================*/
/** End of an array list */
#define ENDLIST -1

/** Driver description */
#define CARRIER_DRIVER_DESCRIPTION "Carrier interface driver (PCI-VME)"
/** Driver version */
#define CARRIER_DRIVER_VERSION "1.0" /* "$Id: carrierDrvr.c,v 1.2 2009/12/01 17:33:57 lewis Exp dcobas $" */
/** Driver Author */
#define CARRIER_DRIVER_AUTHOR "Nicolas Serafini, EIC2 SA"
/** Driver license */
#define CARRIER_DRIVER_LICENSE "GPL"

/** Max number of supported carriers */
#define MAX_CARRIER 16

static int lun[MAX_CARRIER];
static int num_lun;
module_param_array(lun, int, &num_lun, S_IRUGO);

/* IO/ID space */
static int mod_address_ioid[MAX_CARRIER];
static int num_address_ioid;
module_param_array(mod_address_ioid, int, &num_address_ioid, S_IRUGO);		/* Modifier address */
static int base_address_ioid[MAX_CARRIER];
static int num_base_address_ioid;
module_param_array(base_address_ioid, int, &num_base_address_ioid, S_IRUGO);	/* Base address */
static int data_width_ioid[MAX_CARRIER];
static int num_data_width_ioid;
module_param_array(data_width_ioid, int, &num_data_width_ioid, S_IRUGO);	/* Data width */
static int wind_size_ioid[MAX_CARRIER];
static int num_wind_size_ioid;
module_param_array(wind_size_ioid, int, &num_wind_size_ioid, S_IRUGO);		/* Memory size */

/* Memory space */
static int mod_address_mem[MAX_CARRIER];
static int num_address_mem;
module_param_array(mod_address_mem, int, &num_address_mem, S_IRUGO);		/* Modifier address */
static int base_address_mem[MAX_CARRIER];
static int num_base_address_mem;
module_param_array(base_address_mem, int, &num_base_address_mem, S_IRUGO);	/* Base address */
static int data_width_mem[MAX_CARRIER];
static int num_data_width_mem;
module_param_array(data_width_mem, int, &num_data_width_mem, S_IRUGO);	/* Data width */
static int wind_size_mem[MAX_CARRIER];
static int num_wind_size_mem;
module_param_array(wind_size_mem, int, &num_wind_size_mem, S_IRUGO);		/* Memory size */

/*============================================================================*/
/* Structure section                                                          */
/*============================================================================*/
/**
 * \struct carrierInfos_t
 * \brief TVME200 and VIPC626 carrier specific info.
 *
 * carrierInfos_t informations specific of the TVME200 and VIPC626 carrier.
 *
 */
typedef struct {
	struct vme_mapping ioidMapping; /**< Used to intefaces with the vmebridge driver for the IoIdInt space*/
	struct vme_mapping memMapping;  /**< Used to intefaces with the vmebridge driver for the Mem space*/
} carrierInfos_t;

/*============================================================================*/
/* Function definition                                                        */
/*============================================================================*/
static int carrierRegister(carrierBoard_t *pCarrier, void *devSpecific);
static void carrierUnregister(carrierBoard_t *pCarrier);
static int slotRequestIrq(carrierBoard_t *pCarrier, slotId_t *pSlotId);
static void slotFreeIrq(carrierBoard_t *pCarrier, slotId_t *pSlotId);
static int setIoIdParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *ioidSpace, struct vme_mapping *ioidMapping);
static int setMemParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *memSpace, struct vme_mapping *memMapping);
static unsigned char slotReadUchar(void *address, unsigned long offset);
static unsigned short slotReadUshort(void *address, unsigned long offset);
static unsigned int slotReadUint(void *address, unsigned long offset);
static void slotWriteUchar(unsigned char value, void *address, unsigned long offset);
static void slotWriteUshort(unsigned short value, void *address, unsigned long offset);
static void slotWriteUint(unsigned int value, void *address, unsigned long offset);
static int slotWriteVector(unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

static int carrierOpen(struct inode *inode, struct file *filp);
static int carrierRelease(struct inode *inode, struct file *filp);
static int carrierInstallAll(void);

/*============================================================================*/
/* File operation                                                               */
/*============================================================================*/
/** @brief File operations definition */
struct file_operations carrierFops =
{
	.owner   = THIS_MODULE,
	.open    = carrierOpen,
	.release = carrierRelease
};

MODULE_DESCRIPTION(CARRIER_DRIVER_DESCRIPTION);
MODULE_AUTHOR(CARRIER_DRIVER_AUTHOR);
MODULE_LICENSE(CARRIER_DRIVER_LICENSE);
MODULE_VERSION(CARRIER_DRIVER_VERSION);


/*============================================================================*/
/* Var section                                                                */
/*============================================================================*/
/** DeBuG info Printout level */
dbgipl_t carrierIPL = IPL_OPEN | IPL_CLOSE | IPL_ERROR | IPL_INFO | IPL_DBG;

/** TVME200 supported Data width */
static int dataWidth[] = { VME_D8, VME_D16, ENDLIST };

/** TVME200 supported A16 Address Modifiers */
static int a16Modifiers[] = { 0x29, 0x2D, ENDLIST };

/** TVME200 supported A24 Address Modifiers */
static int a24Modifiers[] = { 0x39, 0x3A, 0x3D, 0x3E, ENDLIST };

/** TVME200 supported A32 Address Modifiers */
static int a32Modifiers[] = { 0x09, 0x0A, 0x0D, 0x0E, ENDLIST };

/** TVME200 carrier total memory size */
static int carrierMemSize[] = { TVME200_MEM_128k, TVME200_MEM_256k,
	TVME200_MEM_512k, TVME200_MEM_1M,
	TVME200_MEM_2M, TVME200_MEM_4M,
	TVME200_MEM_8M, TVME200_MEM_32M, ENDLIST };

/** TVME200 carrier slot gap */
static int carrierSlotGap[] = { TVME200_MEM_128k_SLOT_GAP, TVME200_MEM_256k_SLOT_GAP,
	TVME200_MEM_512k_SLOT_GAP, TVME200_MEM_1M_SLOT_GAP,
	TVME200_MEM_2M_SLOT_GAP, TVME200_MEM_4M_SLOT_GAP,
	TVME200_MEM_8M_SLOT_GAP, TVME200_MEM_32M_SLOT_GAP, ENDLIST };

/** TVME200 carrier total slot size */
static int carrierSlotSize[] = { TVME200_MEM_128k_SLOT_SIZE, TVME200_MEM_256k_SLOT_SIZE,
	TVME200_MEM_512k_SLOT_SIZE, TVME200_MEM_1M_SLOT_SIZE,
	TVME200_MEM_2M_SLOT_SIZE, TVME200_MEM_4M_SLOT_SIZE,
	TVME200_MEM_8M_SLOT_SIZE, TVME200_MEM_32M_SLOT_SIZE, ENDLIST };

/** Driver name */
static char carrierDriverName[NAME_MAX];

/** Major number of the module */
static int carrierMajorNumber = 0;

/** Number of boards installed */
static unsigned int nbInstalledBoards = 0;

/** Array of all the Installed Carrier board */
static carrierBoard_t* carrierBoards = NULL;

/*============================================================================*/
/* Internal lib. functions                                                    */
/*============================================================================*/
/**
 * @brief Initialisation of the TVME200 board.
 *
 * @param pCarrier    - Base structure of the carrier.
 * @param devSpecific - Specific device informations.
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 */
static int carrierRegister(carrierBoard_t *pCarrier, void *devSpecific)
{
	int  i;
	int res = 0;
	paramsVME_t *vmeParams = (paramsVME_t *) devSpecific;
	carrierInfos_t *vmeCarrier = NULL;
	int carrierMemIndex;

	/* Check if carrier is already registered */
	if (pCarrier->carrierSpecific != NULL) {
		PRNT_ERR(carrierIPL, "Carrier number %d already registered !", pCarrier->carrierNumber);
		res = -EINVAL;
		goto out_err;
	}

	/* Create the Carrier specific infos */
	vmeCarrier = (carrierInfos_t*) kzalloc(sizeof(carrierInfos_t), GFP_KERNEL);
	if (vmeCarrier == NULL) {
		PRNT_ABS_ERR("Carrier [%s %d] unable to allocate memory for new carrier !",
				TVME200_SHORTNAME,
				pCarrier->carrierNumber);
		res = -ENOMEM;
		goto out_err;
	}

	/* Set the IoId space parameters to the mapping structure */
	res = setIoIdParams(pCarrier, &(vmeParams->ioidSpace), &(vmeCarrier->ioidMapping));
	if (res){
		goto out_free;
	}

	/* Set the mem space parameters to the mapping structure */
	carrierMemIndex = setMemParams(pCarrier, &(vmeParams->memSpace), &(vmeCarrier->memMapping));
	if (carrierMemIndex < 0){
		res = -EINVAL;
		goto out_free;
	}

	/* Find mapping for IoIdInt space */
	res = vme_find_mapping(&(vmeCarrier->ioidMapping), 1);
	if (res){
		PRNT_ERR(carrierIPL, "Carrier [%s %d] unable to find window mapping for IO space !",
				TVME200_SHORTNAME,
				pCarrier->carrierNumber);
		goto out_free;
	}

	/* Find mapping for Mem space */
	res = vme_find_mapping(&(vmeCarrier->memMapping), 1);
	if (res){
		PRNT_ERR(carrierIPL, "Carrier [%s %d] unable to find window mapping for MEM space !",
				TVME200_SHORTNAME,
				pCarrier->carrierNumber);
		goto out_releaseIoId;
	}

	/* Set all slot physical address space */
	for (i=0; i<TVME200_NB_SLOT; i++) {
		pCarrier->slots[i].ioPhys.address = (void*)(vmeCarrier->ioidMapping.pci_addrl + TVME200_IO_SPACE_OFF + TVME200_IO_SPACE_GAP*i);
		pCarrier->slots[i].ioPhys.size = TVME200_IO_SPACE_SIZE;
		pCarrier->slots[i].idPhys.address = (void*)(vmeCarrier->ioidMapping.pci_addrl + TVME200_ID_SPACE_OFF + TVME200_ID_SPACE_GAP*i);
		pCarrier->slots[i].idPhys.size = TVME200_ID_SPACE_SIZE;
		pCarrier->slots[i].memPhys.address = (void*)(vmeCarrier->memMapping.pci_addrl + carrierSlotGap[carrierMemIndex]*i);
		pCarrier->slots[i].memPhys.size = carrierSlotSize[carrierMemIndex];
	}

	/* Set the vmeCarrier into the specific place */
	pCarrier->carrierSpecific = (void *) vmeCarrier;

	return 0;

out_releaseIoId :
	/* Release IO ID mapping */
	vme_release_mapping(&(vmeCarrier->ioidMapping), 1);
out_free :
	/* Free */
	kfree(vmeCarrier);
out_err :
	return res;
}

/**
 * @brief Unregister a TVME200 board.
 *
 * @param pCarrier    - Base structure of the board.
 *
 */
static void carrierUnregister(carrierBoard_t *pCarrier)
{
	int i;
	carrierInfos_t *vmeCarrier = (carrierInfos_t *) pCarrier->carrierSpecific;

	/* Reset the carrier specific pointer */
	pCarrier->carrierSpecific = NULL;

	/* Unmap space */
	vme_release_mapping(&(vmeCarrier->ioidMapping), 1);
	vme_release_mapping(&(vmeCarrier->memMapping), 1);

	/* Free the carrier infos structure */
	kfree(vmeCarrier);

	/* Reset all slot physical address space */
	for (i=0; i<TVME200_NB_SLOT; i++) {
		pCarrier->slots[i].ioPhys.address = NULL;
		pCarrier->slots[i].ioPhys.size = 0;
		pCarrier->slots[i].idPhys.address = NULL;
		pCarrier->slots[i].idPhys.size = 0;
		pCarrier->slots[i].memPhys.address = NULL;
		pCarrier->slots[i].memPhys.size = 0;
	}
}

/******************************************************************************
 * 			XXX SIG:
 ******************************************************************************
 */

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
	if (pSlotId->slotPosition >= TVME200_NB_SLOT) {
		PRNT_INFO(carrierIPL, "Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.",
				TVME200_SHORTNAME,
				pSlotId->carrierNumber, pSlotId->slotPosition,
				TVME200_NB_SLOT-1);
		return NULL;
	}

	/* Verify if slot is Installed */
	if (pCarrier->slots[pSlotId->slotPosition].slotId == NULL) {
		PRNT_INFO(carrierIPL, "Slot [%s %d:%d] is not registered !",
				TVME200_SHORTNAME,
				pSlotId->carrierNumber, pSlotId->slotPosition);
		return NULL;
	}

	return (pCarrier);
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
	if (slotPosition >= TVME200_NB_SLOT){
		PRNT_ABS_ERR("Slot [%s %d:%d] doesn't exist! Last carrier slot is %d.",
				TVME200_SHORTNAME,
				carrierNumber, slotPosition,
				TVME200_NB_SLOT-1);
		goto out;
	}

	/* Lock Carrier */
	if (mutex_lock_interruptible(&(pCarrier->carrierMutex))){
		goto out;
	}

	/* Verify if slot is already Installed */
	if (pCarrier->slots[slotPosition].slotId != NULL) {
		PRNT_ABS_ERR("Slot [%s %d:%d] already installed !",
				TVME200_SHORTNAME,
				carrierNumber, slotPosition);
		goto out_unlock;
	}

	/* Allocate the new slot */
	pSlotId = (slotId_t*) kzalloc(sizeof(slotId_t), GFP_KERNEL);
	if (pSlotId == NULL) {
		PRNT_ABS_ERR("Slot [%s %d:%d] Unable to allocate memory for new slot !",
				TVME200_SHORTNAME,
				carrierNumber, slotPosition);
		goto out_unlock;
	}

	/* Test name length */
	if (strlen(boardName) > SLOT_BOARD_NAME_SIZE) {
		PRNT_ABS_WARN("Slot [%s %d:%d] name (%s) too long (%d char > %d char MAX). Will be truncated!\n",
				TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
						pCarrier->carrierNumber, pSlotId->slotPosition);
				res = -EINVAL;
				goto out_unlock;
			}
			/* Get the address space to map */
			virtAddrSpace = &(pSlotId->memSpace);

			/* Verify if memory requested is not too big */
			if (memorySize > pCarrier->slots[pSlotId->slotPosition].memPhys.size){
				PRNT_ABS_ERR("Slot [%s %d:%d] request is 0x%X memory, only 0x%X available !",
						TVME200_SHORTNAME,
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
					TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
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
						TVME200_SHORTNAME,
						pSlotId->carrierNumber, pSlotId->slotPosition);
				goto out_unlock;
			}
			/* Get the MEM space */
			virtAddrSpace = &(pSlotId->memSpace);
			break;
		default :
			PRNT_ERR(carrierIPL, "Slot [%s %d:%d] space number %d doesn't exist !",
					TVME200_SHORTNAME,
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
				TVME200_SHORTNAME,
				pSlotId->carrierNumber, pSlotId->slotPosition);
		res = -EINVAL;
		goto out_unlock;
	}

	/* Allocate the IRQ struct */
	pSlotIRQ = (slotIrq_t*) kzalloc(sizeof(slotIrq_t), GFP_KERNEL);
	if (pSlotIRQ == NULL) {
		PRNT_ABS_ERR("Slot [%s %d:%d] unable to allocate memory for IRQ !",
				TVME200_SHORTNAME,
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
					TVME200_SHORTNAME,
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
	res = slotRequestIrq(pCarrier, pSlotId);

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
				TVME200_SHORTNAME,
				pSlotId->carrierNumber, pSlotId->slotPosition);
		res = -EINVAL;
		goto out_unlock;
	}

	/* Unregister IRQ carrier specific */
	slotFreeIrq(pCarrier, pSlotId);

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
					TVME200_SHORTNAME,
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
	*value = slotReadUchar(pSlotAddrSpace->address, offset);

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
					TVME200_SHORTNAME,
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
	*value = slotReadUshort(pSlotAddrSpace->address, offset);

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
					TVME200_SHORTNAME,
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
	*value = slotReadUint(pSlotAddrSpace->address, offset);

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
					TVME200_SHORTNAME,
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
	slotWriteUchar(value, pSlotAddrSpace->address, offset);

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
					TVME200_SHORTNAME,
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
	slotWriteUshort(value, pSlotAddrSpace->address, offset);

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
					TVME200_SHORTNAME,
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
	slotWriteUint(value, pSlotAddrSpace->address, offset);

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

	/* Do the specific carrier write */
	return (slotWriteVector(vector, pSlotId, space, offset));
}
EXPORT_SYMBOL(ipSlotWriteVector);

/***********************************************************
 * 			XXX SIG
 ***********************************************************
 */

/**
 * @brief Register an IRQ for the specified slot.
 *
 * @param pCarrier - Carrier on which slot is
 * @param pSlotId  - Slot to register IRQ.
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 *
 */
static int slotRequestIrq(carrierBoard_t *pCarrier, slotId_t *pSlotId)
{
	slotIrq_t *pSlotIrq = pCarrier->slots[pSlotId->slotPosition].irq;

	/* Request the IRQ to VME interface */
	return (vme_request_irq(pSlotIrq->vector, pSlotIrq->handler, pSlotIrq->arg, pSlotIrq->name));
}

/**
 * @brief Free the IRQ of the specified slot.
 *
 * @param pCarrier - Carrier on which slot is
 * @param pSlotId  - Slot to register IRQ.
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 *
 */
static void slotFreeIrq(carrierBoard_t *pCarrier, slotId_t *pSlotId)
{
	slotIrq_t *pSlotIrq = pCarrier->slots[pSlotId->slotPosition].irq;

	/* Free the IRQ on the VME interface */
	vme_free_irq(pSlotIrq->vector);
}

/**
 * @brief TVME200 slot unsigned char read
 *
 * @param address - address to read
 * @param offset  - Offset from the address
 *
 * @return  value readed
 *
 */
static unsigned char slotReadUchar(void *address, unsigned long offset)
{
	unsigned char value;

	/* Read value */
	value = ioread8(address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error reading unsigned char at 0x%p...", address + offset);
	}

	/* No swap needed */
	return value;
}

/**
 * @brief TVME200 slot unsigned short read
 *
 * @param address - address to read
 * @param offset  - Offset from the address
 *
 * @return  value readed
 *
 */
static unsigned short slotReadUshort(void *address, unsigned long offset)
{
	unsigned short value;

	/* Read value */
	value = ioread16(address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error reading unsigned short at 0x%p...", address + offset);
	}
	/* Intel CPU : Bytes lanes swap (AABB -> BBAA) */
	/* PowerPc CPU : no swap needed */
	return SWAPSBB(value);
}

/**
 * @brief TVME200 slot unsigned int read
 *
 * @param address - address to read
 * @param offset  - Offset from the address
 *
 * @return  value readed
 *
 */
static unsigned int slotReadUint(void *address, unsigned long offset)
{
	unsigned int value;

	/* Read value */
	value = ioread32(address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error reading unsigned int at 0x%p...", address + offset);
	}

	/* Intel CPU : swaps word and byte lanes of the long word (AABBCCDD -> DDCCBBAA) */
	/* PowerPc CPU : No swap needed */
	return SWAPLBB(value);
}

/**
 * @brief TVME200 slot unsigned char write
 *
 * @param value   - value to write
 * @param address - address to write
 * @param offset  - Offset from the address
 *
 */
static void slotWriteUchar(unsigned char value, void *address, unsigned long offset)
{
	/* Write value */
	iowrite8(value, address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error writing unsigned char at 0x%p...", address + offset);
	}
}

/**
 * @brief TVME200 slot unsigned short write
 *
 * @param value   - value to write
 * @param address - address to write
 * @param offset  - Offset from the address
 *
 */
static void slotWriteUshort(unsigned short value, void *address, unsigned long offset)
{
	/* Intel CPU : Bytes lanes swap (AABB -> BBAA) */
	/* PowerPc CPU : no swap needed */
	iowrite16(SWAPSBB(value), address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error writing unsigned short at 0x%p...", address + offset);
	}
}

/**
 * @brief TVME200 slot unsigned int write
 *
 * @param value   - value to write
 * @param address - address to write
 * @param offset  - Offset from the address
 *
 */
static void slotWriteUint(unsigned int value, void *address, unsigned long offset)
{
	/* Intel CPU : swaps word and byte lanes of the long word (AABBCCDD -> DDCCBBAA) */
	/* PowerPc CPU : No swap needed */
	iowrite32(SWAPLBB(value), address + offset);

	/* check error */
	if(vme_bus_error_check(1)){
		PRNT_ERR(carrierIPL, "VME bus error writing unsigned int at 0x%p...", address + offset);
	}
}

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
static int slotWriteVector(unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset)
{
	return (ipSlotWriteUchar(vector, pSlotId, space, offset));
}

/**
 * @brief Check the Io Id space parameters and set it into
 *        the mapping structure.
 *
 * @param pCarrier    - Carrier to set params
 * @param ioidSpace   - IoId space parameters
 * @param ioidMapping - VME mapping structure
 *
 * @return  0 - in case of success.
 * @return <0 - otherwise.
 *
 */
static int setIoIdParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *ioidSpace, struct vme_mapping *ioidMapping)
{
	int i = 0;

	PRNT_INFO(carrierIPL, "Carrier [%s %d] IO space size is ignored and fixed to 0x%X.",
			TVME200_SHORTNAME,
			pCarrier->carrierNumber, TVME200_IOIDINT_SIZE);

	/* Test Address Modifiers parameter */
	i = 0;
	while (a16Modifiers[i] != ioidSpace->addressModifier){
		if (a16Modifiers[i] == ENDLIST){
			PRNT_ERR(carrierIPL, "Carrier [%s %d] address modifier (0x%X) of ID and IO space is not supported !",
					TVME200_SHORTNAME,
					pCarrier->carrierNumber, ioidSpace->addressModifier);
			return -EINVAL;
		}
		i++;
	};

	/* Test Data width parameter */
	i = 0;
	while (dataWidth[i] != ioidSpace->dataWidth){
		if (dataWidth[i] == ENDLIST){
			PRNT_ERR(carrierIPL, "Carrier [%s %d] data width (%d) of ID and IO space is not supported !",
					TVME200_SHORTNAME,
					pCarrier->carrierNumber, ioidSpace->dataWidth);
			return -EINVAL;
		}
		i++;
	};

	/* Test Base address parameter */
	/* Base address must be a boundary of Io Id space size */
	if (ioidSpace->baseAddress % TVME200_IOIDINT_SIZE){
		PRNT_ERR(carrierIPL, "Carrier [%s %d] base address of ID and IO space must be a boundary equal of the space size (0x%X) !",
				TVME200_SHORTNAME,
				pCarrier->carrierNumber, TVME200_IOIDINT_SIZE);
		return -EINVAL;
	}

	/* Set the IO ID Space infos */
	ioidMapping->window_num = 0;
	ioidMapping->am = ioidSpace->addressModifier;
	ioidMapping->data_width = ioidSpace->dataWidth;
	ioidMapping->vme_addru = 0;
	ioidMapping->vme_addrl = ioidSpace->baseAddress;
	ioidMapping->sizeu = 0;
	ioidMapping->sizel = TVME200_IOIDINT_SIZE;

	return 0;
}

/**
 * @brief Check the Mem space parameters and set it into
 *        the mapping structure.
 *
 * @param pCarrier   - Carrier to set params
 * @param memSpace   - Mem space parameters
 * @param memMapping - VME mapping structure
 *
 * @return >=0 - The index into array of the Mem size that is used.
 * @return  <0 - if there is a parameter error.
 *
 */
static int setMemParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *memSpace, struct vme_mapping *memMapping)
{
	int index = 0;
	int i = 0;
	int *addrModifier;

	/* Find the correct memory size of the carrier */
	index = 0;
	while (carrierMemSize[index] != memSpace->windowSize){
		if (carrierMemSize[index] == ENDLIST){
			PRNT_ERR(carrierIPL, "Carrier [%s %d] memory size (0x%X) is not supported !",
					TVME200_SHORTNAME,
					pCarrier->carrierNumber, memSpace->windowSize);
			return -EINVAL;
		}
		index++;
	};

	/* Get the correct A24 or A32 address modifiers list */
	if(carrierMemSize[index] == TVME200_MEM_32M){
		addrModifier = &(a32Modifiers[0]);
	}
	else {
		addrModifier = &(a24Modifiers[0]);
	}

	/* Test Address Modifiers parameter */
	i = 0;
	while (addrModifier[i] != memSpace->addressModifier){
		if (addrModifier[i] == ENDLIST){
			PRNT_ERR(carrierIPL, "Carrier [%s %d] address modifier (0x%X) of MEM space is not supported !",
					TVME200_SHORTNAME,
					pCarrier->carrierNumber, memSpace->addressModifier);
			return -EINVAL;
		}
		i++;
	};

	/* Test Data width parameter */
	i = 0;
	while (dataWidth[i] != memSpace->dataWidth){
		if (dataWidth[i] == ENDLIST){
			PRNT_ERR(carrierIPL, "Carrier [%s %d] The data width (%d) of MEM space is not supported !",
					TVME200_SHORTNAME,
					pCarrier->carrierNumber, memSpace->dataWidth);
			return -EINVAL;
		}
		i++;
	};

	/* Test Base address parameter */
	/* Base address must be a boundary of Memory size */
	if (memSpace->baseAddress % carrierMemSize[index]){
		PRNT_ERR(carrierIPL, "Carrier [%s %d] base address of MEM space must be a boundary equal of the space size (0x%X) !",
				TVME200_SHORTNAME,
				pCarrier->carrierNumber, carrierMemSize[index]);
		return -EINVAL;
	}

	/* Set the Mem Space infos */
	memMapping->window_num = 0;
	memMapping->am = memSpace->addressModifier;
	memMapping->data_width = memSpace->dataWidth;
	memMapping->vme_addru = 0;
	memMapping->vme_addrl = memSpace->baseAddress;
	memMapping->sizeu = 0;
	memMapping->sizel = memSpace->windowSize;

	return index;
}

/******************************************************
 * 			XXX SIG:
 ******************************************************
 */
static int carrierOpen(struct inode *inode, struct file *filp)
{
	return 0;
}

static int carrierRelease(struct inode *inode, struct file *filp)
{
	return 0;
}

static int __init carrierDrvrInitModule(void)
{

	PRNT_INSTALL(carrierIPL, "Carrier driver loading...");

	/* Registering the char device */
	carrierMajorNumber = register_chrdev(0, carrierDriverName, &carrierFops);
	if(carrierMajorNumber <= 0) {
		PRNT_ERR(carrierIPL, "Can't register character module %s.", carrierDriverName);
		return -ENODEV;
	}

	/* Reset the number of installed boards */
	nbInstalledBoards = num_lun;

	PRNT_INSTALL(carrierIPL, "Carrier driver loaded.");

	carrierInstallAll();

	return carrierMajorNumber;
}

static void carrierUninstall(carrierBoard_t *carrier)
{
	int i;

	/* Test if carrier is installed */
	if (carrier->carrierSpecific != NULL) {
		/* Release all slots */
		for (i=0;i<TVME200_NB_SLOT;i++){
			ipSlotUnregister(carrier->slots[i].slotId);
		}
		/* Release board specific */
		carrierUnregister(carrier);
	}
	if (carrier->slots != NULL) {
		/* Free Slots */
		kfree(carrier->slots);
	}
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

	/* Free Boards */
	kfree(carrierBoards);

	PRNT_UNINST(carrierIPL, "Carrier driver unloaded.");
}

static int carrierInstall(carrierBoard_t *carrier, void *params, unsigned int carrierNumber)
{
	int res = 0;

	PRNT_INSTALL(carrierIPL, "Carrier TVME200 number %d installing...", carrierNumber);

	/* Set carrier number */
	carrier->carrierNumber = carrierNumber;
	/* Reset specific */
	carrier->carrierSpecific = NULL;
	/* Init slots */
	carrier->slots = NULL;
	carrier->slots = (carrierSlot_t*) kzalloc(TVME200_NB_SLOT * sizeof(carrierSlot_t), GFP_KERNEL);
	if (carrier->slots == NULL) {
		PRNT_ABS_ERR("Carrier %d, unable to allocate slots memory !", carrier->carrierNumber);
		res = -ENOMEM;
		goto out_err;
	}

	/* call specific init */
	res = carrierRegister(carrier, params);
	if (res){
		PRNT_ABS_ERR("Carrier %d, unable to init board !", carrier->carrierNumber);
		goto out_free;
	}

	/* Init Mutex */
	mutex_init(&(carrier->carrierMutex));

	PRNT_INSTALL(carrierIPL,"Carrier %s number %d installed.", TVME200_SHORTNAME, carrierNumber);

	return 0;

out_free :
	/* Free slot allocation */
	kfree(carrier->slots);
	carrier->slots = NULL;
out_err :
	return res;
}


static int carrierInstallAll(void)
{
	paramsVME_t paramsVME;
	int i,j;
	int res = 0;

	/* Count the number of module that need to be installed */
	nbInstalledBoards = num_lun;

	/* Now we know the number of carrier boards so we can allocate memory */
	carrierBoards = (carrierBoard_t*) kzalloc(nbInstalledBoards * sizeof(carrierBoard_t), GFP_KERNEL);
	if (carrierBoards == NULL) {
		PRNT_ABS_ERR("Unable to allocate carrier boards structure !");
		res = -ENOMEM;
		goto out_err;
	}

	/* Install all modules */
	/* reset the vme params to 0 */
	memset(&paramsVME, 0, sizeof(paramsVME_t));
	/* Set the params */
	for(i = 0; i < num_lun; i++) {
		/* IoId space */
		paramsVME.ioidSpace.addressModifier = mod_address_ioid[i];
		paramsVME.ioidSpace.baseAddress = base_address_ioid[i];
		paramsVME.ioidSpace.dataWidth = data_width_ioid[i];
		paramsVME.ioidSpace.windowSize = wind_size_ioid[i];
		/* Mem space */
		paramsVME.memSpace.addressModifier = mod_address_mem[i];
		paramsVME.memSpace.baseAddress = base_address_mem[i];
		paramsVME.memSpace.dataWidth = data_width_mem[i];
		paramsVME.memSpace.windowSize = wind_size_mem[i];

		/* Install the carrier */
		res = carrierInstall(&carrierBoards[i], &paramsVME, i);
		if (res) {
			PRNT_ERR(carrierIPL, "Error during carrier install !");
			goto out_uninst;
		}
	}; 

	return 0;

out_uninst :
	/* Uninstall already installed boards */
	for (j = 0; j<i; j++){
		carrierUninstall(&carrierBoards[i]);
	}

	nbInstalledBoards = 0;
	kfree(carrierBoards);
out_err :
	return res;
}


module_init(carrierDrvrInitModule);
module_exit(carrierDrvrExitModule);
