GNU ld version 2.8.1 (with BFD 2.8.1)

Allocating common symbols
Common symbol       size              file

clk                 0x148             tg8.o
hist                0x3e88            tg8.o
doImm               0x2               tg8.o
match               0x208             tg8.o
camBusy             0x2               tg8.o
imm_context         0x40              tg8.o
sim                 0x4               tg8.o
time_event_index    0x10              tg8.o
mbx_ccr             0x2               tg8.o
rcv_error           0x2               tg8.o
imm_ccr             0x2               tg8.o
timing_frame        0x4               tg8.o
info                0x15c             tg8.o
dpm                 0x4               tg8.o
xlx                 0x4               tg8.o
context_c           0xc               tg8.o
wild_c              0x20              tg8.o
mbx_pc              0x4               tg8.o
tel                 0x108             tg8.o
act                 0x2400            tg8.o
mbx_context         0x40              tg8.o
atQueue             0x90              tg8.o
eprog               0xf6              tg8.o
imm_pc              0x4               tg8.o
regic               0x14              tg8.o
in_use              0x710             tg8.o
tpu                 0x4               tg8.o
cam                 0x4               tg8.o
xr_context          0x40              tg8.o
dm                  0xc               tg8.o
immQueue            0x90              tg8.o

Memory Configuration

Name             Origin             Length            
*default*        0x00000000         0xffffffff

Linker script and memory map

Address of section .text set to 0x4000
Address of section .data set to 0x7e00
Address of section .bss set to 0x7e00
LOAD tg8.o
                0x00000000                PROVIDE (__stack, 0x0)
                0x00002000                .=0x2000

.text           0x00004000     0x3760
 CREATE_OBJECT_SYMBOLS
 *(.text)
 .text          0x00004000     0x3760 tg8.o
                0x00007008                StartActions
                0x00004000                _main
                0x000072ec                ImmProcess
                0x00006508                Tpu1_Isr
                0x00007294                dummyImmEtc
                0x00006482                InsertToCam
                0x00006756                JmpToMonitor
                0x000065e0                Tpu4_Isr
                0x00006788                BusError_Isr
                0x00004008                Main
                0x00006598                Tpu3_Isr
                0x0000729a                ImmCompletion
                0x000051ca                DecToBcd
                0x00006670                Tpu6_Isr
                0x00006628                Tpu5_Isr
                0x000066b8                Tpu7_Isr
                0x00007508                StartImmActions
                0x00006790                Dsc_Isr
                0x00006e8c                AtProcess
                0x00006434                SetIntSourceMask
                0x00006430                dummyIsrEtc
                0x00006788                PrivViolation_Isr
                0x00006550                Tpu2_Isr
                0x00006700                Tpu8_Isr
                0x00006748                Abort_Isr
                0x000067b8                Xr_Isr
                0x00004004                main_prog
                0x000064da                ClearCam
                0x00006776                Spurious_Isr
                0x000067b0                dummyIsr
                0x00006788                AddressError_Isr
                0x0000526a                UtcToTime
                0x00006e02                AtStartProcess
                0x00006792                Default_Isr
                0x00006e26                AtCompletion
                0x000064fc                Tpu0_Isr
 *(.dynrel)
 *(.hash)
 *(.dynsym)
 *(.dynstr)
 *(.rules)
 *(.need)
                0x00007760                _etext=.
                0x00007760                __etext=.
                0x00020000                .=ALIGN(0x20000)

.data           0x00007e00        0x0
 *(.dynamic)
 *(.got)
 *(.plt)
 *(.data)
 *(.linux-dynamic)
                0x00007e00                _edata=.
                0x00007e00                __edata=.

.bss            0x00007e00     0x7358
                0x00007e00                __bss_start=.
 *(.bss)
 .bss           0x00007e00       0xa8 tg8.o
 *(COMMON)
 COMMON         0x00007ea8     0x72b0 tg8.o
                0x00007ea8                clk
                0x00007ff0                hist
                0x0000be78                doImm
                0x0000be7c                match
                0x0000c084                camBusy
                0x0000c088                imm_context
                0x0000c0c8                sim
                0x0000c0cc                time_event_index
                0x0000c0dc                mbx_ccr
                0x0000c0de                rcv_error
                0x0000c0e0                imm_ccr
                0x0000c0e4                timing_frame
                0x0000c0e8                info
                0x0000c244                dpm
                0x0000c248                xlx
                0x0000c24c                context_c
                0x0000c258                wild_c
                0x0000c278                mbx_pc
                0x0000c27c                tel
                0x0000c384                act
                0x0000e784                mbx_context
                0x0000e7c4                atQueue
                0x0000e854                eprog
                0x0000e94c                imm_pc
                0x0000e950                regic
                0x0000e964                in_use
                0x0000f074                tpu
                0x0000f078                cam
                0x0000f07c                xr_context
                0x0000f0bc                dm
                0x0000f0c8                immQueue
                0x0000f158                _end=ALIGN(0x4)
                0x0000f158                __end=ALIGN(0x4)
OUTPUT(tg8.exe a.out-zero-big)
