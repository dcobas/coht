#NO_APP
gcc2_compiled.:
___gnu_compiled_c:
#APP
	
  CONVERTOR                       =1	/* 0x0001 	1	 |NUM | */
  TG8                             =0	/* 0x0000 	0	 |NUM | */
  TG8PUB_H                        =0	/* 0x0000 	0	 |NUM | */
  _MVME_167_                      =0	/* 0x0000 	0	 |NUM | */
  _PS_                            =0	/* 0x0000 	0	 |NUM | */
  DEBUG                           =1	/* 0x0001 	1	 |NUM | */
  TG8_TYPES                       =0	/* 0x0000 	0	 |NUM | */

  Uchar                           =1	/* 0x0001 	1	 |TYPE| char */

  Byte                            =1	/* 0x0001 	1	 |TYPE| char */

  Ushort                          =2	/* 0x0002 	2	 |TYPE| short */

  Word                            =2	/* 0x0002 	2	 |TYPE| short */

  Uint                            =4	/* 0x0004 	4	 |TYPE| int */

  DWord                           =4	/* 0x0004 	4	 |TYPE| int */
  Tg8NO_MACHINE                   =0	/* 0x0000 	0	 |NUM | */

  Tg8Machine                      =4	/* 0x0004 	4	 |TYPE|  */
  Tg8LHC                          =1	/* 0x0001 	1	 |NUM | */
  Tg8SPS                          =2	/* 0x0002 	2	 |NUM | */
  Tg8CPS                          =3	/* 0x0003 	3	 |NUM | */
  Tg8PSB                          =4	/* 0x0004 	4	 |NUM | */
  Tg8LEA                          =5	/* 0x0005 	5	 |NUM | */
  Tg8MACHINES                     =6	/* 0x0006 	6	 |NUM | */
  Tg8MILLISECOND_HEADER           =1	/* 0x0001 	1	 |NUM | */

  Tg8EventHeader                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SECOND_HEADER                =2	/* 0x0002 	2	 |NUM | */
  Tg8MINUTE_HEADER                =3	/* 0x0003 	3	 |NUM | */
  Tg8HOUR_HEADER                  =4	/* 0x0004 	4	 |NUM | */
  Tg8DAY_HEADER                   =5	/* 0x0005 	5	 |NUM | */
  Tg8MONTH_HEADER                 =6	/* 0x0006 	6	 |NUM | */
  Tg8YEAR_HEADER                  =7	/* 0x0007 	7	 |NUM | */
  Tg8psTIME_HEADER                =8	/* 0x0008 	8	 |NUM | */
  Tg8psDATE_HEADER                =9	/* 0x0009 	9	 |NUM | */
  TgvUTC_LOW_HEADER               =181	/* 0x00b5 	181	 |NUM | */
  TgvUTC_HIGH_HEADER              =182	/* 0x00b6 	182	 |NUM | */
  Tg8SSC_HEADER                   =0	/* 0x0000 	0	 |NUM | */

  Tg8MachineEventHeader           =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SUPER_CYCLE_HEADER           =2	/* 0x0002 	2	 |NUM | */
  Tg8TELEGRAM_HEADER              =3	/* 0x0003 	3	 |NUM | */
  Tg8SIMPLE_EVENT_HEADER          =1	/* 0x0001 	1	 |NUM | */
  Tg8psTCU_EVENT_HEADER           =5	/* 0x0005 	5	 |NUM | */
  Tg8SPS_SSC_HEADER               =32	/* 0x0020 	32	 |NUM | */
  Tg8LHC_SIMPLE_HEADER            =17	/* 0x0011 	17	 |NUM | */
  Tg8SPS_SIMPLE_HEADER            =33	/* 0x0021 	33	 |NUM | */
  Tg8CPS_SIMPLE_HEADER            =49	/* 0x0031 	49	 |NUM | */
  Tg8PSB_SIMPLE_HEADER            =65	/* 0x0041 	65	 |NUM | */
  Tg8LEA_SIMPLE_HEADER            =81	/* 0x0051 	81	 |NUM | */
  Tg8CPS_TCU_HEADER               =53	/* 0x0035 	53	 |NUM | */
  Tg8PSB_TCU_HEADER               =69	/* 0x0045 	69	 |NUM | */
  Tg8LEA_TCU_HEADER               =85	/* 0x0055 	85	 |NUM | */
  Tg8LHC_TELEGRAM_HEADER          =19	/* 0x0013 	19	 |NUM | */
  Tg8SPS_TELEGRAM_HEADER          =35	/* 0x0023 	35	 |NUM | */
  Tg8CPS_TELEGRAM_HEADER          =51	/* 0x0033 	51	 |NUM | */
  Tg8PSB_TELEGRAM_HEADER          =67	/* 0x0043 	67	 |NUM | */
  Tg8LEA_TELEGRAM_HEADER          =83	/* 0x0053 	83	 |NUM | */
  Tg8READY_TELEGRAM               =254	/* 0x00fe 	254	 |NUM | */
  Tg8GROUPS                       =24	/* 0x0018 	24	 |NUM | */

  Tg8LongEvent                    =4	/* 0x0004 	4	 |TYPE| long */

  Tg8ShortEvent                   =4	/* 0x0004 	4	 |TYPE|  */
  Event_half1                     =0	/* 0x0000 	0	 |VAR | Tg8ShortEvent.short 	=2= */
  Event_half2                     =2	/* 0x0002 	2	 |VAR | Tg8ShortEvent.short 	=2= */

  Tg8AnyEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_2                          =1	/* 0x0001 	1	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8AnyEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8AnyEvent.char 	=1= */

  Tg8SimpleEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SimpleEvent.char 	=1= */
  Event_code                      =1	/* 0x0001 	1	 |VAR | Tg8SimpleEvent.char 	=1= */
  Cycle_type                      =2	/* 0x0002 	2	 |VAR | Tg8SimpleEvent.char 	=1= */
  Cycle_number                    =3	/* 0x0003 	3	 |VAR | Tg8SimpleEvent.char 	=1= */

  Tg8SscEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_low                         =1	/* 0x0001 	1	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_mid                         =2	/* 0x0002 	2	 |VAR | Tg8SscEvent.char 	=1= */
  Scn_msb                         =3	/* 0x0003 	3	 |VAR | Tg8SscEvent.char 	=1= */

  Tg8MillisecondEvent             =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Ticks_1ms                       =1	/* 0x0001 	1	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Dcare_1                         =2	/* 0x0002 	2	 |VAR | Tg8MillisecondEvent.char 	=1= */
  Dcare_2                         =3	/* 0x0003 	3	 |VAR | Tg8MillisecondEvent.char 	=1= */

  Tg8SecondEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8SecondEvent.char 	=1= */
  Tg8_second                      =1	/* 0x0001 	1	 |VAR | Tg8SecondEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8SecondEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8SecondEvent.char 	=1= */

  Tg8MinuteEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MinuteEvent.char 	=1= */
  Tg8_minute                      =1	/* 0x0001 	1	 |VAR | Tg8MinuteEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8MinuteEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8MinuteEvent.char 	=1= */

  Tg8HourEvent                    =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8HourEvent.char 	=1= */
  Tg8_hour                        =1	/* 0x0001 	1	 |VAR | Tg8HourEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8HourEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8HourEvent.char 	=1= */

  Tg8DayEvent                     =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8DayEvent.char 	=1= */
  Tg8_day                         =1	/* 0x0001 	1	 |VAR | Tg8DayEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8DayEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8DayEvent.char 	=1= */

  Tg8MonthEvent                   =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8MonthEvent.char 	=1= */
  Tg8_month                       =1	/* 0x0001 	1	 |VAR | Tg8MonthEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8MonthEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8MonthEvent.char 	=1= */

  Tg8YearEvent                    =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8YearEvent.char 	=1= */
  Tg8_year                        =1	/* 0x0001 	1	 |VAR | Tg8YearEvent.char 	=1= */
  Byte_3                          =2	/* 0x0002 	2	 |VAR | Tg8YearEvent.char 	=1= */
  Byte_4                          =3	/* 0x0003 	3	 |VAR | Tg8YearEvent.char 	=1= */

  Tg8psDateEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8psDateEvent.char 	=1= */
  psYear                          =1	/* 0x0001 	1	 |VAR | Tg8psDateEvent.char 	=1= */
  psMonth                         =2	/* 0x0002 	2	 |VAR | Tg8psDateEvent.char 	=1= */
  psDay                           =3	/* 0x0003 	3	 |VAR | Tg8psDateEvent.char 	=1= */

  Tg8psTimeEvent                  =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8psTimeEvent.char 	=1= */
  psHour                          =1	/* 0x0001 	1	 |VAR | Tg8psTimeEvent.char 	=1= */
  psMinute                        =2	/* 0x0002 	2	 |VAR | Tg8psTimeEvent.char 	=1= */
  psSecond                        =3	/* 0x0003 	3	 |VAR | Tg8psTimeEvent.char 	=1= */

  Tg8TelegramEvent                =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | Tg8TelegramEvent.char 	=1= */
  Group_number                    =1	/* 0x0001 	1	 |VAR | Tg8TelegramEvent.char 	=1= */
  Group_value                     =2	/* 0x0002 	2	 |VAR | Tg8TelegramEvent.short 	=2= */

  TgvUtc                          =4	/* 0x0004 	4	 |TYPE|  */
  Header                          =0	/* 0x0000 	0	 |VAR | TgvUtc.char 	=1= */
  Junk                            =1	/* 0x0001 	1	 |VAR | TgvUtc.char 	=1= */
  UtcWord                         =2	/* 0x0002 	2	 |VAR | TgvUtc.short 	=2= */

  Tg8Event                        =4	/* 0x0004 	4	 |TYPE|  */
  Any                             =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8AnyEvent 	=4= */
  Long                            =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8LongEvent 	=4= */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8ShortEvent 	=4= */
  Simple                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SimpleEvent 	=4= */
  Ssc                             =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SscEvent 	=4= */
  Millisecond                     =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MillisecondEvent 	=4= */
  Tg8Second                       =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8SecondEvent 	=4= */
  Tg8Minute                       =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MinuteEvent 	=4= */
  Tg8Hour                         =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8HourEvent 	=4= */
  Tg8Day                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8DayEvent 	=4= */
  Tg8Month                        =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8MonthEvent 	=4= */
  Tg8Year                         =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8YearEvent 	=4= */
  psDate                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8psDateEvent 	=4= */
  psTime                          =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8psTimeEvent 	=4= */
  Telegram                        =0	/* 0x0000 	0	 |VAR | Tg8Event.Tg8TelegramEvent 	=4= */
  Utc                             =0	/* 0x0000 	0	 |VAR | Tg8Event.TgvUtc 	=4= */
  Tg8DONT_CARE                    =255	/* 0x00ff 	255	 |NUM | */
  Tg8DISABLED                     =0	/* 0x0000 	0	 |NUM | */

  Tg8ActionState                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ENABLED                      =1	/* 0x0001 	1	 |NUM | */
  Tg8INTERRUPT                    =2	/* 0x0002 	2	 |NUM | */
  Tg8NO_INTERRUPT                 =3	/* 0x0003 	3	 |NUM | */
  Tg8PPMT_DISABLED                =4	/* 0x0004 	4	 |NUM | */
  Tg8PPMT_ENABLED                 =5	/* 0x0005 	5	 |NUM | */
  Tg8GT_NUM                       =1	/* 0x0001 	1	 |NUM | */

  Tg8GroupType                    =4	/* 0x0004 	4	 |TYPE|  */
  Tg8GT_EXC                       =2	/* 0x0002 	2	 |NUM | */
  Tg8GT_BIT                       =3	/* 0x0003 	3	 |NUM | */
  Tg8ACTIONS                      =256	/* 0x0100 	256	 |NUM | */
  Tg8PPM_LINES                    =24	/* 0x0018 	24	 |NUM | */
  Tg8CW_INT_BITN                  =14	/* 0x000e 	14	 |NUM | */
  Tg8CW_INT_BITM                  =3	/* 0x0003 	3	 |NUM | */
  Tg8DO_NOTHING                   =0	/* 0x0000 	0	 |NUM | */

  Tg8ResultMode                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DO_OUTPUT                    =1	/* 0x0001 	1	 |NUM | */
  Tg8DO_INTERRUPT                 =2	/* 0x0002 	2	 |NUM | */
  Tg8DO_OUTPUT_AND_INTERRUPT      =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_CNT_BITN                  =8	/* 0x0008 	8	 |NUM | */
  Tg8CW_CNT_BITM                  =7	/* 0x0007 	7	 |NUM | */
  Tg8CW_START_BITN                =6	/* 0x0006 	6	 |NUM | */
  Tg8CW_START_BITM                =3	/* 0x0003 	3	 |NUM | */
  Tg8CM_NORMAL                    =0	/* 0x0000 	0	 |NUM | */

  Tg8CounterMode                  =4	/* 0x0004 	4	 |TYPE|  */
  Tg8CM_CHAINED                   =1	/* 0x0001 	1	 |NUM | */
  Tg8CM_EXTERNAL                  =2	/* 0x0002 	2	 |NUM | */
  Tg8CW_PPML_BITN                 =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_PPML_BITM                 =3	/* 0x0003 	3	 |NUM | */
  Tg8TM_NORMAL                    =0	/* 0x0000 	0	 |NUM | */

  Tg8TimingType                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8TM_PPM                       =1	/* 0x0001 	1	 |NUM | */
  Tg8TM_LINE                      =3	/* 0x0003 	3	 |NUM | */
  Tg8CW_STATE_BITN                =2	/* 0x0002 	2	 |NUM | */
  Tg8CW_STATE_BITM                =1	/* 0x0001 	1	 |NUM | */
  Tg8CW_CLOCK_BITN                =0	/* 0x0000 	0	 |NUM | */
  Tg8CW_CLOCK_BITM                =3	/* 0x0003 	3	 |NUM | */
  Tg8CLK_MILLISECOND              =0	/* 0x0000 	0	 |NUM | */

  Tg8ClockTrain                   =4	/* 0x0004 	4	 |TYPE|  */
  Tg8CLK_CABLE                    =1	/* 0x0001 	1	 |NUM | */
  Tg8CLK_X1                       =2	/* 0x0002 	2	 |NUM | */
  Tg8CLK_X2                       =3	/* 0x0003 	3	 |NUM | */

  Tg8User                         =8	/* 0x0008 	8	 |TYPE|  */
  uEvent                          =0	/* 0x0000 	0	 |VAR | Tg8User.Tg8Event 	=4= */
  uControl                        =4	/* 0x0004 	4	 |VAR | Tg8User.Word 	=2= */
  uDelay                          =6	/* 0x0006 	6	 |VAR | Tg8User.Word 	=2= */

  Tg8Gate                         =4	/* 0x0004 	4	 |TYPE|  */
  Machine                         =0	/* 0x0000 	0	 |VAR | Tg8Gate.Byte 	=1= */
  GroupNum                        =1	/* 0x0001 	1	 |VAR | Tg8Gate.Byte 	=1= */
  GroupVal                        =2	/* 0x0002 	2	 |VAR | Tg8Gate.Word 	=2= */

  Tg8PpmLine                      =12	/* 0x000c 	12	 |TYPE|  */
  Action                          =0	/* 0x0000 	0	 |VAR | Tg8PpmLine.Tg8User 	=8= */
  Gate                            =8	/* 0x0008 	8	 |VAR | Tg8PpmLine.Tg8Gate 	=4= */

  Tg8PpmUser                      =152	/* 0x0098 	152	 |TYPE|  */
  Trigger                         =0	/* 0x0000 	0	 |VAR | Tg8PpmUser.Tg8Event 	=4= */
  Machine                         =4	/* 0x0004 	4	 |VAR | Tg8PpmUser.Byte 	=1= */
  GroupNum                        =5	/* 0x0005 	5	 |VAR | Tg8PpmUser.Byte 	=1= */
  GroupType                       =6	/* 0x0006 	6	 |VAR | Tg8PpmUser.Byte 	=1= */
  Dim                             =7	/* 0x0007 	7	 |VAR | Tg8PpmUser.Byte 	=1= */
  LineGv                          =8	/* 0x0008 	8	 |VAR | Tg8PpmUser.Word[24] 	=48= */
  LineCw                          =56	/* 0x0038 	56	 |VAR | Tg8PpmUser.Word[24] 	=48= */
  LineDelay                       =104	/* 0x0068 	104	 |VAR | Tg8PpmUser.Word[24] 	=48= */

  Tg8Object                       =156	/* 0x009c 	156	 |TYPE|  */
  Id                              =0	/* 0x0000 	0	 |VAR | Tg8Object.int 	=4= */
  Timing                          =4	/* 0x0004 	4	 |VAR | Tg8Object.Tg8PpmUser 	=152= */
  Tg8SEL_STATE                    =1	/* 0x0001 	1	 |NUM | */

  Tg8Selector                     =4	/* 0x0004 	4	 |TYPE|  */
  Tg8SEL_DELAY                    =2	/* 0x0002 	2	 |NUM | */

  Tg8Recording                    =16	/* 0x0010 	16	 |TYPE|  */
  rSc                             =0	/* 0x0000 	0	 |VAR | Tg8Recording.Uint 	=4= */
  rOcc                            =4	/* 0x0004 	4	 |VAR | Tg8Recording.Uint 	=4= */
  rOut                            =8	/* 0x0008 	8	 |VAR | Tg8Recording.Uint 	=4= */
  rNumOfTrig                      =12	/* 0x000c 	12	 |VAR | Tg8Recording.Word 	=2= */
  rOvwrCnt                        =14	/* 0x000e 	14	 |VAR | Tg8Recording.Byte 	=1= */
  rOvwrAct                        =15	/* 0x000f 	15	 |VAR | Tg8Recording.Byte 	=1= */

  Tg8IntAction                    =4	/* 0x0004 	4	 |TYPE|  */
  iRcvErr                         =0	/* 0x0000 	0	 |VAR | Tg8IntAction.Byte 	=1= */
  iOvwrAct                        =1	/* 0x0001 	1	 |VAR | Tg8IntAction.Byte 	=1= */
  iAct                            =2	/* 0x0002 	2	 |VAR | Tg8IntAction.Byte 	=1= */
  iSem                            =3	/* 0x0003 	3	 |VAR | Tg8IntAction.Byte 	=1= */

  Tg8Interrupt                    =20	/* 0x0014 	20	 |TYPE|  */
  iEvent                          =0	/* 0x0000 	0	 |VAR | Tg8Interrupt.Tg8Event 	=4= */
  iSc                             =4	/* 0x0004 	4	 |VAR | Tg8Interrupt.Uint 	=4= */
  iOcc                            =8	/* 0x0008 	8	 |VAR | Tg8Interrupt.Uint 	=4= */
  iOut                            =12	/* 0x000c 	12	 |VAR | Tg8Interrupt.Uint 	=4= */
  iExt                            =16	/* 0x0010 	16	 |VAR | Tg8Interrupt.Tg8IntAction 	=4= */
  Tg8CHANNELS                     =8	/* 0x0008 	8	 |NUM | */
  Tg8IMM_INTS                     =8	/* 0x0008 	8	 |NUM | */

  Tg8InterruptTable               =320	/* 0x0140 	320	 |TYPE|  */
  CntInter                        =0	/* 0x0000 	0	 |VAR | Tg8InterruptTable.Tg8Interrupt[8] 	=160= */
  ImmInter                        =160	/* 0x00a0 	160	 |VAR | Tg8InterruptTable.Tg8Interrupt[8] 	=160= */
  Tg8HISTORIES                    =1000	/* 0x03e8 	1000	 |NUM | */

  Tg8History                      =16	/* 0x0010 	16	 |TYPE|  */
  hEvent                          =0	/* 0x0000 	0	 |VAR | Tg8History.Tg8Event 	=4= */
  hSc                             =4	/* 0x0004 	4	 |VAR | Tg8History.Uint 	=4= */
  hOcc                            =8	/* 0x0008 	8	 |VAR | Tg8History.Uint 	=4= */
  hRcvErr                         =12	/* 0x000c 	12	 |VAR | Tg8History.Byte 	=1= */
  hHour                           =13	/* 0x000d 	13	 |VAR | Tg8History.Byte 	=1= */
  hMinute                         =14	/* 0x000e 	14	 |VAR | Tg8History.Byte 	=1= */
  hSecond                         =15	/* 0x000f 	15	 |VAR | Tg8History.Byte 	=1= */
  Tg8CLOCKS                       =16	/* 0x0010 	16	 |NUM | */

  Tg8Clock                        =20	/* 0x0014 	20	 |TYPE|  */
  cMsEvent                        =0	/* 0x0000 	0	 |VAR | Tg8Clock.Tg8Event 	=4= */
  cNumOfMs                        =4	/* 0x0004 	4	 |VAR | Tg8Clock.Uint 	=4= */
  cSc                             =8	/* 0x0008 	8	 |VAR | Tg8Clock.Uint 	=4= */
  cOcc                            =12	/* 0x000c 	12	 |VAR | Tg8Clock.Uint 	=4= */
  cRcvErr                         =16	/* 0x0010 	16	 |VAR | Tg8Clock.Byte 	=1= */
  cHour                           =17	/* 0x0011 	17	 |VAR | Tg8Clock.Byte 	=1= */
  cMinute                         =18	/* 0x0012 	18	 |VAR | Tg8Clock.Byte 	=1= */
  cSecond                         =19	/* 0x0013 	19	 |VAR | Tg8Clock.Byte 	=1= */

  Tg8DateTime                     =12	/* 0x000c 	12	 |TYPE|  */
  aYear                           =0	/* 0x0000 	0	 |VAR | Tg8DateTime.Byte 	=1= */
  aMonth                          =1	/* 0x0001 	1	 |VAR | Tg8DateTime.Byte 	=1= */
  aDay                            =2	/* 0x0002 	2	 |VAR | Tg8DateTime.Byte 	=1= */
  aSpare1                         =3	/* 0x0003 	3	 |VAR | Tg8DateTime.Byte 	=1= */
  aRcvErr                         =4	/* 0x0004 	4	 |VAR | Tg8DateTime.Byte 	=1= */
  aHour                           =5	/* 0x0005 	5	 |VAR | Tg8DateTime.Byte 	=1= */
  aMinute                         =6	/* 0x0006 	6	 |VAR | Tg8DateTime.Byte 	=1= */
  aSecond                         =7	/* 0x0007 	7	 |VAR | Tg8DateTime.Byte 	=1= */
  aMilliSecond                    =8	/* 0x0008 	8	 |VAR | Tg8DateTime.Word 	=2= */
  aMsDrift                        =10	/* 0x000a 	10	 |VAR | Tg8DateTime.Word 	=2= */

  Tg8Aux                          =368	/* 0x0170 	368	 |TYPE|  */
  aEvent                          =0	/* 0x0000 	0	 |VAR | Tg8Aux.Tg8Event 	=4= */
  aMsEvent                        =4	/* 0x0004 	4	 |VAR | Tg8Aux.Tg8Event 	=4= */
  aScLen                          =8	/* 0x0008 	8	 |VAR | Tg8Aux.Uint 	=4= */
  aScTime                         =12	/* 0x000c 	12	 |VAR | Tg8Aux.Uint 	=4= */
  aSc                             =16	/* 0x0010 	16	 |VAR | Tg8Aux.Uint 	=4= */
  aNumOfSc                        =20	/* 0x0014 	20	 |VAR | Tg8Aux.Uint 	=4= */
  aTrace                          =24	/* 0x0018 	24	 |VAR | Tg8Aux.Word 	=2= */
  aIntSrc                         =26	/* 0x001a 	26	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfBus                       =28	/* 0x001c 	28	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfSpur                      =30	/* 0x001e 	30	 |VAR | Tg8Aux.Word 	=2= */
  aMbox                           =32	/* 0x0020 	32	 |VAR | Tg8Aux.Word 	=2= */
  aCoco                           =34	/* 0x0022 	34	 |VAR | Tg8Aux.short 	=2= */
  aFwStat                         =36	/* 0x0024 	36	 |VAR | Tg8Aux.Word 	=2= */
  aSscHeader                      =38	/* 0x0026 	38	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfEv                        =40	/* 0x0028 	40	 |VAR | Tg8Aux.Word 	=2= */
  aPrNumOfEv                      =42	/* 0x002a 	42	 |VAR | Tg8Aux.Word 	=2= */
  aNumOfAct                       =44	/* 0x002c 	44	 |VAR | Tg8Aux.Word 	=2= */
  aAlarms                         =46	/* 0x002e 	46	 |VAR | Tg8Aux.Word 	=2= */
  aDt                             =48	/* 0x0030 	48	 |VAR | Tg8Aux.Tg8DateTime 	=12= */
  aFwVer                          =60	/* 0x003c 	60	 |VAR | Tg8Aux.Byte[16] 	=16= */
  aSem                            =76	/* 0x004c 	76	 |VAR | Tg8Aux.Word 	=2= */
  aQueueSize                      =78	/* 0x004e 	78	 |VAR | Tg8Aux.Word 	=2= */
  aMaxQueueSize                   =80	/* 0x0050 	80	 |VAR | Tg8Aux.Word 	=2= */
  aMovedFrames                    =82	/* 0x0052 	82	 |VAR | Tg8Aux.Word 	=2= */
  aMovedScTime                    =84	/* 0x0054 	84	 |VAR | Tg8Aux.Uint 	=4= */
  aMovedSc                        =88	/* 0x0058 	88	 |VAR | Tg8Aux.Uint 	=4= */
  aProcFrames                     =92	/* 0x005c 	92	 |VAR | Tg8Aux.Word 	=2= */
  aModStatus                      =94	/* 0x005e 	94	 |VAR | Tg8Aux.Word 	=2= */
  aQueue                          =96	/* 0x0060 	96	 |VAR | Tg8Aux.Tg8Event[8] 	=32= */
  aTelegLHC                       =128	/* 0x0080 	128	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegSPS                       =176	/* 0x00b0 	176	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegCPS                       =224	/* 0x00e0 	224	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegPSB                       =272	/* 0x0110 	272	 |VAR | Tg8Aux.Word[24] 	=48= */
  aTelegLEA                       =320	/* 0x0140 	320	 |VAR | Tg8Aux.Word[24] 	=48= */
  Tg8FS_RUNNING                   =1	/* 0x0001 	1	 |NUM | */

  Tg8FirmwareStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8FS_ALARMS                    =2	/* 0x0002 	2	 |NUM | */
  Tg8ALARM_OK                     =0	/* 0x0000 	0	 |NUM | */

  Tg8Alarm                        =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ALARM_ISR                    =1	/* 0x0001 	1	 |NUM | */
  Tg8ALARM_LOST_IMM               =2	/* 0x0002 	2	 |NUM | */
  Tg8ALARM_LOST_OUT               =4	/* 0x0004 	4	 |NUM | */
  Tg8ALARM_MANY_PROC              =8	/* 0x0008 	8	 |NUM | */
  Tg8ALARM_QUEUED_PROC            =16	/* 0x0010 	16	 |NUM | */
  Tg8ALARM_DIFF_EVN               =32	/* 0x0020 	32	 |NUM | */
  Tg8ALARM_DIFF_LEN               =64	/* 0x0040 	64	 |NUM | */
  Tg8ALARM_MOVED_PROC             =128	/* 0x0080 	128	 |NUM | */
  Tg8ALARM_MOVED_IMM              =256	/* 0x0100 	256	 |NUM | */
  Tg8ALARM_ACT_DISBL              =512	/* 0x0200 	512	 |NUM | */
  Tg8ALARM_IMMQ_OVF               =1024	/* 0x0400 	1024	 |NUM | */
  Tg8ALARM_MBX_BUSY               =2048	/* 0x0800 	2048	 |NUM | */
  Tg8ALARM_UNCOM                  =4096	/* 0x1000 	4096	 |NUM | */
  Tg8ALARM_BAD_SWITCH             =8192	/* 0x2000 	8192	 |NUM | */
  Tg8ALARM_INT_LOST               =16384	/* 0x4000 	16384	 |NUM | */
  Tg8ST_OK                        =0	/* 0x0000 	0	 |NUM | */

  Tg8SelfTesrError                =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ERR_OK                       =0	/* 0x0000 	0	 |NUM | */

  Tg8Error                        =4	/* 0x0004 	4	 |TYPE|  */
  Tg8ERR_ILLEGAL_OPERATION        =-1	/* 0xffffffff 	-1	 |NUM | */
  Tg8ERR_ILLEGAL_ARG              =-2	/* 0xfffffffe 	-2	 |NUM | */
  Tg8ERR_TIMEOUT                  =-3	/* 0xfffffffd 	-3	 |NUM | */
  Tg8ERR_WRONG_DIM                =-4	/* 0xfffffffc 	-4	 |NUM | */
  Tg8ERR_WRONG_DELAY              =-5	/* 0xfffffffb 	-5	 |NUM | */
  Tg8ERR_WRONG_CLOCK              =-6	/* 0xfffffffa 	-6	 |NUM | */
  Tg8ERR_WRONG_MODE               =-7	/* 0xfffffff9 	-7	 |NUM | */
  Tg8ERR_REJECTED                 =-8	/* 0xfffffff8 	-8	 |NUM | */
  Tg8ERR_BAD_OBJECT               =-9	/* 0xfffffff7 	-9	 |NUM | */
  Tg8ERR_NO_ACK                   =-10	/* 0xfffffff6 	-10	 |NUM | */
  Tg8ERR_NOT_RUN                  =-11	/* 0xfffffff5 	-11	 |NUM | */
  Tg8ERR_NO_FILE                  =-12	/* 0xfffffff4 	-12	 |NUM | */
  Tg8ERR_PENDING                  =-13	/* 0xfffffff3 	-13	 |NUM | */
  Tg8ERR_BUS_ERR                  =-14	/* 0xfffffff2 	-14	 |NUM | */
  Tg8ERR_FIRMWARE                 =-15	/* 0xfffffff1 	-15	 |NUM | */
  Tg8ERR_TABLE_FULL               =-16	/* 0xfffffff0 	-16	 |NUM | */
  Tg8ERR_BAD_VECT                 =-17	/* 0xffffffef 	-17	 |NUM | */
  Tg8ERR_BAD_SWITCH               =-18	/* 0xffffffee 	-18	 |NUM | */
  Tg8ERR_BAD_TRIGGER              =-19	/* 0xffffffed 	-19	 |NUM | */

  Tg8StatusBlock                  =32	/* 0x0020 	32	 |TYPE|  */
  Dt                              =0	/* 0x0000 	0	 |VAR | Tg8StatusBlock.Tg8DateTime 	=12= */
  ScTime                          =12	/* 0x000c 	12	 |VAR | Tg8StatusBlock.Uint 	=4= */
  Epc                             =16	/* 0x0010 	16	 |VAR | Tg8StatusBlock.Uint 	=4= */
  Evo                             =20	/* 0x0014 	20	 |VAR | Tg8StatusBlock.Word 	=2= */
  Hw                              =22	/* 0x0016 	22	 |VAR | Tg8StatusBlock.Word 	=2= */
  Fw                              =24	/* 0x0018 	24	 |VAR | Tg8StatusBlock.Word 	=2= */
  Cc                              =26	/* 0x001a 	26	 |VAR | Tg8StatusBlock.Word 	=2= */
  Am                              =28	/* 0x001c 	28	 |VAR | Tg8StatusBlock.Word 	=2= */
  SpareWS                         =30	/* 0x001e 	30	 |VAR | Tg8StatusBlock.Word 	=2= */
  Tg8DrvrMODULE_ENABLED           =1	/* 0x0001 	1	 |NUM | */

  Tg8DrvrSoftStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DrvrMODULE_FIRMWARE_LOADED   =4	/* 0x0004 	4	 |NUM | */
  Tg8DrvrMODULE_USER_TABLE_VALID  =8	/* 0x0008 	8	 |NUM | */
  Tg8DrvrMODULE_RUNNING           =16	/* 0x0010 	16	 |NUM | */
  Tg8DrvrMODULE_FIRMWARE_ERROR    =32	/* 0x0020 	32	 |NUM | */
  Tg8DrvrMODULE_HARDWARE_ERROR    =64	/* 0x0040 	64	 |NUM | */
  Tg8DrvrMODULE_SWITCH_SET        =128	/* 0x0080 	128	 |NUM | */
  Tg8DrvrMODULE_TIMED_OUT         =256	/* 0x0100 	256	 |NUM | */
  Tg8DrvrDRIVER_DEBUG_ON          =512	/* 0x0200 	512	 |NUM | */
  Tg8DrvrDRIVER_ALARMS            =1024	/* 0x0400 	1024	 |NUM | */
  Tg8HARDWARE                     =0	/* 0x0000 	0	 |NUM | */
  MCP_I                           =0	/* 0x0000 	0	 |NUM | */
  McpERAM_START                   =0	/* 0x0000 	0	 |NUM | */
  McpBUG_END                      =12288	/* 0x3000 	12288	 |NUM | */
  McpERAM_END                     =65536	/* 0x10000 	65536	 |NUM | */
  McpPROM_START                   =393216	/* 0x60000 	393216	 |NUM | */
  McpPROM_END                     =524288	/* 0x80000 	524288	 |NUM | */
  McpSIM_BASE                     =-1536	/* 0xfffffa00 	-1536	 |NUM | */
  McpSBR_BASE                     =-1280	/* 0xfffffb00 	-1280	 |NUM | */
  McpQSM_BASE                     =-1024	/* 0xfffffc00 	-1024	 |NUM | */
  McpTPU_BASE                     =-512	/* 0xfffffe00 	-512	 |NUM | */
  McpTPU_ACT                      =-256	/* 0xffffff00 	-256	 |NUM | */
  McpSim_spare                    =-1536	/* 0xfffffa00 	-1536	 |NUM | */
  McpQsm_spare                    =-1024	/* 0xfffffc00 	-1024	 |NUM | */
  McpSbr_spare                    =-1280	/* 0xfffffb00 	-1280	 |NUM | */
  McpTpu_spare                    =-512	/* 0xfffffe00 	-512	 |NUM | */

  McpSim                          =-1408	/* 0xfffffa80 	-1408	 |TYPE|  */
  SimSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpSim.char[-1536] 	=-1536= */
  SimConf                         =-1536	/* 0xfffffa00 	-1536	 |VAR | McpSim.short 	=2= */
  SimTest                         =-1534	/* 0xfffffa02 	-1534	 |VAR | McpSim.short 	=2= */
  SimSync                         =-1532	/* 0xfffffa04 	-1532	 |VAR | McpSim.short 	=2= */
  SimReset                        =-1530	/* 0xfffffa06 	-1530	 |VAR | McpSim.short 	=2= */
  SimTestE                        =-1528	/* 0xfffffa08 	-1528	 |VAR | McpSim.short 	=2= */
  SimU0a                          =-1526	/* 0xfffffa0a 	-1526	 |VAR | McpSim.short 	=2= */
  SimU0c                          =-1524	/* 0xfffffa0c 	-1524	 |VAR | McpSim.short 	=2= */
  SimU0e                          =-1522	/* 0xfffffa0e 	-1522	 |VAR | McpSim.short 	=2= */
  SimDataE0                       =-1520	/* 0xfffffa10 	-1520	 |VAR | McpSim.short 	=2= */
  SimDataE1                       =-1518	/* 0xfffffa12 	-1518	 |VAR | McpSim.short 	=2= */
  SimDirE                         =-1516	/* 0xfffffa14 	-1516	 |VAR | McpSim.short 	=2= */
  SimPinE                         =-1514	/* 0xfffffa16 	-1514	 |VAR | McpSim.short 	=2= */
  SimDataF0                       =-1512	/* 0xfffffa18 	-1512	 |VAR | McpSim.short 	=2= */
  SimDataF1                       =-1510	/* 0xfffffa1a 	-1510	 |VAR | McpSim.short 	=2= */
  SimDirF                         =-1508	/* 0xfffffa1c 	-1508	 |VAR | McpSim.short 	=2= */
  SimPinF                         =-1506	/* 0xfffffa1e 	-1506	 |VAR | McpSim.short 	=2= */
  SimProtect                      =-1504	/* 0xfffffa20 	-1504	 |VAR | McpSim.short 	=2= */
  SimPic                          =-1502	/* 0xfffffa22 	-1502	 |VAR | McpSim.short 	=2= */
  SimPit                          =-1500	/* 0xfffffa24 	-1500	 |VAR | McpSim.short 	=2= */
  SimServ                         =-1498	/* 0xfffffa26 	-1498	 |VAR | McpSim.short 	=2= */
  SimU28                          =-1496	/* 0xfffffa28 	-1496	 |VAR | McpSim.short 	=2= */
  SimU2a                          =-1494	/* 0xfffffa2a 	-1494	 |VAR | McpSim.short 	=2= */
  SimU2c                          =-1492	/* 0xfffffa2c 	-1492	 |VAR | McpSim.short 	=2= */
  SimU2e                          =-1490	/* 0xfffffa2e 	-1490	 |VAR | McpSim.short 	=2= */
  SimTestMrA                      =-1488	/* 0xfffffa30 	-1488	 |VAR | McpSim.short 	=2= */
  SimTestMrB                      =-1486	/* 0xfffffa32 	-1486	 |VAR | McpSim.short 	=2= */
  SimTestScA                      =-1484	/* 0xfffffa34 	-1484	 |VAR | McpSim.char 	=1= */
  TstScB                          =-1483	/* 0xfffffa35 	-1483	 |VAR | McpSim.char 	=1= */
  SimTestRep                      =-1482	/* 0xfffffa36 	-1482	 |VAR | McpSim.short 	=2= */
  SimTestCtl                      =-1480	/* 0xfffffa38 	-1480	 |VAR | McpSim.short 	=2= */
  SimTestDis                      =-1478	/* 0xfffffa3a 	-1478	 |VAR | McpSim.short 	=2= */
  SimU3c                          =-1476	/* 0xfffffa3c 	-1476	 |VAR | McpSim.short 	=2= */
  SimU3e                          =-1474	/* 0xfffffa3e 	-1474	 |VAR | McpSim.short 	=2= */
  SimDataC                        =-1472	/* 0xfffffa40 	-1472	 |VAR | McpSim.short 	=2= */
  SimU42                          =-1470	/* 0xfffffa42 	-1470	 |VAR | McpSim.short 	=2= */
  SimCSpin                        =-1468	/* 0xfffffa44 	-1468	 |VAR | McpSim.int 	=4= */
  SimCSboot                       =-1464	/* 0xfffffa48 	-1464	 |VAR | McpSim.int 	=4= */
  SimCS0                          =-1460	/* 0xfffffa4c 	-1460	 |VAR | McpSim.int 	=4= */
  SimCS1                          =-1456	/* 0xfffffa50 	-1456	 |VAR | McpSim.int 	=4= */
  SimCS2                          =-1452	/* 0xfffffa54 	-1452	 |VAR | McpSim.int 	=4= */
  SimCS3                          =-1448	/* 0xfffffa58 	-1448	 |VAR | McpSim.int 	=4= */
  SimCS4                          =-1444	/* 0xfffffa5c 	-1444	 |VAR | McpSim.int 	=4= */
  SimCS5                          =-1440	/* 0xfffffa60 	-1440	 |VAR | McpSim.int 	=4= */
  SimCS6                          =-1436	/* 0xfffffa64 	-1436	 |VAR | McpSim.int 	=4= */
  SimCS7                          =-1432	/* 0xfffffa68 	-1432	 |VAR | McpSim.int 	=4= */
  SimCS8                          =-1428	/* 0xfffffa6c 	-1428	 |VAR | McpSim.int 	=4= */
  SimCS9                          =-1424	/* 0xfffffa70 	-1424	 |VAR | McpSim.int 	=4= */
  SimCS10                         =-1420	/* 0xfffffa74 	-1420	 |VAR | McpSim.int 	=4= */
  SimU78                          =-1416	/* 0xfffffa78 	-1416	 |VAR | McpSim.int 	=4= */
  SimU7c                          =-1412	/* 0xfffffa7c 	-1412	 |VAR | McpSim.int 	=4= */

  McpSbr                          =-1274	/* 0xfffffb06 	-1274	 |TYPE|  */
  SbrSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpSbr.char[-1280] 	=-1280= */
  SbrConf                         =-1280	/* 0xfffffb00 	-1280	 |VAR | McpSbr.short 	=2= */
  SbrTest                         =-1278	/* 0xfffffb02 	-1278	 |VAR | McpSbr.short 	=2= */
  SbrBaSt                         =-1276	/* 0xfffffb04 	-1276	 |VAR | McpSbr.short 	=2= */

  McpQsm                          =-688	/* 0xfffffd50 	-688	 |TYPE|  */
  QsmSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpQsm.char[-1024] 	=-1024= */
  QsmConf                         =-1024	/* 0xfffffc00 	-1024	 |VAR | McpQsm.short 	=2= */
  QsmTest                         =-1022	/* 0xfffffc02 	-1022	 |VAR | McpQsm.short 	=2= */
  QsmIntLev                       =-1020	/* 0xfffffc04 	-1020	 |VAR | McpQsm.char 	=1= */
  QsmIntVec                       =-1019	/* 0xfffffc05 	-1019	 |VAR | McpQsm.char 	=1= */
  QsmU06                          =-1018	/* 0xfffffc06 	-1018	 |VAR | McpQsm.short 	=2= */
  QsmSciC0                        =-1016	/* 0xfffffc08 	-1016	 |VAR | McpQsm.short 	=2= */
  QsmSciC1                        =-1014	/* 0xfffffc0a 	-1014	 |VAR | McpQsm.short 	=2= */
  QsmSciS                         =-1012	/* 0xfffffc0c 	-1012	 |VAR | McpQsm.short 	=2= */
  QsmSciD                         =-1010	/* 0xfffffc0e 	-1010	 |VAR | McpQsm.short 	=2= */
  QsmU10                          =-1008	/* 0xfffffc10 	-1008	 |VAR | McpQsm.short 	=2= */
  QsmU12                          =-1006	/* 0xfffffc12 	-1006	 |VAR | McpQsm.short 	=2= */
  QsmU14                          =-1004	/* 0xfffffc14 	-1004	 |VAR | McpQsm.char 	=1= */
  QsmPd                           =-1003	/* 0xfffffc15 	-1003	 |VAR | McpQsm.char 	=1= */
  QsmPin                          =-1002	/* 0xfffffc16 	-1002	 |VAR | McpQsm.char 	=1= */
  QsmDir                          =-1001	/* 0xfffffc17 	-1001	 |VAR | McpQsm.char 	=1= */
  QsmSpiC0                        =-1000	/* 0xfffffc18 	-1000	 |VAR | McpQsm.short 	=2= */
  QsmSpiC1                        =-998	/* 0xfffffc1a 	-998	 |VAR | McpQsm.short 	=2= */
  QsmSpiC2                        =-996	/* 0xfffffc1c 	-996	 |VAR | McpQsm.short 	=2= */
  QsmSpiC3                        =-994	/* 0xfffffc1e 	-994	 |VAR | McpQsm.char 	=1= */
  QsmSpiS                         =-993	/* 0xfffffc1f 	-993	 |VAR | McpQsm.char 	=1= */
  QsmU20                          =-992	/* 0xfffffc20 	-992	 |VAR | McpQsm.char[224] 	=224= */
  QsmRecv_ram                     =-768	/* 0xfffffd00 	-768	 |VAR | McpQsm.short[16] 	=32= */
  QsmTran_ram                     =-736	/* 0xfffffd20 	-736	 |VAR | McpQsm.short[16] 	=32= */
  QsmComd_ram                     =-704	/* 0xfffffd40 	-704	 |VAR | McpQsm.char[16] 	=16= */

  McpTpu                          =-472	/* 0xfffffe28 	-472	 |TYPE|  */
  TpuSpare_bytes                  =0	/* 0x0000 	0	 |VAR | McpTpu.char[-512] 	=-512= */
  TpuConf                         =-512	/* 0xfffffe00 	-512	 |VAR | McpTpu.short 	=2= */
  TpuCR                           =-510	/* 0xfffffe02 	-510	 |VAR | McpTpu.short 	=2= */
  TpuDsc                          =-508	/* 0xfffffe04 	-508	 |VAR | McpTpu.short 	=2= */
  TpuDss                          =-506	/* 0xfffffe06 	-506	 |VAR | McpTpu.short 	=2= */
  TpuIc                           =-504	/* 0xfffffe08 	-504	 |VAR | McpTpu.short 	=2= */
  TpuIe                           =-502	/* 0xfffffe0a 	-502	 |VAR | McpTpu.short 	=2= */
  TpuCfs0                         =-500	/* 0xfffffe0c 	-500	 |VAR | McpTpu.short 	=2= */
  TpuCfs1                         =-498	/* 0xfffffe0e 	-498	 |VAR | McpTpu.short 	=2= */
  TpuCfs2                         =-496	/* 0xfffffe10 	-496	 |VAR | McpTpu.short 	=2= */
  TpuCfs3                         =-494	/* 0xfffffe12 	-494	 |VAR | McpTpu.short 	=2= */
  TpuHs0                          =-492	/* 0xfffffe14 	-492	 |VAR | McpTpu.short 	=2= */
  TpuHs1                          =-490	/* 0xfffffe16 	-490	 |VAR | McpTpu.short 	=2= */
  TpuHsr0                         =-488	/* 0xfffffe18 	-488	 |VAR | McpTpu.short 	=2= */
  TpuHsr1                         =-486	/* 0xfffffe1a 	-486	 |VAR | McpTpu.short 	=2= */
  TpuCp0                          =-484	/* 0xfffffe1c 	-484	 |VAR | McpTpu.short 	=2= */
  TpuCp1                          =-482	/* 0xfffffe1e 	-482	 |VAR | McpTpu.short 	=2= */
  TpuIs                           =-480	/* 0xfffffe20 	-480	 |VAR | McpTpu.short 	=2= */
  TpuLink                         =-478	/* 0xfffffe22 	-478	 |VAR | McpTpu.short 	=2= */
  TpuSgl                          =-476	/* 0xfffffe24 	-476	 |VAR | McpTpu.short 	=2= */
  TpuDcn                          =-474	/* 0xfffffe26 	-474	 |VAR | McpTpu.short 	=2= */
  McpCS_BA_ADDRESS_MASK           =-524288	/* 0xfff80000 	-524288	 |NUM | */
  McpCS_BA_ADDRESS_SHIFT          =8	/* 0x0008 	8	 |NUM | */
  McpCS_BA_BLKSZ_2K               =0	/* 0x0000 	0	 |NUM | */
  McpCS_BA_BLKSZ_8K               =65536	/* 0x10000 	65536	 |NUM | */
  McpCS_BA_BLKSZ_16K              =131072	/* 0x20000 	131072	 |NUM | */
  McpCS_BA_BLKSZ_64K              =196608	/* 0x30000 	196608	 |NUM | */
  McpCS_BA_BLKSZ_128K             =262144	/* 0x40000 	262144	 |NUM | */
  McpCS_BA_BLKSZ_256K             =327680	/* 0x50000 	327680	 |NUM | */
  McpCS_BA_BLKSZ_512K             =393216	/* 0x60000 	393216	 |NUM | */
  McpCS_BA_BLKSZ_1M               =458752	/* 0x70000 	458752	 |NUM | */
  McpCS_OP_MODE_SYNC              =32768	/* 0x8000 	32768	 |NUM | */
  McpCS_OP_MODE_ASYNC             =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_BYTE_DISABLE           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_BYTE_LOWER             =8192	/* 0x2000 	8192	 |NUM | */
  McpCS_OP_BYTE_UPPER             =16384	/* 0x4000 	16384	 |NUM | */
  McpCS_OP_BYTE_BOTH              =24576	/* 0x6000 	24576	 |NUM | */
  McpCS_OP_RW_RO                  =2048	/* 0x0800 	2048	 |NUM | */
  McpCS_OP_RW_WO                  =4096	/* 0x1000 	4096	 |NUM | */
  McpCS_OP_RW_RW                  =6144	/* 0x1800 	6144	 |NUM | */
  McpCS_OP_STRB_ADDRESS           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_STRB_DATA              =1024	/* 0x0400 	1024	 |NUM | */
  McpCS_OP_DSACK_NOWAIT           =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_DSACK_WAIT_1           =64	/* 0x0040 	64	 |NUM | */
  McpCS_OP_DSACK_WAIT_2           =128	/* 0x0080 	128	 |NUM | */
  McpCS_OP_DSACK_WAIT_3           =192	/* 0x00c0 	192	 |NUM | */
  McpCS_OP_DSACK_WAIT_4           =256	/* 0x0100 	256	 |NUM | */
  McpCS_OP_DSACK_WAIT_5           =320	/* 0x0140 	320	 |NUM | */
  McpCS_OP_DSACK_WAIT_6           =384	/* 0x0180 	384	 |NUM | */
  McpCS_OP_DSACK_WAIT_7           =448	/* 0x01c0 	448	 |NUM | */
  McpCS_OP_DSACK_WAIT_8           =512	/* 0x0200 	512	 |NUM | */
  McpCS_OP_DSACK_WAIT_9           =576	/* 0x0240 	576	 |NUM | */
  McpCS_OP_DSACK_WAIT_10          =640	/* 0x0280 	640	 |NUM | */
  McpCS_OP_DSACK_WAIT_11          =704	/* 0x02c0 	704	 |NUM | */
  McpCS_OP_DSACK_WAIT_12          =768	/* 0x0300 	768	 |NUM | */
  McpCS_OP_DSACK_WAIT_13          =832	/* 0x0340 	832	 |NUM | */
  McpCS_OP_DSACK_FAST             =896	/* 0x0380 	896	 |NUM | */
  McpCS_OP_DSACK_EXTERNAL         =960	/* 0x03c0 	960	 |NUM | */
  McpCS_OP_SPACE_CPU              =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_SPACE_USER             =16	/* 0x0010 	16	 |NUM | */
  McpCS_OP_SPACE_SUPER            =32	/* 0x0020 	32	 |NUM | */
  McpCS_OP_SPACE_BOTH             =48	/* 0x0030 	48	 |NUM | */
  McpCS_OP_IPL_ANY                =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_IPL_1                  =2	/* 0x0002 	2	 |NUM | */
  McpCS_OP_IPL_2                  =4	/* 0x0004 	4	 |NUM | */
  McpCS_OP_IPL_3                  =6	/* 0x0006 	6	 |NUM | */
  McpCS_OP_IPL_4                  =8	/* 0x0008 	8	 |NUM | */
  McpCS_OP_IPL_5                  =10	/* 0x000a 	10	 |NUM | */
  McpCS_OP_IPL_6                  =12	/* 0x000c 	12	 |NUM | */
  McpCS_OP_IPL_7                  =14	/* 0x000e 	14	 |NUM | */
  McpCS_OP_AVEC_EXTERNAL          =0	/* 0x0000 	0	 |NUM | */
  McpCS_OP_AVEC_ENABLED           =1	/* 0x0001 	1	 |NUM | */
  McpVECTOR_BUS_ERROR             =8	/* 0x0008 	8	 |NUM | */
  McpVECTOR_ADDRESS_ERROR         =12	/* 0x000c 	12	 |NUM | */
  McpVECTOR_ILLEGAL_INSTRUCTION   =16	/* 0x0010 	16	 |NUM | */
  McpVECTOR_ZERO_DIVISION         =20	/* 0x0014 	20	 |NUM | */
  McpVECTOR_PRIVILEGE_VIOLATION   =32	/* 0x0020 	32	 |NUM | */
  McpVECTOR_SPURIOUS_INTERRUPT    =96	/* 0x0060 	96	 |NUM | */
  McpVECTOR_AUTO_1                =100	/* 0x0064 	100	 |NUM | */
  McpVECTOR_AUTO_2                =104	/* 0x0068 	104	 |NUM | */
  McpVECTOR_AUTO_3                =108	/* 0x006c 	108	 |NUM | */
  McpVECTOR_AUTO_4                =112	/* 0x0070 	112	 |NUM | */
  McpVECTOR_AUTO_5                =116	/* 0x0074 	116	 |NUM | */
  McpVECTOR_AUTO_6                =120	/* 0x0078 	120	 |NUM | */
  McpVECTOR_AUTO_7                =124	/* 0x007c 	124	 |NUM | */
  McpTRAP_VECTOR_BASE             =32	/* 0x0020 	32	 |NUM | */
  Tg8DISABLE_INTERRUPTS           =9984	/* 0x2700 	9984	 |NUM | */
  Tg8ENABLE_INTERRUPTS            =9216	/* 0x2400 	9216	 |NUM | */
  Tg8ENABLE_WATCHDOG              =160	/* 0x00a0 	160	 |NUM | */
  Tg8ENABLE_BUSMONITOR            =4	/* 0x0004 	4	 |NUM | */
  Tg8RAM_BASE                     =0	/* 0x0000 	0	 |NUM | */
  Tg8DPM_BASE                     =65536	/* 0x10000 	65536	 |NUM | */
  Tg8CAM_BASE                     =69632	/* 0x11000 	69632	 |NUM | */
  Tg8XLX_BASE                     =73728	/* 0x12000 	73728	 |NUM | */
  Tg8DSC_VECTOR                   =112	/* 0x0070 	112	 |NUM | */
  Tg8DPM_VECTOR                   =116	/* 0x0074 	116	 |NUM | */
  Tg8XLX_VECTOR                   =120	/* 0x0078 	120	 |NUM | */
  Tg8ABT_VECTOR                   =124	/* 0x007c 	124	 |NUM | */
  Tg8VME_INTERRUPT                =67580	/* 0x107fc 	67580	 |NUM | */
  Tg8DSC_INTERRUPT                =67582	/* 0x107fe 	67582	 |NUM | */
  Tg8TPU_BASE_VECTOR              =64	/* 0x0040 	64	 |NUM | */
  Tg8TPU_INT_LEVEL                =7	/* 0x0007 	7	 |NUM | */
  Tg8RAM_SIZE                     =2048	/* 0x0800 	2048	 |NUM | */

  Tg8Ram                          =2048	/* 0x0800 	2048	 |TYPE| char[2048] */
  Tg8COUNTERS                     =8	/* 0x0008 	8	 |NUM | */
  Tg8Xlx_spare                    =73728	/* 0x12000 	73728	 |NUM | */

  Tg8Xlx                          =73766	/* 0x12026 	73766	 |TYPE|  */
  XlxSpare_Bytes                  =0	/* 0x0000 	0	 |VAR | Tg8Xlx.char[73728] 	=73728= */
  XlxDelay                        =73728	/* 0x12000 	73728	 |VAR | Tg8Xlx.short[8] 	=16= */
  XlxConfig                       =73744	/* 0x12010 	73744	 |VAR | Tg8Xlx.short[8] 	=16= */
  XWsscRframe1                    =73760	/* 0x12020 	73760	 |VAR | Tg8Xlx.short 	=2= */
  XRframe2                        =73762	/* 0x12022 	73762	 |VAR | Tg8Xlx.short 	=2= */
  WClrRerr                        =73764	/* 0x12024 	73764	 |VAR | Tg8Xlx.short 	=2= */
  ConfOUTPUT                      =1	/* 0x0001 	1	 |NUM | */

  Tg8CounterConfig                =4	/* 0x0004 	4	 |TYPE|  */
  ConfINTERRUPT                   =4	/* 0x0004 	4	 |NUM | */
  ConfCABLE_CLOCK                 =8	/* 0x0008 	8	 |NUM | */
  ConfEXT_CLOCK_1                 =16	/* 0x0010 	16	 |NUM | */
  ConfEXT_CLOCK_2                 =24	/* 0x0018 	24	 |NUM | */
  ConfEXT_START                   =32	/* 0x0020 	32	 |NUM | */
  ConfNORMAL                      =64	/* 0x0040 	64	 |NUM | */
  ConfCHAINE                      =128	/* 0x0080 	128	 |NUM | */
  ConfBURST                       =192	/* 0x00c0 	192	 |NUM | */
  XrDATA_OVERFLOW                 =1	/* 0x0001 	1	 |NUM | */

  Tg8XrHardwareStatus             =4	/* 0x0004 	4	 |TYPE|  */
  XrPARITY_ERROR                  =2	/* 0x0002 	2	 |NUM | */
  XrEND_SEQUENCE                  =4	/* 0x0004 	4	 |NUM | */
  XrMID_BIT                       =8	/* 0x0008 	8	 |NUM | */
  XrSTART_SEQUENCE                =16	/* 0x0010 	16	 |NUM | */
  XrDISABLED                      =32	/* 0x0020 	32	 |NUM | */
  XrMS_MISSING                    =64	/* 0x0040 	64	 |NUM | */
  XrMS_WATCH_DOG                  =128	/* 0x0080 	128	 |NUM | */
  Tg8HS_COUNTER_1_INTERRUPT       =1	/* 0x0001 	1	 |NUM | */

  Tg8HardwareStatus               =4	/* 0x0004 	4	 |TYPE|  */
  Tg8HS_COUNTER_2_INTERRUPT       =2	/* 0x0002 	2	 |NUM | */
  Tg8HS_COUNTER_3_INTERRUPT       =4	/* 0x0004 	4	 |NUM | */
  Tg8HS_COUNTER_4_INTERRUPT       =8	/* 0x0008 	8	 |NUM | */
  Tg8HS_COUNTER_5_INTERRUPT       =16	/* 0x0010 	16	 |NUM | */
  Tg8HS_COUNTER_6_INTERRUPT       =32	/* 0x0020 	32	 |NUM | */
  Tg8HS_COUNTER_7_INTERRUPT       =64	/* 0x0040 	64	 |NUM | */
  Tg8HS_COUNTER_8_INTERRUPT       =128	/* 0x0080 	128	 |NUM | */
  Tg8HS_DPRAM_INTERRUPT           =256	/* 0x0100 	256	 |NUM | */
  Tg8HS_RECEIVER_ENABLED          =512	/* 0x0200 	512	 |NUM | */
  Tg8HS_PENDING_DOWNLOAD          =1024	/* 0x0400 	1024	 |NUM | */
  Tg8HS_XILINX_XR_LOADED          =2048	/* 0x0800 	2048	 |NUM | */
  Tg8HS_XILINX_X1_LOADED          =4096	/* 0x1000 	4096	 |NUM | */
  Tg8HS_XILINX_X2_LOADED          =8192	/* 0x2000 	8192	 |NUM | */
  Tg8HS_EXTERNAL_CLOCK_JUMPER     =16384	/* 0x4000 	16384	 |NUM | */
  Tg8HS_SELF_TEST_ERROR           =32768	/* 0x8000 	32768	 |NUM | */
  Tg8INTERRUPT_MASK               =511	/* 0x01ff 	511	 |NUM | */
  Tg8HW_OK_STATUS                 =14336	/* 0x3800 	14336	 |NUM | */
  ClrINT_CHANNEL_0                =65534	/* 0xfffe 	65534	 |NUM | */

  Tg8TpuClearInterrupt            =4	/* 0x0004 	4	 |TYPE|  */
  ClrINT_CHANNEL_1                =65533	/* 0xfffd 	65533	 |NUM | */
  ClrINT_CHANNEL_2                =65531	/* 0xfffb 	65531	 |NUM | */
  ClrINT_CHANNEL_3                =65527	/* 0xfff7 	65527	 |NUM | */
  ClrINT_CHANNEL_4                =65519	/* 0xffef 	65519	 |NUM | */
  ClrINT_CHANNEL_5                =65503	/* 0xffdf 	65503	 |NUM | */
  ClrINT_CHANNEL_6                =65471	/* 0xffbf 	65471	 |NUM | */
  ClrINT_CHANNEL_7                =65407	/* 0xff7f 	65407	 |NUM | */
  ClrINT_CHANNEL_8                =65279	/* 0xfeff 	65279	 |NUM | */
  Tg8CAM_SIZE                     =256	/* 0x0100 	256	 |NUM | */
  Tg8ACTIONS_NUMBER               =256	/* 0x0100 	256	 |NUM | */
  CamDATA_MODE                    =2	/* 0x0002 	2	 |NUM | */

  Tg8CamControl                   =4	/* 0x0004 	4	 |TYPE|  */
  CamCOMMAND_MODE                 =65533	/* 0xfffd 	65533	 |NUM | */
  CamWRITE                        =57344	/* 0xe000 	57344	 |NUM | */
  CamCLEAR_SKIP_BITS              =38912	/* 0x9800 	38912	 |NUM | */
  CamSET_SKIP_BIT                 =37888	/* 0x9400 	37888	 |NUM | */
  CamSTATUS_MULTIPLE              =14	/* 0x000e 	14	 |NUM | */
  CamMATCH                        =32768	/* 0x8000 	32768	 |NUM | */
  CamMUL                          =16384	/* 0x4000 	16384	 |NUM | */
  CamBYTE_MASK                    =255	/* 0x00ff 	255	 |NUM | */
  SelfTestOk                      =4	/* 0x0004 	4	 |NUM | */

  OkLedEnum                       =4	/* 0x0004 	4	 |TYPE|  */
  OkLed                           =8	/* 0x0008 	8	 |NUM | */
  Tg8CSBOOT                       =100952240	/* 0x60468b0 	100952240	 |NUM | */
  Tg8CS0                          =217150	/* 0x3503e 	217150	 |NUM | */
  Tg8CS1                          =208958	/* 0x3303e 	208958	 |NUM | */
  Tg8CS2                          =223294	/* 0x3683e 	223294	 |NUM | */
  Tg8CS3                          =18906160	/* 0x1207c30 	18906160	 |NUM | */
  Tg8CS4                          =-497649	/* 0xfff8680f 	-497649	 |NUM | */
  Tg8CS5                          =-497651	/* 0xfff8680d 	-497651	 |NUM | */
  Tg8CS6                          =17855536	/* 0x1107430 	17855536	 |NUM | */
  Tg8CS7                          =17852464	/* 0x1106830 	17852464	 |NUM | */
  Tg8CS8                          =16808944	/* 0x1007bf0 	16808944	 |NUM | */
  Tg8CS9                          =0	/* 0x0000 	0	 |NUM | */
  Tg8CS10                         =0	/* 0x0000 	0	 |NUM | */
  Tg8SELF_TEST                    =0	/* 0x0000 	0	 |NUM | */
  MbxST_GO                        =1234	/* 0x04d2 	1234	 |NUM | */

  MbxStatus                       =4	/* 0x0004 	4	 |TYPE|  */
  MbxST_BUSY                      =5678	/* 0x162e 	5678	 |NUM | */
  MbxST_DONE                      =9012	/* 0x2334 	9012	 |NUM | */
  MbxST_BUS_ERROR                 =3456	/* 0x0d80 	3456	 |NUM | */
  MbxOP_READ                      =-1	/* 0xffffffff 	-1	 |NUM | */

  MbxOperation                    =4	/* 0x0004 	4	 |TYPE|  */
  MbxOP_WRITE                     =1	/* 0x0001 	1	 |NUM | */
  MbxOP_NONE                      =0	/* 0x0000 	0	 |NUM | */
  MbxCC_BUSY                      =-1	/* 0xffffffff 	-1	 |NUM | */

  MbxComplCode                    =4	/* 0x0004 	4	 |TYPE|  */
  MbxCC_DONE                      =1	/* 0x0001 	1	 |NUM | */
  MbxCC_BOOT                      =14402	/* 0x3842 	14402	 |NUM | */
  MbxCC_332Bug                    =14404	/* 0x3844 	14404	 |NUM | */
  MbxCC_DOWNLOAD                  =14422	/* 0x3856 	14422	 |NUM | */
  MbxCC_DOWNLOAD_PENDING          =18040	/* 0x4678 	18040	 |NUM | */
  MbxCC_FIRMWARE_RUNNING          =30583	/* 0x7777 	30583	 |NUM | */
  Tg8DPRAM_SIZE                   =2048	/* 0x0800 	2048	 |NUM | */
  StCOUNTERS                      =8	/* 0x0008 	8	 |NUM | */

  StMemory                        =4	/* 0x0004 	4	 |TYPE|  */
  Bot                             =0	/* 0x0000 	0	 |VAR | StMemory.Ushort 	=2= */
  Top                             =2	/* 0x0002 	2	 |VAR | StMemory.Ushort 	=2= */

  StFault                         =20	/* 0x0014 	20	 |TYPE|  */
  Code                            =0	/* 0x0000 	0	 |VAR | StFault.int 	=4= */
  Templ                           =4	/* 0x0004 	4	 |VAR | StFault.int 	=4= */
  Actual                          =8	/* 0x0008 	8	 |VAR | StFault.int 	=4= */
  At                              =12	/* 0x000c 	12	 |VAR | StFault.Ushort 	=2= */
  Dir                             =14	/* 0x000e 	14	 |VAR | StFault.Ushort 	=2= */
  BusInt                          =16	/* 0x0010 	16	 |VAR | StFault.Ushort 	=2= */
  N_of_bus                        =18	/* 0x0012 	18	 |VAR | StFault.Ushort 	=2= */

  StDateTime                      =16	/* 0x0010 	16	 |TYPE|  */
  Year                            =0	/* 0x0000 	0	 |VAR | StDateTime.short 	=2= */
  Month                           =2	/* 0x0002 	2	 |VAR | StDateTime.short 	=2= */
  Day                             =4	/* 0x0004 	4	 |VAR | StDateTime.short 	=2= */
  Hour                            =6	/* 0x0006 	6	 |VAR | StDateTime.short 	=2= */
  Minute                          =8	/* 0x0008 	8	 |VAR | StDateTime.short 	=2= */
  Second                          =10	/* 0x000a 	10	 |VAR | StDateTime.short 	=2= */
  Ms                              =12	/* 0x000c 	12	 |VAR | StDateTime.int 	=4= */

  StDpmInfo                       =348	/* 0x015c 	348	 |TYPE|  */
  N_of_bus                        =0	/* 0x0000 	0	 |VAR | StDpmInfo.int 	=4= */
  N_of_frames                     =4	/* 0x0004 	4	 |VAR | StDpmInfo.int 	=4= */
  EventFrame                      =8	/* 0x0008 	8	 |VAR | StDpmInfo.int 	=4= */
  CycleNum                        =12	/* 0x000c 	12	 |VAR | StDpmInfo.int 	=4= */
  CycleDur                        =16	/* 0x0010 	16	 |VAR | StDpmInfo.short 	=2= */
  CTrain                          =18	/* 0x0012 	18	 |VAR | StDpmInfo.short 	=2= */
  Date                            =20	/* 0x0014 	20	 |VAR | StDpmInfo.StDateTime 	=16= */
  TestProg                        =36	/* 0x0024 	36	 |VAR | StDpmInfo.Ushort 	=2= */
  TestPassed                      =38	/* 0x0026 	38	 |VAR | StDpmInfo.Ushort 	=2= */
  FaultType                       =40	/* 0x0028 	40	 |VAR | StDpmInfo.Ushort 	=2= */
  FaultCnt                        =42	/* 0x002a 	42	 |VAR | StDpmInfo.Ushort 	=2= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  Ram                             =44	/* 0x002c 	44	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  Dpram                           =68	/* 0x0044 	68	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Mem                             =20	/* 0x0014 	20	 |VAR | _su_.StMemory 	=4= */
  At                              =0	/* 0x0000 	0	 |VAR | _su_.Ushort 	=2= */
  Cnt                             =2	/* 0x0002 	2	 |VAR | _su_.Ushort 	=2= */
  Match                           =24	/* 0x0018 	24	 |VAR | _su_._su_ 	=4= */
  Cam                             =92	/* 0x005c 	92	 |VAR | StDpmInfo._su_ 	=28= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault 	=20= */
  Rcv                             =20	/* 0x0014 	20	 |VAR | _su_.Ushort 	=2= */
  Ssc                             =22	/* 0x0016 	22	 |VAR | _su_.Ushort 	=2= */
  Xilinx                          =120	/* 0x0078 	120	 |VAR | StDpmInfo._su_ 	=24= */
  Err                             =0	/* 0x0000 	0	 |VAR | _su_.StFault[8] 	=160= */
  Bad                             =160	/* 0x00a0 	160	 |VAR | _su_.Ushort 	=2= */
  Delay                           =162	/* 0x00a2 	162	 |VAR | _su_.Ushort 	=2= */
  Counter                         =144	/* 0x0090 	144	 |VAR | StDpmInfo._su_ 	=164= */
  DscInt                          =308	/* 0x0134 	308	 |VAR | StDpmInfo.int[9] 	=36= */
  MbxInt                          =344	/* 0x0158 	344	 |VAR | StDpmInfo.int 	=4= */

  StDpmHead                       =12	/* 0x000c 	12	 |TYPE|  */
  IoDirection                     =0	/* 0x0000 	0	 |VAR | StDpmHead.short 	=2= */
  DoneFlag                        =2	/* 0x0002 	2	 |VAR | StDpmHead.short 	=2= */
  FirmwareStatus                  =4	/* 0x0004 	4	 |VAR | StDpmHead.short 	=2= */
  VectorOffset                    =6	/* 0x0006 	6	 |VAR | StDpmHead.short 	=2= */
  ExceptionPC                     =8	/* 0x0008 	8	 |VAR | StDpmHead.int 	=4= */

  StDpm                           =2054	/* 0x0806 	2054	 |TYPE|  */
  Head                            =0	/* 0x0000 	0	 |VAR | StDpm.StDpmHead 	=12= */
  Info                            =12	/* 0x000c 	12	 |VAR | StDpm.StDpmInfo 	=348= */
  Buffer                          =360	/* 0x0168 	360	 |VAR | StDpm.short[841] 	=1682= */
  XilinxStatus                    =2042	/* 0x07fa 	2042	 |VAR | StDpm.short 	=2= */
  InterruptDsc                    =2044	/* 0x07fc 	2044	 |VAR | StDpm.short 	=2= */
  InterruptMcp                    =2046	/* 0x07fe 	2046	 |VAR | StDpm.short 	=2= */
  HardwareStatus                  =2048	/* 0x0800 	2048	 |VAR | StDpm.short 	=2= */
  Vector                          =2050	/* 0x0802 	2050	 |VAR | StDpm.short 	=2= */
  CounterInterruptMask            =2052	/* 0x0804 	2052	 |VAR | StDpm.short 	=2= */
  BI_INIT                         =1	/* 0x0001 	1	 |NUM | */

  StBusInt                        =4	/* 0x0004 	4	 |TYPE|  */
  BI_RAM                          =2	/* 0x0002 	2	 |NUM | */
  BI_RAM_TEST                     =3	/* 0x0003 	3	 |NUM | */
  BI_DPRAM                        =4	/* 0x0004 	4	 |NUM | */
  BI_DPRAM_TEST                   =5	/* 0x0005 	5	 |NUM | */
  BI_CAM                          =6	/* 0x0006 	6	 |NUM | */
  BI_CAM_TEST                     =7	/* 0x0007 	7	 |NUM | */
  BI_XILINX                       =8	/* 0x0008 	8	 |NUM | */
  BI_XILINX_TEST                  =9	/* 0x0009 	9	 |NUM | */
  XI_READF1                       =10	/* 0x000a 	10	 |NUM | */
  XI_READF2                       =11	/* 0x000b 	11	 |NUM | */
  XI_READERR                      =12	/* 0x000c 	12	 |NUM | */
  XI_RESETERR                     =13	/* 0x000d 	13	 |NUM | */
  XI_SSC                          =14	/* 0x000e 	14	 |NUM | */
  BI_COUNTER_TEST                 =15	/* 0x000f 	15	 |NUM | */
  CI_DELAY                        =16	/* 0x0010 	16	 |NUM | */
  CI_CONFIG                       =17	/* 0x0011 	17	 |NUM | */
  N_OF_BI                         =18	/* 0x0012 	18	 |NUM | */
  D_FORWARD                       =0	/* 0x0000 	0	 |NUM | */

  StDirection                     =4	/* 0x0004 	4	 |TYPE|  */
  D_BACKWARD                      =1	/* 0x0001 	1	 |NUM | */
  N_OF_DIR                        =2	/* 0x0002 	2	 |NUM | */
  A_BYTE                          =0	/* 0x0000 	0	 |NUM | */

  StAccess                        =4	/* 0x0004 	4	 |TYPE|  */
  A_WORD                          =1	/* 0x0001 	1	 |NUM | */
  A_INT32                         =2	/* 0x0002 	2	 |NUM | */
  N_OF_ACC                        =3	/* 0x0003 	3	 |NUM | */
  TM_AA                           =0	/* 0x0000 	0	 |NUM | */

  StTemplate                      =4	/* 0x0004 	4	 |TYPE|  */
  TM_55                           =1	/* 0x0001 	1	 |NUM | */
  TM_00                           =2	/* 0x0002 	2	 |NUM | */
  N_OF_TEMPL                      =3	/* 0x0003 	3	 |NUM | */
  TC_A                            =1	/* 0x0001 	1	 |NUM | */

  StCamTemplate                   =4	/* 0x0004 	4	 |TYPE|  */
  TC_B                            =2	/* 0x0002 	2	 |NUM | */
  TC_C                            =3	/* 0x0003 	3	 |NUM | */
  TC_D                            =4	/* 0x0004 	4	 |NUM | */
  TC_E                            =5	/* 0x0005 	5	 |NUM | */
  CO_COUNT_FAST                   =1	/* 0x0001 	1	 |NUM | */

  StCounting                      =4	/* 0x0004 	4	 |TYPE|  */
  CO_BLOCKED                      =2	/* 0x0002 	2	 |NUM | */
  CO_NO_DSC_INT                   =3	/* 0x0003 	3	 |NUM | */
  N_OF_CONT_ER                    =4	/* 0x0004 	4	 |NUM | */
  T_RAM                           =1	/* 0x0001 	1	 |NUM | */

  StTest                          =4	/* 0x0004 	4	 |TYPE|  */
  T_DPRAM                         =2	/* 0x0002 	2	 |NUM | */
  T_CAM                           =4	/* 0x0004 	4	 |NUM | */
  T_XILINX                        =8	/* 0x0008 	8	 |NUM | */
  T_MS                            =16	/* 0x0010 	16	 |NUM | */
  T_COUNTERS                      =32	/* 0x0020 	32	 |NUM | */
  E_CAM_RW                        =1	/* 0x0001 	1	 |NUM | */

  StErrCode                       =4	/* 0x0004 	4	 |TYPE|  */
  E_CAM_MATCH                     =2	/* 0x0002 	2	 |NUM | */
  Tg8OP_NO_COMMAND                =0	/* 0x0000 	0	 |NUM | */

  Tg8Operation                    =4	/* 0x0004 	4	 |TYPE|  */
  Tg8OP_PING_MODULE               =1	/* 0x0001 	1	 |NUM | */
  Tg8OP_GET_STATUS                =2	/* 0x0002 	2	 |NUM | */
  Tg8OP_SET_SSC_HEADER            =3	/* 0x0003 	3	 |NUM | */
  Tg8OP_SET_STATE                 =4	/* 0x0004 	4	 |NUM | */
  Tg8OP_SET_DELAY                 =5	/* 0x0005 	5	 |NUM | */
  Tg8OP_CLEAR_USER_TABLE          =6	/* 0x0006 	6	 |NUM | */
  Tg8OP_SET_USER_TABLE            =7	/* 0x0007 	7	 |NUM | */
  Tg8OP_GET_USER_TABLE            =8	/* 0x0008 	8	 |NUM | */
  Tg8OP_GET_RECORDING_TABLE       =9	/* 0x0009 	9	 |NUM | */
  Tg8OP_GET_HISTORY_TABLE         =10	/* 0x000a 	10	 |NUM | */
  Tg8OP_GET_CLOCK_TABLE           =11	/* 0x000b 	11	 |NUM | */
  Tg8OP_SET_PPM_TIMING            =12	/* 0x000c 	12	 |NUM | */
  Tg8OP_SET_GATE                  =13	/* 0x000d 	13	 |NUM | */
  Tg8OP_SET_DIM                   =14	/* 0x000e 	14	 |NUM | */
  Tg8OP_GET_PPM_LINE              =15	/* 0x000f 	15	 |NUM | */
  Tg8OP_SIMULATE_PULSE            =16	/* 0x0010 	16	 |NUM | */
  Tg8OP_SELFTEST_RESULT           =17	/* 0x0011 	17	 |NUM | */
  Tg8HR_INT_VME                   =2044	/* 0x07fc 	2044	 |NUM | */

  Tg8HardwareRegister             =4	/* 0x0004 	4	 |TYPE|  */
  Tg8HR_INT_MCP                   =2046	/* 0x07fe 	2046	 |NUM | */
  Tg8HR_STATUS                    =2048	/* 0x0800 	2048	 |NUM | */
  Tg8HR_INTERRUPT                 =2050	/* 0x0802 	2050	 |NUM | */
  Tg8HR_CLEAR_INT                 =2052	/* 0x0804 	2052	 |NUM | */
  Tg8HR_END                       =-1	/* 0xffffffff 	-1	 |NUM | */
  Tg8ENABLE_INT                   =0	/* 0x0000 	0	 |NUM | */

  Tg8TimingInterrupt              =4	/* 0x0004 	4	 |TYPE|  */
  Tg8DISABLE_INT                  =2	/* 0x0002 	2	 |NUM | */
  Tg8ENABLE_SYNC_INT              =1	/* 0x0001 	1	 |NUM | */
  Tg8DISABLE_SYNC_INT             =3	/* 0x0003 	3	 |NUM | */
  Tg8IS_MAILBOX                   =0	/* 0x0000 	0	 |NUM | */

  Tg8InterruptSource              =4	/* 0x0004 	4	 |TYPE|  */
  Tg8IS_IMM                       =1	/* 0x0001 	1	 |NUM | */
  Tg8IS_ERR                       =2	/* 0x0002 	2	 |NUM | */
  Tg8ISM_MAILBOX                  =1	/* 0x0001 	1	 |NUM | */
  Tg8ISM_IMM                      =2	/* 0x0002 	2	 |NUM | */
  Tg8ISM_ERR                      =4	/* 0x0004 	4	 |NUM | */

  Tg8MbxOpHdr                     =4	/* 0x0004 	4	 |TYPE|  */
  Row                             =0	/* 0x0000 	0	 |VAR | Tg8MbxOpHdr.Word 	=2= */
  Number                          =2	/* 0x0002 	2	 |VAR | Tg8MbxOpHdr.Word 	=2= */

  Tg8MbxRwAction                  =12	/* 0x000c 	12	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwAction.Tg8MbxOpHdr 	=4= */
  Actions                         =4	/* 0x0004 	4	 |VAR | Tg8MbxRwAction.Tg8User[1] 	=8= */

  Tg8MbxRwPpmAction               =156	/* 0x009c 	156	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwPpmAction.Tg8MbxOpHdr 	=4= */
  Actions                         =4	/* 0x0004 	4	 |VAR | Tg8MbxRwPpmAction.Tg8PpmUser[1] 	=152= */

  Tg8MbxRwPpmLine                 =16	/* 0x0010 	16	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwPpmLine.Tg8MbxOpHdr 	=4= */
  Lines                           =4	/* 0x0004 	4	 |VAR | Tg8MbxRwPpmLine.Tg8PpmLine[1] 	=12= */

  Tg8MbxRwGate                    =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwGate.Tg8MbxOpHdr 	=4= */
  Gates                           =4	/* 0x0004 	4	 |VAR | Tg8MbxRwGate.Tg8Gate[1] 	=4= */

  Tg8MbxRwDim                     =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRwDim.Tg8MbxOpHdr 	=4= */
  Dims                            =4	/* 0x0004 	4	 |VAR | Tg8MbxRwDim.Byte[4] 	=4= */

  Tg8MbxRecording                 =20	/* 0x0014 	20	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxRecording.Tg8MbxOpHdr 	=4= */
  Recs                            =4	/* 0x0004 	4	 |VAR | Tg8MbxRecording.Tg8Recording[1] 	=16= */

  Tg8MbxHistory                   =20	/* 0x0014 	20	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxHistory.Tg8MbxOpHdr 	=4= */
  Hist                            =4	/* 0x0004 	4	 |VAR | Tg8MbxHistory.Tg8History[1] 	=16= */

  Tg8MbxClock                     =24	/* 0x0018 	24	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxClock.Tg8MbxOpHdr 	=4= */
  Clks                            =4	/* 0x0004 	4	 |VAR | Tg8MbxClock.Tg8Clock[1] 	=20= */

  Tg8MbxClearAction               =4	/* 0x0004 	4	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxClearAction.Tg8MbxOpHdr 	=4= */

  Tg8MbxActionState               =8	/* 0x0008 	8	 |TYPE|  */
  Hdr                             =0	/* 0x0000 	0	 |VAR | Tg8MbxActionState.Tg8MbxOpHdr 	=4= */
  State                           =4	/* 0x0004 	4	 |VAR | Tg8MbxActionState.Word 	=2= */
  Aux                             =6	/* 0x0006 	6	 |VAR | Tg8MbxActionState.Word 	=2= */
  Tg8MBX_BUFF_SIZE                =1338	/* 0x053a 	1338	 |NUM | */

  Tg8BlockData                    =1338	/* 0x053a 	1338	 |TYPE|  */
  Mask                            =0	/* 0x0000 	0	 |VAR | _su_.Word 	=2= */
  SpareMsk                        =2	/* 0x0002 	2	 |VAR | _su_.Word 	=2= */
  SimPulse                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData._su_ 	=4= */
  HistoryBlock                    =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxHistory 	=20= */
  RwAction                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwAction 	=12= */
  RwPpmAction                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwPpmAction 	=156= */
  RwPpmLine                       =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwPpmLine 	=16= */
  RwGate                          =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwGate 	=8= */
  RwDim                           =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRwDim 	=8= */
  Recording                       =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxRecording 	=20= */
  ClockBlock                      =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxClock 	=24= */
  ClearAction                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxClearAction 	=4= */
  ActionState                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8MbxActionState 	=8= */
  DateTime                        =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8DateTime 	=12= */
  StatusBlock                     =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Tg8StatusBlock 	=32= */
  SelfTestResult                  =0	/* 0x0000 	0	 |VAR | Tg8BlockData.StDpmInfo 	=348= */
  MbxBuff                         =0	/* 0x0000 	0	 |VAR | Tg8BlockData.Byte[1338] 	=1338= */
  Tg8Dpm_spare                    =65536	/* 0x10000 	65536	 |NUM | */

  Tg8Dpm                          =67574	/* 0x107f6 	67574	 |TYPE|  */
  DpmSpare_Bytes                  =0	/* 0x0000 	0	 |VAR | Tg8Dpm.char[65536] 	=65536= */
  ExceptionVector                 =65536	/* 0x10000 	65536	 |VAR | Tg8Dpm.short 	=2= */
  Spare_ev                        =65538	/* 0x10002 	65538	 |VAR | Tg8Dpm.short 	=2= */
  ExceptionPC                     =65540	/* 0x10004 	65540	 |VAR | Tg8Dpm.Uint 	=4= */
  It                              =65544	/* 0x10008 	65544	 |VAR | Tg8Dpm.Tg8InterruptTable 	=320= */
  At                              =65864	/* 0x10148 	65864	 |VAR | Tg8Dpm.Tg8Aux 	=368= */
  BlockData                       =66232	/* 0x102b8 	66232	 |VAR | Tg8Dpm.Tg8BlockData 	=1338= */
  HrIntVme                        =67570	/* 0x107f2 	67570	 |VAR | Tg8Dpm.short 	=2= */
  HrIntMpc                        =67572	/* 0x107f4 	67572	 |VAR | Tg8Dpm.short 	=2= */

  Tg8Action                       =32	/* 0x0020 	32	 |TYPE|  */
  User                            =0	/* 0x0000 	0	 |VAR | Tg8Action.Tg8User 	=8= */
  Gate                            =8	/* 0x0008 	8	 |VAR | Tg8Action.Tg8Gate 	=4= */
  CntControl                      =12	/* 0x000c 	12	 |VAR | Tg8Action.Word 	=2= */
  CntNum                          =14	/* 0x000e 	14	 |VAR | Tg8Action.Byte 	=1= */
  Enabled                         =15	/* 0x000f 	15	 |VAR | Tg8Action.Byte 	=1= */
  Rec                             =16	/* 0x0010 	16	 |VAR | Tg8Action.Tg8Recording 	=16= */

  InterTelegram                   =264	/* 0x0108 	264	 |TYPE|  */
  aTeleg                          =0	/* 0x0000 	0	 |VAR | InterTelegram.Word[6]* 	=24= */
  nextLHC                         =24	/* 0x0018 	24	 |VAR | InterTelegram.Word[24] 	=48= */
  nextSPS                         =72	/* 0x0048 	72	 |VAR | InterTelegram.Word[24] 	=48= */
  nextCPS                         =120	/* 0x0078 	120	 |VAR | InterTelegram.Word[24] 	=48= */
  nextPSB                         =168	/* 0x00a8 	168	 |VAR | InterTelegram.Word[24] 	=48= */
  nextLEA                         =216	/* 0x00d8 	216	 |VAR | InterTelegram.Word[24] 	=48= */

  InterActTable                   =9216	/* 0x2400 	9216	 |TYPE|  */
  Pointers                        =0	/* 0x0000 	0	 |VAR | InterActTable.Tg8Action[256]* 	=1024= */
  Table                           =1024	/* 0x0400 	1024	 |VAR | InterActTable.Tg8Action[256] 	=8192= */

  StartedAct                      =520	/* 0x0208 	520	 |TYPE|  */
  EndMa                           =0	/* 0x0000 	0	 |VAR | StartedAct.Byte* 	=4= */
  ImmEndMa                        =4	/* 0x0004 	4	 |VAR | StartedAct.Byte* 	=4= */
  Matches                         =8	/* 0x0008 	8	 |VAR | StartedAct.Byte[256] 	=256= */
  ImmMatches                      =264	/* 0x0108 	264	 |VAR | StartedAct.Byte[256] 	=256= */
  QUEUE_SIZE                      =32	/* 0x0020 	32	 |NUM | */

  Queue                           =144	/* 0x0090 	144	 |TYPE|  */
  Bottom                          =0	/* 0x0000 	0	 |VAR | Queue.Uint* 	=4= */
  Top                             =4	/* 0x0004 	4	 |VAR | Queue.Uint* 	=4= */
  End                             =8	/* 0x0008 	8	 |VAR | Queue.Uint* 	=4= */
  Size                            =12	/* 0x000c 	12	 |VAR | Queue.int 	=4= */
  Buffer                          =16	/* 0x0010 	16	 |VAR | Queue.Uint[32] 	=128= */

  InterClockTable                 =328	/* 0x0148 	328	 |TYPE|  */
  N_of_clocks                     =0	/* 0x0000 	0	 |VAR | InterClockTable.short 	=2= */
  Clock_i                         =2	/* 0x0002 	2	 |VAR | InterClockTable.short 	=2= */
  Clock_p                         =4	/* 0x0004 	4	 |VAR | InterClockTable.Tg8Clock* 	=4= */
  Clocks                          =8	/* 0x0008 	8	 |VAR | InterClockTable.Tg8Clock[16] 	=320= */

  InterHistoryTable               =16008	/* 0x3e88 	16008	 |TYPE|  */
  N_of_histories                  =0	/* 0x0000 	0	 |VAR | InterHistoryTable.short 	=2= */
  History_i                       =2	/* 0x0002 	2	 |VAR | InterHistoryTable.short 	=2= */
  History_p                       =4	/* 0x0004 	4	 |VAR | InterHistoryTable.Tg8History* 	=4= */
  Histories                       =8	/* 0x0008 	8	 |VAR | InterHistoryTable.Tg8History[1000] 	=16000= */

  EProg                           =246	/* 0x00f6 	246	 |TYPE|  */
  IntSrc                          =0	/* 0x0000 	0	 |VAR | EProg.Word 	=2= */
  Alarms                          =2	/* 0x0002 	2	 |VAR | EProg.Word 	=2= */
  Event                           =4	/* 0x0004 	4	 |VAR | EProg.Tg8Event 	=4= */
  ImmEvent                        =8	/* 0x0008 	8	 |VAR | EProg.Tg8Event 	=4= */
  WildCard                        =12	/* 0x000c 	12	 |VAR | EProg.Word 	=2= */
  ImmWildCard                     =14	/* 0x000e 	14	 |VAR | EProg.Word 	=2= */
  ImmRun                          =16	/* 0x0010 	16	 |VAR | EProg.Word 	=2= */
  ImmSize                         =18	/* 0x0012 	18	 |VAR | EProg.Word 	=2= */
  ImmNxt                          =20	/* 0x0014 	20	 |VAR | EProg.Word 	=2= */
  ImmInt                          =22	/* 0x0016 	22	 |VAR | EProg.Tg8Interrupt[8] 	=160= */
  iFired                          =182	/* 0x00b6 	182	 |VAR | EProg.Tg8Interrupt[8]* 	=32= */
  rFired                          =214	/* 0x00d6 	214	 |VAR | EProg.Tg8Recording[8]* 	=32= */
  SUBSET_MASK                     =-268435456	/* 0xf0000000 	-268435456	 |NUM | */
  WILDCARDS_NUMBER                =8	/* 0x0008 	8	 |NUM | */

  Tg8TimeStamp                    =3	/* 0x0003 	3	 |TYPE|  */
  tsHour                          =0	/* 0x0000 	0	 |VAR | Tg8TimeStamp.Byte 	=1= */
  tsMinute                        =1	/* 0x0001 	1	 |VAR | Tg8TimeStamp.Byte 	=1= */
  tsSecond                        =2	/* 0x0002 	2	 |VAR | Tg8TimeStamp.Byte 	=1= */

  Tg8ScTime                       =8	/* 0x0008 	8	 |TYPE|  */
  stScNum                         =0	/* 0x0000 	0	 |VAR | Tg8ScTime.Uint 	=4= */
  stMs                            =4	/* 0x0004 	4	 |VAR | Tg8ScTime.Uint 	=4= */

  Tg8TwoBytes                     =2	/* 0x0002 	2	 |TYPE|  */
  Byte_1                          =0	/* 0x0000 	0	 |VAR | Tg8TwoBytes.Uchar 	=1= */
  Byte_2                          =1	/* 0x0001 	1	 |VAR | Tg8TwoBytes.Uchar 	=1= */

  Tg8TwoShorts                    =4	/* 0x0004 	4	 |TYPE|  */
  Short_1                         =0	/* 0x0000 	0	 |VAR | Tg8TwoShorts.short 	=2= */
  Short_2                         =2	/* 0x0002 	2	 |VAR | Tg8TwoShorts.short 	=2= */

  Tg8ShortOrTwoBytes              =2	/* 0x0002 	2	 |TYPE|  */
  Bytes                           =0	/* 0x0000 	0	 |VAR | Tg8ShortOrTwoBytes.Tg8TwoBytes 	=2= */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8ShortOrTwoBytes.short 	=2= */

  Tg8LongOrTwoShorts              =4	/* 0x0004 	4	 |TYPE|  */
  Short                           =0	/* 0x0000 	0	 |VAR | Tg8LongOrTwoShorts.Tg8TwoShorts 	=4= */
  Long                            =0	/* 0x0000 	0	 |VAR | Tg8LongOrTwoShorts.long 	=4= */
  ANY_OTHER_EVENT_INDEX           =0	/* 0x0000 	0	 |NUM | */

  EventIndex                      =4	/* 0x0004 	4	 |TYPE|  */
  MILLISECOND_EVENT_INDEX         =1	/* 0x0001 	1	 |NUM | */
  TIME_EVENT_INDEX                =2	/* 0x0002 	2	 |NUM | */
  DATE_EVENT_INDEX                =3	/* 0x0003 	3	 |NUM | */

	 .text
      .globl __main
      __main: bra Main 
#NO_APP
.text
	.even
.globl _main_prog
_main_prog:
	link a6,#0
#APP
	
 .text
 .globl Main
Main: 
#NO_APP
	jbsr _Init
	jbsr _MbxProcess
L1:
	unlk a6
	rts
	.even
_Init:
	link a6,#-8
	moveml #0x3030,sp@-
#APP
	 movew #Tg8DISABLE_INTERRUPTS,SR 
#NO_APP
	moveq #4,d3
	movel d3,a6@(-8)
	movew #255,a2
L3:
	tstl a2
	jge L6
	jra L4
	.even
L6:
	movel a6@(-8),a0
	movel #_Default_Isr,a0@
	addql #4,a6@(-8)
L5:
	subql #1,a2
	jra L3
	.even
L4:
	movel #256,a6@(-8)
	movel #_Tpu0_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu1_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu2_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu3_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu4_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu5_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu6_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu7_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	addql #4,a6@(-8)
	movel #_Tpu8_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #124,d3
	movel d3,a6@(-8)
	movel #_Abort_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #120,d3
	movel d3,a6@(-8)
	movel #_Xr_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #112,d3
	movel d3,a6@(-8)
	movel #_Dsc_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #8,d3
	movel d3,a6@(-8)
	movel #_BusError_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #12,d3
	movel d3,a6@(-8)
	movel #_AddressError_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #32,d3
	movel d3,a6@(-8)
	movel #_PrivViolation_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	moveq #96,d3
	movel d3,a6@(-8)
	movel #_Spurious_Isr,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #128,a6@(-8)
	movel #_AtCompletion,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #132,a6@(-8)
	movel #_InsertToCam,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #136,a6@(-8)
	movel #_ClearCam,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #140,a6@(-8)
	movel #_SetIntSourceMask,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #144,a6@(-8)
	movel #_AtStartProcess,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #148,a6@(-8)
	movel #_ImmCompletion,a6@(-4)
	movel a6@(-8),a0
	movel a6@(-4),a0@
	movel #-1536,_sim
	movel #-512,_tpu
	movel #73728,_xlx
	movel #69632,_cam
	movel #65536,_dpm
	movel _sim,a0
	movel #18906160,a0@(88)
	movel _sim,a0
	movel #-497649,a0@(92)
	movel _sim,a0
	movel #-497651,a0@(96)
	movel _sim,a0
	movel #17855536,a0@(100)
	movel _sim,a0
	movel #17852464,a0@(104)
	movel _sim,a0
	movel #16808944,a0@(108)
	movel _tpu,a0
	movew #7,a0@
	movel _tpu,a0
	movew #1856,a0@(8)
	movel _tpu,a0
	movew #10,a0@(14)
	movel _tpu,a0
	movew #-21846,a0@(16)
	movel _tpu,a0
	movew #-21846,a0@(18)
	movel _tpu,a0
	movew #1,a0@(20)
	movel _tpu,a0
	movew #21845,a0@(22)
	movel _tpu,a0
	clrw a0@(10)
#APP
	 movew #0,TpuIs
#NO_APP
	movew #-256,a2
	clrl d2
L7:
	moveq #15,d3
	cmpl d2,d3
	jge L10
	jra L8
	.even
L10:
	movel d2,d0
	movel d0,d1
	lsll #2,d1
	lea _var,a0
	movel a2,a0@(2,d1:l)
	addql #8,a2
	addql #8,a2
L9:
	addql #1,d2
	jra L7
	.even
L8:
	nop
	clrl d2
L11:
	moveq #8,d3
	cmpl d2,d3
	jge L14
	jra L12
	.even
L14:
	movel d2,d0
	movel d0,d1
	lsll #2,d1
	lea _var,a0
	movel a0@(2,d1:l),a3
	movew #7,a3@
	addql #2,a3
	movew #14,a3@
	addql #2,a3
	movew #1,a3@
	addql #2,a3
	clrw a3@
	addql #2,a3
L13:
	addql #1,d2
	jra L11
	.even
L12:
	movel _tpu,a0
	movew #1,a0@(24)
	movel _tpu,a0
	movew #21845,a0@(26)
	movel _tpu,a0
	movew #2,a0@(28)
	movel _tpu,a0
	movew #-21845,a0@(30)
	movel _tpu,a0
	movew #510,a0@(10)
	movel _sim,a0
	movew #-15,a0@(30)
	movel _sim,a0
	movew #14,a0@(28)
	movel _sim,a0
	movel _sim,a1
	movew a1@(26),d0
	movew d0,d1
	orw #12,d1
	movew d1,a0@(26)
	movel _sim,a0
	clrw a0@(26)
	movel _sim,a0
	movel _sim,a1
	movew a1@(26),d0
	movew d0,d1
	orw #12,d1
	movew d1,a0@(26)
	movel _xlx,a0
	movew #32,a0@(32)
	jbsr _SoftInit
L2:
	moveml a6@(-24),#0xc0c
	unlk a6
	rts
	.even
_CamInitialize:
	link a6,#0
#APP
	 andiw #CamCOMMAND_MODE,SimDataF1 
	 movel _cam,a0
		       movew #0,a0@
	 moveal _cam,a0
	 movew  #CamWRITE,d0
	 movel  #255,d1
      0: oriw   #CamDATA_MODE,SimDataF1  /* Set data mode */
	 movew  #0,a0@                   /* Clear the comparand register */
	 movew  #0,a0@                   /* words 0 1 2. */
	 movew  #0,a0@
	 andiw  #CamCOMMAND_MODE,SimDataF1 /* Set command mode */
	 movew  d0,a0@                     /* Do the COPY command */
	 addiw  #1,d0                      /* Next CAM array address */
	 dbf    d1,0b
	 movew  #0,a0@ 
#NO_APP
L15:
	unlk a6
	rts
LC0:
	.ascii "Apr  3 2006\0"
	.even
_SoftInit:
	link a6,#-16
	movel d2,sp@-
#APP
	 movew #Tg8DISABLE_INTERRUPTS,SR 
#NO_APP
	movel _sim,a0
	clrw a0@(32)
	jbsr _CamInitialize
	pea 348:w
	clrl sp@-
	pea _info
	jbsr _memset
	addql #8,sp
	addql #4,sp
	movel _dpm,a0
	clrw a0@
	movel _dpm,a0
	clrl a0@(4)
	pea 320:w
	clrl sp@-
	movel _dpm,d0
	addql #8,d0
	movel d0,sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 368:w
	clrl sp@-
	movel _dpm,d0
	addl #328,d0
	movel d0,sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 1338:w
	clrl sp@-
	movel _dpm,d0
	addl #696,d0
	movel d0,sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
	clrw _camBusy
	pea 246:w
	clrl sp@-
	pea _eprog
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 9216:w
	clrl sp@-
	pea _act
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 328:w
	clrl sp@-
	pea _clk
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 16008:w
	clrl sp@-
	pea _hist
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 520:w
	clrl sp@-
	pea _match
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 264:w
	clrl sp@-
	pea _tel
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 144:w
	clrl sp@-
	pea _atQueue
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 144:w
	clrl sp@-
	pea _immQueue
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 8:w
	clrl sp@-
	pea _ins
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 66:w
	clrl sp@-
	pea _var
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 1808:w
	clrl sp@-
	pea _in_use
	jbsr _memset
	addql #8,sp
	addql #4,sp
	clrl _wild_c
	movel #255,_wild_c+4
	movel #65280,_wild_c+8
	movel #65535,_wild_c+12
	movel #16711680,_wild_c+16
	movel #16711935,_wild_c+20
	movel #16776960,_wild_c+24
	movel #16777215,_wild_c+28
	pea 16:w
	clrl sp@-
	pea _time_event_index
	jbsr _memset
	addql #8,sp
	addql #4,sp
	moveb #1,_time_event_index+1
	moveb #2,_time_event_index+4
	moveb #2,_time_event_index+3
	moveb #2,_time_event_index+2
	moveb #2,_time_event_index+8
	moveb #3,_time_event_index+7
	moveb #3,_time_event_index+6
	moveb #3,_time_event_index+5
	moveb #3,_time_event_index+9
	movel _dpm,d2
	addl #456,d2
	movel d2,_tel+4
	movel _dpm,d2
	addl #504,d2
	movel d2,_tel+8
	movel _dpm,d2
	addl #552,d2
	movel d2,_tel+12
	movel _dpm,d2
	addl #600,d2
	movel d2,_tel+16
	movel _dpm,d2
	addl #648,d2
	movel d2,_tel+20
	clrl a6@(-8)
	movel #_act+1024,a6@(-4)
L17:
	cmpl #255,a6@(-8)
	jle L20
	jra L18
	.even
L20:
	movel a6@(-8),d0
	movel d0,d1
	movel d1,d0
	lsll #2,d0
	lea _act,a0
	movel a6@(-4),a0@(d0:l)
L19:
	addql #1,a6@(-8)
	moveq #32,d2
	addl d2,a6@(-4)
	jra L17
	.even
L18:
	movel #_clk+8,_clk+4
	movel #_hist+8,_hist+4
	movel #_atQueue+16,d0
	movel d0,_atQueue
	movel d0,_atQueue+4
	movel #_atQueue+144,_atQueue+8
	movel #_immQueue+16,d0
	movel d0,_immQueue
	movel d0,_immQueue+4
	movel #_immQueue+144,_immQueue+8
	pea 12:w
	pea LC0
	movel _dpm,d0
	addl #388,d0
	movel d0,sp@-
	jbsr _memcpy16
	addql #8,sp
	addql #4,sp
	movel _xlx,a0
	movew a0@(36),a6@(-10)
	movel _xlx,a0
	clrw a0@(36)
	moveb a6@(-9),_rcv_error
	clrb _rcv_error+1
	movel _dpm,a0
	lea a0@(380),a0
	movew _rcv_error,a0@
	movel _dpm,a0
	clrw a0@(360)
	movel _dpm,a0
	clrw a0@(362)
	movel _dpm,a0
	movew #32,a0@(366)
	movel _dpm,a0
	movew #1,a0@(364)
	movel _sim,a0
	movew #160,a0@(32)
	moveb #31,_dm+11
	moveb #31,_dm+9
	moveb #31,_dm+7
	moveb #31,_dm+6
	moveb #31,_dm+4
	moveb #31,_dm+2
	moveb #31,_dm
	moveb #28,_dm+1
	moveb #30,_dm+10
	moveb #30,_dm+8
	moveb #30,_dm+5
	moveb #30,_dm+3
#APP
	 movew #Tg8ENABLE_INTERRUPTS,SR 
#NO_APP
L16:
	movel a6@(-20),d2
	unlk a6
	rts
	.even
_InsertAction:
	link a6,#-40
	moveml #0x3030,sp@-
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-8)
	pea 32:w
	clrl sp@-
	movel a6@(-8),sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
	tstl a6@(12)
	jeq L22
	movel a6@(8),a6@(-4)
	movel a6@(-4),a0
	clrw d0
	moveb a0@(7),d0
	movew d0,a6@(-36)
	clrl d0
	movew _ins,d0
	movew a6@(-36),a0
	lea a0@(0,d0:l),a1
	cmpl #256,a1
	jle L23
	moveq #-4,d0
	jra L21
	.even
L23:
	movel a6@(-8),a0
	movel a6@(-4),a1
	movel a1@,a0@
	movel a6@(-8),a0
	movel a6@(-4),a1
	movew a1@(56),a0@(4)
	movel a6@(-8),a0
	movel a6@(-4),a1
	movew a1@(104),a0@(6)
	movel a6@(-8),a0
	movel a6@(-4),a1
	moveb a1@(4),d1
	moveb d1,d0
	lslb #4,d0
	movel a6@(-4),a1
	moveb d0,d2
	orb a1@(6),d2
	moveb d2,a0@(8)
	movel a6@(-8),a0
	movel a6@(-4),a1
	moveb a1@(5),a0@(9)
	movel a6@(-8),a0
	movel a6@(-4),a1
	movew a1@(8),a0@(10)
	jra L24
	.even
L22:
	movel a6@(-8),a0
	movel a6@(8),a1
	movel a1@,d2
	movel a1@(4),d3
	movel d2,a0@
	movel d3,a0@(4)
	movew #1,a6@(-36)
L24:
	movel a6@(-8),a0
	moveb a6@(-35),d0
	moveb d0,d1
	lslb #1,d1
	moveb d1,a0@(15)
	movel a6@(-8),a0
	movel a0@,a6@(-16)
	movel a6@(-8),a0
	movew a0@(4),a6@(-32)
	movew a6@(-32),d1
	movew d1,d0
	asrw #8,d0
	movew d0,d3
	andw #7,d3
	movew d3,a6@(-24)
	tstw a6@(-24)
	jeq L25
	movew a6@(-24),d0
	subqw #1,d0
	jra L26
	.even
L25:
	moveq #7,d0
L26:
	movew d0,a6@(-24)
	movel a6@(-8),a0
	moveb a6@(-23),a0@(14)
	movew a6@(-32),a0
	movel a0,sp@-
	jbsr _GetXConfiguration
	addql #4,sp
	movew d0,a6@(-30)
	tstw a6@(-30)
	jge L27
	movew a6@(-30),a0
	movel a0,d0
	jra L21
	.even
L27:
	movel a6@(-8),a0
	movew a6@(-30),a0@(12)
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L28
	movel a6@(-8),a0
	cmpw #65535,a0@(6)
	jne L29
	moveq #-5,d0
	jra L21
	.even
L29:
	movel a6@(-8),a0
	addqw #1,a0@(6)
	movew #1,a6@(-38)
	jra L30
	.even
L28:
	movel a6@(-8),a0
	clrw a0@(6)
	clrw a6@(-38)
L30:
	movew a6@(-32),d1
	movew d1,d0
	moveq #14,d2
	asrw d2,d0
	movew d0,d1
	andw #3,d1
	tstw d1
	jne L31
	orw #4,a6@(-32)
	movel a6@(-8),a0
	movew a6@(-32),a0@(4)
L31:
	movew a6@(-32),d1
	movew d1,d0
	asrw #3,d0
	movew d0,d1
	andw #3,d1
	cmpw #3,d1
	jeq L32
	clrw a6@(-22)
	cmpb #255,a6@(-15)
	jne L33
	orw #4,a6@(-22)
L33:
	cmpb #255,a6@(-14)
	jne L34
	orw #2,a6@(-22)
L34:
	cmpb #255,a6@(-13)
	jne L35
	orw #1,a6@(-22)
L35:
	movew a6@(-22),d0
	extl d0
	moveq #1,d1
	movel d1,d3
	lsll d0,d3
	movel d3,d0
	movew d0,a6@(-28)
	cmpb #1,a6@(-16)
	jeq L36
	clrl d0
	moveb a6@(-16),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-16),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(256),d2
	orb a6@(-27),d2
	moveb d2,a0@(256)
	moveb a6@(-16),d1
	moveb d1,d0
	lsrb #4,d0
	clrw d1
	moveb d0,d1
	movew d1,a6@(-26)
	movew a6@(-26),d0
	extl d0
	moveq #1,d1
	movel d1,d3
	lsll d0,d3
	movel d3,d0
	movew d0,a6@(-26)
	movew a6@(-22),d0
	andw #4,d0
	tstw d0
	jne L37
	clrl d0
	moveb a6@(-15),d0
	lea _in_use,a0
	clrl d1
	moveb a6@(-15),d1
	lea _in_use,a1
	moveb a1@(d1:l),d2
	orb a6@(-25),d2
	moveb d2,a0@(d0:l)
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L38
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(512),d3
	orb a6@(-27),d3
	moveb d3,a0@(512)
	jra L39
	.even
L38:
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(768),d2
	orb a6@(-27),d2
	moveb d2,a0@(768)
L39:
	jra L40
	.even
L37:
	nop
	movew #255,a6@(-24)
L41:
	tstw a6@(-24)
	jge L44
	jra L40
	.even
L44:
	movew a6@(-24),a0
	lea _in_use,a1
	movew a6@(-24),a2
	lea _in_use,a3
	moveb a2@(a3:l),d3
	orb a6@(-25),d3
	moveb d3,a0@(a1:l)
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L45
	movew a6@(-24),a0
	movel #_in_use,d0
	lea a0@(0,d0:l),a0
	movew a6@(-24),a1
	movel #_in_use,d0
	lea a1@(0,d0:l),a1
	moveb a1@(512),d2
	orb a6@(-27),d2
	moveb d2,a0@(512)
	jra L43
	.even
L45:
	movew a6@(-24),a0
	movel #_in_use,d0
	lea a0@(0,d0:l),a0
	movew a6@(-24),a1
	movel #_in_use,d0
	lea a1@(0,d0:l),a1
	moveb a1@(768),d3
	orb a6@(-27),d3
	moveb d3,a0@(768)
L46:
L43:
	subqw #1,a6@(-24)
	jra L41
	.even
L42:
L40:
	jra L47
	.even
L36:
	clrl d0
	moveb a6@(-16),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-16),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(1280),d2
	orb a6@(-27),d2
	moveb d2,a0@(1280)
	clrl d0
	moveb a6@(-16),d0
	lea _time_event_index,a0
	clrw d1
	moveb a0@(d0:l),d1
	movew d1,a6@(-26)
	movew a6@(-26),d0
	extl d0
	moveq #1,d1
	movel d1,d3
	lsll d0,d3
	movel d3,d0
	movew d0,a6@(-26)
	movew a6@(-22),d0
	andw #4,d0
	tstw d0
	jne L48
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(1024),d2
	orb a6@(-25),d2
	moveb d2,a0@(1024)
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L49
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(1296),d3
	orb a6@(-27),d3
	moveb d3,a0@(1296)
	jra L50
	.even
L49:
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb a6@(-15),d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a1@(1552),d2
	orb a6@(-27),d2
	moveb d2,a0@(1552)
L50:
	jra L47
	.even
L48:
	nop
	movew #255,a6@(-24)
L52:
	tstw a6@(-24)
	jge L55
	jra L47
	.even
L55:
	movew a6@(-24),a0
	movel #_in_use,d0
	lea a0@(0,d0:l),a0
	movew a6@(-24),a1
	movel #_in_use,d0
	lea a1@(0,d0:l),a1
	moveb a1@(1024),d3
	orb a6@(-25),d3
	moveb d3,a0@(1024)
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L56
	movew a6@(-24),a0
	movel #_in_use,d0
	lea a0@(0,d0:l),a0
	movew a6@(-24),a1
	movel #_in_use,d0
	lea a1@(0,d0:l),a1
	moveb a1@(1296),d2
	orb a6@(-27),d2
	moveb d2,a0@(1296)
	jra L54
	.even
L56:
	movew a6@(-24),a0
	movel #_in_use,d0
	lea a0@(0,d0:l),a0
	movew a6@(-24),a1
	movel #_in_use,d0
	lea a1@(0,d0:l),a1
	moveb a1@(1552),d3
	orb a6@(-27),d3
	moveb d3,a0@(1552)
L57:
L54:
	subqw #1,a6@(-24)
	jra L52
	.even
L53:
L51:
L47:
	movew a6@(-32),d1
	movew d1,d0
	asrw #2,d0
	movew d0,d1
	andw #1,d1
	tstw d1
	jne L58
	movel a6@(-16),_ins+2
	movew a6@(-30),d0
	andw #1,d0
	tstw d0
	jeq L59
	movew #255,d0
	jra L60
	.even
L59:
	clrw d0
L60:
	movew d0,_ins+6
	jra L61
	.even
L58:
	clrl _ins+2
	clrw _ins+6
L61:
	tstl a6@(12)
	jeq L62
	movel a6@(-8),a0
	movel a6@(-8),a1
	movew a1@(4),d2
	orw #8,d2
	movew d2,a0@(4)
	movew #1,a6@(-24)
	movel a6@(-8),a6@(-12)
L63:
	movew a6@(-36),d3
	cmpw a6@(-24),d3
	jgt L66
	jra L62
	.even
L66:
	moveq #32,d2
	addl d2,a6@(-12)
	movel a6@(-12),d0
	movel a6@(-8),d1
	movel d1,a0
	movel d0,a1
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a6@(-12),a1
	movel a6@(-4),a0
	movew a6@(-24),a2
	movel a2,d0
	movel d0,d1
	addl d1,d1
	movew a0@(8,d1:l),a1@(10)
	movel a6@(-12),a0
	movel a6@(-4),a1
	movew a6@(-24),a2
	movel a2,d0
	movel d0,d1
	addl d1,d1
	movew a1@(56,d1:l),d0
	movew d0,a6@(-34)
	movew d0,a0@(4)
	movel a6@(-12),a0
	movel a6@(-4),a1
	movew a6@(-24),a2
	movel a2,d0
	movel d0,d1
	addl d1,d1
	movew a6@(-38),d3
	addw a1@(104,d1:l),d3
	movew d3,a0@(6)
	movew a6@(-34),d1
	movew d1,d0
	asrw #8,d0
	movew d0,d2
	andw #7,d2
	movew d2,a6@(-22)
	tstw a6@(-22)
	jeq L67
	movew a6@(-22),d0
	subqw #1,d0
	jra L68
	.even
L67:
	moveq #7,d0
L68:
	movew d0,a6@(-22)
	movew a6@(-22),a0
	movel a6@(-8),a1
	clrl d0
	moveb a1@(14),d0
	cmpl a0,d0
	jeq L69
	moveq #-2,d0
	jra L21
	.even
L69:
	movew a6@(-34),a0
	movel a0,sp@-
	jbsr _GetXConfiguration
	addql #4,sp
	movew d0,a6@(-22)
	tstw a6@(-22)
	jge L70
	movew a6@(-22),a0
	movel a0,d0
	jra L21
	.even
L70:
	movew a6@(-22),d0
	andw #1,d0
	movew a6@(-30),d1
	andw #1,d1
	cmpw d0,d1
	jeq L71
	moveq #-2,d0
	jra L21
	.even
L71:
	movel a6@(-12),a0
	movew a6@(-22),a0@(12)
	movew a6@(-22),d0
	andw #1,d0
	tstw d0
	jeq L72
	movel a6@(-12),a0
	tstw a0@(6)
	jne L73
	moveq #-5,d0
	jra L21
	.even
L73:
	jra L74
	.even
L72:
	movel a6@(-12),a0
	clrw a0@(6)
L74:
	movew a6@(-34),d1
	movew d1,d0
	moveq #14,d3
	asrw d3,d0
	movew d0,d1
	andw #3,d1
	tstw d1
	jne L75
	orw #4,a6@(-34)
L75:
	orw #24,a6@(-34)
	movel a6@(-12),a0
	movew a6@(-34),a0@(4)
	movew a6@(-34),d1
	movew d1,d0
	asrw #2,d0
	movew d0,d1
	andw #1,d1
	tstw d1
	jne L65
	movel a6@(-12),a0
	movel a6@(-12),a1
	moveb a1@(15),d2
	orb #1,d2
	moveb d2,a0@(15)
L76:
L65:
	addqw #1,a6@(-24)
	jra L63
	.even
L64:
L62:
#APP
	trap #1
#NO_APP
L32:
	movew a6@(-32),d1
	movew d1,d0
	asrw #2,d0
	movew d0,d1
	andw #1,d1
	tstw d1
	jne L77
	movel a6@(-8),a0
	movel a6@(-8),a1
	moveb a1@(15),d3
	orb #1,d3
	moveb d3,a0@(15)
L77:
	movew a6@(-36),d2
	addw d2,_ins
	movel _dpm,a0
	movew a0@(372),d0
	cmpw _ins,d0
	jcc L78
	movel _dpm,a0
	movew _ins,a0@(372)
L78:
	clrl d0
	jra L21
	.even
L21:
	moveml a6@(-56),#0xc0c
	unlk a6
	rts
	.even
_GetXConfiguration:
	link a6,#-4
	movel d2,sp@-
	clrl a6@(-4)
	movel a6@(8),d0
	movel d0,d1
	moveq #14,d2
	asrl d2,d1
	moveq #3,d0
	andl d1,d0
	moveq #2,d2
	cmpl d0,d2
	jeq L83
	moveq #2,d2
	cmpl d0,d2
	jlt L86
	moveq #1,d2
	cmpl d0,d2
	jeq L81
	jra L80
	.even
L86:
	moveq #3,d2
	cmpl d0,d2
	jeq L82
	jra L80
	.even
L81:
	moveq #1,d2
	orl d2,a6@(-4)
	jra L80
	.even
L82:
	moveq #1,d2
	orl d2,a6@(-4)
L83:
	moveq #4,d2
	orl d2,a6@(-4)
	jra L80
	.even
L85:
L80:
	moveq #3,d0
	andl a6@(8),d0
	moveq #2,d2
	cmpl d0,d2
	jeq L89
	moveq #2,d2
	cmpl d0,d2
	jlt L93
	tstl d0
	jeq L88
	jra L91
	.even
L93:
	moveq #3,d2
	cmpl d0,d2
	jeq L90
	jra L91
	.even
L88:
	jra L87
	.even
L89:
	moveq #16,d2
	orl d2,a6@(-4)
	jra L87
	.even
L90:
	moveq #24,d2
	orl d2,a6@(-4)
	jra L87
	.even
L91:
	moveq #-6,d0
	jra L79
	.even
L87:
	movel a6@(8),d0
	movel d0,d1
	asrl #6,d1
	moveq #3,d0
	andl d1,d0
	moveq #1,d2
	cmpl d0,d2
	jeq L96
	moveq #1,d2
	cmpl d0,d2
	jlt L100
	tstl d0
	jeq L95
	jra L98
	.even
L100:
	moveq #2,d2
	cmpl d0,d2
	jeq L97
	jra L98
	.even
L95:
	moveq #64,d2
	orl d2,a6@(-4)
	jra L94
	.even
L96:
	orw #128,a6@(-2)
	jra L94
	.even
L97:
	moveq #32,d2
	orl d2,a6@(-4)
	jra L94
	.even
L98:
	moveq #-7,d0
	jra L79
	.even
L94:
	movel a6@(-4),d1
	movel d1,d0
	jra L79
	.even
L79:
	movel sp@+,d2
	unlk a6
	rts
	.even
_memset:
	link a6,#-8
	movel d2,sp@-
	movel a6@(12),d0
	movew d0,a6@(-2)
	movel a6@(8),a6@(-6)
	movel a6@(16),d0
	movel d0,d1
	lsrl #1,d1
	movel d1,a6@(16)
L102:
	subql #1,a6@(16)
	moveq #-1,d2
	cmpl a6@(16),d2
	jne L104
	jra L103
	.even
L104:
	movel a6@(-6),a0
	movew a6@(-2),a0@
	addql #2,a6@(-6)
	jra L102
	.even
L103:
L101:
	movel sp@+,d2
	unlk a6
	rts
	.even
_memcpy16:
	link a6,#0
	movel a6@(16),d1
	movel d1,d0
	asrl #1,d0
L106:
	tstl d0
	jgt L109
	jra L107
	.even
L109:
	movel a6@(8),a0
	movel a6@(12),a1
	movew a1@,a0@
	addql #2,a6@(12)
	addql #2,a6@(8)
L108:
	subql #1,d0
	jra L106
	.even
L107:
	moveq #1,d1
	andl a6@(16),d1
	tstl d1
	jeq L110
	movel a6@(8),a0
	movel a6@(12),a1
	moveb a1@,a0@
L110:
L105:
	unlk a6
	rts
	.even
_bcopy:
	link a6,#0
	movel a6@(16),d1
	movel d1,d0
	asrl #1,d0
L112:
	tstl d0
	jgt L115
	jra L113
	.even
L115:
	movel a6@(12),a0
	movel a6@(8),a1
	movew a1@,a0@
	addql #2,a6@(8)
	addql #2,a6@(12)
L114:
	subql #1,d0
	jra L112
	.even
L113:
	moveq #1,d1
	andl a6@(16),d1
	tstl d1
	jeq L116
	movel a6@(12),a0
	movel a6@(8),a1
	moveb a1@,a0@
L116:
L111:
	unlk a6
	rts
	.even
_Debug:
	link a6,#-4
	movel a6@(8),d0
	movew d0,a6@(-2)
L118:
	movel _dpm,a0
	movew a0@(352),d0
	tstw d0
	jne L120
	jra L119
	.even
L120:
	jra L118
	.even
L119:
	movel _dpm,a0
	movew a6@(-2),a0@(352)
L117:
	unlk a6
	rts
	.even
.globl _DecToBcd
_DecToBcd:
	link a6,#-4
	moveml #0x3c00,sp@-
	movel a6@(8),d0
	moveb d0,a6@(-1)
	clrw d1
	moveb a6@(-1),d1
	clrl d0
	movew d1,d0
	movel d0,d1
	movel d1,d2
	addl d2,d2
	movel d2,d3
	addl d0,d3
	movel d3,d1
	lsll #4,d1
	movel d3,d2
	addl d1,d2
	movel d2,d1
	lsll #8,d1
	addl d1,d2
	movel d2,d1
	lsll #2,d1
	addl d1,d0
	movel d0,d2
	clrw d2
	swap d2
	movew d2,d1
	lsrw #3,d1
	moveb d1,d0
	lslb #4,d0
	clrw d1
	moveb a6@(-1),d1
	clrl d2
	movew d1,d2
	movel d2,d3
	movel d3,d4
	addl d4,d4
	movel d4,d5
	addl d2,d5
	movel d5,d3
	lsll #4,d3
	movel d5,d4
	addl d3,d4
	movel d4,d3
	lsll #8,d3
	addl d3,d4
	movel d4,d3
	lsll #2,d3
	addl d3,d2
	movel d2,d3
	clrw d3
	swap d3
	movew d3,d2
	lsrw #3,d2
	movew d2,d3
	movew d3,d4
	lslw #2,d4
	addw d4,d2
	movew d2,d3
	lslw #1,d3
	subw d3,d1
	moveb d1,d2
	andb #15,d2
	orb d2,d0
	clrl d1
	moveb d0,d1
	movel d1,d0
	jra L121
	.even
L121:
	moveml sp@+,#0x3c
	unlk a6
	rts
	.even
.globl _UtcToTime
_UtcToTime:
	link a6,#-24
	movel d4,sp@-
	movel d3,sp@-
	movel d2,sp@-
	addl #-1041379200,_utc
	clrl a6@(-10)
L123:
	movel #31536000,a6@(-4)
	clrb a6@(-5)
	movel a6@(-10),d0
	addql #3,d0
	moveq #3,d1
	andl d0,d1
	tstl d1
	jne L126
	addl #86400,a6@(-4)
	moveb #1,a6@(-5)
L126:
	movel _utc,d4
	cmpl a6@(-4),d4
	jcs L127
	movel a6@(-4),d4
	subl d4,_utc
	jra L128
	.even
L127:
	jra L124
	.even
L128:
	addql #1,a6@(-10)
L125:
	jra L129
	.even
	jra L124
	.even
L129:
	jra L123
	.even
L124:
	moveb a6@(-7),d0
	addqb #3,d0
	clrl d1
	moveb d0,d1
	movel d1,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-22)
	clrl a6@(-10)
L130:
	movel a6@(-10),d1
	lea _dm,a0
	clrl d0
	moveb a0@(d1:l),d0
	movel d0,d1
	movel d1,d2
	addl d2,d2
	movel d2,d1
	addl d0,d1
	movel d1,d0
	lsll #4,d0
	movel d0,d4
	subl d1,d4
	movel d4,d1
	movel d1,d0
	lsll #4,d0
	movel d0,d4
	subl d1,d4
	movel d4,d1
	movel d1,d0
	lsll #7,d0
	movel d0,a6@(-4)
	tstb a6@(-5)
	jeq L133
	moveq #1,d4
	cmpl a6@(-10),d4
	jne L133
	addl #86400,a6@(-4)
L133:
	movel _utc,d4
	cmpl a6@(-4),d4
	jcs L134
	movel a6@(-4),d4
	subl d4,_utc
	jra L135
	.even
L134:
	jra L131
	.even
L135:
	addql #1,a6@(-10)
L132:
	jra L136
	.even
	jra L131
	.even
L136:
	jra L130
	.even
L131:
	moveb a6@(-7),d0
	addqb #1,d0
	clrl d1
	moveb d0,d1
	movel d1,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-21)
	movel _utc,d1
	movel #0xc22e4507,d2
	movel d1,d0
	mulul d2,d1:d0
	movel d1,d0
	clrw d0
	swap d0
	moveb d0,d1
	addqb #1,d1
	clrl d0
	moveb d1,d0
	movel d0,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-20)
	movel _utc,d0
	movel #0xc22e4507,d3
	movel d0,d1
	mulul d3,d2:d1
	movel d2,d1
	clrw d1
	swap d1
	movel d1,d2
	movel d2,d3
	addl d3,d3
	movel d3,d2
	addl d1,d2
	movel d2,d1
	lsll #4,d1
	movel d1,d4
	subl d2,d4
	movel d4,d2
	movel d2,d1
	lsll #4,d1
	movel d1,d4
	subl d2,d4
	movel d4,d2
	movel d2,d1
	lsll #7,d1
	subl d1,d0
	movel d0,_utc
	movel _utc,d1
	movel #0x91a2b3c5,d2
	movel d1,d0
	mulul d2,d1:d0
	movel d1,d0
	moveq #11,d4
	lsrl d4,d0
	clrl d1
	moveb d0,d1
	movel d1,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-17)
	movel _utc,d0
	movel #0x91a2b3c5,d3
	movel d0,d1
	mulul d3,d2:d1
	movel d2,d1
	moveq #11,d4
	lsrl d4,d1
	movel d1,d2
	movel d2,d3
	lsll #3,d3
	movel d3,d2
	subl d1,d2
	movel d2,d3
	lsll #5,d3
	addl d3,d1
	movel d1,d2
	lsll #4,d2
	subl d2,d0
	movel d0,_utc
	movel _utc,d1
	movel #0x88888889,d2
	movel d1,d0
	mulul d2,d1:d0
	movel d1,d0
	lsrl #5,d0
	clrl d1
	moveb d0,d1
	movel d1,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-16)
	movel _utc,d0
	movel #0x88888889,d3
	movel d0,d1
	mulul d3,d2:d1
	movel d2,d1
	lsrl #5,d1
	movel d1,d2
	movel d2,d3
	lsll #4,d3
	movel d3,d4
	subl d1,d4
	movel d4,d1
	movel d1,d2
	lsll #2,d2
	subl d2,d0
	movel d0,_utc
	clrl d0
	moveb _utc+3,d0
	movel d0,sp@-
	jbsr _DecToBcd
	addql #4,sp
	moveb d0,a6@(-15)
	pea 12:w
	lea a6@(-22),a0
	movel a0,sp@-
	movel _dpm,d0
	addl #376,d0
	movel d0,sp@-
	jbsr _memcpy16
	addql #8,sp
	addql #4,sp
	jra L122
	.even
L122:
	moveml a6@(-36),#0x1c
	unlk a6
	rts
	.even
_MbxProcess:
	link a6,#-56
	moveml #0x3830,sp@-
	nop
L138:
#APP
	movew #0x55,SimServ
		  movew #0xAA,SimServ
	oriw  #OkLed,SimDataF1
		  andiw #~OkLed,SimDataF1
		  oriw  #OkLed,SimDataF1
#NO_APP
	movel _xlx,a0
	movew a0@(36),d0
	movew d0,a6@(-36)
	tstw d0
	jeq L141
	moveb a6@(-35),_rcv_error
	movel _dpm,a0
	lea a0@(380),a0
	movew _rcv_error,a0@
L141:
	tstw _eprog
	jeq L142
	movel _dpm,a0
	movew a0@(354),d0
	tstw d0
	jne L142
#APP
	trap  #3
#NO_APP
L142:
	movel _dpm,a0
	movew a0@(360),d1
	clrl d0
	movew d1,d0
	moveq #17,d3
	cmpl d0,d3
	jcs L266
	movel d0,d1
	addl d1,d1
	movel #L267,a0
	movew a0@(d1:l),d0
	jmp pc@(2,d0:w)
L267:
	.word L144-L267
	.word L145-L267
	.word L152-L267
	.word L265-L267
	.word L244-L267
	.word L228-L267
	.word L185-L267
	.word L194-L267
	.word L154-L267
	.word L161-L267
	.word L167-L267
	.word L176-L267
	.word L202-L267
	.word L216-L267
	.word L222-L267
	.word L209-L267
	.word L146-L267
	.word L153-L267
	.even
L144:
	jra L140
	.even
L145:
	jra L143
	.even
L146:
	movel _dpm,a0
	movew a0@(696),d0
	clrl d1
	movew d0,d1
	movel d1,a6@(-24)
	clrl a6@(-28)
L147:
	tstl a6@(-24)
	jne L150
	jra L148
	.even
L150:
	moveq #1,d0
	andl a6@(-24),d0
	tstl d0
	jeq L149
	movel _xlx,a0
	movel a6@(-28),d0
	movel d0,d1
	movel d1,d0
	addl d0,d0
	movew #1,a0@(d0:l)
	movel _xlx,a0
	movel a6@(-28),d0
	movel d0,d1
	movel d1,d0
	addl d0,d0
	movew #65,a0@(16,d0:l)
	movel a6@(-28),d0
	movel d0,d1
	movel d1,d2
	lsll #2,d2
	addl d2,d0
	movel d0,d1
	lsll #2,d1
	movel d1,d0
	addql #8,d0
	movel _dpm,d4
	addl d0,d4
	movel d4,a6@(-44)
	movel a6@(-28),d0
	movel d0,d1
	movel d1,d0
	lsll #2,d0
	movel #_eprog,d1
	movel d1,a0
	addl d0,a0
	movel a6@(-44),a0@(182)
	movel a6@(-28),d0
	movel d0,d1
	movel d1,d0
	lsll #2,d0
	movel #_eprog,d1
	movel d1,a0
	addl d0,a0
	movel #_sim_rec,a0@(214)
	movel a6@(-44),a0
	moveq #-1,d3
	movel d3,a0@(12)
	movel a6@(-44),a0
	clrl a0@
	movel a6@(-44),a0
	movel _dpm,a1
	movel a1@(344),a0@(4)
	movel a6@(-44),a0
	movel _dpm,a1
	movel a1@(340),a0@(8)
	moveb _rcv_error,a6@(-40)
	clrb a6@(-39)
	clrb a6@(-38)
	moveb #1,a6@(-37)
	movel a6@(-44),a1
	lea a6@(-40),a0
	addql #8,a1
	addql #8,a1
	moveb a0@,a1@
	addql #1,a1
	addql #1,a0
	moveb a0@,a1@
	addql #1,a1
	addql #1,a0
	moveb a0@,a1@
	addql #1,a1
	addql #1,a0
	moveb a0@,a1@
	addql #1,a1
	addql #1,a0
L151:
L149:
	addql #1,a6@(-28)
	movel a6@(-24),d0
	movel d0,d1
	asrl #1,d1
	movel d1,a6@(-24)
	jra L147
	.even
L148:
	jra L143
	.even
L152:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-44)
	movel a6@(-44),a0
	movel _dpm,a1
	movew a1@(364),a0@(24)
	movel a6@(-44),a0
	movel _dpm,a1
	movew a1@(374),a0@(28)
	movel _dpm,a0
	clrw a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	andw #65533,d1
	movew d1,a0@(364)
	movel a6@(-44),a0
	movel _dpm,a1
	movew a1@(362),a0@(26)
	movel a6@(-44),a0
	movel _dpm,a1
	movew a1@,a0@(20)
	movel a6@(-44),a0
	movel _dpm,a1
	movel a1@(4),a0@(16)
	movel a6@(-44),a0
	movel _dpm,a1
	movel a1@(340),a0@(12)
	movel a6@(-44),d0
	movel _dpm,a1
	lea a1@(376),a0
	movel d0,a1
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel _dpm,a1
	clrw a1@(408)
	jra L143
	.even
L153:
	movel _dpm,a0
	lea a0@(696),a1
	pea 348:w
	movel a1,sp@-
	pea _info
	jbsr _bcopy
	addql #8,sp
	addql #4,sp
	jra L143
	.even
L154:
	movel _dpm,a3
	lea a3@(696),a3
	movel a3,a6@(-44)
	movel a6@(-44),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-44),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L155
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L155:
	nop
	movel a6@(-44),a3
	addql #4,a3
	movel a3,a6@(-8)
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L156:
	tstl a6@(-28)
	jgt L159
	jra L157
	.even
L159:
	movel a6@(-8),a0
	movel a6@(-4),a1
	movel a1@,d3
	movel a1@(4),d4
	movel d3,a0@
	movel d4,a0@(4)
	movel a6@(-8),a0
	tstw a0@(6)
	jeq L158
	movel a6@(-8),a0
	subqw #1,a0@(6)
L160:
L158:
	subql #1,a6@(-28)
	addqw #1,_ins
	addql #8,a6@(-8)
	moveq #32,d4
	addl d4,a6@(-4)
	jra L156
	.even
L157:
	movel a6@(-44),a0
	movew _ins,a0@
	jra L143
	.even
L161:
	movel _dpm,a3
	lea a3@(696),a3
	movel a3,a6@(-44)
	movel a6@(-44),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-44),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L162
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L162:
	nop
	movel a6@(-44),a3
	addql #4,a3
	movel a3,a6@(-48)
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L163:
	tstl a6@(-28)
	jgt L166
	jra L164
	.even
L166:
	movel a6@(-48),d0
	movel a6@(-4),a1
	lea a1@(16),a0
	movel d0,a1
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
L165:
	subql #1,a6@(-28)
	addqw #1,_ins
	moveq #16,d3
	addl d3,a6@(-48)
	moveq #32,d4
	addl d4,a6@(-4)
	jra L163
	.even
L164:
	movel a6@(-44),a0
	movew _ins,a0@
	jra L143
	.even
L167:
	movel _dpm,a3
	lea a3@(696),a3
	movel a3,a6@(-48)
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	movel a6@(-48),a0
	clrl d0
	movew a0@,d0
	movel d0,a6@(-32)
	tstl a6@(-32)
	jne L168
	movew _hist+2,a3
	movel a3,a6@(-32)
L168:
	cmpl #999,a6@(-32)
	jle L169
	movel #999,a6@(-32)
L169:
	movel a6@(-32),d0
	movel d0,d1
	movel d1,d0
	lsll #4,d0
	movel #_hist+8,d1
	movel d1,d3
	addl d0,d3
	movel d3,a6@(-44)
	clrl a6@(-24)
	movel a6@(-48),d4
	addql #4,d4
	movel d4,a6@(-52)
L170:
	movel a6@(-24),a3
	cmpl a6@(-28),a3
	jlt L173
	jra L171
	.even
L173:
	subql #1,a6@(-32)
	tstl a6@(-32)
	jge L174
	movel #999,a6@(-32)
	movel a6@(-32),d0
	movel d0,d1
	movel d1,d0
	lsll #4,d0
	movel #_hist+8,d1
	movel d1,d3
	addl d0,d3
	movel d3,a6@(-44)
	jra L175
	.even
L174:
	moveq #-16,d4
	addl d4,a6@(-44)
L175:
	movel a6@(-52),d0
	movel a6@(-44),d1
	movel d1,a0
	movel d0,a1
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
L172:
	addql #1,a6@(-24)
	moveq #16,d3
	addl d3,a6@(-52)
	jra L170
	.even
L171:
	movel a6@(-48),a0
	movew a6@(-30),a0@
	jra L143
	.even
L176:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	movel a6@(-48),a0
	clrl d0
	movew a0@,d0
	movel d0,a6@(-32)
	tstl a6@(-32)
	jne L177
	movew _clk+2,a3
	movel a3,a6@(-32)
L177:
	moveq #15,d3
	cmpl a6@(-32),d3
	jge L178
	moveq #15,d4
	movel d4,a6@(-32)
L178:
	movel a6@(-32),d0
	movel d0,d1
	movel d1,d2
	lsll #2,d2
	addl d2,d0
	movel d0,d1
	lsll #2,d1
	movel #_clk+8,d0
	movel d0,a3
	addl d1,a3
	movel a3,a6@(-44)
	clrl a6@(-24)
	movel a6@(-48),d3
	addql #4,d3
	movel d3,a6@(-56)
L179:
	movel a6@(-24),d4
	cmpl a6@(-28),d4
	jlt L182
	jra L180
	.even
L182:
	subql #1,a6@(-32)
	tstl a6@(-32)
	jge L183
	moveq #15,d3
	movel d3,a6@(-32)
	movel a6@(-32),d0
	movel d0,d1
	movel d1,d2
	lsll #2,d2
	addl d2,d0
	movel d0,d1
	lsll #2,d1
	movel #_clk+8,d0
	movel d0,d4
	addl d1,d4
	movel d4,a6@(-44)
	jra L184
	.even
L183:
	moveq #-20,d3
	addl d3,a6@(-44)
L184:
	movel a6@(-56),d0
	movel a6@(-44),d1
	movel d1,a0
	movel d0,a1
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
	movel a0@,a1@
	addql #4,a1
	addql #4,a0
L181:
	addql #1,a6@(-24)
	moveq #20,d4
	addl d4,a6@(-56)
	jra L179
	.even
L180:
	movel a6@(-48),a0
	movew a6@(-30),a0@
	jra L143
	.even
L185:
	movel _dpm,a3
	lea a3@(696),a3
	movel a3,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d0
	andw #255,d0
	clrl d1
	movew d0,d1
	movel d1,d0
	movel d0,a6@(-32)
	movew d0,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L186
	clrl d0
	movew _ins,d0
	movel #256,d3
	subl d0,d3
	movel d3,a6@(-28)
L186:
	clrl _ins+2
	clrw _ins+6
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L187:
	tstl a6@(-28)
	jle L191
	movel _dpm,a0
	movew a0@(372),d0
	cmpw _ins,d0
	jhi L190
	jra L191
	.even
L191:
	jra L188
	.even
L190:
	pea 32:w
	clrl sp@-
	movel a6@(-4),sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
#APP
	trap #1
#NO_APP
L189:
	subql #1,a6@(-28)
	moveq #32,d4
	addl d4,a6@(-4)
	addqw #1,_ins
	jra L187
	.even
L188:
	movel _dpm,a0
	movew a0@(372),d0
	cmpw _ins,d0
	jne L192
	movel _dpm,a0
	movew a6@(-30),a0@(372)
	tstl a6@(-32)
	jne L192
	pea 320:w
	clrl sp@-
	movel _dpm,d0
	addql #8,d0
	movel d0,sp@-
	jbsr _memset
	addql #8,sp
	addql #4,sp
	pea 1808:w
	clrl sp@-
	pea _in_use
	jbsr _memset
	addql #8,sp
	addql #4,sp
L193:
L192:
	jra L143
	.even
L194:
	movel _dpm,a3
	lea a3@(696),a3
	movel a3,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L195
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L195:
	nop
	movel a6@(-48),a3
	addql #4,a3
	movel a3,a6@(-8)
L196:
	tstl a6@(-28)
	jgt L199
	jra L197
	.even
L199:
	clrl sp@-
	movel a6@(-8),sp@-
	jbsr _InsertAction
	addql #8,sp
	movel d0,a6@(-24)
	tstl a6@(-24)
	jge L198
	movel _dpm,a0
	movew _ins,a0@(352)
	movel _dpm,a0
	movew a6@(-22),a0@(362)
	jra L201
	.even
L200:
L198:
	subql #1,a6@(-28)
	addql #8,a6@(-8)
	jra L196
	.even
L197:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L202:
	movel _dpm,d3
	addl #696,d3
	movel d3,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d4
	andw #255,d4
	movew d4,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L203
	clrl d0
	movew _ins,d0
	movew #256,a3
	subl d0,a3
	movel a3,a6@(-28)
L203:
	nop
	movel a6@(-48),d3
	addql #4,d3
	movel d3,a6@(-12)
L204:
	tstl a6@(-28)
	jgt L207
	jra L205
	.even
L207:
	pea 1:w
	movel a6@(-12),sp@-
	jbsr _InsertAction
	addql #8,sp
	movel d0,a6@(-24)
	tstl a6@(-24)
	jge L206
	movel _dpm,a0
	movew _ins,a0@(352)
	movel _dpm,a0
	movew a6@(-22),a0@(362)
	jra L201
	.even
L208:
L206:
	subql #1,a6@(-28)
	addl #152,a6@(-12)
	jra L204
	.even
L205:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L209:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L210
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L210:
	nop
	movel a6@(-48),a3
	addql #4,a3
	movel a3,a6@(-16)
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L211:
	tstl a6@(-28)
	jgt L214
	jra L212
	.even
L214:
	movel a6@(-16),a0
	movel a6@(-4),a1
	movel a1@,d3
	movel a1@(4),d4
	movel d3,a0@
	movel d4,a0@(4)
	movel a6@(-16),a0
	tstw a0@(6)
	jeq L215
	movel a6@(-16),a0
	subqw #1,a0@(6)
L215:
	movel a6@(-16),a0
	movel a6@(-4),a1
	movel a1@(8),a0@(8)
L213:
	subql #1,a6@(-28)
	addqw #1,_ins
	moveq #12,d4
	addl d4,a6@(-16)
	moveq #32,d3
	addl d3,a6@(-4)
	jra L211
	.even
L212:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L216:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L217
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L217:
	nop
	movel a6@(-48),a3
	addql #4,a3
	movel a3,a6@(-20)
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L218:
	tstl a6@(-28)
	jgt L221
	jra L219
	.even
L221:
	movel a6@(-4),a0
	movel a6@(-20),a1
	movel a1@,a0@(8)
L220:
	subql #1,a6@(-28)
	addql #4,a6@(-20)
	moveq #32,d3
	addl d3,a6@(-4)
	addqw #1,_ins
	jra L218
	.even
L219:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L222:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L223
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L223:
	nop
	movel a6@(-48),a3
	addql #4,a3
	movel a3,a6@(-44)
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L224:
	tstl a6@(-28)
	jgt L227
	jra L225
	.even
L227:
	movel a6@(-4),a0
	movel a6@(-4),a1
	moveb a1@(15),d3
	andb #1,d3
	moveb d3,a0@(15)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movel a6@(-44),a2
	moveb a2@,d1
	moveb d1,d0
	lslb #1,d0
	moveb a1@(15),d4
	orb d0,d4
	moveb d4,a0@(15)
L226:
	subql #1,a6@(-28)
	addql #1,a6@(-44)
	moveq #32,d3
	addl d3,a6@(-4)
	addqw #1,_ins
	jra L224
	.even
L225:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L228:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L229
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L229:
	nop
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L230:
	tstl a6@(-28)
	jgt L233
	jra L231
	.even
L233:
	movel a6@(-4),a0
	movew a0@(12),d0
	andw #1,d0
	tstw d0
	jeq L232
	movel a6@(-48),a0
	cmpw #65535,a0@(4)
	jne L235
	movel _dpm,a0
	movew _ins,a0@(352)
	movel _dpm,a0
	movew #-5,a0@(362)
	jra L201
	.even
L235:
	movel a6@(-48),a0
	tstw a0@(6)
	jeq L236
	movel a6@(-4),a0
	movew a0@(12),d3
	andw #65511,d3
	movew d3,a6@(-34)
	movel a6@(-48),a0
	movel a6@(-48),a1
	movew a1@(6),d0
	andw #3,d0
	movew d0,d1
	movew d1,a0@(6)
	clrl d0
	movew d1,d0
	moveq #2,d4
	cmpl d0,d4
	jeq L239
	moveq #2,d3
	cmpl d0,d3
	jlt L243
	tstl d0
	jeq L238
	jra L241
	.even
L243:
	moveq #3,d4
	cmpl d0,d4
	jeq L240
	jra L241
	.even
L238:
	jra L237
	.even
L239:
	orw #16,a6@(-34)
	jra L237
	.even
L240:
	orw #24,a6@(-34)
	jra L237
	.even
L241:
	movel _dpm,a0
	movew _ins,a0@(352)
	movel _dpm,a0
	movew #-6,a0@(362)
	jra L201
	.even
L237:
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(4),d3
	andw #65532,d3
	movew d3,a0@(4)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movel a6@(-48),a2
	movew a1@(4),d4
	orw a2@(6),d4
	movew d4,a0@(4)
	movel a6@(-4),a0
	movew a6@(-34),a0@(12)
L236:
	movel a6@(-4),a0
	movel a6@(-48),a1
	movew a1@(4),a3
	addqw #1,a3
	movew a3,a0@(6)
L234:
L232:
	subql #1,a6@(-28)
	moveq #32,d3
	addl d3,a6@(-4)
	addqw #1,_ins
	jra L230
	.even
L231:
	movel a6@(-48),a0
	movew _ins,a0@
	jra L143
	.even
L244:
	movel _dpm,d4
	addl #696,d4
	movel d4,a6@(-48)
	movel a6@(-48),a0
	movew a0@,d3
	andw #255,d3
	movew d3,_ins
	movel a6@(-48),a0
	clrl d0
	movew a0@(2),d0
	movel d0,a6@(-28)
	clrl d0
	movew _ins,d0
	addl a6@(-28),d0
	cmpl #256,d0
	jle L245
	clrl d0
	movew _ins,d0
	movel #256,d4
	subl d0,d4
	movel d4,a6@(-28)
L245:
	nop
	clrl d0
	movew _ins,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
L246:
	tstl a6@(-28)
	jgt L249
	jra L247
	.even
L249:
	movel a6@(-48),a0
	clrl d0
	movew a0@(4),d0
	moveq #5,d3
	cmpl d0,d3
	jcs L248
	movel d0,d1
	addl d1,d1
	movel #L263,a0
	movew a0@(d1:l),d0
	jmp pc@(2,d0:w)
L263:
	.word L257-L263
	.word L251-L263
	.word L261-L263
	.word L262-L263
	.word L260-L263
	.word L254-L263
	.even
L251:
	movel a6@(-4),a0
	movew a0@(4),d1
	movew d1,d0
	lsrw #3,d0
	movew d0,d1
	andw #3,d1
	tstw d1
	jeq L254
	jra L253
	.even
L252:
L254:
	movel a6@(-4),a0
	movel a0@,_ins+2
	movel a6@(-4),a0
	movew a0@(12),d0
	andw #1,d0
	tstw d0
	jeq L255
	movew #255,d0
	jra L256
	.even
L255:
	clrw d0
L256:
	movew d0,_ins+6
#APP
	trap #1
#NO_APP
L253:
	movel a6@(-4),a0
	movel a6@(-4),a1
	moveb a1@(15),d4
	orb #1,d4
	moveb d4,a0@(15)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(4),d3
	andw #65531,d3
	movew d3,a0@(4)
	jra L248
	.even
L257:
	movel a6@(-4),a0
	movew a0@(4),d1
	movew d1,d0
	lsrw #3,d0
	movew d0,d1
	andw #3,d1
	tstw d1
	jeq L260
	jra L259
	.even
L258:
L260:
	clrl _ins+2
	clrw _ins+6
#APP
	trap #1
#NO_APP
L259:
	movel a6@(-4),a0
	movel a6@(-4),a1
	moveb a1@(15),d4
	andb #254,d4
	moveb d4,a0@(15)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(4),d3
	orw #4,d3
	movew d3,a0@(4)
	jra L248
	.even
L261:
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(12),d4
	orw #4,d4
	movew d4,a0@(12)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(4),d3
	orw #32768,d3
	movew d3,a0@(4)
	jra L248
	.even
L262:
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(12),d4
	andw #65531,d4
	movew d4,a0@(12)
	movel a6@(-4),a0
	movel a6@(-4),a1
	movew a1@(4),d3
	andw #32767,d3
	movew d3,a0@(4)
	jra L248
	.even
L264:
L250:
L248:
	subql #1,a6@(-28)
	moveq #32,d4
	addl d4,a6@(-4)
	addqw #1,_ins
	jra L246
	.even
L247:
	jra L143
	.even
L265:
	movel _xlx,a0
	movew a0@(36),d0
	movew d0,a3
	movel a3,a6@(-24)
	movel _xlx,a0
	clrw a0@(36)
	movel _xlx,a0
	movel _dpm,a1
	movew a1@(366),a0@(32)
	jra L143
	.even
L266:
	movel _dpm,a0
	movew #-1,a0@(362)
	jra L201
	.even
L143:
	movel _dpm,a0
	clrw a0@(362)
L201:
	movel _dpm,a0
	clrw a0@(360)
	movel _dpm,a0
	movew a0@(354),d1
	movew d1,d0
	andw #1,d0
	tstw d0
	jeq L268
	orw #2048,_eprog+2
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw _eprog+2,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	orw #2,d1
	movew d1,a0@(364)
L268:
	orw #1,_eprog
L140:
	jra L138
	.even
L139:
L137:
	moveml a6@(-76),#0xc1c
	unlk a6
	rts
	.even
.globl _dummyIsrEtc
_dummyIsrEtc:
	link a6,#0
#APP
	 .text
	 .globl _SetIntSourceMask
_SetIntSourceMask:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  /* Mask all interrupts */
	 moveml a2/a3,_regic
       
#NO_APP
	tstw _eprog
	jeq L270
	movel _dpm,a0
	movew a0@(354),d1
	tstw d1
	jne L270
	movel _dpm,a0
	movew _eprog,a0@(354)
	clrw _eprog
	movel _dpm,a0
	lea a0@(2044),a0
	movew #1,a0@
L270:
#APP
	  moveml  _regic,a2/a3
	   rte 
	 .globl _InsertToCam
_InsertToCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 moveml d4/a2/a3,_regic 
	 oriw  #CamDATA_MODE,SimDataF1 
#NO_APP
	movel _cam,a0
	movew _ins+2,a0@
	movel _cam,a0
	movew _ins+4,a0@
	movel _cam,a0
	movew _ins+6,a0@
#APP
	 andiw #CamCOMMAND_MODE,SimDataF1 
#NO_APP
	movel _cam,a0
	movew _ins,d1
	orw #57344,d1
	movew d1,a0@
#APP
	 moveml _regic,d4/a2/a3
	 rte 
	 .globl _ClearCam
_ClearCam:
	 movew  #Tg8DISABLE_INTERRUPTS,SR
	 movel a0,_regic 
	 andiw #CamCOMMAND_MODE,SimDataF1 
	 movel _cam,a0
		       movew #0,a0@
	 movel _regic,a0
         rte 
	 .globl _Tpu0_Isr
_Tpu0_Isr:
       movew  #Tg8DISABLE_INTERRUPTS,SR
       andw   #-1-Tg8HS_DPRAM_INTERRUPT,TpuIs /* Clear the TPU ch.'0' inter.*/
       rte

	
       .globl _Tpu1_Isr
_Tpu1_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((1-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((1-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_1,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu2_Isr
_Tpu2_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((2-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((2-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_2,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu3_Isr
_Tpu3_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((3-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((3-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_3,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu4_Isr
_Tpu4_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((4-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((4-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_4,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu5_Isr
_Tpu5_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((5-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((5-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_5,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu6_Isr
_Tpu6_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((6-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((6-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_6,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu7_Isr
_Tpu7_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((7-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((7-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_7,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	
       .globl _Tpu8_Isr
_Tpu8_Isr:
       movew   #Tg8DISABLE_INTERRUPTS,SR
       moveml  a0,_context_c              /* Save registers */
       moveal _eprog+iFired+((8-1)*4),a0  /* Points to the interr. table */
       movew   At+aScTime,a0@(iOut)
       movew   At+aScTime+2,a0@(iOut+2)
       moveal _eprog+rFired+((8-1)*4),a0  /* Points to the recording table */
       movew   At+aScTime,a0@(rOut)
       movew   At+aScTime+2,a0@(rOut+2)
       moveml _context_c,a0	             /* Restore registers */
       andw   #ClrINT_CHANNEL_8,TpuIs     /* Clear the TPU channel 'c' interrupt */
       rte

	 .globl _Abort_Isr
_Abort_Isr:
	 movew #Tg8DISABLE_INTERRUPTS,SR
         movel #JmpToMonitor,sp@(2)
         rte
         .globl JmpToMonitor
JmpToMonitor: 
	 movew #Tg8DISABLE_INTERRUPTS,SR 
#NO_APP
	movel _tpu,a0
	clrw a0@(10)
	movel _sim,a0
	clrw a0@(32)
#APP
	
         movel #0x6e12a,a0  /* and start the 332 BUG */
         jmp   a0@
	
	 .globl _Spurious_Isr
_Spurious_Isr:
	 movel d0,sp@-
	 movel Tg8DSC_INTERRUPT,d0              /* Clear DSC Interrupt */
	 movel sp@+,d0
	 addql #1,At+aNumOfSpur
	 rte
   
	 .globl _BusError_Isr
_BusError_Isr:
	 .globl _AddressError_Isr
_AddressError_Isr:
	 .globl _PrivViolation_Isr
_PrivViolation_Isr:
	 addql #1,At+aNumOfBus
	 rte 
	
	.text
	.globl  _Dsc_Isr
_Dsc_Isr:  rte
	
 .globl _Default_Isr
 _Default_Isr:
  movew #Tg8DISABLE_INTERRUPTS,SR
  movew sp@(6),ExceptionVector
  movel sp@(2),ExceptionPC
  stop  #Tg8DISABLE_INTERRUPTS
  rte
#NO_APP
L269:
	unlk a6
	rts
	.even
.globl _dummyIsr
_dummyIsr:
	link a6,#0
	moveml #0x3e30,sp@-
#APP
	 .text
	 .globl _Xr_Isr
_Xr_Isr:
	  movew   #Tg8DISABLE_INTERRUPTS,SR
	  moveml  d0-d7/a0-a6,_xr_context
#NO_APP
	tstw _camBusy
	jne L272
	tstl _atQueue+12
	jne L272
	tstw _eprog+16
	jeq L273
#APP
	 movew   sp@,_imm_ccr    /* Save CCR */
                movel   sp@(2),_imm_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_imm_context /* Save registers */
	      
#NO_APP
	jra L272
	.even
L273:
#APP
	 movew   sp@,_mbx_ccr    /* Save CCR */
                movel   sp@(2),_mbx_pc  /* Save PC */
	        moveml  d0-d7/a0-a6,_mbx_context /* Save registers */
	      
#NO_APP
L274:
L272:
#APP
	 movel  XWsscRframe1,d0       /* Read frame's word1 and word2 */
	 rorw   #8,d0                 /* Unscramble incomming frame byte order */
	 movew  d0,_timing_frame+2
	 swap   d0
	 rorw   #8,d0
	 movew  d0,_timing_frame
       
#NO_APP
	clrw d0
	moveb _timing_frame,d0
	movew d0,_curhd
	cmpw #1,_curhd
	jne L275
	movew _timing_frame+2,_curms
	movew _curms,d6
	subw _lstms,d6
	movew d6,_incms
	cmpw #4,_incms
	jls L276
	cmpw #3,_curms
	jhi L277
	movew _curms,a3
	addqw #1,a3
	movew a3,_incms
	movew _curms,_lstms
	jra L278
	.even
L277:
	movew #1,_incms
	addqw #1,_lstms
	movew _lstms,d6
	addqw #1,d6
	movew d6,_curms
L278:
	jra L279
	.even
L276:
	movew _curms,_lstms
L279:
	movew _sscms,d6
	cmpw _curms,d6
	jcc L280
	movew _curms,_sscms
	jra L281
	.even
L280:
	movew _incms,d6
	addw d6,_sscms
L281:
	movel _dpm,a0
	clrl d0
	movew _sscms,d0
	movel d0,a0@(340)
	movel _dpm,a0
	movel _timing_frame,a0@(332)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(384),d0
	movew d0,d1
	addw _incms,d1
	movew d1,a0@(384)
	movel _xlx,a0
	movew a0@(36),d2
	moveb d2,_rcv_error
	movew _rcv_error,d2
	movel _dpm,a0
	lea a0@(380),a0
	movew d2,a0@
	tstl _atQueue+12
	jeq L282
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw #128,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movew _atQueue+14,a0@(410)
	movel _dpm,a0
	movel _dpm,a1
	movel a1@(340),a0@(412)
	movel _dpm,a0
	movel _dpm,a1
	movel a1@(344),a0@(416)
	jra L283
	.even
L282:
	movel _dpm,a0
	clrw a0@(420)
L283:
	tstb _rcv_error
	jeq L284
	movel _xlx,a0
	clrw a0@(36)
	movel _clk+4,a0
	movel _dpm,a1
	movel a1@(332),a0@
	movel _clk+4,a0
	movel _dpm,a1
	movel a1@(344),a0@(8)
	movel _clk+4,a0
	movel _dpm,a1
	movel a1@(340),a0@(12)
	movel _clk+4,a3
	lea a3@(16),a0
	movel _dpm,a1
	lea a1@(380),a1
	movel a1@,a0@
	addqw #1,_clk+2
	cmpw #15,_clk+2
	jle L285
	clrw _clk+2
	movel #_clk+8,_clk+4
	jra L286
	.even
L285:
	moveq #20,d6
	addl d6,_clk+4
L286:
	movel _clk+4,a0
	clrl a0@(4)
L284:
	tstw _eprog+18
	jeq L287
	movew #1,_doImm
	jra L288
	.even
L287:
	clrl d0
	moveb _timing_frame+1,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	moveb a0@(1024),d0
	andb #2,d0
	tstb d0
	jeq L289
	jra L288
	.even
L289:
	jra L290
	.even
L275:
	cmpw #36,_curhd
	jeq L293
	movew _curhd,d0
	andw #15,d0
	cmpw #4,d0
	jeq L292
	jra L293
	.even
L293:
	movew _curhd,d0
	andw #15,d0
	cmpw #3,d0
	jeq L292
	jra L291
	.even
L292:
	jra L290
	.even
L291:
	cmpw #32,_curhd
	jne L294
	movew _sscms,d0
	movew d0,d3
	lsrw #4,d3
	clrl d1
	movew d3,d1
	movel d1,d3
	movel d3,d4
	addl d4,d4
	movel d4,d3
	addl d1,d3
	movel d3,d4
	lsll #5,d4
	movel d4,d5
	addl d1,d5
	movel d5,d3
	lsll #3,d3
	movel d5,d4
	addl d3,d4
	movel d4,d3
	addl d1,d3
	movel d3,d1
	clrw d1
	swap d1
	movew d1,d3
	movew d3,d4
	lslw #2,d4
	movew d4,d3
	addw d1,d3
	movew d3,d1
	lslw #4,d1
	movew d1,a3
	subw d3,a3
	movew a3,d3
	movew d3,d1
	lslw #4,d1
	subw d1,d0
	movew d0,_remms
	tstw _remms
	jeq L295
	movew #1200,d6
	subw _remms,d6
	movew d6,_remms
L295:
	movew _remms,d6
	addw d6,_sscms
	movel _dpm,a0
	clrl d0
	movew _sscms,d0
	movel d0,a0@(336)
	movel _dpm,a0
	clrl d0
	movew _curms,d0
	movel d0,a0@(340)
	movew _curms,_sscms
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(368),a0@(370)
	movel _dpm,a0
	clrw a0@(368)
	movel _dpm,a0
	clrl d0
	moveb _timing_frame+1,d0
	clrl d1
	moveb _timing_frame+2,d1
	lsll #8,d1
	movel d0,a1
	addl d1,a1
	clrl d0
	moveb _timing_frame+3,d0
	swap d0
	clrw d0
	addl d0,a1
	movel a1,a0@(344)
	movel _dpm,d0
	movel _dpm,a0
	movel _dpm,a1
	movel a1@(348),a2
	lea a2@(1),a1
	movel a1,a0@(348)
L294:
	movew _curhd,d0
	andw #240,d0
	cmpw #32,d0
	jne L296
	movel _dpm,a0
	movel _timing_frame,a0@(328)
	movel _dpm,d0
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(368),d0
	movew d0,d1
	addqw #1,d1
	movew d1,a0@(368)
	movel _hist+4,a0
	movel _timing_frame,a0@
	movel _hist+4,a0
	movel _dpm,a1
	movel a1@(344),a0@(4)
	movel _hist+4,a0
	movel _dpm,a1
	movel a1@(340),a0@(8)
	movel _hist+4,a3
	lea a3@(12),a0
	movel _dpm,a1
	lea a1@(380),a1
	movel a1@,a0@
	addqw #1,_hist+2
	cmpw #999,_hist+2
	jle L297
	clrw _hist+2
	movel #_hist+8,_hist+4
	jra L296
	.even
L297:
	moveq #16,d6
	addl d6,_hist+4
L298:
L296:
	clrl d0
	movew _curhd,d0
	cmpl #181,d0
	jeq L300
	cmpl #182,d0
	jeq L301
	jra L302
	.even
L300:
	clrl d0
	movew _timing_frame+2,d0
	movel d0,_utc
	jra L299
	.even
L301:
	clrl d0
	movew _timing_frame+2,d0
	swap d0
	clrw d0
	orl d0,_utc
	movel _utc,_utcl
	jbsr _UtcToTime
	clrl _utc
	movel _dpm,a0
	moveb a0@(381),_rcv_error+1
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(384),a0@(386)
	movel _dpm,a0
	clrw a0@(384)
	jra L299
	.even
L302:
	movew _curhd,d0
	movew d0,d1
	lsrw #4,d1
	clrl d0
	movew d1,d0
	tstl d0
	jeq L306
	moveq #2,d6
	cmpl d0,d6
	jeq L304
	jra L308
	.even
L304:
	clrl d0
	moveb _timing_frame+1,d0
	lea _in_use,a0
	moveb a0@(d0:l),d0
	andb #4,d0
	tstb d0
	jne L305
	jra L290
	.even
L305:
	jra L303
	.even
L306:
	clrl d0
	moveb _timing_frame+1,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	moveb a0@(1024),d0
	andb #1,d0
	tstb d0
	jeq L308
	jra L303
	.even
L307:
L308:
	jra L303
	.even
L303:
	jra L299
	.even
L299:
	nop
L288:
	moveq #3,d6
	cmpl _atQueue+12,d6
	jge L311
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw #16,d1
	movew d1,a0@(374)
	moveq #31,d6
	cmpl _atQueue+12,d6
	jge L311
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw #8,d1
	movew d1,a0@(374)
	jra L290
	.even
L312:
L311:
	movel _dpm,d0
	movel _atQueue+12,d1
	movel d1,d3
	movel d3,d1
	lsll #2,d1
	movel d0,a0
	addl d1,a0
	movel _timing_frame,a0@(424)
	addql #1,_atQueue+12
	movel _atQueue+4,a0
	movel _timing_frame,a0@
	addql #4,_atQueue+4
	moveq #31,d6
	cmpl _immQueue+12,d6
	jge L313
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw #1024,d1
	movew d1,a0@(374)
	jra L314
	.even
L313:
	addql #1,_immQueue+12
	movel _immQueue+4,a0
	movel _timing_frame,a0@
	addql #4,_immQueue+4
L314:
	movel _dpm,a0
	movew _atQueue+14,a0@(406)
	movel _dpm,a0
	movew a0@(408),d0
	clrl d1
	movew d0,d1
	cmpl _atQueue+12,d1
	jge L315
	movel _dpm,a0
	movew _atQueue+14,a0@(408)
L315:
	moveq #1,d6
	cmpl _atQueue+12,d6
	jne L316
	tstw _camBusy
	jne L316
#APP
	
      movel  #_AtProcess,sp@(2)
      rte 
#NO_APP
L316:
	nop
L290:
#APP
	
   moveml  _xr_context,d0-d7/a0-a6
   rte 
	 .globl _AtStartProcess
_AtStartProcess:
	 movew  #Tg8DISABLE_INTERRUPTS,SR    /* Mask all interrupts  */
         movew   sp@,_imm_ccr   /* Save CCR */
         movel   sp@(2),_imm_pc /* Save PC */
	 moveml  d0-d7/a0-a6,_imm_context /* Save registers */
         movel  #_AtProcess,sp@(2) /* Serve the next entry in the AT queue */
	 rte
   
	 .globl _AtCompletion
_AtCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  
#NO_APP
	subql #1,_atQueue+12
	tstl _atQueue+12
	jne L317
	movel #_atQueue+16,d0
	movel d0,_atQueue
	movel d0,_atQueue+4
	tstw _eprog+16
	jeq L318
#APP
	 movew  _imm_ccr,sp@
		        movel  _imm_pc,sp@(2)
		        moveml _imm_context,d0-d7/a0-a6
		        rte
#NO_APP
L318:
#APP
	
         movew  #1,_eprog+ImmRun
         movel  #_ImmProcess,sp@(2) /* Serve the next entry in the IMM queue */
	 rte
#NO_APP
L317:
	addql #4,_atQueue
#APP
	 movel  #_AtProcess,sp@(2)
		   rte
	.globl _AtProcess
_AtProcess: 
#NO_APP
L319:
	movel _atQueue,a0
	movel a0@,_eprog+4
	cmpb #1,_eprog+4
	jeq L320
	clrl d0
	moveb _eprog+5,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb _eprog+4,d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a0@(512),d0
	andb a1@(256),d0
	clrw d1
	moveb d0,d1
	movew d1,_eprog+12
	jra L321
	.even
L320:
	clrl d0
	moveb _eprog+5,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb _eprog+4,d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a0@(1296),d0
	andb a1@(1280),d0
	clrw d1
	moveb d0,d1
	movew d1,_eprog+12
L321:
#APP
	
   /* For each wild card combination present scan the CAM */

   movew   _eprog+WildCard,d4    /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+Event,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */
   moveal  #_match+Matches,a2	 /* a2 points to the resulting list      */

0: btst    #0,d4                 /* Is next wildcard presents? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+Event        /* to the comparand register in 3 steps*/

   /* Load the template for the CAM */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+Event,a1@         /* Load the first 16 bits            */
   movew   _eprog+Event+2,a1@       /* Load the second 16 bits           */
   movew   #0xFF,a1@		    /* Load 3rd word (0xFF for chan. output) */

   /* Start the CAM looking for the template */
   andw    #CamCOMMAND_MODE,SimDataF1 /* Set Command mode for CAM */

   /* Wait for the match to be completed while setting command mode */
1: movel   d0,d0                   /* The matching is in progress: wait   */
   movew   a1@,d0                  /* Read the CAM status                 */
   blt     2f                      /* If no more matches, escape          */

   /* For each match place the action number in resulting buffer (Matches)*/

   moveb   d0,a2@+                 /* Put it to the resulting list        */
   btst    #CamSTATUS_MULTIPLE,d0  /* Multiple matches ?                  */
   bne     2f                      /* No -> break                         */
   andiw   #CamBYTE_MASK,d0        /* Which is the action number          */
   oriw    #CamSET_SKIP_BIT,d0     /* Set the Skip bit for the match      */
   movew   d0,a1@                  /* And look for the next match         */
   bra     1b

   /* No more matches for that wildcard - stop the CAM work */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard combination */
   bra     0b                   /* Loop ... */

   /* Start all the actions we have prepared in the matching list */
4:
   cmpal   #_match+Matches,a2	/* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+EndMa     	/* Set the End of list (match.End)	*/
   bsr     _StartActions	/* Post-processing of the list of started actions */

5: 
#NO_APP
	movel _dpm,d0
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(420),d0
	movew d0,d1
	addqw #1,d1
	movew d1,a0@(420)
	movel _dpm,d0
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(406),d0
	movew d0,d1
	subqw #1,d1
	movew d1,a0@(406)
	moveq #1,d6
	cmpl _atQueue+12,d6
	jge L322
	subql #1,_atQueue+12
	addql #4,_atQueue
	jra L319
	.even
L322:
#APP
	 trap #0 
#NO_APP
L271:
	moveml a6@(-28),#0xc7c
	unlk a6
	rts
	.even
.globl _StartActions
_StartActions:
	link a6,#-40
	movel a2,sp@-
	movel d2,sp@-
	nop
	movel #_match+8,a6@(-28)
L324:
	movel a6@(-28),d2
	cmpl _match,d2
	jne L327
	jra L325
	.even
L327:
	movel a6@(-28),a0
	clrw d0
	moveb a0@,d0
	movew d0,a6@(-30)
	movew a6@(-30),a0
	movel a0,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),a6@(-4)
	movel a6@(-4),a0
	moveb a0@(15),d0
	andb #1,d0
	tstb d0
	jne L328
	jra L326
	.even
L328:
	movel a6@(-4),a0
	clrw d0
	moveb a0@(14),d0
	movew d0,a6@(-32)
	movel _xlx,a0
	movew a6@(-32),a1
	movel a1,d0
	movel d0,d1
	addl d1,d1
	movel a6@(-4),a1
	movew a1@(6),a0@(d1:l)
	movel _xlx,a0
	movew a6@(-32),a1
	movel a1,d0
	movel d0,d1
	addl d1,d1
	movel a6@(-4),a1
	movew a1@(12),a0@(16,d1:l)
	moveq #20,d1
	movew a6@(-32),d0
	muls d1,d0
	movel d0,d1
	addql #8,d1
	movel _dpm,a2
	addl d1,a2
	movel a2,a6@(-12)
	movew a6@(-32),a0
	movel a0,d0
	movel d0,d1
	lsll #2,d1
	movel #_eprog,d0
	movel d0,a0
	addl d1,a0
	movel a6@(-12),a0@(182)
	movew a6@(-32),a0
	movel a0,d0
	movel d0,d1
	lsll #2,d1
	movel #_eprog,d0
	movel d0,a0
	addl d1,a0
	moveq #16,d2
	addl a6@(-4),d2
	movel d2,a0@(214)
	movel a6@(-4),a0
	movel _dpm,a1
	movel a1@(344),a0@(16)
	movel a6@(-4),a0
	movel _dpm,a1
	movel a1@(340),a0@(20)
	movel a6@(-4),a0
	moveq #-1,d2
	movel d2,a0@(24)
	movel a6@(-4),a0
	addqw #1,a0@(28)
	movel a6@(-12),a2
	lea a2@(16),a0
	movew a0@,a6@(-16)
	movel a6@(-12),a2
	lea a2@(18),a0
	movew a0@,a6@(-14)
	movel a6@(-12),a0
	moveq #-1,d2
	cmpl a0@(12),d2
	jne L329
	orw #4,_eprog+2
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw _eprog+2,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	orw #2,d1
	movew d1,a0@(364)
L329:
	tstb a6@(-13)
	jeq L330
	movel a6@(-4),a0
	movew a0@(12),d0
	andw #4,d0
	tstw d0
	jeq L330
	movew a6@(-32),a0
	movel a0,d0
	movel d0,d1
	lsll #2,d1
	movel #_eprog,d0
	movel d0,a0
	addl d1,a0
	movel #_sim_int,a0@(182)
	movel a6@(-4),a0
	moveb a6@(-14),a0@(31)
	movel a6@(-4),a0
	addqb #1,a0@(30)
	orw #16384,_eprog+2
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw _eprog+2,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	orw #2,d1
	movew d1,a0@(364)
	jra L326
	.even
L330:
	movel a6@(-12),a0
	movel _atQueue,a1
	movel a1@,a0@
	movel a6@(-12),a0
	movel _dpm,a1
	movel a1@(344),a0@(4)
	movel a6@(-12),a0
	movel _dpm,a1
	movel a1@(340),a0@(8)
	movel a6@(-12),a0
	moveq #-1,d2
	movel d2,a0@(12)
	moveb _rcv_error,a6@(-20)
	clrb a6@(-19)
	moveb a6@(-29),d2
	addqb #1,d2
	moveb d2,a6@(-18)
	movel a6@(-4),a0
	movew a0@(12),d1
	movew d1,d0
	lsrw #2,d0
	moveb d0,d1
	andb #1,d1
	moveb d1,a6@(-17)
	movel a6@(-12),a2
	lea a2@(16),a0
	movew a6@(-20),a0@
	movel a6@(-12),a2
	lea a2@(18),a0
	movew a6@(-18),a0@
L326:
	addql #1,a6@(-28)
	jra L324
	.even
L325:
L323:
	movel sp@+,d2
	movel sp@+,a2
	unlk a6
	rts
	.even
.globl _dummyImmEtc
_dummyImmEtc:
	link a6,#0
	movel d2,sp@-
#APP
	 .globl _ImmCompletion
_ImmCompletion:
	 movew  #Tg8DISABLE_INTERRUPTS,SR  
#NO_APP
	subql #1,_immQueue+12
	tstl _immQueue+12
	jne L332
	movel #_immQueue+16,d0
	movel d0,_immQueue
	movel d0,_immQueue+4
	clrw _eprog+16
#APP
	 movew  _mbx_ccr,sp@
		      movel  _mbx_pc,sp@(2)
		      moveml _mbx_context,d0-d7/a0-a6
		      rte
#NO_APP
L332:
	addql #4,_immQueue
#APP
	 movel  #_ImmProcess,sp@(2)
		   rte
	.globl _ImmProcess
_ImmProcess: 
#NO_APP
L333:
	movel _immQueue,a0
	movel a0@,_eprog+8
	tstw _doImm
	jeq L334
	movel _dpm,a0
	movew _eprog+18,a0@(352)
	movew _eprog,d0
	andw #2,d0
	tstw d0
	jeq L335
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw #256,d1
	movew d1,a0@(374)
	jra L336
	.even
L335:
	moveq #20,d1
	movew _eprog+18,d0
	mulu d1,d0
	movel d0,sp@-
	pea _eprog+22
	movel _dpm,d0
	addl #168,d0
	movel d0,sp@-
	jbsr _memcpy16
	addql #8,sp
	addql #4,sp
	orw #2,_eprog
	movel _dpm,a0
	movew a0@(354),d0
	tstw d0
	jne L337
#APP
	trap  #3
#NO_APP
L337:
	clrw _eprog+20
	clrw _eprog+18
L336:
	clrw _doImm
L334:
	cmpb #1,_eprog+8
	jeq L338
	clrl d0
	moveb _eprog+9,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb _eprog+4,d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a0@(768),d0
	andb a1@(256),d0
	clrw d1
	moveb d0,d1
	movew d1,_eprog+14
	jra L339
	.even
L338:
	clrl d0
	moveb _eprog+9,d0
	movel #_in_use,d1
	movel d1,a0
	addl d0,a0
	clrl d0
	moveb _eprog+4,d0
	movel #_in_use,d1
	movel d1,a1
	addl d0,a1
	moveb a0@(1552),d0
	andb a1@(1280),d0
	clrw d1
	moveb d0,d1
	movew d1,_eprog+14
L339:
#APP
	
   /* For each wild card combination present scan the CAM */

   moveal  #_match+ImmMatches,a2 /* a2 points to the resulting list      */
   movew   _eprog+ImmWildCard,d4 /* d4 is the wildcards combination mask */
   beq     5f                    /* An event is not in use - escape */

   movel   _eprog+ImmEvent,d1	 /* d1 is the queued timing frame */
   lea     _wild_c,a0            /* a0 points to the wildcards */
   moveal  _cam,a1               /* a1 points to the CAM register */

0: btst    #0,d4                 /* Is a wildcard present? */
   beq     3f                    /* No, try next */

   /* Look for an event */

   movel   d1,d2                  /* We will OR the timing frame with    */
   orl     a0@,d2                 /* the wildcard bit mask               */
   movel   d2,_eprog+ImmEvent     /* to the comparand register in 3 steps*/

   /* Load the template for the CAM. Interrupts OFF */

   movew   #1,_camBusy              /* Protect the CAM access for the AT-process */

   oriw    #CamDATA_MODE,SimDataF1  /* Set Data mode for CAM             */
   movew   _eprog+ImmEvent,a1@      /* Load the first 16 bits            */
   movew   _eprog+ImmEvent+2,a1@    /* Load the second 16 bits           */
   movew   #0x00,a1@		    /* Load 3rd word (ZERO for imm.action) */

   /* Start the CAM looking for the template */
   andw    #CamCOMMAND_MODE,SimDataF1 /* Set Command mode for CAM */

   /* Wait for the match to be completed while setting command mode */

1: movel   d0,d0                   /* The matching is in progress: wait   */
   movew   a1@,d0                  /* Read the CAM status                 */
   blt     2f                      /* If no more matches, escape          */

   /* For each match place the action number in resulting buffer (Matches)*/

   moveb   d0,a2@+                 /* Put it to the resulting list        */
   btst    #CamSTATUS_MULTIPLE,d0  /* Multiple matches ?                  */
   bne     2f                      /* No -> break                         */
   andiw   #CamBYTE_MASK,d0        /* Which is the action number          */
   oriw    #CamSET_SKIP_BIT,d0     /* Set the Skip bit for the match      */
   movew   d0,a1@                  /* And look for the next match         */
   bra     1b

   /* No more matches for that wildcard - stop the CAM work. Interrupts ON */

2: movew   #CamCLEAR_SKIP_BITS,a1@  /* Clear all skip bits */

   /* Start the AT-process if the frames queue is not empty now */

   movew   #Tg8DISABLE_INTERRUPTS,SR 
   tstl    _atQueue+Size
   beq     5f                   /* No pending frames, continue */

   movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   trap    #4                   /* and start the AT-process. */

5: movew   #0,_camBusy          /* Allow the CAM access for the AT-process */
   movew  #Tg8ENABLE_INTERRUPTS,SR 

  /* Try with the next wildcard type */

3: lsrw    #1,d4                /* Try the next one */
   tstw    d4
   beq     4f                   /* No more wildcards at all, escape */

   addql   #4,a0                /* The next wildcard type */
   bra     0b                   /* Loop ... */
4:
   cmpal   #_match+ImmMatches,a2 /* a2 points to the resulting list */
   beq     5f

   movel   a2,_match+ImmEndMa    /* Set the End of list (match.End) */
   bsr     _StartImmActions	 /* Post-processing of the list of started actions */

5: 
#NO_APP
	moveq #1,d2
	cmpl _immQueue+12,d2
	jge L340
	subql #1,_immQueue+12
	addql #4,_immQueue
	jra L333
	.even
L340:
	movel _dpm,d0
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(404),d0
	movew d0,d1
	addqw #1,d1
	movew d1,a0@(404)
#APP
	 trap #5 
#NO_APP
L331:
	movel a6@(-4),d2
	unlk a6
	rts
	.even
.globl _StartImmActions
_StartImmActions:
	link a6,#0
	movel a2,sp@-
	movel d2,sp@-
	nop
	movel #_match+264,_alist
L342:
	movel _alist,d2
	cmpl _match+4,d2
	jcs L345
	jra L343
	.even
L345:
	movel _alist,a0
	clrw d0
	moveb a0@,d0
	movew d0,_actn
	clrl d0
	movew _actn,d0
	movel d0,d1
	lsll #2,d1
	lea _act,a0
	movel a0@(d1:l),_a
	movel _a,a0
	moveb a0@(15),d0
	andb #1,d0
	tstb d0
	jne L346
	jra L344
	.even
L346:
	moveq #20,d1
	movew _eprog+20,d0
	mulu d1,d0
	movel #_eprog+22,d1
	movel d1,a2
	addl d0,a2
	movel a2,_ip
	addqw #1,_eprog+20
	cmpw #7,_eprog+18
	jhi L347
	addqw #1,_eprog+18
	movel _ip,a0
	clrb a0@(19)
	jra L348
	.even
L347:
	orw #2,_eprog+2
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw _eprog+2,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	orw #2,d1
	movew d1,a0@(364)
	cmpw #8,_eprog+20
	jls L348
	clrw _eprog+20
	movel #_eprog+22,_ip
L349:
L348:
	movel _a,a0
	movel _dpm,a1
	movel a1@(344),a0@(16)
	movel _a,a0
	movel _dpm,a1
	movel a1@(340),a0@(20)
	movel _a,a0
	movel _a,a1
	movel a1@(20),a0@(24)
	movel _a,a0
	addqw #1,a0@(28)
	movel _ip,a2
	lea a2@(16),a0
	movew a0@,_u
	movel _ip,a2
	lea a2@(18),a0
	movew a0@,_u+2
	moveb _rcv_error,_uu
	tstb _u+3
	jeq L350
	movel _a,a0
	moveb _u+2,a0@(31)
	movel _a,a0
	addqb #1,a0@(30)
	orw #16384,_eprog+2
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(374),d0
	movew d0,d1
	orw _eprog+2,d1
	movew d1,a0@(374)
	movel _dpm,a0
	movel _dpm,a1
	movew a1@(364),d0
	movew d0,d1
	orw #2,d1
	movew d1,a0@(364)
	jra L344
	.even
L350:
	clrb _uu+1
	movel _ip,a0
	movel _immQueue,a1
	movel a1@,a0@
	movel _ip,a0
	movel _dpm,a1
	movel a1@(344),a0@(4)
	movel _ip,a0
	movel _dpm,a1
	movel a1@(340),a0@(8)
	movel _ip,a0
	movel _ip,a1
	movel a1@(8),a0@(12)
	moveb _actn+1,d2
	addqb #1,d2
	moveb d2,_uu+2
	moveb #1,_uu+3
	movel _ip,a2
	lea a2@(16),a0
	movew _uu,a0@
	movel _ip,a2
	lea a2@(18),a0
	movew _uu+2,a0@
L344:
	addql #1,_alist
	jra L342
	.even
L343:
L341:
	movel sp@+,d2
	movel sp@+,a2
	unlk a6
	rts
.comm _sim,4
.comm _tpu,4
.comm _xlx,4
.comm _cam,4
.comm _dpm,4
.comm _mbx_ccr,2
.comm _imm_ccr,2
.comm _mbx_pc,4
.comm _imm_pc,4
.comm _mbx_context,64
.comm _imm_context,64
.comm _xr_context,64
.comm _context_c,12
.comm _regic,20
.comm _timing_frame,4
.comm _rcv_error,2
.lcomm _ins,8
.lcomm _ins_sav,8
.lcomm _var,66
.lcomm _sim_rec,16
.lcomm _sim_int,20
.lcomm _utc,4
.lcomm _utcl,4
.comm _dm,12
.lcomm _curms,2
.lcomm _lstms,2
.lcomm _incms,2
.lcomm _curhd,2
.lcomm _sscms,2
.lcomm _remms,2
.comm _wild_c,32
.comm _time_event_index,16
.comm _in_use,1808
.comm _camBusy,2
.comm _doImm,2
.comm _eprog,246
.comm _act,9216
.comm _clk,328
.comm _match,520
.comm _atQueue,144
.comm _immQueue,144
.comm _hist,16008
.comm _tel,264
.comm _info,348
.lcomm _a,4
.lcomm _aa,4
.lcomm _ip,4
.lcomm _u,4
.lcomm _uu,4
.lcomm _actn,2
.lcomm _dim,2
.lcomm _mach,2
.lcomm _gt,2
.lcomm _gv,2
.lcomm _alist,4
