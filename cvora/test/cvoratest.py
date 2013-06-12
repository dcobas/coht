#!   /usr/bin/env	python
#    coding: utf8

import cmd
import sys
from ctypes import *

libcvora = '../libcvora.L865.so'

class CvoraCmd(cmd.Cmd):

    literal_mode = {
        0 : 'reserved but parallel input for the moment',
        1 : 'one optical input 16 bits - Input 2 is ignored',
        2 : 'one copper Input 16 bits',
        3 : 'Btrain counters',
        4 : 'parallel input',
        5 : 'two optical inputs 16 bits',
        6 : 'two copper Inputs 16 bits',
        7 : '32 Serial Inputs on rear panel (P2 connector).',
    }

    def __init__(self, libcvora=libcvora, lun=0):
        cmd.Cmd.__init__(self)
        self.lib = CDLL(libcvora)
        self.fd  = self.lib.cvora_init(lun)
        self.lun = lun

    def do_lun(self, arg):
        if arg == '':
            print 'lun %d, fd = %d' % (self.lun, self.fd)
            return
        lun = int(arg)
        self.lib.cvora_close(self.fd)
        self.lun = lun
        self.fd  = self.lib.cvora_init(lun)

    def do_mode(self, arg):
        if arg == '':
            mode = c_int()
            self.lib.cvora_get_mode(self.fd, byref(mode))
            print 'mode = %d (%s)' % (mode.value, self.literal_mode[mode.value])
            return
        mode = int(arg)
        print self.lib.cvora_set_mode(self.fd, mode)

    def do_version(self, arg):
        version = c_uint()
        self.lib.cvora_get_version(self.fd, byref(version))
        version = version.value
        print '%x' % version

    def do_status(self, arg):
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

        return '''
        Module version: %x
        Module status: %s
        IRQ vector: 0x%x
        IRQ status: %s
        Polarity: %s
        Counter overflow: %s
        Memory overflow: %s
        ''' % (
            version,
            module_enable     and  "enabled"    or  "disabled",
            irq_vector,
            int_enable        and  "enabled"    or  "disabled",
            polarity          and  "positive"   or  "negative",
            counter_overflow  and  "Overflown"  or  "OK",
            memory_overflow   and  "Overflown"  or  "OK",
        )

    def do_samples(self, arg):
        size = c_int()
        self.lib.cvora_get_sample_size(self.fd, byref(size))
        size = size.value
        buffer = create_string_buffer(size+1)
        actsize = c_int()
        self.lib.cvora_read_samples(self.fd, size, byref(actsize), byref(buffer))
        actsize = actsize.value
        print '%d (%d) samples' % (size, actsize)
        if False:
            print '%di (%d) samples, buffer = [%x %x %x %x ... ]' % (
                    size, actsize,
                    ord(buffer[0]), ord(buffer[1]), ord(buffer[2]), ord(buffer[3]),)

    def do_quit(self, arg):
        return True

    do_q = do_quit

    def do_EOF(self, arg):
        print
        return True

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
