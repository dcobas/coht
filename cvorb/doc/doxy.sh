# doxy.sh
# Invokes doxygen modifying a pre-defined doxygen config file.
#
# Known bugs:
# - Exclamation marks (!) in any of the arguments break the script due to
#   delimiter collision when invoking sed.


OUTPUT_DIR=doxygen_output

usage()
{
cat << EOF
`basename $0`: generate doxygen documentation
    Usage: `basename $0` [-o<OUTPUT_DIRECTORY>] [-n<PROJECT_NAME>] input_files
      options:
	-h Show this message
	-n Name of the project
	-o Output directory for the generated files (default: $OUTPUT_DIR)
    Example: `basename $0` -o"doxygen_output" -n"MyLib" mylib.c include/mylib.h
EOF
}

while getopts "hn:o:" OPTION
do
    case $OPTION in
	h)
	    usage
	    exit 1
	    ;;
	n)
	    NAME="$OPTARG"
	    ;;
	o)
	    OUTPUT_DIR="$OPTARG"
	    ;;
	?)
	    usage
	    exit
	    ;;
    esac
done

# set subsequent non-option arguments to $1...n
shift $((OPTIND-1))
OPTIND=1

if [ -z "$*" ]
then
    echo "No input files given"
    usage
    exit 1
fi

cat $(dirname $0)/default.doxycfg | \
    sed "/^[ 	]*PROJECT_NAME[ 	]*=/s!!& $NAME!" | \
    sed "/^[ 	]*OUTPUT_DIRECTORY[ 	]*=/s!!& $OUTPUT_DIR!" | \
    sed "/^[ 	]*INPUT[ 	]*=/s!!& $*!" | \
    sed "/^[ 	]*QUIET[ 	]/s!.*!QUIET	= YES!" | \
    sed "/^[ 	]*EXTRACT_ALL[ 	]/s!.*!EXTRACT_ALL	= YES!" | \
    doxygen -
