#!/bin/sh
echo Build tar file for ctrp delivery without cleaning

cp makedir/Makefile ./tar_pci
cp makedir/Kbuild ./tar_pci
cp ../driver/*.[ch] ./tar_pci
tar -cjf tar_pci.tar.bz2 tar_pci/
