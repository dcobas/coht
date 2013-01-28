#include <stdio.h>
#include <stddef.h>

//       size_t offsetof(type, member);

#include "ctrdrvr.h"

int main(int argc, char **argv)
{
printf("sizeof(CtrDrvrConnection)=%d\n",sizeof(CtrDrvrConnection));
printf("sizeof(CtrDrvrClientList)=%d\n",sizeof(CtrDrvrClientList));
printf("sizeof(CtrDrvrClientConnections)=%d\n",sizeof(CtrDrvrClientConnections));
printf("sizeof(CtrDrvrReadBuf)=%d\n",sizeof(CtrDrvrReadBuf));
printf("sizeof(CtrDrvrWriteBuf)=%d\n",sizeof(CtrDrvrWriteBuf));
printf("sizeof(CtrDrvrAction)=%d\n",sizeof(CtrDrvrAction));
printf("sizeof(CtrDrvrCtimBinding)=%d\n",sizeof(CtrDrvrCtimBinding));
printf("sizeof(CtrDrvrCtimObjects)=%d\n",sizeof(CtrDrvrCtimObjects));
printf("sizeof(CtrDrvrPtimBinding)=%d\n",sizeof(CtrDrvrPtimBinding));
printf("sizeof(CtrDrvrPtimObjects)=%d\n",sizeof(CtrDrvrPtimObjects));
printf("sizeof(CtrDrvrModuleAddress)=%d\n",sizeof(CtrDrvrModuleAddress));
printf("sizeof(CtrDrvrVersion)=%d\n",sizeof(CtrDrvrVersion));
printf("sizeof(CtrDrvrHptdcIoBuf)=%d\n",sizeof(CtrDrvrHptdcIoBuf));
printf("sizeof(CtrDrvrCounterConfigurationBuf)=%d\n",sizeof(CtrDrvrCounterConfigurationBuf));
printf("sizeof(CtrDrvrCounterHistoryBuf)=%d\n",sizeof(CtrDrvrCounterHistoryBuf));
printf("sizeof(CtrdrvrRemoteCommandBuf)=%d\n",sizeof(CtrdrvrRemoteCommandBuf));
printf("sizeof(CtrDrvrCounterMaskBuf)=%d\n",sizeof(CtrDrvrCounterMaskBuf));
printf("sizeof(CtrDrvrTgmBuf)=%d\n",sizeof(CtrDrvrTgmBuf));
printf("sizeof(CtrDrvrRawIoBlock)=%d\n",sizeof(CtrDrvrRawIoBlock));
printf("sizeof(CtrDrvrReceptionErrors)=%d\n",sizeof(CtrDrvrReceptionErrors));
printf("sizeof(CtrDrvrBoardId)=%d\n",sizeof(CtrDrvrBoardId));

#ifdef CTR_PCI
printf("CtrDrvrModuleAddress.PciSlot = %d\n", offsetof(CtrDrvrModuleAddress,PciSlot));
printf("CtrDrvrModuleAddress.ModuleNumber = %d\n", offsetof(CtrDrvrModuleAddress,ModuleNumber));
printf("CtrDrvrModuleAddress.DeviceId = %d\n", offsetof(CtrDrvrModuleAddress,DeviceId));
printf("CtrDrvrModuleAddress.VendorId = %d\n", offsetof(CtrDrvrModuleAddress,VendorId));
printf("CtrDrvrModuleAddress.MemoryMap = %d\n", offsetof(CtrDrvrModuleAddress,MemoryMap));
printf("CtrDrvrModuleAddress.LocalMap = %d\n", offsetof(CtrDrvrModuleAddress,LocalMap));
#endif
#ifdef CTR_VME
printf("CtrDrvrModuleAddress.VMEAddress= %d\n", offsetof(CtrDrvrModuleAddress,VMEAddress));
printf("CtrDrvrModuleAddress.JTGAddress= %d\n", offsetof(CtrDrvrModuleAddress,JTGAddress));
printf("CtrDrvrModuleAddress.InterruptVector= %d\n", offsetof(CtrDrvrModuleAddress,InterruptVector));
printf("CtrDrvrModuleAddress.InterruptLevel= %d\n", offsetof(CtrDrvrModuleAddress,InterruptLevel));
printf("CtrDrvrModuleAddress.CopyAddress= %d\n", offsetof(CtrDrvrModuleAddress,CopyAddress));
#endif
return 0;
}
