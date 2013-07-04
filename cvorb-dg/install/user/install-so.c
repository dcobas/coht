/* install program UEP's */

#include <dg/modinst.h>
#include <dg/dal.h>

/*

 *WARNING*

  DAL is linked with this .so library, so that in can be used here.
  DAL in turn is using drvrutil library in case of ppc4.

  Workaround, that allows this .so to be load
  from within installation program is needed.
  Whithout it -- dlopen will fail because of unresovled symbols.
  dlerror reports -- Symbol not found: "IocModulPointer" "IocGetErrorMessage"

  This functions are called only if DaEnableAccess() is called with IOMMAP
  requested as an access method.

  So *DO NOT* call DaEnableAccess() with IOMMAP parameter!
  Use *ONLY* IOCTL access method!

  Otherwise two stub functions will be called, which is not what you'll expect.
*/
#ifdef __Lynx__
int __attribute__((weak)) IocModulPointer(int d1, int d2, int d3, char** d4)
{
        return 0;
}

void __attribute__((weak)) IocGetErrorMessage(int d1, char **d2)
{
        return;
}
#endif

int Cvorb_inst_prologue(struct list_head *md_list)
{
	/* WARNING: Driver is still not installed in this point of time */
	return 0;
}

int Cvorb_inst_epilogue(struct list_head *md_list)
{
	/* WARNING: Don't call DaEnableAccess() with IOMMAP parameter!
	   Use *ONLY* IOCTL access method!

	   Driver already installed */
	return 0;
}
