echo "Make timtest for PowerPc LynxOs Version 4+"
gmake CPU=ppc4 clean all install

# echo "Make timtest for Linux Kernel 2.6+ Discless"
# gmake CPU=L86 clean all install

echo "Make timtest for Intel platform LynxOs Version 4+"
gmake CPU=x86 clean all install

# echo "Make timtest for Intel platform LynxOs Version 3"
# gmake CPU=68k clean all install

echo "Make timtest for Linux work station with Disc"
gmake -f GNUmakefile.linux CPU=L864 clean all install
