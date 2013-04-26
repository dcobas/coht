#!/bin/sh
echo Clean tar_pci directory

cd tar_pci; make clean; rm -f *.[ch]
