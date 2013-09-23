#/bin/csh

########################################################################
#       Description:    Takes command line args and feeds them
#                       into tsvytest.L864.
#
#                       Note: This script only works under linux
#                       Requires additional work to incrase robustness
#
#
#       Author: Paul Kennerley
#
#######################################################################
for x in "$@"
do
	args=$args' '${x}
done

echo  $args  |/mcr/tim/tsvytest
