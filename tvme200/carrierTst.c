/**
 * @file carrierTst.c
 *
 * @brief Carrier test software
 *
 * @author Nicolas Serafini, EIC2 SA
 *
 * @date Created on 17/02/2009
 *
 * @version $Id$
 */
/*============================================================================*/
/* Include Section                                                            */
/*============================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "carrier.h"
#include "carrierTstDrvr.h"

/*============================================================================*/
/* Define section                                                             */
/*============================================================================*/
/** Carrier test node */
#define CARRIER_TEST_NODE   "/dev/carrierTst"

/*============================================================================*/
/* Enum section                                                               */
/*============================================================================*/
/**
 * \enum actions_e
 * \brief All menu actions.
 */
typedef enum {
    ACTION_QUIT              = 0,  /**< To quit */
    ACTION_SLOT_REGISTER     = 1,  /**< To register slot */
    ACTION_SLOT_UNREGISTER   = 2,  /**< To unregister slot */
    ACTION_SLOT_MAP          = 3,  /**< To Map a space of a slot */
    ACTION_SLOT_UNMAP        = 4,  /**< To unmap a space of a slot */
    ACTION_SLOT_READ_IDPROM  = 5,  /**< To read slot IDPROM */
    ACTION_SLOT_READ_UCHAR   = 6,  /**< To read an unsigned char value */
    ACTION_SLOT_READ_USHORT  = 7,  /**< To read an unsigned short value */
    ACTION_SLOT_READ_UINT    = 8,  /**< To read an unsigned int value */
    ACTION_SLOT_WRITE_UCHAR  = 9,  /**< To write an unsigned char value */
    ACTION_SLOT_WRITE_USHORT = 10, /**< To write an unsigned short value */
    ACTION_SLOT_WRITE_UINT   = 11, /**< To write an unsigned int value */
    ACTION_SLOT_INSTALL_IRQ  = 12, /**< To install an IRQ */
    ACTION_SLOT_FREE_IRQ     = 13, /**< To free an irq */
    ACTION_SLOT_IRQ_COUNTER  = 14, /**< To read the irq counter */
} actions_e;

/*============================================================================*/
/* Function definition                                                        */
/*============================================================================*/
static actions_e getAction(void);
static void getSlot(slot_t *pSlot);
static void getSlotName(slot_t *pSlot);
static void getSpace(slot_t *pSlot);
static void getMemory(slot_t *pSlot);
static void getOffset(slot_t *pSlot);
static void getUchar(slot_t *pSlot);
static void getUshort(slot_t *pSlot);
static void getUint(slot_t *pSlot);
static void readIdProm(slot_t *pSlot, int carrierFd);
static void getVector(slot_t *pSlot);

/*============================================================================*/
/* Code Section                                                               */
/*============================================================================*/
/**
* @brief main entry of software.
*
* @param argc
* @param *argv[]
*
* @return int
*/
int main(int argc, char *argv[])
{
    actions_e action;
    int carrierFd = -1;
    int ioctlRes;
    slot_t slot;

    /* Open the device */
    carrierFd = open(CARRIER_TEST_NODE, O_RDWR);
    if (carrierFd == -1)
    {
            printf("Can't open %s node\n", CARRIER_TEST_NODE);
            return(-1);
    }

    /* Menu infinite loop */
    do {
        /* Get action to do */
        action = getAction();

        /* Get slot */
        if (action != ACTION_QUIT) {
            getSlot(&slot);
        }

        switch(action){
        case ACTION_SLOT_REGISTER :
            /* Get slot name */
            getSlotName(&slot);
            /* Register slot */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_REGISTER, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_REGISTER fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_UNREGISTER :
            /* Unregister slot */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_UNREGISTER, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_UNREGISTER fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_MAP :
            /* Get space to map */
            getSpace(&slot);
            /* Get memory */
            if (slot.space == SLOT_MEM_SPACE) {
                getMemory(&slot);
            }
            /* Map space */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_MAP_SPACE, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_MAP_SPACE fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_UNMAP :
            /* Get space to unmap */
            getSpace(&slot);
            /* Unmap space */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_UNMAP_SPACE, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_UNMAP_SPACE fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_READ_IDPROM :
            /* Read the ID Prom */
            readIdProm(&slot, carrierFd);
            break;
        case ACTION_SLOT_READ_UCHAR :
            /* Get space to read */
            getSpace(&slot);
            /* Get offset to read */
            getOffset(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_READ_UCHAR fails. [error %d]\n",ioctlRes);
                break;
            }
            /* Print result */
            printf("  Value readed : 0x%x\n", slot.ucharValue);
            break;
        case ACTION_SLOT_READ_USHORT :
            /* Get space to read */
            getSpace(&slot);
            /* Get offset to read */
            getOffset(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_READ_USHORT, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_READ_USHORT fails. [error %d]\n",ioctlRes);
                break;
            }
            /* Print result */
            printf("  Value readed : 0x%x\n", slot.ushortValue);
            break;
        case ACTION_SLOT_READ_UINT :
            /* Get space to read */
            getSpace(&slot);
            /* Get offset to read */
            getOffset(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_READ_UINT, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_READ_UINT fails. [error %d]\n",ioctlRes);
                break;
            }
            /* Print result */
            printf("  Value readed : 0x%x\n", slot.uintValue);
            break;
        case ACTION_SLOT_WRITE_UCHAR :
            /* Get space to write */
            getSpace(&slot);
            /* Get offset to write */
            getOffset(&slot);
            /* Get value to write */
            getUchar(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_WRITE_UCHAR, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_WRITE_UCHAR fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_WRITE_USHORT :
            /* Get space to write */
            getSpace(&slot);
            /* Get offset to write */
            getOffset(&slot);
            /* Get value to write */
            getUshort(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_WRITE_USHORT, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_WRITE_USHORT fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_WRITE_UINT :
            /* Get space to write */
            getSpace(&slot);
            /* Get offset to write */
            getOffset(&slot);
            /* Get value to write */
            getUint(&slot);
            /* Read */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_WRITE_UINT, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_WRITE_UINT fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_INSTALL_IRQ :
            /* Get the vector */
            getVector(&slot);
            /* Install IRQ */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_IRQ_INSTALL, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_IRQ_INSTALL fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_FREE_IRQ :
            /* Free IRQ */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_IRQ_FREE, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_IRQ_FREE fails. [error %d]\n",ioctlRes);
                break;
            }
            break;
        case ACTION_SLOT_IRQ_COUNTER :
            /* Get IRQ counter */
            ioctlRes = ioctl(carrierFd, CARRIER_SLOT_GET_IRQ_COUNTER, &slot);
            if (ioctlRes){
                printf("IOCTL CARRIER_SLOT_GET_IRQ_COUNTER fails. [error %d]\n",ioctlRes);
                break;
            }
            /* Print result */
            printf("  IRQ counter : %d\n", slot.counter);
            break;
        default :
            break;
        }

    }while(action != ACTION_QUIT);

    /* Close device */
    close(carrierFd);

    return 0;
}

/**
* @brief print the menu and return an action to do.
*
* @param Null
*
* @return Action to do.
*/
static actions_e getAction(void)
{
    int res = -1;
    actions_e action;

    do
    {
        printf("\n\n");
        printf("+-------------------------+\n");
        printf("| ------- Actions ------- |\n");
        printf("+-------------------------+\n");
        printf("| %02d : Slot register      |\n", ACTION_SLOT_REGISTER);
        printf("| %02d : Slot unregister    |\n", ACTION_SLOT_UNREGISTER);
        printf("| %02d : Slot map space     |\n", ACTION_SLOT_MAP);
        printf("| %02d : Slot unmap space   |\n", ACTION_SLOT_UNMAP);
        printf("| %02d : Slot read IDPROM   |\n", ACTION_SLOT_READ_IDPROM);
        printf("| %02d : Slot read UCHAR    |\n", ACTION_SLOT_READ_UCHAR);
        printf("| %02d : Slot read USHORT   |\n", ACTION_SLOT_READ_USHORT);
        printf("| %02d : Slot read UINT     |\n", ACTION_SLOT_READ_UINT);
        printf("| %02d : Slot write UCHAR   |\n", ACTION_SLOT_WRITE_UCHAR);
        printf("| %02d : Slot write USHORT  |\n", ACTION_SLOT_WRITE_USHORT);
        printf("| %02d : Slot write UINT    |\n", ACTION_SLOT_WRITE_UINT);
        printf("| %02d : Slot install IRQ   |\n", ACTION_SLOT_INSTALL_IRQ);
        printf("| %02d : Slot free IRQ      |\n", ACTION_SLOT_FREE_IRQ);
        printf("| %02d : Slot IRQ counter   |\n", ACTION_SLOT_IRQ_COUNTER);
        printf("| %02d : Quit               |\n", ACTION_QUIT);
        printf("+-------------------------+\n");
        printf("  Select action : ");
        scanf("%2d", &res);
    }while ((res < ACTION_QUIT) || (res > ACTION_SLOT_IRQ_COUNTER));

    action = res;

    return (action);
}

/**
* @brief Get the carrier number and the slot position
*
* @param pSlot - Slot
*/
static void getSlot(slot_t *pSlot)
{
    printf("Slot selection\n");
    printf("  Carrier number : ");
    scanf("%2d", &(pSlot->carrierNumber));
    printf("  Slot number : ");
    scanf("%2d", &(pSlot->slotPosition));
}

/**
* @brief Get the slot name
*
* @param pSlot - Slot
*/
static void getSlotName(slot_t *pSlot)
{
    printf("                        |-------------------------------------------------|\n");
    printf("  Slot name (no space) : ");
    scanf("%49s", pSlot->boardName);
}

/**
* @brief Get the memory size
*
* @param pSlot - Slot
*/
static void getMemory(slot_t *pSlot)
{
    printf("  Memory size : 0x");
    scanf("%x", &(pSlot->memory));
}

/**
* @brief Get the offset
*
* @param pSlot - Slot
*/
static void getOffset(slot_t *pSlot)
{
    printf("  Offset : 0x");
    scanf("%lx", &(pSlot->offset));
}

/**
* @brief Get the vector
*
* @param pSlot - Slot
*/
static void getVector(slot_t *pSlot)
{
    unsigned int value;

    printf("  Vector : 0x");
    scanf("%2x", &value);

    pSlot->vector = value;
}

/**
* @brief Get the offset
*
* @param pSlot - Slot
*/
static void getUchar(slot_t *pSlot)
{
    unsigned int value;

    printf("  Value : 0x");
    scanf("%2x", &value);

    pSlot->ucharValue = (unsigned char) value;
}

/**
* @brief Get the offset
*
* @param pSlot - Slot
*/
static void getUshort(slot_t *pSlot)
{
    printf("  Value : 0x");
    scanf("%hx", &(pSlot->ushortValue));
}

/**
* @brief Get the offset
*
* @param pSlot - Slot
*/
static void getUint(slot_t *pSlot)
{
    printf("  Value : 0x");
    scanf("%x", &(pSlot->uintValue));
}

/**
* @brief Get the slot memory space
*
* @param pSlot - Slot
*/
static void getSpace(slot_t *pSlot)
{
    int space = -1;

    do
    {
        printf("Slot space\n");
        printf("   +---------------+ \n");
        printf("   | %d : IO space  |\n", SLOT_IO_SPACE);
        printf("   | %d : ID space  |\n", SLOT_ID_SPACE);
        printf("   | %d : MEM space |\n", SLOT_MEM_SPACE);
        printf("   +---------------+ \n");
        printf("  Select space : ");
        scanf("%1d", &space);
    }while (!((space != SLOT_IO_SPACE) || (space != SLOT_ID_SPACE) || (space != SLOT_MEM_SPACE)));

    pSlot->space = space;
}

/**
* @brief Read ID Prom of the selected carrier
*
* @param pSlot     - Slot
* @param carrierFd - File descriptor
*/
static void readIdProm(slot_t *pSlot, int carrierFd)
{
    /* Set slot space */
    pSlot->space = SLOT_ID_SPACE;

    printf("\n");
    printf("+------------------------------------+\n");
    printf("| ------ Slot [%02d:%02d] ID PROM ------ |\n", pSlot->carrierNumber, pSlot->slotPosition);
    printf("+------------------------------------+\n");
    printf("| Addr | Description          | Val. |\n");
    printf("+------+----------------------+------+\n");
    /* Read I */
    pSlot->offset = SLOT_IDPROM_OFFSET_I;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | ASCII \"I\"            | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | ASCII \"I\"            | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read P */
    pSlot->offset = SLOT_IDPROM_OFFSET_P;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | ASCII \"P\"            | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | ASCII \"P\"            | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read A */
    pSlot->offset = SLOT_IDPROM_OFFSET_A;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | ASCII \"A\"            | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | ASCII \"A\"            | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read C */
    pSlot->offset = SLOT_IDPROM_OFFSET_C;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | ASCII \"C\"            | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | ASCII \"C\"            | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Manufacturer ID */
    pSlot->offset = SLOT_IDPROM_OFFSET_MANUFACTURER_ID;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Manufacturer ID      | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Manufacturer ID      | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Model number */
    pSlot->offset = SLOT_IDPROM_OFFSET_MODEL;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Model number         | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Model number         | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Revision */
    pSlot->offset = SLOT_IDPROM_OFFSET_REVISION;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Revision             | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Revision             | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Reserved */
    pSlot->offset = SLOT_IDPROM_OFFSET_RESERVED;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Reserved             | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Reserved             | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Driver ID low byte */
    pSlot->offset = SLOT_IDPROM_OFFSET_DRIVER_ID_L;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Driver ID, low byte  | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Driver ID, low byte  | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Driver ID high byte */
    pSlot->offset = SLOT_IDPROM_OFFSET_DRIVER_ID_H;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Driver ID, high byte | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Driver ID, high byte | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read Number of bytes */
    pSlot->offset = SLOT_IDPROM_OFFSET_NUM_BYTES;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | Nb. bytes used       | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | Nb. bytes used       | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    /* Read CRC */
    pSlot->offset = SLOT_IDPROM_OFFSET_CRC;
    if (ioctl(carrierFd, CARRIER_SLOT_READ_UCHAR, pSlot)){
        printf("| 0x%02lx | CRC                  | err. |\n", pSlot->offset);
    }
    else {
        printf("| 0x%02lx | CRC                  | 0x%02x |\n", pSlot->offset, pSlot->ucharValue);
    }

    printf("+------------------------------------+\n");
}
