/*********Predefined symbols **************/

SymDef  predef_sym[]={{ "include",INCLUDE,0},
		      { "define" ,DEFINE ,0},
		      { "typedef",TYPEDEF,0},
		      { "struct" ,STRUCT ,0},
		      { "union"  ,UNION  ,0},
		      { "enum"   ,ENUM   ,0},
		      {"unsigned",UNSIGNED,0},
		      { "sizeof" ,SIZEOF ,0},
		      { "if"     ,IF     ,0},
		      { "ifdef"  ,IFDEF  ,0},
		      { "ifndef" ,IFNDEF ,0},
		      { "else"   ,ELSE   ,0},
		      { "endif"  ,ENDIF  ,0},
		      { "char"   ,CHAR   ,sizeof(char)  },
		      { "short"  ,SHORT  ,sizeof(short) },
		      { "int"    ,INT    ,sizeof(int)   },
		      { "long"   ,LONG   ,sizeof(long)  },
		      { "float"  ,FLOAT  ,sizeof(float) },
		      { "double",DOUBLE  ,sizeof(double)},
		      { "string",STRING  ,sizeof(string)},
/* end-of-list */
		      { NULL,0,0}
	     };

#include <yacc.c>

FILE *InitParser(int argc,char *argv[])
{
 return stdin;
}

Main(int argc,char **argv)
{
 while (--argc) Summary(*++argv);
}

Summary(char *cmd)
{register symbol *sym= symtab, *t,*b;
 register char form= cmd[0];
 register char *tn,*bn,*pl;
 register int v,w,l,d,T,nl,V;
 local char dp[16];

 switch (form) {
  case 'a':       /*asm */
    printf("\n/**Form: %c**/\n\n",form);
    printf("asm(\"\n");
    break;
  case 's':       /*sed */
    break;
  case 'r':       /*repl*/
    strcpy(symb,cmd+1);
    if (Include()) {
      freopen("repl.log","w",stderr);
      mylex();
    };
    return;
  default: ;
    printf("\n**Form: %c**\n\n",form);
 };

 while (sym) {
   T= sym->Type;
   if (T < NUM ||  /*keyword */
       sym->Name == NoName) goto next;        /*block definition */
   t= sym->TypeDef; tn= (t? (t->Name==NoName? "_su_": t->Name) : NoName);
   b= sym->Block;   bn= (b? (b->Name==NoName? "_su_": b->Name) : NoName);
   v= sym->Val;
   d= sym->VarDim;
   l= sym->VarPtrLevel;
   w= SizeOf(sym);
   V= (T==TYPE? w:v);
   nl= strlen(sym->Name);
   pl= (l==0? NoName: l==1? "*": "*+");
   if (d)
     sprintf(dp,"[%d]",(d==-1? 0: d) );
   else
     dp[0]='\0';
   strcat(dp,pl);

   switch (form) {
    case 'a':       /*asm */
      if (T==TYPE) putchar('\n');
      printf("  %s",sym->Name);
      while (nl<32) {putchar(' '); nl++;}
      printf("=%d\t/* 0x%04x ",V,V);
      break;
    case 's':       /*sed */
      if (T==NUM || T==SYM)
	if (MoreSymbols(sym))
	  fprintf(stderr,"Name:%s is not unique\n",sym->Name);
	else
	  printf("s/#0x%x,/#%s,/g\n",v,sym->Name);
      goto next;
    default:
      printf("%s:\t ",sym->Name);
   };

   switch (sym->Type) {
    case NUM:
       printf("\t%d\t |NUM |",v);
       break;
    case TYPE:
       printf("\t%d\t |TYPE| %s%s",w,tn,dp);
       break;
    case STRU:
       printf("\t%d\t |STRU| %s.%s%s",v,bn,tn,dp);
       break;
    case UNIO:
       printf("\t%d\t |UNIO| %s.%s%s",v,bn,tn,dp);
       break;
    case ENU:
       printf("\t%d\t |ENUM|",v);
       break;
    case SYM:
       printf("\t%d\t |VAR | %s.%s%s \t=%d=",v,bn,tn,dp,w);
       break;
    default:
       printf("\t%d\t |????| %s.%s",v,bn,tn);
   };

   switch (form) {
    case 'a':       /*asm */
      printf(" */");
      break;
    case 's':       /*sed */
      break;
    default:
      ;
   };

   putchar('\n');
next:
   sym= sym->Next;
 };

 switch (form) {
  case 'a':       /*asm */
    printf("\");\n");
    break;
  case 's':       /*sed */
    break;
  default: ;
 };
}

/************************LOCAL ROUTINES**************************/

mylex()  /*Lexical analizer routine for repl*/
{int t,i,val;
 char c, prc,trm, *beg;
 symbol *s;

 while (!f_err) {

 while ((c=GetChar())==' ' || c=='\t');
 beg= f_pos-1;  /*start position of item */

 if (isdigit(c)) {      /*Number */
   if (c=='0') {
     c= GetChar();
     if (c=='x' || c=='X') {    /*0xHEXALNUMBER */
       GetHexNum(&yylval.Num);
subs:  prc= *(beg-1);   /*the previous &char. */
       trm= *f_pos;     /*the terminator */
       val= yylval.Num;
       if (beg != f_string &&
	  (prc=='#' || prc==',' || prc=='(' ) &&
	  (!isalpha(trm)) &&    /*ignore EG 0b, 4f in asm codes */
	  (val!=0 && val!=1)
	  ) {
	if (prc != '(' )  {/*use NUM,TYPE */
	  s= FindByVal(NUM,val);
	  if (!s) s=FindByVal(TYPE,val);
	}
	else               /*use SYM only (e.g. structure's fields) */
	  s= (symbol*)0;
	if (!s) s=FindByVal(SYM,val);
	fprintf (stderr,"**%4d\t%c%x=%d\t '%s' ** %s",
		 nline,prc,val,val,(s? s->Name: ""),f_string);
	if (!s) fprintf (stderr,"?Not matches at all\n");
	else
	if (MoreSymbols(s) ) {
	  fprintf (stderr,"?Non uniq: ");
	  do fprintf(stderr," %s",s->Name);
	     while (s=MoreSymbols(s) );
	  fputc('\n',stderr);
	}
	else {        /*subsitute the name inside of number */
	 register char *src,*dst;
	 register int r,len;
	 t= strlen(s->Name);    /*size of name */
	 i= f_pos - beg;        /*size of numeric */
	 if (i<t) {     /*shift the rest of file string */
	   len= strlen(f_string)+1;
	   src= f_string+len;
	   dst= src+(t-i);
	   r  = src - f_pos;
	   while (r--) *--dst= *--src;
	 }
	 else if (t<i) { /*pad with blanks */
	   dst= beg+t;
	   r= i-t;
	   while (r--) *dst++= ' ';
	 };
	 memcpy(beg,s->Name,t);
	 fprintf (stderr,"Result: %s",f_string);
	 f_pos= beg+t;
	 f_char= *(f_pos-1);    /*last char. of item */
	 c= f_char;
	};
	fputc('\n',stderr);
       };
       goto cont;
     }
     else if (isdigit(c)) { /*Octal number */
       GetOctalNum(&yylval.Num);
       goto subs;
     };
   };
   UnGetChar(c);
   if (!isdigit(c))
     yylval.Num= 0;
   else
     GetNum(&yylval.Num);
   goto subs;
 };

 if (c=='_' || isalpha(c)) {      /*Identifier */
   i= 0;
   while (c=='_' || isalnum(c)) {
     symb[i++]= c;
     c= GetChar();
   };
   UnGetChar(c);
   symb[i++]= 0;
   yylval.Sym= NULL;
 };

cont:
 yylval.Num= (int)c;    /*Delimiter */
 if (c=='\n') fputs(f_string,stdout);

 }/*!f_err*/
}

/************************LOCAL ROUTINES**************************/

TypeDef(register symbol *var)
{register int T;
 var->Type= TYPE;
 if (typtr->Name==NoName) { /*replace unnamed block with type_name */
   typtr->Name= malloc(strlen(var->Name)+1);     /*memory to copy name */
   if (!typtr->Name)
     Error("No enough memory for:%s\n",var->Name);
   else
     strcpy(typtr->Name,var->Name);
remove:
   typtr->Type= TYPE;
   if (var != typtr) Remove(var);
   else typtr->TypeDef = NULL; /* itself is a typedef */
 }
 else {
   if (!strcmp(typtr->Name,var->Name)) goto remove;
   if (typtr->Type == SYM) {    /*Declare the block name as block_typed */
     switch (typtr->VarType) {
       case EnumType:   T= ENU;  break;
       case StructType: T= STRU; break;
       case UnionType:  T= UNIO; break;
       default:         T= 0;
     };
     if (T != 0) typtr->Type= T;   /*set block 'Type' */
   };
 };
 Typedef= 0;
}

/* SET THE CURRENT TYPE */
SetType(register symbol *sym)
{
 typtr= sym;
 return 0;
}

/* SET THE CURRENT VARIABLE */
SetVar(register symbol *sym)
{
 if (sym->Type != SYM)
   Error("Variable name is expected: %s, type=%d",sym->Name,sym->Type);
 else {
   if (!(sym->VarType == UndefinedNameType || Typedef && sym==typtr))
     Error("Name is already in use: %s, type=%d",sym->Name,sym->Type);
   else {
     if (!typtr)
       Error("Type is omitted for variable: %s,type=%d",sym->Name,sym->Type);
     else {
       sym->VarType= typtr->VarType;    /*Define type of variable */
       sym->TypeDef= typtr;
       sym->Block  = blkptr;
       varptr= sym;                /*Save pointer to the current variable */
       return 0;
     };
   };
 };
 return -1;
}

SetDimension(register symbol *sym, int exp)
{
 if (sym != varptr)
   Error("??setdim var:%s array:%s",varptr->Name,sym->Name);
 else {
   if (!exp) exp= -1;
   sym->VarDim= (sym->VarDim? sym->VarDim * exp: exp);
 };
}

SetPtrLevel(register symbol *sym)
{
 if (sym != varptr)
   Error("??ptrlevel var:%s pointer:%s",varptr->Name,sym->Name);
 else {
   sym->VarPtrLevel++;
 };
}

VarInList(register symbol *sym)
{register symbol *s;
 int size= SizeOf(sym), off;
 if (blkptr) {
   off= blkptr->Val;
   if (blkptr->VarType != UnionType) {   /* not union? */
     if (sym->VarType>0 && (off&1)) off++; /*Align the size on the even boundary */
     sym->Val= off;              /*set offset for the field */
     off += size;                /*adjust the size of parent block */
   }
   else {
     sym->Val= 0;               /*set offset for the field to zero*/
     if (off<size) off= size;   /*get max(sizes) */
   };
   blkptr->Val= off;            /*top of block */
   if ((s=LookUnique(sym)) && s->Val != sym->Val)
     fprintf(stderr,"Warning: name '%s' is not unique, its values: %d %d\n",
		sym->Name, sym->Val,s->Val);
 };
}

SetEnum(register symbol *sym, int nxt,int exp)
{
 if (sym->Type == SYM && sym->VarType == UndefinedNameType) {
   sym->Type = NUM;
   sym->Block= NULL;    /*top level of program */
   sym->Val  = (nxt? enumv: exp);
   enumv= 1+ sym->Val;
 }
 else
   Error("Name:%s is used already, its type:%d",sym->Name,sym->Type);
}

/*Forward reference to some block EG struct or union */
MarkBlock(int type)
{
 OpenBlock(type);
 CloseBlock();
}

OpenBlock(int type)
{register symbol *blk;
 register int T;
 switch (type) {
   case EnumType:   T= ENU ; enumv= 0; break;
   case StructType: T= STRU; break;
   case UnionType:  T= UNIO; break;
 };
 if (nameptr)
   blk= nameptr;
 else {
   blk= Insert(dyname,SYM,0);
   if (!blk) return;
   dyname[0]++;
 };
 if (!Typedef) blk->Type= T;       /*set lexical symbol type */
 blk->VarType= type;    /*set kind of block */
 blk->Block = blkptr;   /*nested block */
 if (type==EnumType) blk->Val= sizeof(int);  /*size of enum type == int */
 blkptr= blk;           /*current block */
}

CloseBlock()
{
  if (blkptr->Name[1]==dyname[1]) {
    blkptr->Name= NoName;   /*unnamed block */
    dyname[0]--;        /*close the dynamical block */
  };
  typtr=  blkptr;       /*new defined type */
  blkptr= blkptr->Block;
}

SizeOf(register symbol *sym)
{register int size,dim;
 if (!sym || sym->Type==SYM && sym->VarType==UndefinedNameType) return 0;
 if (sym->VarPtrLevel) size= sizeof(void*);
 else
   if (sym->TypeDef)             /*type of variable? */
     size= SizeOf(sym->TypeDef); /* -- get sizeof(type) */
   else
     size= sym->Val;             /*type definition */
 dim= sym->VarDim;
 if (dim == 0) dim= 1;          /*var. or type dimension */
 else if (dim == -1) dim= 0;
 return size*dim;
}

NoVarList()
{
 if (!blkptr && typtr &&
      typtr->VarType >= UnionType && typtr->Name[1]!=dyname[1])
   {;/*Type Only */ }
 else
   Error("Variable name is expected. blk=%x ty=%x\n",blkptr,typtr);
}

/* --convP.c-- */
