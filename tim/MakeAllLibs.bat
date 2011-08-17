echo "Make timlib for PowerPc LynxOs Version 4+"
gmake CPU=ppc4 clean all install

# echo "Make timlib for Linux Kernel 2.6+ Discless"
# gmake CPU=L86 clean all install

echo "Make timlib for Intel platform LynxOs Version 4+"
gmake CPU=x86 clean all install

# echo "Make timlib for Motorola 68k platform LynxOs 3"
# gmake CPU=68k clean all install

echo "Make timlib for Linux work station + Disc"
gmake CPU=L864 clean all install

echo "Make timlib for CES Linux platform LynxOs 4"
gmake CPU=Lces clean all install

