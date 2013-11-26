#!   /usr/bin/env	python
#    coding: utf8

import cmd
import sys
import os
import os.path
from ctypes import *
import struct

testpath = os.path.realpath(sys.argv[0])
testdir  = os.path.dirname(testpath)
libcvora = os.path.join(testdir, 'cvora/libcvora.so')

class CvoraCmd(cmd.Cmd):

    literal_mode = {
         0 : 'cvora_reserved1: reserved',
         1 : 'cvora_optical_16: front panel optical input 1 only (16-bit serial, SCI protocol)',
         2 : 'cvora_copper_16: front panel copper input 1 only (16-bit serial, SCI protocol)',
         3 : 'cvora_btrain_counter: 32-bit up/down counter (btrain)',
         4 : 'cvora_parallel_input: parallel rtm input (32-bit)',
         5 : 'cvora_optical_2_16: front panel optical input 1 and 2 (2x 16-bit serial, SCI protocol)',
         6 : 'cvora_copper_2_16: front panel copper input 1 and 2 (2x 16-bit serial, SCI protocol)',
         7 : 'cvora_serial_32: rtm copper inputs (32x 16-bit serial, SCI protocol)',
         8 : 'cvora_reserved2: reserved',
         9 : 'cvora_optical_16_cvorb: front panel optical input 1 only (16-bit serial, CVORB protocol)',
        10 : 'cvora_copper_16_cvorb: front panel copper input 1 only (16-bit serial, CVORB protocol)',
        11 : 'cvora_2_16_up: 2x 16-bit up counter',
        12 : 'cvora_rtm_parallel_strobe: rtm parallel strobe',
        13 : 'cvora_optical_2_16_cvorb: front optical input 1 and 2 (2x 16-bit serial, CVORB protocol)',
        14 : 'cvora_copper_2_16_cvorb: front copper input 1 and 2 (2x 16-bit serial, CVORB protocol)',
        15 : 'cvora_serial_32_cvorb: rtm copper inputs (32x 16-bit serial, CVORB protocol)',
    }

    def __init__(self, libcvora=libcvora, lun=0):
        cmd.Cmd.__init__(self)
        self.lib = CDLL(libcvora)
        self.fd  = self.lib.cvora_init(lun)
        self.lun = lun

    def do_lun(self, arg):
        """lun [lun]: select current module lun to work with"""
        if arg == '':
            print 'lun %d, fd = %d' % (self.lun, self.fd)
            return
        lun = int(arg)
        self.lib.cvora_close(self.fd)
        self.lun = lun
        self.fd  = self.lib.cvora_init(lun)

    def do_mode(self, arg):
        """mode [mode]: set operation mode for current module"""
        if arg == '':
            mode = c_int()
            self.lib.cvora_get_mode(self.fd, byref(mode))
            print 'mode = %d (%s)' % (mode.value, self.literal_mode[mode.value])
            return
        mode = int(arg)
        print self.lib.cvora_set_mode(self.fd, mode)

    def do_modes(self, arg):
        """modes: show CVORA modes"""
        for k, v in self.literal_mode.items():
            print '%2d: %s' % (k, v)

    def do_polarity(self, arg):
        """polarity [1|positive|0|negative]: set pulse polarity"""
        if arg == '':
            polarity = c_int()
            self.lib.cvora_get_pulse_polarity(self.fd, byref(polarity))
            print 'polarity %s' % (polarity.value and "positive" or "negative")
            return
        if arg in [ "1", "positive" ]:
            polarity = 1
        elif arg in [ "0", "negative" ]:
            polarity = 0
        else:
            print "invalid polarity"
            return
        print self.lib.cvora_set_pulse_polarity(self.fd, polarity)

    def do_version(self, arg):
        """version: get module firmware version"""
        version = c_uint()
        self.lib.cvora_get_version(self.fd, byref(version))
        version = version.value
        print '%x.%x' % ( version >> 8, version & 0xff )

    def do_status(self, arg):
        """status: show and decrypt status register"""
        status = c_uint()
        self.lib.cvora_get_hardware_status(self.fd, byref(status))
        print 'status = 0x%08x' % status.value
        print self.interpret_status(status.value)

    def interpret_status(self, status):
        polarity          = status  & 1
        module_enable     = (status & 2)            >> 1
        int_enable        = (status & (1<<2))       >> 2
        counter_overflow  = (status & (1<<6))       >> 6
        memory_overflow   = (status & (1<<7))       >> 7
        irq_vector        = (status & (0xff<<8))    >> 8
        version           = (status & (0xffff<<16)) >> 16

        return '''\
        Module version: %x
        Module status: %s
        IRQ vector: 0x%x
        IRQ status: %s
        Polarity: %s
        Counter overflow: %s
        Memory overflow: %s''' % (
            version,
            module_enable     and  "enabled"    or  "disabled",
            irq_vector,
            int_enable        and  "enabled"    or  "disabled",
            polarity          and  "positive"   or  "negative",
            counter_overflow  and  "Overflown"  or  "OK",
            memory_overflow   and  "Overflown"  or  "OK",
        )

    def print_samples(self, buf, size):
        format = size/4 * 'I'
        values = struct.unpack(format, buf[:size])
        col = 0
        for v in values:
            print '%08x' % v,
            col += 1
            if col >= 8:
                print
                col = 0
        if 0 < col:
            print

    def do_samples(self, arg):
        """samples: show current sample buffer"""
        size = c_int()
        self.lib.cvora_get_sample_size(self.fd, byref(size))
        size = size.value
        buffer = create_string_buffer(size+1)
        actsize = c_int()
        self.lib.cvora_read_samples(self.fd, size, byref(actsize), byref(buffer))
        actsize = actsize.value
        print '%d (%d) samples' % (size/4, actsize/4)
        self.print_samples(buffer, actsize)

    def do_soft_start(self, arg):
        """soft_start   issue a SOFT START command"""
        self.lib.cvora_soft_start(self.fd)
    def do_soft_stop(self, arg):
        """soft_stop   issue a SOFT STOP command"""
        self.lib.cvora_soft_stop(self.fd)
    def do_soft_rearm(self, arg):
        """soft_rearm   issue a SOFT REARM command"""
        self.lib.cvora_soft_rearm(self.fd)
    def do_clock(self, arg):
        """clock    display observed clock frequency"""
        freq = c_int()
        self.lib.cvora_get_clock_frequency(self.fd, byref(freq))
        print '%d Hz' % freq.value

    def do_channel_mask(self, arg):
        """channel_mask [mask]: get/set channel mask"""
        if arg == '':
            mask = c_uint()
            self.lib.cvora_get_channels_mask(self.fd, byref(mask))
            print 'mask: %08x' % mask.value
            return
        mask = int(arg, 0)
        print self.lib.cvora_set_channels_mask(self.fd, mask)

    def do_enable(self, arg):
        """enable: enable module"""
        self.lib.cvora_enable_module(self.fd)
    def do_disable(self, arg):
        """disable: disable module"""
        self.lib.cvora_disable_module(self.fd)

    def do_irq_enable(self, arg):
        """irq_enable: enable interrupts"""
        self.lib.cvora_enable_interrupts(self.fd)
    def do_irq_disable(self, arg):
        """irq_disable: disable interrupts"""
        self.lib.cvora_disable_interrupts(self.fd)
    def do_irq_timeout(self, arg):
        """irq_timeout [timeout]: get/set interrupt wait timeout"""
        if arg == '':
            timeout = c_int()
            self.lib.cvora_get_timeout(self.fd, byref(timeout))
            print 'timeout %d' % timeout.value
            return
        timeout = int(arg, 0)
        print self.lib.cvora_set_timeout(self.fd, timeout)

    def do_irq_wait(self):
        """irq_wait: wait for an interrupt"""
        self.lib.cvora_wait(self.fd)

    def do_quit(self, arg):
        """quit, q: exit from test program"""
        return True

    def do_EOF(self, arg):
        """EOF: exit from test program"""
        print
        return True

    def do_help(self, arg):
        """help [cmd|all]: print help for [all] command[s]"""
        global cmd
        if arg != 'all':
            cmd.Cmd.do_help(self, arg)
            return
        for comm in CvoraCmd.__dict__:
            if comm[:3] != 'do_':
                continue
            h = getattr(CvoraCmd, comm).__doc__
            if not h:
                continue
            idx = h.find(':')
            cmd = h[:idx]
            hlp = h[idx:].lstrip()
            print '%-20s\t%s' % (cmd + ':', hlp)


    do_q = do_quit
    do_h = cmd.Cmd.do_help

if __name__ == '__main__':

    command = CvoraCmd()
    command.cmdloop()


"""
int cvora_init(int lun);
int cvora_get_version(cvora_t *h, int *version);
int cvora_get_mode(cvora_t *h, enum cvora_mode *mode);
int cvora_set_mode(cvora_t *h, enum cvora_mode mode);
int cvora_get_hardware_status(cvora_t *h, unsigned int *status);
int cvora_set_pulse_polarity(cvora_t *h, int polarity);
int cvora_get_pulse_polarity(cvora_t *h, int *polarity);
int cvora_set_module_enabled(cvora_t *h, int enabled);
int cvora_get_module_enabled(cvora_t *h, int *enabled);
int cvora_set_irq_enabled(cvora_t *h, int enabled);
int cvora_get_irq_enabled(cvora_t *h, int *enabled);
int cvora_enable_interrupts(cvora_t *h);
int cvora_disable_interrupts(cvora_t *h);
int cvora_set_irq_vector(cvora_t *h, int vector);
int cvora_get_irq_vector(cvora_t *h, int *vector);
int cvora_wait(cvora_t *h);
int cvora_get_sample_size(cvora_t *h, int *memsz);
int cvora_read_samples(cvora_t *h, int maxsz, int *actsz, unsigned int *buf);
int cvora_soft_start(cvora_t *h);
int cvora_soft_stop(cvora_t *h);
int cvora_get_dac(cvora_t *h, unsigned int *dacv);
int cvora_get_clock_frequency(cvora_t *h, unsigned int *freq);
int cvora_set_plot_input(cvora_t *h, unsigned int plti);
int cvora_get_channels_mask(cvora_t *h, unsigned int *chans);
int cvora_set_channels_mask(cvora_t *h, unsigned int chans);
"""
