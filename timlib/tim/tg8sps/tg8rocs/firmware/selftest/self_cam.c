/*****************************************************************/
/* CAM test                                                      */
/*****************************************************************/

CamTest() {
int i,tem; short st,adr,nm,dir,t, inv=0x5A5A;
unsigned short wb[3],rb[3];

  bus_int = 0;

  dpm->Info.Cam.Mem.Bot = 0;
  dpm->Info.Cam.Mem.Top = Tg8CAM_SIZE;
  dpm->Info.Cam.Err.At = 0;
  dpm->Info.TestProg = T_CAM;
  dpm->Info.Cam.Err.Code = E_CAM_RW;

  dir = D_FORWARD;
  for (i=0,tem=0; i<Tg8CAM_SIZE; i++,tem++) {

    dpm->Info.Cam.Err.At = i;
    t = TC_A; /* The 1st test selected */
    wb[0]= tem; wb[1]= ~tem; wb[2]= tem^inv;
    CamWriteArray(i,wb);
    CamReadArray(i,rb);

    if (bus_int) {
bi_cam:
      dpm->Info.Cam.Err.BusInt = BI_CAM_TEST;
      dpm->Info.Cam.Err.N_of_bus = bus_int;
      dpm->Info.N_of_bus += bus_int;
      goto faila;
    };

    if (rb[0]!=wb[0] || rb[1]!=wb[1] ||rb[2]!=wb[2]) {
faila: dpm->Info.FaultType |= T_CAM;
       dpm->Info.FaultCnt++;
       dpm->Info.Cam.Err.Templ = wb[0];
       dpm->Info.Cam.Err.Actual= rb[0];
       dpm->Info.Cam.Err.Dir = dir | (t<<8);
       return;
    };
/** Skip the test type B.
    t = TC_B;
    wb[0]= ~tem; wb[1]= tem^inv; wb[2]= tem;
    CamWriteArray(i,wb);
    CamReadArray(i,rb);
    if (rb[0]!=wb[0] || rb[1]!=wb[1] ||rb[2]!=wb[2]) goto faila;
**/
  };

  dir = D_BACKWARD;
  for (i=Tg8CAM_SIZE,tem=0; i>0; tem++) {
    dpm->Info.Cam.Err.At = --i;
    t = TC_C; /* Test selected */
    wb[0]= ~tem; wb[1]= tem; wb[2]= tem^inv;
    CamWriteArray(i,wb);
    CamReadArray(i,rb);

    if (bus_int) {
      dpm->Info.Cam.Err.BusInt = BI_CAM_TEST;
      dpm->Info.Cam.Err.N_of_bus = bus_int;
      dpm->Info.N_of_bus += bus_int;
      goto faila;
    };
    if (rb[0]!=wb[0] || rb[1]!=wb[1] ||rb[2]!=wb[2]) goto faila;
/** Skip the test type D.
    t = TC_D;
    wb[0]= tem^inv; wb[1]= tem; wb[2]= ~tem;
    CamWriteArray(i,wb);
    CamReadArray(i,rb);
    if (rb[0]!=wb[0] || rb[1]!=wb[1] ||rb[2]!=wb[2]) goto faila;
**/
    /* Data for the matching following */
    t = TC_E;
    wb[0]= tem; wb[1]= tem^inv; wb[2]= ~tem;
    CamWriteArray(i,wb);
    CamReadArray(i,rb);
    if (rb[0]!=wb[0] || rb[1]!=wb[1] ||rb[2]!=wb[2]) goto faila;
  };

  /* Look for matching. Templated where written by the test E right now. */

  dpm->Info.Cam.Err.Code = E_CAM_MATCH;
  dpm->Info.Cam.Err.Dir = dir | (t<<8);

  for (i=Tg8CAM_SIZE,tem=0; i>0; tem++) {
    dpm->Info.Cam.Err.At = --i;
    nm = 0;
    wb[0]= tem; wb[1]= tem^inv; wb[2]= ~tem;
    CamMatch(wb); /* Start the matching */
    for (;;) {
      asm volatile (" movel d0,d0"); /* The matching is in progress: wait  */
      st = *cam;                     /* Read the CAM status */
      if (st & CamMATCH) break;      /* No more matches */
      adr = st & 0xFF;               /* Take an address */
      dpm->Info.Cam.Match.At = adr;
      dpm->Info.Cam.Match.Cnt++;
      nm++;                               /* How many matches */
      if (st & CamSTATUS_MULTIPLE) break; /* Single match found */
      *cam = adr | CamSET_SKIP_BIT;       /* Continue matching  */
    };
    /* Check the result */
    if (nm != 1 ||
        dpm->Info.Cam.Err.At != dpm->Info.Cam.Match.At) goto faila;
    /* Continue with the next template */
  };

  dpm->Info.TestPassed |= T_CAM;
  dpm->Info.Cam.Err.Code = 0;
}

/* eof */
