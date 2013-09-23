#if BUFFERED
 void UnGetChar(char c);
#endif
/************ Common codes ************/

main(int argc,char **argv)
{
 progname= argv[0];             /*Get the program name */
 inpf= InitParser(argc,argv);
 if (!inpf) exit(1);            /*Abort in case of illegal input file */
 LineSeek(0);                   /*Position on the begin of a file */
 InitSymbols(predef_sym);       /*Create the symbols table */
 yyparse();                     /*Do parsing */
 Main(argc,argv);               /*User applied 'main' program */
 exit(0);
}

yylex()  /*****************Lexical analizer routine******************/
{int t,i;
 int c;
 symbol *s;
lex:
 SkipBlanks();

 if (!compfl && c != '#') { /*Ignore this line */
ignline:
   if ((int)c == EOF) return E_O_F;
   if (!GetString(symb,sizeof(symb))) {
     Error("Unclosed conditional bracket at EOF");
     return E_O_F;
   };
#if BUFFERED
   f_char= ' '; /*any char. diff. from the '\n' */
#endif
   c= '\n';
 };

 if (c == '#' && !compfl) {
   SkipBlanks();
   if (c=='i' || c=='I') goto ignline;
   UnGetChar(c);
   c= '#';
   compfl= -1;
 };

 if (c=='%') goto ignline;      /* %comment line */

 if (c=='/') {
   if ((c=GetChar())=='*') { /*Comment */
comm:
     while ((int)(c=GetChar()) != EOF && c != '*');
     if ((int)c==EOF) return E_O_F;
     while ((c=GetChar()) == '*');
     if (c != '/') goto comm;
     goto lex;     /*get the next symbol */
   };
   UnGetChar(c);
   c= '/';
 };

 if (c=='\'') {
   yylval.Num= GetChar();
   if ((c=GetChar()) != '\'') Error("' expected");
   return NUM;
 };

 if (isdigit(c)) {      /*Number */
   if (c=='0') {
     c= GetChar();
     if (c=='x' || c=='X') {    /*0xHEXALNUMBER */
       GetHexNum(&yylval.Num);
       return NUM;
     };
     if (isdigit(c)) { /*Octal number */
       UnGetChar(c);
       GetOctalNum(&yylval.Num);
       return NUM;
     };
   };
   UnGetChar(c);
   if (!isdigit(c))
     yylval.Num= 0;
   else
     GetNum(&yylval.Num);
   return NUM;
 };

 if (c=='_' || isalpha(c)) {      /*Identifier */
   register symbol *blk= blkptr;
   i= 0;
   while (c=='_' || isalnum(c)) {
     symb[i++]= c;
     c= GetChar();
   };
   UnGetChar(c);
   symb[i++]= 0;

   do {
     s= LookUp(symb,blk);
     if (!blk) break;
     blk= blk->Block;
   } while (!s);
   if (!s) {
     s= Insert(symb,SYM,0);
     if (!s) {
       Error("Symbol table is overflowed, symbol:%s",symb);
       return E_O_F;
     };
     s->Block= blkptr;
   };

   idnptr= s;
   yylval.Sym= s;
   if (s->Type == NUM) return SYM;
/**********
     yylval.Num= s->Val;
     return NUM;
   };
**********/
   return s->Type;
 };

 if ((int)c == EOF) return E_O_F;
 t= (int)c;
 if (c=='=' || c=='<' || c=='>') {
   i= GetChar();
   if (c==i)
     t= (c=='<'? SHL:
	 c=='>'? SHR: EQ);
   else
     UnGetChar(i);
 };
 yylval.Num= t;    /*Delimiter */
 return t;
}

yyerror(char *s)
{
 fprintf(stderr,"%s: ***Lev:%d. Line %d: %s ***\n",progname,inclev,nline,s);
#if BUFFERED
 fprintf(stderr,"%s%s***\n",f_hist,f_string);
#endif
 error=1; errors++;
}

void Error(char *fmt, ...)
{register long *p= (long*)&fmt;
 fprintf(stderr,"%s: *** ",progname);
 fprintf(stderr,fmt,p[1],p[2],p[3]);
 fputs(" ***\n",stderr);
 yyerror("Context error");
}

/***************** Additional C codes ************************/

InitSymbols(register SymDef *symp)
{register symbol *sym;
 while (symp->name) {
   sym= Insert(symp->name,symp->type,symp->val);
   if (!sym) return;
   sym->VarType= KeyWordType;
   symp++;
 };
}

symbol *FindByName(register char *name,int type)
{register symbol *sym= symtab;
 while (sym) {
   if (sym->Type == type && !strcmp(sym->Name,name) ) return sym;
   sym= sym->Next;
 };
 return NULL;
}

symbol *FindByVal(register int type,register int val)
{register symbol *sym= symtab;
 while (sym) {
   if (sym->Type == type && sym->Val == val) return sym;
   sym= sym->Next;
 };
 return NULL;
}

symbol *MoreNames(register symbol *sym)
{register int   type= sym->Type;
 register char *name= sym->Name;
 while (sym= sym->Next) {
   if (sym->Type == type && !strcmp(sym->Name,name) ) return sym;
 };
 return NULL;
}

symbol *MoreSymbols(register symbol *sym)
{register int type,val;
 type= sym->Type;
 val = sym->Val ;
 while (sym= sym->Next) {
   if (sym->Type == type && sym->Val == val) return sym;
 };
 return NULL;
}

symbol *LookUnique(register symbol *sym)
{register symbol *s;
 s= symtop;
 while (s) {
   if (s!=sym && !strcmp(s->Name,sym->Name)) return s;
   s= s->Prev;
 };
 return NULL;
}

symbol *LookUp(char *name,symbol *blk)
{
 register symbol *s= symtop;
 while (s) {
   if (!strcmp(s->Name,name) && s->Block == blk) return s;
   s= s->Prev;
 };
 return NULL;
}

symbol *Insert(char *name, int type,int val)
{register symbol *s;
 s= malloc(sizeof(symbol)+strlen(name)+1);
 if (s) {
   s->Name= (char*)&s[1];
   strcpy(s->Name,name);
   s->Type   = type;
   s->Val    = val;
   s->VarType= UndefinedNameType;
   s->VarPtrLevel= 0;
   s->VarDim = 0;
   s->TypeDef= NULL;
   s->Block  = blkptr; /***NULL;***/
   s->Prev   = symtop;
   s->Next   = NULL;            /*end of table */
   if (symtop)
     symtop->Next= s;
   symtop= s;
   if (!symtab) symtab= s;      /*table begins */
 };
 return s;
}

Remove(register symbol *sym)
 {register symbol *next,*prev;
  next= sym->Next;
  prev= sym->Prev;
  if (next) next->Prev= sym->Prev;
  else      symtop= prev;
  if (prev) prev->Next= sym->Next;
  else      symtab= next;
  free(sym);
 }

LineSeek(int n)
{
 if (!n) {      /*new file */
   inpfiles[inclev].File= inpf;
   inpfiles[inclev].Pos = nline= n;
 }
 else {         /*reopen file after including */
   inclev-- ;
   inpf = inpfiles[inclev].File;
   nline= inpfiles[inclev].Pos;
 };
 error= 0;              /*clean the error flag */
#if BUFFERED            /*buffered i/o */
 f_line= nline;
 f_char='\n';
 f_err = 0;
#endif
}

Include()
{register FILE *inp;
 if (compfl<=0) return;
 if (inclev<MAXLEVEL) {
   inp= fopen(symb,"r");
   if (inp) {
     inclev++ ;
     inpf= inp;
     LineSeek(0);
     fprintf(stderr,"%s: Translate the file: %s. Level:%d\n",progname,symb,inclev);
     return 1;
   };
   Error("No such file:%s",symb);
 }
 else Error("Too many nested include-files");
 return 0;
}

Eof()
{
 fclose(inpf);
 yyclearin;
 if (!inclev) return 1; /*end of compilation */
 LineSeek(-1);
 compfl= 1;             /*enable compilation */
 return 0;
}

Define(register symbol *sym, int exp)
{
 if (compfl<=0) return;
 sym->Type   = NUM;
 sym->Block  = TopLevelBlock;
 sym->VarType= KeyWordType;
 sym->Val    = exp;
}

IfDef(register symbol *sym,int cond)
{
 if (compfl<0) compfl= 0;
 if (compfl>0 || cond==ELSE || cond==ENDIF)
 switch (cond) {
   case IFNDEF:
	   complv++;
	   compfl= (sym->Type != NUM);
	   break;
   case IFDEF:
	   complv++;
	   compfl= (sym->Type == NUM);
	   break;
   case IF:
	   complv++;
	   compfl= (long) sym;
	   break;
   case ELSE:
	   if (complv==complev || complv==complev+1) compfl ^= 1;
	   break;
   case ENDIF:
	   if (complv <= 0) {
	     Error("Do not use 'endif' without 'if' pair");
	     complv= 0;
	     return;
	   };
	   if (complv==complev || complv==complev+1) compfl= 1;
	   complv-- ;
	   break;
 };
 if (compfl>0) complev= complv;
}

SetFileName(register char term)
{register int c;
 register int i=0;
 while ((int)(c=GetChar()) != EOF && c!='\n' && c!=term) symb[i++]=c;
 symb[i]= 0;
 if (c!=term) Error("Non-closed file name:%s",symb);
 if (c=='\n') UnGetChar(c);
 yylval.Num= c;
 yychar= ((int)c==EOF? E_O_F: yylex());
}

#if BUFFERED            /*buffered i/o */

char *GetString(char *s,int l)
{
 f_pos= f_string;
 if (f_err) return NULL;
 strcpy(f_hist,f_string);
 if (!fgets(f_string,sizeof(f_string),inpf)) {
   f_err= EOF;
   return NULL;
 };
 f_line++; nline++;
 return f_string;
}

GetChar()
{
 if (f_char == '\n') GetString(f_string,sizeof(f_string));
 if (f_err) return EOF;
 f_char= *f_pos++ ;
 return f_char;
}

void UnGetChar(char c)
{
 f_pos--;
 f_char= ' '; /*any char. diff. from '\n' */
}

GetHexNum(int *p)      /*read integer number in hexadecimal format */
{int v=0;
 while (GetChar() != EOF) {
   if (isdigit(f_char)) v=(v<<4)+(f_char-'0');
   else
     if (f_char>='a' && f_char<='f') v=(v<<4)+(f_char-'a'+10);
     else
       if (f_char>='A' && f_char<='F') v=(v<<4)+(f_char-'A'+10);
       else {
	 UnGetChar(f_char);
	 break;
       };
 };
 return (*p=v);
}

GetOctalNum(int *p)      /*read integer number in octal format */
{int v=0;
 while (GetChar() != EOF) {
   if (isdigit(f_char)) v=(v<<3)+(f_char-'0');
   else {
     UnGetChar(f_char);
     break;
   };
 };
 return (*p=v);
}

GetNum(int *p)      /*read integer number in decimal format */
{int v=0;
 while (GetChar() != EOF) {
   if (isdigit(f_char)) v=v*10+(f_char-'0');
   else {
     UnGetChar(f_char);
     break;
   };
 };
 return (*p=v);
}
#endif

/* --yacc.c-- */
