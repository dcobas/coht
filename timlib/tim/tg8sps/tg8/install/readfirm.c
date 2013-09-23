/* This routine reads the executable a.out-type file with the Tg8 firmware  */
/* code and fills the FirmwareObject structure. Returns failure in the      */
/* following cases:                                                         */
/* 1) Problems in opening or reading file                                   */
/* 2) The file is not of a.out executable type                              */
/* the #include <a.out.h> and #include <file.h> should be present           */

int Tg8ReadFirmware(file, fp)
char *file;
Tg8FirmwareObject *fp; {
int f,l; struct exec hdr;

  f = open(file,0,O_RDONLY);
  if (f<0) return(Tg8ERR_NO_FILE);

  if (read(f,&hdr,sizeof(hdr)) != sizeof(hdr)) return(Tg8ERR_BAD_OBJECT);
  if (hdr.a_magic != ZMAGIC && hdr.a_magic != IMAGIC) return(Tg8ERR_BAD_OBJECT);

  fp->StartAddress = hdr.a_entry;
  l = hdr.a_text + hdr.a_data;
  fp->Size = (l+1)/2;
  if (fp->Size > Tg8FIRMWARE_OBJECT_SIZE) {
    printf("**** Too big firmware program: %d. Max=%d ****\n",
	   fp->Size,Tg8FIRMWARE_OBJECT_SIZE);
    return Tg8ERR_BAD_OBJECT;
  };
  if (read(f,fp->Object,l) != l) return(Tg8ERR_BAD_OBJECT);
  return (Tg8ERR_OK);
}

/* eof */



