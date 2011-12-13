/**
 * Process module parameters to build an instlib driver tree
 * for VME bus types.
 *
 * Julian Lewis BE/CO/HT Tue 22 Nov 2011
 */

#include <libinstkernel.h>
#include <skeluser.h>

#define MODBUG

#ifdef MODBUG
#ifdef CONFIG_BUS_PCI

/**
 * For debug, print the tree
 */

void InsLibPrintVmeSpace(InsLibVmeAddressSpace * vmeas)
{
	if (vmeas) {
		printk("[VME:Am:0x%X:Dw:%d:Ws:0x%X:Ba:0x%X:Fm:%d]",
		       vmeas->AddressModifier,
		       vmeas->DataWidth,
		       vmeas->WindowSize,
		       vmeas->BaseAddress, vmeas->FreeModifierFlag);
		InsLibPrintVmeSpace(vmeas->Next);
	}
}

void InsLibPrintVme(InsLibVmeModuleAddress * vmema)
{
	if (vmema)
		InsLibPrintVmeSpace(vmema->VmeAddressSpace);
}

void InsLibPrintModule(InsLibModlDesc * modld)
{
	if (modld) {
		printk("[MOD:%s Bt:%d:Mn:%d",
		       modld->ModuleName,
		       modld->BusType, modld->ModuleNumber);
		if (modld->Isr) {
			printk(":Fl:%d:Vec:0x%X:Lvl:%d",
			       modld->Isr->IsrFlag,
			       modld->Isr->Vector, modld->Isr->Level);
		}
		printk("]");
		if (modld->BusType == InsLibBusTypeVME)
			InsLibPrintVme(modld->ModuleAddress);
		printk("\n");
		InsLibPrintModule(modld->Next);
	}
}

void InsLibPrintDriver(InsLibDrvrDesc * drvrd)
{
	if (drvrd) {
		printk("[DRV:%s Dg:%d Df:0x%X Ef:0x%X Mc:%d]\n",
		       drvrd->DrvrName,
		       drvrd->isdg,
		       drvrd->DebugFlags,
		       drvrd->EmulationFlags, drvrd->ModuleCount);

		InsLibPrintModule(drvrd->Modules);
		InsLibPrintDriver(drvrd->Next);
	}
}
#endif
#endif

#ifdef CONFIG_BUS_VME
#ifdef __linux__

#include <linux/module.h>

/**
 * Handle VME module parameters
 */

/**
 * Example: insmod drv.ko luns=1,2 vme1=0xC100000,0xC200000 vme2=0x100,0x200 vecs=0xC8,0xC9
 */

#define MAX_DEVS 16

static int luns[MAX_DEVS];	/* Logical unit numbers */
static int vme1[MAX_DEVS];	/* First VME base address */
static int vme2[MAX_DEVS];	/* Second VME base address */
static int vecs[MAX_DEVS];	/* Interrupt vectors */

static unsigned int luns_num;
static unsigned int vme1_num;
static unsigned int vme2_num;
static unsigned int vecs_num;

module_param_array(luns, int, &luns_num, 0444);	/* Vector */
module_param_array(vme1, int, &vme1_num, 0444);	/* Vector */
module_param_array(vme2, int, &vme2_num, 0444);	/* Vector */
module_param_array(vecs, int, &vecs_num, 0444);	/* Vector */

MODULE_PARM_DESC(luns, "Logical unit numbers");
MODULE_PARM_DESC(vme1, "First map base addresses");
MODULE_PARM_DESC(vme2, "Second map base addresses");
MODULE_PARM_DESC(vecs, "Interrupt vectors");

#define DRV_NAME_LEN 64
static char dname[DRV_NAME_LEN] = { 0 };

module_param_string(dname, dname, DRV_NAME_LEN, 0);
MODULE_PARM_DESC(dname, "Optional driver name");

MODULE_DESCRIPTION(SkelDrvrCOMMENT);
MODULE_AUTHOR(SkelDrvrAUTHOR);
MODULE_LICENSE("GPL");

/**
 * =========================================================
 * @brief Validate insmod args
 * @return 1=OK 0=Bad
 */

static int check_args(char *name)
{
	int i, lun;

	if ((luns_num <= 0)
	    || (luns_num > MAX_DEVS)) {
		printk("%s:bad LUN count:%d, out of [1..%d] range\n",
		       name, luns_num, MAX_DEVS);
		return 0;
	}
	if (((vme1_num != 0) && (luns_num != vme1_num))
	    || ((vme2_num != 0) && (luns_num != vme2_num))
	    || ((vecs_num != 0) && (luns_num != vecs_num))) {
		printk
		    ("%s:bad VME par count vme1:%d vme2:%d vecs:%d should be:%d\n",
		     name, vme1_num, vme2_num, vecs_num, luns_num);
		return 0;
	}
	for (i = 0; i < luns_num; i++) {
		lun = luns[i];
		if ((lun < 0) || (lun >= MAX_DEVS)) {
			printk("%s:LUN:%d@%d out of [0..%d]\n",
			       name, lun, i, MAX_DEVS - 1);
			return 0;
		}
	}
	return 1;
}

/**
 * =========================================================
 * @brief Build module address list
 * @param i Module index
 * @return List of addresses for given lun
 */

InsLibVmeAddressSpace *build_address(int i)
{

	InsLibVmeAddressSpace *ah = NULL, *ap, *apn;

	if (vme1_num) {

		ah = kmalloc(sizeof(InsLibVmeAddressSpace), GFP_KERNEL);
		if (ah == NULL)
			return NULL;

		ap = ah;

		ap->AddressModifier = SkelDrvrAMD1;
		ap->WindowSize = SkelDrvrWIN1;
		ap->DataWidth = SkelDrvrDWIDTH1;
		ap->Endian = InsLibEndianBIG;
		ap->BaseAddress = vme1[i];
		ap->FreeModifierFlag = SkelDrvrFREE1;
		strcpy(ap->Comment, SkelDrvrADDR_COMMENT_1);
		ap->Next = NULL;
	}

	if ((vme1_num) && (vme2_num)) {

		apn = kmalloc(sizeof(InsLibVmeAddressSpace), GFP_KERNEL);
		if (apn == NULL)
			return NULL;

		ap->Next = apn;
		ap = apn;

		ap->AddressModifier = SkelDrvrAMD2;
		ap->WindowSize = SkelDrvrWIN2;
		ap->DataWidth = SkelDrvrDWIDTH2;
		ap->Endian = InsLibEndianBIG;
		ap->BaseAddress = vme2[i];
		ap->FreeModifierFlag = SkelDrvrFREE2;
		strcpy(ap->Comment, SkelDrvrADDR_COMMENT_2);
		ap->Next = NULL;
	}
	return ah;
}

/**
 * =========================================================
 * @brief Build module list
 * @return List of modules
 */

InsLibModlDesc *build_modules(void)
{

	InsLibModlDesc *mh, *mp, *nmp = NULL;
	InsLibIntrDesc *ip;
	InsLibVmeModuleAddress *vma;
	int i;

	mh = kmalloc(sizeof(InsLibModlDesc), GFP_KERNEL);
	if (mh == NULL)
		return NULL;
	mp = mh;

	for (i = 0; i < luns_num; i++) {

		mp->BusType = InsLibBusTypeVME;
		mp->ModuleNumber = luns[i];
		mp->IgnoreErrors = 0;

		if (vecs_num) {
			ip = kmalloc(sizeof(InsLibIntrDesc), GFP_KERNEL);
			if (ip) {
				ip->IsrFlag = 1;
				ip->Vector = vecs[i];
				ip->Level = SkelDrvrLEVEL;
				strcpy(ip->Comment,
				       SkelDrvrINTERRUPT_COMMENT);
			}
		} else
			ip = NULL;

		mp->Isr = ip;
		mp->Extra = NULL;
		strcpy(mp->Comment, SkelDrvrMODULE_COMMENT);
		strcpy(mp->ModuleName, SkelDrvrMODULE_NAME);

		vma = kmalloc(sizeof(InsLibVmeModuleAddress), GFP_KERNEL);
		if (vma) {
			strcpy(vma->Comment,
			       SkelDrvrMODULE_ADDRESS_COMMENT);
			vma->VmeAddressSpace = build_address(i);
		}
		mp->ModuleAddress = vma;

		if (i == luns_num - 1)
			nmp = NULL;
		else
			nmp = kmalloc(sizeof(InsLibModlDesc), GFP_KERNEL);

		mp->Next = nmp;
		mp = nmp;
	}
	return mh;
}

/**
 * =========================================================
 * @brief Build an inslib tree
 * @return driver tree or NULL
 */

InsLibDrvrDesc *build_drvr(void)
{

	InsLibDrvrDesc *dp;
	int cc;

	cc = check_args(SkelDrvrNAME);
	if (!cc)
		return NULL;

	dp = kmalloc(sizeof(InsLibDrvrDesc), GFP_KERNEL);
	if (dp == NULL)
		return NULL;

	dp->Next = NULL;
	if (!strlen(dname))
		strcpy(dname, SkelDrvrNAME);
	strcpy(dp->DrvrName, dname);
	dp->DebugFlags = 0;
	dp->EmulationFlags = 0;
	dp->ModuleCount = luns_num;
	strcpy(dp->Comment, SkelDrvrCOMMENT);
	dp->isdg = 0;
	dp->Modules = build_modules();
#ifdef MODBUG
	InsLibPrintDriver(dp);
#endif
	return dp;
}
#endif				/* __linux__ */

/**
 * =========================================================
 * @brief fill in a driver tree with constants from skeluser
 * @param dp point to the tree to be filled.
 *
 * The driver tree under LynxOs may be built by an insmod
 * look alike called "insmAd" that takes the same set of
 * module parameters as the Linux insmod. As insmAd can
 * not know what address modifiers to put in the tree, this
 * must be done later by the driver itself calling this
 * set_tree_defaults function that will use the constants
 * defined in skeluser.h
 */

void set_tree_defaults(InsLibDrvrDesc * dp)
{
	InsLibModlDesc *mp;
	InsLibIntrDesc *ip;
	InsLibVmeModuleAddress *vma;
	InsLibVmeAddressSpace *ap;

	if (!strlen(dname))
		strcpy(dname, SkelDrvrNAME);
	strcpy(dp->DrvrName, dname);
	strcpy(dp->Comment, SkelDrvrCOMMENT);
	mp = dp->Modules;
	while (mp) {
		strcpy(mp->Comment, SkelDrvrMODULE_COMMENT);
		strcpy(mp->ModuleName, SkelDrvrMODULE_NAME);
		ip = mp->Isr;
		if (ip) {
			ip->Level = SkelDrvrLEVEL;
			strcpy(ip->Comment, SkelDrvrINTERRUPT_COMMENT);
		}
		vma = mp->ModuleAddress;
		if (vma) {
			strcpy(vma->Comment,
			       SkelDrvrMODULE_ADDRESS_COMMENT);
			ap = vma->VmeAddressSpace;
			if (ap) {
				ap->AddressModifier = SkelDrvrAMD1;
				ap->WindowSize = SkelDrvrWIN1;
				ap->DataWidth = SkelDrvrDWIDTH1;
				ap->Endian = InsLibEndianBIG;
				ap->FreeModifierFlag = SkelDrvrFREE1;
				strcpy(ap->Comment,
				       SkelDrvrADDR_COMMENT_1);
				ap = ap->Next;
			}
			if (ap) {
				ap->AddressModifier = SkelDrvrAMD2;
				ap->WindowSize = SkelDrvrWIN2;
				ap->DataWidth = SkelDrvrDWIDTH2;
				ap->Endian = InsLibEndianBIG;
				ap->FreeModifierFlag = SkelDrvrFREE2;
				strcpy(ap->Comment,
				       SkelDrvrADDR_COMMENT_2);
			}
		}
		mp = mp->Next;
	}
#ifdef MODBUG
	InsLibPrintDriver(dp);
#endif

}
#endif				/* CONFIG_BUS_VME */

#ifdef CONFIG_BUS_PCI
#ifdef __linux__
InsLibDrvrDesc *build_drvr(void)
{
	return NULL;
}
#endif
void set_tree_defaults(InsLibDrvrDesc * dp)
{
	return;
}
#endif

#ifdef CONFIG_BUS_CAR
#ifdef __linux__
InsLibDrvrDesc *build_drvr(void)
{
	return NULL;
}
#endif
void set_tree_defaults(InsLibDrvrDesc * dp)
{
	return;
}
#endif
