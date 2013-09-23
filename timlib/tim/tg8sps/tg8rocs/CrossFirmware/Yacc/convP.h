#include <yacc.h>

#define debug 0

/********Local macros */

#define EnumType          254
#define StructType        253
#define UnionType         252

/********Local variables */

 local symbol *varptr=NULL,    /*pointer to the last selected variable */
	      *typtr =NULL;    /*                             type  */

 local int enumv;               /*the next enum field sequenced value */
 local byte Typedef = 0;        /*1=typedef statement is parsered */

/* --convP.h-- */
