#include <stdio.h>
#include <stddef.h>

#define CTR_PCI

#include "ctrdrvr.h"

int main(int argc, char **argv)
{

printf("\n*** SIZES ***\n\n");

printf("sizeof(CtrDrvrConnection)=%lu\n", (unsigned long) sizeof(CtrDrvrConnection));
printf("sizeof(CtrDrvrClientList)=%lu\n", (unsigned long) sizeof(CtrDrvrClientList));
printf("sizeof(CtrDrvrClientConnections)=%lu\n", (unsigned long) sizeof(CtrDrvrClientConnections));
printf("sizeof(CtrDrvrReadBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrReadBuf));
printf("sizeof(CtrDrvrWriteBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrWriteBuf));
printf("sizeof(CtrDrvrAction)=%lu\n", (unsigned long) sizeof(CtrDrvrAction));
printf("sizeof(CtrDrvrCtimBinding)=%lu\n", (unsigned long) sizeof(CtrDrvrCtimBinding));
printf("sizeof(CtrDrvrCtimObjects)=%lu\n", (unsigned long) sizeof(CtrDrvrCtimObjects));
printf("sizeof(CtrDrvrPtimBinding)=%lu\n", (unsigned long) sizeof(CtrDrvrPtimBinding));
printf("sizeof(CtrDrvrPtimObjects)=%lu\n", (unsigned long) sizeof(CtrDrvrPtimObjects));
printf("sizeof(CtrDrvrModuleAddress)=%lu\n", (unsigned long) sizeof(CtrDrvrModuleAddress));
printf("sizeof(CtrDrvrVersion)=%lu\n", (unsigned long) sizeof(CtrDrvrVersion));
printf("sizeof(CtrDrvrHptdcIoBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrHptdcIoBuf));
printf("sizeof(CtrDrvrCounterConfigurationBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrCounterConfigurationBuf));
printf("sizeof(CtrDrvrCounterHistoryBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrCounterHistoryBuf));
printf("sizeof(CtrdrvrRemoteCommandBuf)=%lu\n", (unsigned long) sizeof(CtrdrvrRemoteCommandBuf));
printf("sizeof(CtrDrvrCounterMaskBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrCounterMaskBuf));
printf("sizeof(CtrDrvrTgmBuf)=%lu\n", (unsigned long) sizeof(CtrDrvrTgmBuf));
printf("sizeof(CtrDrvrRawIoBlock)=%lu\n", (unsigned long) sizeof(CtrDrvrRawIoBlock));
printf("sizeof(CtrDrvrReceptionErrors)=%lu\n", (unsigned long) sizeof(CtrDrvrReceptionErrors));
printf("sizeof(CtrDrvrBoardId)=%lu\n", (unsigned long) sizeof(CtrDrvrBoardId));

printf("\n*** OFFSETS ***\n\n");

printf("CtrDrvrModuleAddress.PciSlot = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,PciSlot));
printf("CtrDrvrModuleAddress.ModuleNumber = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,ModuleNumber));
printf("CtrDrvrModuleAddress.DeviceId = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,DeviceId));
printf("CtrDrvrModuleAddress.VendorId = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,VendorId));
printf("CtrDrvrModuleAddress.MemoryMap = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,MemoryMap));
printf("CtrDrvrModuleAddress.LocalMap = %lu\n", (unsigned long) offsetof(CtrDrvrModuleAddress,LocalMap));

return 0;
}
