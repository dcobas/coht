/**
 * @file carrierVIPC61x.c
 *
 * @brief VIPC61x Carrier interface
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 21/01/2009
 *
 * Specific carrier interface for the SBS VIPC610 and SBS VIPC616 boards.
 *
 * @version $Id: carrierVIPC61x.c,v 1.1 2009/12/01 14:13:03 lewis Exp dcobas $
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <asm/io.h>

#include "vmebus.h"
#include "drvr_dbg_prnt.h"

#include "carrier.h"
#include "carrierCommon.h"
#include "carrierVIPC61x.h"
#include "VIPC61x.h"

/*============================================================================*/
/* Define section                                                             */
/*============================================================================*/
/** End of an array list */
#define ENDLIST -1

/*============================================================================*/
/* Structure section                                                          */
/*============================================================================*/
/**
 * \struct carrierInfos_t
 * \brief VIPC610 and VIPC616 carrier specific info.
 *
 * carrierInfos_t informations specific of the VIPC610 and VIPC616 carrier.
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
static int carrierProcShow(carrierBoard_t *pCarrier, char *page);
static int setIoIdParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *ioidSpace, struct vme_mapping *ioidMapping);
static int setMemParams(carrierBoard_t *pCarrier, vmeAddrSpace_t *memSpace, struct vme_mapping *memMapping);
static unsigned char slotReadUchar(void *address, unsigned long offset);
static unsigned short slotReadUshort(void *address, unsigned long offset);
static unsigned int slotReadUint(void *address, unsigned long offset);
static void slotWriteUchar(unsigned char value, void *address, unsigned long offset);
static void slotWriteUshort(unsigned short value, void *address, unsigned long offset);
static void slotWriteUint(unsigned int value, void *address, unsigned long offset);
static int slotWriteVector(unsigned char vector, slotId_t *pSlotId, slotSpace_e space, unsigned long offset);

/*============================================================================*/
/* Var section                                                                */
/*============================================================================*/
/** DeBuG info Printout level */
extern dbgipl_t carrierIPL;

/** VIPC616 carrier board API */
carrierAPI_t carrierVIPC616API = { longName : VIPC616_LONGNAME,
                                   shortName : VIPC616_SHORTNAME,
                                   numberOfSlot : VIPC61X_NB_SLOT,
                                   type : CARRIER_VIPC616,
                                   carrierRegister : carrierRegister,
                                   carrierUnregister : carrierUnregister,
                                   carrierProcShow : carrierProcShow,
                                   slotRequestIrq : slotRequestIrq,
                                   slotFreeIrq : slotFreeIrq,
                                   slotReadUchar : slotReadUchar,
                                   slotReadUshort : slotReadUshort,
                                   slotReadUint : slotReadUint,
                                   slotWriteUchar : slotWriteUchar,
                                   slotWriteUshort : slotWriteUshort,
                                   slotWriteUint : slotWriteUint,
                                   slotWriteVector : slotWriteVector};

/** VIPC610 carrier board API */
carrierAPI_t carrierVIPC610API = { longName : VIPC610_LONGNAME,
                                   shortName : VIPC610_SHORTNAME,
                                   numberOfSlot : VIPC61X_NB_SLOT,
                                   type : CARRIER_VIPC610,
                                   carrierRegister : carrierRegister,
                                   carrierUnregister : carrierUnregister,
                                   carrierProcShow : carrierProcShow,
                                   slotRequestIrq : slotRequestIrq,
                                   slotFreeIrq : slotFreeIrq,
                                   slotReadUchar : slotReadUchar,
                                   slotReadUshort : slotReadUshort,
                                   slotReadUint : slotReadUint,
                                   slotWriteUchar : slotWriteUchar,
                                   slotWriteUshort : slotWriteUshort,
                                   slotWriteUint : slotWriteUint,
                                   slotWriteVector : slotWriteVector};

/** VIPC61x supported Data width */
static int dataWidth[] = { VME_D8, VME_D16, ENDLIST };

/** Supported A16 Address Modifiers */
static int a16Modifiers[] = { 0x29, 0x2D, ENDLIST };

/** Supported A24 Address Modifiers */
static int a24Modifiers[] = { 0x39, 0x3A, 0x3D, 0x3E, ENDLIST };

/** Supported A32 Address Modifiers */
static int a32Modifiers[] = { 0x09, 0x0A, 0x0D, 0x0E, ENDLIST };

/** VIPC610 carrier total memory size */
static int vipc610MemSize[] = { VIPC61X_MEM_512k, VIPC61X_MEM_1M,
                                VIPC61X_MEM_2M, VIPC61X_MEM_4M,
                                VIPC61X_MEM_8M, ENDLIST };

/** VIPC610 carrier slot gap */
static int vipc610SlotGap[] = { VIPC61X_MEM_512k_SLOT_GAP, VIPC61X_MEM_1M_SLOT_GAP,
                                VIPC61X_MEM_2M_SLOT_GAP, VIPC61X_MEM_4M_SLOT_GAP,
                                VIPC61X_MEM_8M_SLOT_GAP, ENDLIST };

/** VIPC610 carrier total slot size */
static int vipc610SlotSize[] = { VIPC61X_MEM_512k_SLOT_SIZE, VIPC61X_MEM_1M_SLOT_SIZE,
                                 VIPC61X_MEM_2M_SLOT_SIZE, VIPC61X_MEM_4M_SLOT_SIZE,
                                 VIPC61X_MEM_8M_SLOT_SIZE, ENDLIST };

/** VIPC616 carrier total memory size */
static int vipc616MemSize[] = { VIPC61X_MEM_512k, VIPC61X_MEM_1M,
                                VIPC61X_MEM_2M, VIPC61X_MEM_4M,
                                VIPC61X_MEM_8M, VIPC61X_MEM_32M, ENDLIST };

/** VIPC616 carrier slot gap */
static int vipc616SlotGap[] = { VIPC61X_MEM_512k_SLOT_GAP, VIPC61X_MEM_1M_SLOT_GAP,
                                VIPC61X_MEM_2M_SLOT_GAP, VIPC61X_MEM_4M_SLOT_GAP,
                                VIPC61X_MEM_8M_SLOT_GAP, VIPC61X_MEM_32M_SLOT_GAP, ENDLIST };

/** VIPC616 carrier total slot size */
static int vipc616SlotSize[] = { VIPC61X_MEM_512k_SLOT_SIZE, VIPC61X_MEM_1M_SLOT_SIZE,
                                 VIPC61X_MEM_2M_SLOT_SIZE, VIPC61X_MEM_4M_SLOT_SIZE,
                                 VIPC61X_MEM_8M_SLOT_SIZE, VIPC61X_MEM_32M_SLOT_SIZE, ENDLIST };

/*============================================================================*/
/* Internal lib. functions                                                    */
/*============================================================================*/
/**
 * @brief Initialisation of the VIPC61x board.
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
                     pCarrier->carrierAPI->shortName,
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
                 pCarrier->carrierAPI->shortName,
                 pCarrier->carrierNumber);
        goto out_free;
    }

    /* Find mapping for Mem space */
    res = vme_find_mapping(&(vmeCarrier->memMapping), 1);
    if (res){
        PRNT_ERR(carrierIPL, "Carrier [%s %d] unable to find window mapping for MEM space !",
                 pCarrier->carrierAPI->shortName,
                 pCarrier->carrierNumber);
        goto out_releaseIoId;
    }

    /* Set all slot physical address space */
    for (i=0; i<pCarrier->carrierAPI->numberOfSlot; i++) {
        pCarrier->slots[i].ioPhys.address = (void*)(vmeCarrier->ioidMapping.pci_addrl + VIPC61X_IO_SPACE_OFF + VIPC61X_IO_SPACE_GAP*i);
        pCarrier->slots[i].ioPhys.size = VIPC61X_IO_SPACE_SIZE;
        pCarrier->slots[i].idPhys.address = (void*)(vmeCarrier->ioidMapping.pci_addrl + VIPC61X_ID_SPACE_OFF + VIPC61X_ID_SPACE_GAP*i);
        pCarrier->slots[i].idPhys.size = VIPC61X_ID_SPACE_SIZE;
        if (pCarrier->carrierAPI->type == CARRIER_VIPC610){
            /* VIPC10 board */
            pCarrier->slots[i].memPhys.address = (void*)(vmeCarrier->memMapping.pci_addrl + vipc610SlotGap[carrierMemIndex]*i);
            pCarrier->slots[i].memPhys.size = vipc610SlotSize[carrierMemIndex];
        }
        else {
            /* VIPC16 board */
            pCarrier->slots[i].memPhys.address = (void*)(vmeCarrier->memMapping.pci_addrl + vipc616SlotGap[carrierMemIndex]*i);
            pCarrier->slots[i].memPhys.size = vipc616SlotSize[carrierMemIndex];
        }

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
 * @brief Unregister a VIPC61x board.
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
    for (i=0; i<pCarrier->carrierAPI->numberOfSlot; i++) {
        pCarrier->slots[i].ioPhys.address = NULL;
        pCarrier->slots[i].ioPhys.size = 0;
        pCarrier->slots[i].idPhys.address = NULL;
        pCarrier->slots[i].idPhys.size = 0;
        pCarrier->slots[i].memPhys.address = NULL;
        pCarrier->slots[i].memPhys.size = 0;
    }
}

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
 * @brief VIPC61x printout for procfs
 *
 * @param pCarrier - Carrier to printout
 * @param page     - Page where data are writted
 *
 * @return  Number of bytes writted.
 *
 */
static int carrierProcShow(carrierBoard_t *pCarrier, char *page)
{
    char *p = page;
    carrierInfos_t *vmeCarrier = (carrierInfos_t *) pCarrier->carrierSpecific;

    p += sprintf(p,"Carrier VME infos\n");
    p += sprintf(p,"    Nb. of slots     %d\n", pCarrier->carrierAPI->numberOfSlot);
    p += sprintf(p,"    IO ID Mapping    Window number     %d\n", vmeCarrier->ioidMapping.window_num);
    p += sprintf(p,"                     Data width        %d\n", vmeCarrier->ioidMapping.data_width);
    p += sprintf(p,"                     Address modifier  0x%02x\n", vmeCarrier->ioidMapping.am);
    p += sprintf(p,"                     Virtual address   0x%p\n", vmeCarrier->ioidMapping.kernel_va);
    p += sprintf(p,"                     PCI address       0x%08x\n", vmeCarrier->ioidMapping.pci_addrl);
    p += sprintf(p,"                     VME address       0x%08x\n", vmeCarrier->ioidMapping.vme_addrl);
    p += sprintf(p,"                     Size              0x%08x\n", vmeCarrier->ioidMapping.sizel);
    p += sprintf(p,"    MEM Mapping      Window number     %d\n", vmeCarrier->memMapping.window_num);
    p += sprintf(p,"                     Data width        %d\n", vmeCarrier->memMapping.data_width);
    p += sprintf(p,"                     Address modifier  0x%02x\n", vmeCarrier->memMapping.am);
    p += sprintf(p,"                     Virtual address   0x%p\n", vmeCarrier->memMapping.kernel_va);
    p += sprintf(p,"                     PCI address       0x%08x\n", vmeCarrier->memMapping.pci_addrl);
    p += sprintf(p,"                     VME address       0x%08x\n", vmeCarrier->memMapping.vme_addrl);
    p += sprintf(p,"                     Size              0x%08x\n", vmeCarrier->memMapping.sizel);

    return p - page;
}

/**
 * @brief VIPC61X slot unsigned char read
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
 * @brief VIPC61X slot unsigned short read
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
 * @brief VIPC61X slot unsigned int read
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
 * @brief VIPC61X slot unsigned char write
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
 * @brief VIPC61X slot unsigned short write
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
 * @brief VIPC61X slot unsigned int write
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
              pCarrier->carrierAPI->shortName,
              pCarrier->carrierNumber, VIPC61X_IOIDINT_SIZE);

    /* Test Address Modifiers parameter */
    i = 0;
    while (a16Modifiers[i] != ioidSpace->addressModifier){
        if (a16Modifiers[i] == ENDLIST){
            PRNT_ERR(carrierIPL, "Carrier [%s %d] address modifier (0x%X) of ID and IO space is not supported !",
                     pCarrier->carrierAPI->shortName,
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
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, ioidSpace->dataWidth);
            return -EINVAL;
        }
        i++;
    };

    /* Test Base address parameter */
    /* Base address must be a boundary of Io Id space size */
    if (ioidSpace->baseAddress % VIPC61X_IOIDINT_SIZE){
        PRNT_ERR(carrierIPL, "Carrier [%s %d] base address of ID and IO space must be a boundary equal of the space size (0x%X) !",
                 pCarrier->carrierAPI->shortName,
                 pCarrier->carrierNumber, VIPC61X_IOIDINT_SIZE);
        return -EINVAL;
    }

    /* Set the IO ID Space infos */
    ioidMapping->window_num = 0;
    ioidMapping->am = ioidSpace->addressModifier;
    ioidMapping->data_width = ioidSpace->dataWidth;
    ioidMapping->vme_addru = 0;
    ioidMapping->vme_addrl = ioidSpace->baseAddress;
    ioidMapping->sizeu = 0;
    ioidMapping->sizel = VIPC61X_IOIDINT_SIZE;

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
    int *addrModifier, *carrierMemSize;

    /* Find the board type */
    if (pCarrier->carrierAPI->type == CARRIER_VIPC610){
        /* VIPC10 board */
        carrierMemSize = &(vipc610MemSize[0]);
    }
    else {
        /* VIPC16 board */
        carrierMemSize = &(vipc616MemSize[0]);
    }

    /* Find the correct memory size of the carrier */
    index = 0;
    while (carrierMemSize[index] != memSpace->windowSize){
        if (carrierMemSize[index] == ENDLIST){
            PRNT_ERR(carrierIPL, "Carrier [%s %d] memory size (0x%X) is not supported !",
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, memSpace->windowSize);
            return -EINVAL;
        }
        index++;
    };

    /* Get the correct A24 or A32 address modifiers list */
    if(carrierMemSize[index] == VIPC61X_MEM_32M){
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
                     pCarrier->carrierAPI->shortName,
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
                     pCarrier->carrierAPI->shortName,
                     pCarrier->carrierNumber, memSpace->dataWidth);
            return -EINVAL;
        }
        i++;
    };

    /* Test Base address parameter */
    /* Base address must be a boundary of Memory size */
    if (memSpace->baseAddress % carrierMemSize[index]){
        PRNT_ERR(carrierIPL, "Carrier [%s %d] base address of MEM space must be a boundary equal of the space size (0x%X) !",
                 pCarrier->carrierAPI->shortName,
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
