/*****************************************************************/
/* RAM test                                                      */
/*****************************************************************/

RamTest() {
union {
  unsigned char  *b;
  unsigned short *w;
  unsigned int   *l;
} m;
int i,t,ln,tem,ph,fr,to,dir;

  bus_int = 0;

  dpm->Info.Ram.Mem.Bot = RAM_BTM;
  dpm->Info.Ram.Mem.Top = RAM_TOP;
  dpm->Info.Ram.Err.At = 0;
  dpm->Info.TestProg = T_RAM;

  for (dir=D_FORWARD; dir<N_OF_DIR-1; dir++) { /* Direction: Forward; Backward */

    for (t=A_BYTE+1; t<N_OF_ACC; t++) { /* Access type: Word, Int32 */
      dpm->Info.Ram.Err.Dir = dir | (t<<8);
      ln = 1<<t;
      
      if (dir==D_FORWARD) {
	fr = RAM_BTM;
	to = RAM_TOP;
      } else {
	fr = RAM_TOP-ln;
	to = RAM_BTM-ln;
	ln = -ln;
      };

      for (i=fr; i!=to; i+=ln) { /* Memory addresses */
	dpm->Info.Ram.Err.At = i;

	m.b = (unsigned char*) i;

	ph  = TM_AA; /* Step 1 ... write 0xAA */
	tem = 0xAAAAAAAA;
	
testa:
	dpm->Info.Ram.Err.Code  = ph;

	if      (t==A_BYTE) *m.b = (tem&=0xFF);
	else if (t==A_WORD) *m.w = (tem&=0xFFFF);
	else if (t==A_INT32) *m.l = tem;
	
	if (bus_int) {
	  dpm->Info.Ram.Err.BusInt = BI_RAM_TEST;
	  dpm->Info.Ram.Err.N_of_bus = bus_int;
	  dpm->Info.N_of_bus += bus_int;
	  goto faila;
	};

	if (t==A_BYTE && *m.b != tem) {
faila:    dpm->Info.FaultType |= T_RAM;
	  dpm->Info.FaultCnt++;
	  dpm->Info.Ram.Err.Templ = tem;
	  dpm->Info.Ram.Err.Actual= *m.l;
	  return;
	};
	if (t==A_WORD && *m.w != tem) goto faila;
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
  dpm->Info.TestPassed |= T_RAM;
}

/* eof */
