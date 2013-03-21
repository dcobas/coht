#!/bin/sh
echo Build tar file for ctrp delivery

cp ../driver/*.[ch] ./tar_pci
tar -cjf tar_pci.tar.bz2 tar_pci/
