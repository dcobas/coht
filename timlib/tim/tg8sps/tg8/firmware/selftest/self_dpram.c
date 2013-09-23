/*****************************************************************/
/* DPRAM test                                                    */
/*****************************************************************/

DpramTest() {
union {
  unsigned char  *b;
  unsigned short *w;
  unsigned int   *l;
} m;
int i,t,ln,tem,ph,fr,to,dir; unsigned char *dpmb;

  sim->SimProtect = Tg8ENABLE_BUSMONITOR;
  bus_int = 0;

  dpmb = (unsigned char*) dpm;
  dpm  = &dpr;

  dpm->Info.Dpram.Mem.Bot = 0;
  dpm->Info.Dpram.Mem.Top = Tg8DPRAM_SIZE-6; /* Except of special 3 short locations */
#if 0
  if (bus_int || dpm->Info.Dpram.Mem.Top != Tg8DPRAM_SIZE-6) {
    /* DPRAM fails, use the local RAM instead of it */
    dpm= &dpr;
    dpm->Info.Dpram.Mem.Bot = 0;
    dpm->Info.Dpram.Mem.Top = Tg8DPRAM_SIZE-6;
  };
#endif
  dpm->Info.Dpram.Err.At = 0;
  dpm->Info.TestProg = T_DPRAM;

  for (dir=D_FORWARD; dir<N_OF_DIR-1; dir++) { /* Direction: 0=Forward; 1=Backward */

    for (t=A_WORD; t<N_OF_ACC; t++) { /* Access type: Word, Int32 */
      dpm->Info.Dpram.Err.Dir = dir | (t<<8);
      ln = 1<<t;
      
      if (dir==D_FORWARD) {
	fr = 0;
	to = dpm->Info.Dpram.Mem.Top;
      } else {
	fr = dpm->Info.Dpram.Mem.Top-ln;
	to = 0-ln;
	ln = -ln;
      };
      
      for (i=fr; i!=to; i+=ln) { /* DP Memory addresses */
	if (dir==D_FORWARD && i>to || dir==D_BACKWARD && i<0) break; /* Done */

	m.b = dpmb + i;
	ph  = TM_AA; /* Step 1 ... write 0xAA */
	tem = 0xAAAAAAAA;
	
testa:
	dpm->Info.Dpram.Err.Code  = ph;

	if (t==A_WORD) *m.w = (tem&=0xFFFF);
	else if (t==A_INT32) *m.l = tem;
	
	if (bus_int) {
	  dpm = &dpr;
	  dpm->Info.Dpram.Err.BusInt = BI_DPRAM_TEST;
	  dpm->Info.Dpram.Err.N_of_bus = bus_int;
	  dpm->Info.N_of_bus += bus_int;
	  goto faila;
	};

	if (t==A_WORD && *m.w != tem) {
faila:    dpm = &dpr;
	  dpm->Info.FaultType |= T_DPRAM;
	  dpm->Info.FaultCnt++;
	  dpm->Info.Dpram.Err.Templ = tem;
	  dpm->Info.Dpram.Err.Actual= *m.l;
	  dpm->Info.Dpram.Err.At  = i;
	  dpm->Info.Dpram.Err.Dir = dir | (t<<8);
	  dpm->Info.Dpram.Err.Code= ph;
	  return;
	};
	if (t==A_INT32 && *m.l != tem) goto faila;
	
	if (++ph == N_OF_TEMPL) continue;

	switch (ph) {
	case TM_55: /* write 0x55 */
	  tem = 0x55555555;
	  break;
	case TM_00: /* write 0x00 */
	  tem = 0;
	  break;
	};
	goto testa;
      };
    };
  };
  dpm->Info.TestPassed |= T_DPRAM;
}

/* eof */
