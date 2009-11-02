/* write-only register initialization
   FIXME. Should they be zero-out to get rid of old values? */
#define INIT_WOP(_wo_nm) wop = (typeof(*wop) *) (_wo_nm)

/* stub for not write-only register */
#define NOT_WOR(_not_wor)

/* save write-only register last written value */
#define WO_WRITE(_wo_data) *wop++ = *(_wo_data)

/**
 * DriverGen ioctl numbers and get their parameters
 * ------------------------------------------------

 * All DriverGen ioctl arguments are comming in predefined format.
 *
 * In standart case (r/w hw registers) - there are 3 arguments:
 * arg[0] -- user-space address
 * arg[1] -- number of elements to r/w
 * arg[2] -- element index, starting from zero
 *
 * In case of service ioctl - arguments can vary Their amount depends on
 * specific ioctl number. See service routines (those are with __SRV__ subword)
 * for more details on parameter amount.
 *
 * For example, if this is a repetitive r/w request
 * (ioctl number is SRV__REP_REG_RW) then we should have 4 arguments,
 * that are packed as follows:
 *
 * arg[0] -- ioctl number
 * arg[1] -- user-space address
 * arg[2] -- number of elements to r/w
 * arg[3] -- element index, starting from zero
 */

/**
 * @brief Get 3 parameters from ioctl argument
 *
 * @param rsz  -- register size
 * @param argp -- ioctl arguments pointer to unpack
 * @param arg0 -- User-space address
 * @param arg1 -- Number of elements
 * @param arg2 -- Register element index (starting from 0)
 *
 * All 3 parameters are [long] bytes long.
 * For internal usage inside r/w ioctl operations
 * System-specific.
 */
#ifdef __Lynx__

#define UNPACK_ARGS(argp, arg0, arg1, arg2)			\
do {								\
	unsigned long *args;					\
								\
	/* check if we can read all arguments */		\
	if (assert_rbounds(s, crwb, (char **)&argp,		\
			   sizeof(unsigned long)*3, NULL))	\
		return SYSERR;					\
								\
	/* unpack arguments */					\
	args = (unsigned long *)argp;				\
	arg0 = (typeof(arg0)) args[0]; /* user-space address */	\
	arg1 = (typeof(arg1)) args[1]; /* number of elements */	\
	arg2 = (typeof(arg2)) args[2]; /* element index */	\
 } while (0)

#else  /* __linux__ */

#define UNPACK_ARGS(argp, arg0, arg1, arg2)			\
do {								\
	unsigned long d[3] = { 0 };				\
	if (copy_from_user(d, argp, sizeof(d))) {		\
		pseterr(EFAULT);				\
		return SYSERR; /* -1 */				\
	}							\
	arg0 = (typeof(arg0)) d[0]; /* user-space address */	\
	arg1 = (typeof(arg1)) d[1]; /* number of elements */	\
	arg2 = (typeof(arg2)) d[2]; /* element index */		\
} while (0)

#endif

/**
 * @brief user-space boundaries checking for register writing operation
 *
 * @param s     -- statics table
 * @param crwb  -- if to check r/w boundaries. Valid only for Lynx
 * @param uaddr -- user-space address to check
 * @param size  -- data size in user space
 * @param back  -- backup user-space address. Valid only for Linux
 *
 * User-space Read bounds assertion. System-specific.
 * Called every time, register write operations are performed
 *
 * @return SYSERR - failed
 * @return OK     - success
 */
static int assert_rbounds(CVORBStatics_t *s, int crwb,
			  char **uaddr, int size, char **back)
{
#ifdef __Lynx__
	if (crwb) {
		/* check, only if requested */
		if ( rbounds((unsigned long) *uaddr) < size) {
			DBG_FAIL();
			pseterr(EFAULT);
			return SYSERR; /* -1 */
		}
	}
#else  /* __linux__ */
	/* need to copy data from the user-space first
	   in order to write it to hw */
	*back = *uaddr;	/* save user-space addres */
	*uaddr = sysbrk(size);	/* allocate space in the kernel */
	if (!*uaddr)
		return SYSERR;
	/* copy data from the user space */
	if (copy_from_user(*uaddr, *back, size)) {
		sysfree(*uaddr, 0);
		return SYSERR;	/* -1 */
	}
#endif
	return OK; /* 0 */
}

/**
 * @brief user-space boundaries checking for register reading operation
 *
 * @param s     -- statics table
 * @param crwb  -- if to check r/w boundaries. Valid only for Lynx
 * @param uaddr -- user-space address to check
 * @param size  -- data size (in bytes) in user space
 * @param back  -- backup user-space address. Valid only for Linux
 *
 * User-space Write bounds assertioin. System-specific.
 * Called everytime, register read operations are performed
 *
 * @return SYSERR - failed
 * @return OK     - success
 */
static int assert_wbounds(CVORBStatics_t *s, int crwb,
			  char **uaddr, int size, char **back)
{
#ifdef __Lynx__
	if (crwb) {
		/* check only if requested */
		if (wbounds((unsigned long) *uaddr) < size) {
			DBG_FAIL();
			pseterr(EFAULT);
			return SYSERR; /* -1 */
		}
	}
#else  /* __linux__ */
	/* will write data in user-space later, after reading register */
	*back = *uaddr; /* save user-space addres to give back data */
	*uaddr = sysbrk(size);
	if (!*uaddr)
		return SYSERR;	/* -1 */
#endif
	return OK;		/* 0 */
}

/**
 * @brief If DPS < Register Size -- we need to swap bytes (Linux only)
 *
 * @param kaddr  -- data address
 * @param rcntr  -- registers amount
 * @param vector -- swap vector

 * @param rsz   -- register size (in bytes) {2, 4, 8}
 * @param rwoa  -- read/write operation amount {2, 4, 8}
 *
 * If Data Port Size is smaller than Register Size -- this is a singular
 * register.
 *
 * byte       -- 8  bit
 * word       -- 16 bit
 * dword      -- 32 bit
 * long dword -- 64 bit
 */
#ifdef __Lynx__
#define swap_data(kaddr, rcntr, vector)
#else  /* __linux__ */

/* stabs. Will never be called. Needed only for functioin generation */
static inline void dg_swab8(char   **addr) { BUG(); }
static inline void dg_swaw16(char  **addr) { BUG(); }
static inline void dg_swal32(char  **addr) { BUG(); }
static inline void dg_swall64(char **addr) { BUG(); }

/* these one are actually used */
static inline void dg_swab16(char **addr)
{ swab16s((__u16 *)*addr);  *addr += 2; }

static inline void dg_swab32(char **addr)
{ swab32s((__u32 *)*addr);  *addr += 4; }
static inline void dg_swaw32(char **addr)
{ swahw32s((__u32 *)*addr); *addr += 4; }

static inline void dg_swab64(char **addr)
{ swab64s((__u64 *)*addr);  *addr += 8; }
static inline void dg_swaw64(char **addr)
{ swahw64s((__u64 *)*addr); *addr += 8; }
static inline void dg_swal64(char **addr)
{ swahl64s((__u64 *)*addr); *addr += 8; }

static inline void swap_data(char *kaddr, int rcntr, void (*vector)(char **))
{
	int i;

	for (i = 0; i < rcntr; i++)
		vector(&kaddr);
}
#endif

/**
 * @brief Called to finish register reading operation
 *
 * @param s     -- statics table
 * @param kaddr -- kernel-space address to copy data from, into user-space
 *                 and free afterwards. Valid only for Linux
 * @param size  -- data size to copy to user space. Valid only for Linux
 * @param uaddr -- user-space address to copy data to. Valid only for Linux
 *
 * @return SYSERR - failed
 * @return OK     - success
 */
static int end_read(CVORBStatics_t *s, char *kaddr,
		    int size, char *uaddr)
{
	int cc = OK;
#ifdef __linux__
	/* give data to the user */
	if (copy_to_user(uaddr, kaddr, size))
		cc = SYSERR;	/* -1 */
	sysfree(kaddr, 0);
#endif
	FINISH_TIME_STAT(); /* timing measurements */
	DBG_OK();
	return cc;
}

/**
 * @brief Called to finish register writing operation
 *
 * @param s     -- statics table
 * @param kaddr -- kernel-space address to free. Valid only for Linux
 *
 * @return OK
 */
static int end_write(CVORBStatics_t *s, char *kaddr)
{
#ifdef __linux__
	sysfree(kaddr, 0);
#endif
	FINISH_TIME_STAT(); /* timing measurements */
	DBG_OK();
	return OK; /* 0 */
}

/**
 * @brief Scalar Register [1 element long] reading
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't).
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly read register (1 - yes, 0 - no)
 *
 * To read a physical register.
 * Register can be bigger than module DPS.
 * It will be read in parts in this case.
 * Register can be read repeatedly.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define GET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz)	\
int get_##_r_nm(register CVORBStatics_t *s, char *ioctlarg,	\
		int crwb, int r_rw)					\
{									\
	int rwoa; /* Read Operation Amount {1, 2, 4, 8}			\
		     If register is bigger than Data Port Size (DPS),	\
		     then it will be read in parts. If this value	\
		     is > 1 - register is considered to be singular one */ \
	unsigned _r_type *regp;  /* register to get */			\
	unsigned _r_type *uaddr; /* user-space buffer pointer */	\
	char *uaddr_back;	 /* backup user-space address */	\
	int rcntr; /* how many times to repetitive read (min 1) */	\
	int ridx;  /* register index					\
		      always 0 in case of scalar register */		\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx); /* get params */	\
	regp = (unsigned _r_type *) &s->card->_blk_idx->_r_nm;		\
	DBG_ADDR(regp, 0, 1);						\
									\
	/* check number of elements to get */				\
	if ( rcntr <= 0) {						\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* how many read operations should be done to read one register */ \
	rwoa = sizeof(s->card->_blk_idx->_r_nm)/sizeof(unsigned _r_type); \
	if (!rwoa) {							\
		/* TODO register is smaller then DPS. NOT supported (yet) */ \
		pseterr(EINVAL);					\
		return SYSERR;						\
	}								\
									\
	/* check (and prepare in case of Linux) user buffer */		\
	if (assert_wbounds(s, crwb, (char **) &uaddr,			\
			   rcntr*rwoa*sizeof(unsigned _r_type),		\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	/* read from device */						\
	if (r_rw)							\
		/* repetitive read access */				\
		__rep_in##_r_port_op((__port_addr_t)regp, uaddr, rcntr*rwoa); \
	else								\
		__rep_in##_r_port_op##_delay_shift((__port_addr_t)regp, uaddr, \
					   rwoa, _r_tloop);		\
									\
	if (rwoa > 1) /* singular reg. should swap data bytes (Linux only) */ \
		swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
	return end_read(s, (char *)uaddr, rcntr*rwoa*sizeof(unsigned _r_type), \
			uaddr_back);					\
}

/**
 * @brief Non-scalar Register [several elements long] reading
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly read register (1 - yes, 0 - no)
 *
 * To read a set of physical registers starting from the given address.
 * Registers can be bigger than module DPS.
 * They will be read in parts in this case.
 * Repetitive read can be done only on one register element.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define GET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
int get_##_r_nm(register CVORBStatics_t *s, char *ioctlarg,	\
		int crwb, int r_rw)					\
{									\
	int rwoa; /* Read Operation Amount {1, 2, 4, 8}			\
		     If register is bigger than Data Port Size (DPS),	\
		     then it will be read in parts. If this value	\
		     is > 1 - register is considered to be singular one */ \
	unsigned _r_type *regp; /* register to get */			\
	unsigned _r_type *uaddr; /* user-space buffer pointer to put results */	\
	char *uaddr_back;	 /* backup user-space address */	\
	int rcntr;  /* number of elements to get from register, or	\
		       how many times to do repetitive read (min 1) */	\
	int  ridx;  /* first element index to get (starting from 0) */	\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx);			\
									\
	/* check first element index and number of elements to get */	\
	if ( (ridx < 0) || (rcntr <= 0) ) {				\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	regp = (unsigned _r_type *) &s->card->_blk_idx->_r_nm[ridx];	\
	DBG_ADDR(regp, ridx, rcntr);					\
									\
	/* how many read operations should be done to read one register */ \
	rwoa = sizeof(s->card->_blk_idx->_r_nm[ridx])/sizeof(unsigned _r_type);	\
	if (!rwoa) {							\
		/* TODO register is smaller then DPS. NOT supported (yet) */ \
		pseterr(EINVAL);					\
		return SYSERR;						\
	}								\
									\
	/* check if all requested elements are within register range	\
	   but only if it's not repetitive read */			\
	if (!r_rw &&							\
	    (ridx + rcntr) > sizeof(s->card->_blk_idx->_r_nm)/sizeof(typeof(s->card->_blk_idx->_r_nm[0]))) { \
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) user buffer */		\
	if (assert_wbounds(s, crwb, (char **)&uaddr,			\
			   rcntr * rwoa * sizeof(unsigned _r_type),	\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	if (r_rw)							\
		/* repetitive read access */				\
		__rep_in##_r_port_op((__port_addr_t)regp, uaddr, rcntr * rwoa);	\
	else								\
		/* all OK - put data in the user buffer */		\
		__rep_in##_r_port_op##_delay_shift((__port_addr_t)regp, uaddr, \
						   rcntr * rwoa, _r_tloop); \
									\
	if (rwoa > 1) /* singular reg. should swap data bytes (Linux only) */ \
		swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
	return end_read(s, (char *)uaddr, rcntr*rwoa*sizeof(unsigned _r_type), \
			uaddr_back);					\
}

/**
 * @brief Scalar Register [1 element long] writing
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly write register (1 - yes, 0 - no)
 *
 * To write into a physical regiser. If register is write only, then last
 * written value will also be saved.
 * Register can be bigger than module DPS.
 * It will be written in parts in this case.
 * Register can be write repeatedly.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define SET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, _r_woinit, _r_wow) \
int set_##_r_nm(register CVORBStatics_t *s, char *ioctlarg,	\
		int crwb, int r_rw)					\
{									\
	int rwoa; /* Write Operation Amount {1, 2, 4, 8}.		\
		     If register is bigger than Data Port Size (DPS),	\
		     then it will be written in parts. If this value	\
		     is > 1 - register is considered to be singular one */ \
	unsigned _r_type *regp;  /* register to set */			\
	unsigned _r_type *uaddr; /* user-space address data pointer */	\
	char *uaddr_back;	 /* backup user-space address */	\
	int rcntr; /* how many times to read (min 1) */			\
	int ridx;  /* register index					\
		      always 0 in case of scalar register */		\
	int i;								\
									\
	/* last history for write-only regs */				\
	unsigned _r_type __attribute__((unused)) *wop = NULL;		\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx); /* get params */	\
	regp = (unsigned _r_type *)&s->card->_blk_idx->_r_nm;		\
	DBG_ADDR(regp, 0, 1);						\
									\
	/* check number of elements to set */				\
	if ( rcntr <= 0) {						\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* write-only pointer initialization */				\
	_r_woinit(&s->card->wo->_r_nm);					\
									\
	/* how many write operations should be done to write one register */ \
	rwoa = sizeof(s->card->_blk_idx->_r_nm)/sizeof(unsigned _r_type); \
	if (!rwoa) {							\
		/* TODO register is smaller then DPS. NOT supported (yet) */ \
		pseterr(EINVAL);					\
		return SYSERR;						\
	}								\
									\
	/* check (and prepare in case of Linux) usr buffer */		\
	if (assert_rbounds(s, crwb, (char **)&uaddr,			\
			   rcntr * rwoa * sizeof(unsigned _r_type),	\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	if (r_rw) { /* repetitive write */				\
									\
		/* save last written value (if write-only reg) */	\
		_r_wow(uaddr);						\
									\
		if (rwoa > 1)						\
			/* singular reg. should swap data bytes (Linux only) */	\
			swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
		/* write into hw */					\
		__rep_out##_r_port_op((__port_addr_t)regp, uaddr, rcntr * rwoa); \
	} else { /* single write */					\
									\
		/* save last written values (if write-only reg) */	\
		for (i = 0; i < rwoa; i++)				\
			_r_wow(&uaddr[i]);				\
									\
		if (rwoa > 1)						\
			/* singular reg. should swap data bytes (Linux only) */	\
			swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
		/* write into hw */					\
		__rep_out##_r_port_op##_delay_shift((__port_addr_t)regp, uaddr,	\
						    rwoa, _r_tloop);	\
	}								\
									\
	return end_write(s, (char *)uaddr);				\
}

/**
 * @brief Non-scalar Register [several elements long] writing
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly write register (1 - yes, 0 - no)
 *
 * To write an array of physical regisers. If register is write only,
 * then last written value will also be saved.
 * Registers can be bigger than module DPS.
 * They will be written in parts in this case.
 * Repetitive write can be done only on one register element.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define SET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, _r_woinit, _r_wow) \
int set_##_r_nm(register CVORBStatics_t *s, char *ioctlarg,	\
		int crwb, int r_rw)					\
{									\
	int rwoa; /* Write Operation Amount {1, 2, 4, 8}.		\
		     If register is bigger than Data Port Size (DPS),	\
		     then it will be written in parts. If this value	\
		     is > 1 - register is considered to be singular one */ \
	unsigned _r_type *regp;  /* register to set */			\
	unsigned _r_type *uaddr; /* user-space address data pointer */	\
	char *uaddr_back;	 /* backup user-space address */	\
	long rcntr;  /* number of elements to set in the current register \
			or how many times to do repetitive write (min 1) */ \
	int  ridx;    /* first element index to set (starting from 0) */ \
									\
	/* last history for write-only regs */				\
	unsigned _r_type __attribute__((unused)) *wop = NULL;		\
	int i;								\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx);			\
									\
	/* check first element index and number of elements to set */	\
	if ( (ridx < 0) || (rcntr <= 0) ) {				\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	regp = (unsigned _r_type *) &s->card->_blk_idx->_r_nm[ridx];	\
	DBG_ADDR(regp, ridx, rcntr);					\
									\
	/* write-only pointer initialization */				\
	_r_woinit(&s->card->wo->_r_nm[ridx]);				\
									\
	/* how many write operations should be done to write one register */ \
	rwoa = sizeof(s->card->_blk_idx->_r_nm[ridx])/sizeof(unsigned _r_type);	\
	if (!rwoa) {							\
		/* TODO register is smaller then DPS. NOT supported (yet) */ \
		pseterr(EINVAL);					\
		return SYSERR;						\
	}								\
									\
	/* check if all requested elements are within register range	\
	   but only if it's not repetitive write */			\
	if (!r_rw &&							\
	    (ridx + rcntr) > sizeof(s->card->_blk_idx->_r_nm)/sizeof(typeof(s->card->_blk_idx->_r_nm[0]))) { \
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) usr buffer */		\
	if (assert_rbounds(s, crwb, (char **)&uaddr,			\
			   rcntr * rwoa * sizeof(unsigned _r_type),	\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	if (r_rw) { /* repetitive write */				\
									\
		/* save last written value (if write-only reg) */	\
		_r_wow(uaddr);						\
									\
		if (rwoa > 1)						\
			/* singular reg. should swap data bytes (Linux only) */	\
			swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
		/* write into hw */					\
		__rep_out##_r_port_op((__port_addr_t)regp, uaddr, rcntr * rwoa); \
	} else { /* multiple writes */					\
									\
		/* save last written values (if write-only reg) */	\
		for (i = 0; i < rcntr * rwoa; i++)			\
			_r_wow(&uaddr[i]);				\
									\
		if (rwoa > 1)						\
			/* singular reg. should swap data bytes (Linux only) */	\
			swap_data((char *)uaddr, rcntr, dg_swa##_r_port_op##_rsz); \
									\
		/* write into hw */					\
		__rep_out##_r_port_op##_delay_shift((__port_addr_t)regp, uaddr,	\
						    rcntr * rwoa, _r_tloop); \
	}								\
									\
	return end_write(s, (char *)uaddr);				\
}

/**
 * @brief Non-scalar software register [several elements long] reading
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly read register (1 - yes, 0 - no)
 *                    Not allowed for spurious registers
 *
 * To get software register, i.e. data from not from the physical module.
 * For now it is @e extraneous registers and last set value of the write-only
 * registers history.
 * Repetitive read is NOT allowed for spurious registers.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define GET_SPUR_REG_ARR(_r_nm, _blk_idx)				\
int get_##_blk_idx##_##_r_nm(register CVORBStatics_t *s, char *ioctlarg, \
			     int crwb, int r_rw)			\
{									\
	typeof(s->card->_blk_idx->_r_nm[0]) *regp; /* register to get */ \
	typeof(s->card->_blk_idx->_r_nm[0]) *uaddr; /* user-space address \
						       data pointer */	\
	char *uaddr_back;  /* backup user-space address */		\
	long rcntr;  /* number of elements to get from the register (min 1) */ \
	int  ridx;    /* first element INDEX to get (starting from 0) */ \
	int i;								\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx);			\
									\
	if (r_rw) {							\
		/* repetitive r/w access is not supported for spurious regs */ \
		DBG_FAIL();						\
		pseterr(ENOTSUPP);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check first element index and number of elements to get */	\
	if ( ( ridx < 0) || (rcntr <= 0) ) {				\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	regp = &s->card->_blk_idx->_r_nm[ridx];				\
	DBG_ADDR(regp, ridx, rcntr);					\
									\
	/* check if all requested elements are within register range */	\
	if ((ridx + rcntr) > sizeof(s->card->_blk_idx->_r_nm)/sizeof(typeof(s->card->_blk_idx->_r_nm[0]))) { \
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) user buffer */		\
	if (assert_wbounds(s, crwb, (char **)&uaddr,			\
			   rcntr * sizeof(typeof (*uaddr)),		\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	/* all OK - put data in the user buffer */			\
	for (i = 0; i < rcntr; i++)					\
		uaddr[i] = regp[i];					\
									\
	return end_read(s, (char *)uaddr, rcntr * sizeof(typeof (*uaddr)), \
			uaddr_back);					\
}

/**
 * @brief Scalar software register [1 element long] reading
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly read register (1 - yes, 0 - no)
 *                    Not allowed for spurious registers
 *
 * To get software register, i.e. data not from the physical module.
 * For now it is @e extraneous registers and last set value of the write-only
 * registers history.
 * Repetitive read is NOT allowed for spurious registers.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define GET_SPUR_REG_SCAL(_r_nm, _blk_idx)				\
int get_##_blk_idx##_##_r_nm(register CVORBStatics_t *s,		\
			     char *ioctlarg, int crwb, int r_rw)	\
{									\
	typeof(s->card->_blk_idx->_r_nm) *regp;  /* register to get */	\
	typeof(s->card->_blk_idx->_r_nm) *uaddr; /* user-space address data \
						    pointer */		\
	char *uaddr_back;  /* backup user-space address */		\
	int rcntr; /* how many times to read. in this case 1. not used */ \
	int ridx;  /* always 0 in case of scalar register. not used */	\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx); /* get params */	\
	regp = &s->card->_blk_idx->_r_nm;				\
	DBG_ADDR(regp, 0, 1);						\
									\
	if (r_rw) {							\
		/* repetitive r/w access is not supported for spurious regs */ \
		DBG_FAIL();						\
		pseterr(ENOTSUPP);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) user buffer */		\
	if (assert_wbounds(s, crwb, (char **)&uaddr,			\
			   sizeof(typeof (*regp)),			\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	/* put register value in the user buffer */			\
	*(typeof (*regp) *)uaddr = *regp;				\
									\
	return end_read(s, (char *)uaddr, sizeof(typeof (*regp)), uaddr_back); \
}

/**
 * @brief Non-scalar software register [several elements long] writing
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly write register (1 - yes, 0 - no)
 *                    Not allowed for spurious registers
 *
 * To set software registers, i.e. registers that are not in physical module.
 * For now it's only extraneous registers.
 * Repetitive write is NOT allowed for spurious registers.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define SET_SPUR_REG_ARR(_r_nm, _blk_idx)				\
int set_##_blk_idx##_##_r_nm(register CVORBStatics_t *s,		\
			     char *ioctlarg, int crwb, int r_rw)	\
{									\
	int rwoa = 1;     /* Write Operation Amount */			\
	typeof(s->card->_blk_idx->_r_nm[0]) *regp;  /* register to set */ \
	typeof(s->card->_blk_idx->_r_nm[0]) *uaddr; /* user-space address \
						       data pointer */	\
	char *uaddr_back;  /* backup user-space address */		\
	int rcntr;  /* number of elements to get in the current register */ \
	int ridx;    /* first element INDEX to get (starting from 0) */	\
	int i;								\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx);			\
									\
	if (r_rw) {							\
		/* repetitive r/w access is not supported for spurious regs */ \
		DBG_FAIL();						\
		pseterr(ENOTSUPP);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check first element index and number of elements to set */	\
	if ( ( ridx < 0) || (rcntr <= 0) ) {				\
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	regp = &s->card->_blk_idx->_r_nm[ridx];				\
	DBG_ADDR(regp, ridx, rcntr);					\
									\
	/* check if all elements are within register range */		\
	if ((ridx + rcntr) > sizeof(s->card->_blk_idx->_r_nm)/sizeof(typeof(s->card->_blk_idx->_r_nm[0]))) { \
		DBG_FAIL();						\
		pseterr(EINVAL);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) usr buffer */		\
	if (assert_rbounds(s, crwb, (char **)&uaddr,			\
			   rcntr * sizeof(typeof (*uaddr)),		\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	/* put register values in the user buffer */			\
	for (i = 0; i < rcntr; i++)					\
		regp[i] = uaddr[i];					\
									\
	return end_write(s, (char *)uaddr);				\
}

/**
 * @brief Scalar software register [1 element long] writing
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 *                    Valid only for Lynx
 * @param r_rw     -- repeatedly write register (1 - yes, 0 - no)
 *                    Not allowed for spurious registers
 *
 * To set software register, i.e. register that are not in physical module.
 * For now it's only extraneous registers.
 * Repetitive write is NOT allowed for spurious registers.
 *
 * @return SYSERR - success
 * @return OK     - failed
 */
#define SET_SPUR_REG_SCAL(_r_nm, _blk_idx)				\
int set_##_blk_idx##_##_r_nm(register CVORBStatics_t *s,		\
			     char *ioctlarg, int crwb, int r_rw)	\
{									\
	int rwoa = 1;     /* Write Operation Amount */			\
	typeof(s->card->_blk_idx->_r_nm) *regp;	/* register to set */	\
	typeof(s->card->_blk_idx->_r_nm) *uaddr; /* user-space address	\
						    data pointer */	\
	char *uaddr_back;  /* backup user-space address */		\
	int rcntr; /* how many times to read. in this case 1. not used */ \
	int ridx;  /* always 0 in case of scalar register. not used */	\
									\
	DBG_BEG(__FUNCTION__);						\
	UNPACK_ARGS(ioctlarg, uaddr, rcntr, ridx); /* get params */	\
	regp = &s->card->_blk_idx->_r_nm;				\
	DBG_ADDR(regp, 0, 1);						\
									\
	if (r_rw) {							\
		/* repetitive r/w access is not supported for spurious regs */ \
		DBG_FAIL();						\
		pseterr(ENOTSUPP);					\
		return SYSERR; /* -1 */					\
	}								\
									\
	/* check (and prepare in case of Linux) usr buffer */		\
	if (assert_rbounds(s, crwb, (char **)&uaddr,			\
			   sizeof(typeof (*regp)),			\
			   &uaddr_back))				\
		return SYSERR;						\
									\
	/* set register */						\
	*regp = *(typeof (*regp) *)ioctlarg;				\
									\
	return end_write(s, (char *)uaddr);				\
}

/*
  For now there are 4 possible register types:
   1. read-only  real       registers
   2. write-only real       registers
   3. read/write real       registers
   4. read/write extraneous registers

  They all in turn are devided in:
   A. Scalar register (consist of one element).
   B. Array  register (consist of several elements).

  If module Data Port Size (DPS) is smaller than register declared size,
  then such register is considered to be singular and will be r/w in parts.

  It's possible to do repetitive r/w on the real registers.

  Register r/w function prototypes can be found in the header file.
*/

/* 1. read-only real registers */
/* A */
#define RO_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	GET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz);

/* B */
#define RO_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	GET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz);


/* 2. write-only real registers */
/* A */
#define WO_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	SET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, INIT_WOP, WO_WRITE); \
	GET_SPUR_REG_SCAL(_r_nm, wo);

/* B */
#define WO_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	SET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, INIT_WOP, WO_WRITE); \
	GET_SPUR_REG_ARR(_r_nm, wo);


/* 3. r/w real registers */
/* A */
#define RW_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	SET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, NOT_WOR, NOT_WOR); \
	GET_REAL_REG_SCAL(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz);

/* B */
#define RW_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz) \
	SET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz, NOT_WOR, NOT_WOR); \
	GET_REAL_REG_ARR(_r_nm, _r_type, _r_tloop, _blk_idx, _r_port_op, _rsz);


/* 4. r/w extraneous registers. They are r/w by default */
/* A */
#define EX_REG_SCAL(_r_nm)		\
	SET_SPUR_REG_SCAL(_r_nm, ex);	\
	GET_SPUR_REG_SCAL(_r_nm, ex);

/* B */
#define EX_REG_ARR(_r_nm)		\
	SET_SPUR_REG_ARR(_r_nm, ex);	\
	GET_SPUR_REG_ARR(_r_nm, ex);

/*****************************************************************************/
/*                                                                           */
/* First that comes - are service routines (with __SRV__ subword), provided  */
/* by default for all automatically generated drivers. Normally they are     */
/* used by the Driver Access Library (aka DAL) only.                         */
/*                                                                           */
/*****************************************************************************/

/**
 * @brief Service function used in ioctl entry point.
 *
 * For repetitive r/w register access.
 */
#ifdef __Lynx__

#define srv_func___SRV__REP_REG_RW(_s, _arg, _crw, _drw)		\
({									\
	r_rw = 1;	/* set repetitive flag on */			\
	com = (int)*_arg; /* set ioctl number */			\
									\
	_arg += sizeof(unsigned long); /* move argument pointer */	\
	goto rep_ioctl; /* repeat ioctl switch */			\
	r_rw;								\
})

#else  /* __linux__ */

#define srv_func___SRV__REP_REG_RW(_s, _arg, _crw, _drw)		\
({									\
	r_rw = 1;	/* set repetitive flag on */			\
									\
	if (get_user(com, (int*)_arg))					\
		return -EFAULT;						\
									\
	_arg += sizeof(unsigned long); /* move argument pointer */	\
	goto rep_ioctl; /* repeat ioctl switch */			\
	r_rw;								\
})

#endif


/**
 * @brief Service function used in ioctl entry point.
 *
 * Enable/Disable/Query r/w bounds checking during register access.
 * Write 1 - to enable, 0 - to disable, 2 - to query current state
 * @e NOTE. Valid _ONLY_ in case of Lynx!
 */
#ifdef __Lynx__

#define srv_func___SRV__RW_BOUNDS(_s, _arg, _crw, _drw)			\
({									\
	int _usrv = (int)*_arg;						\
									\
	if (_usrv != 2)	/* usr wants to change current check state */	\
		c_rwb = _usrv;						\
									\
	c_rwb; /* return current state */				\
})

#else  /* __linux__ */

#define srv_func___SRV__RW_BOUNDS(_s, _arg, _crw, _drw)			\
({									\
	int _usrv;							\
									\
	if (get_user(_usrv, (int*)_arg))				\
		return -EFAULT;						\
									\
	if (_usrv != 2)	/* usr wants to change current check state */	\
		c_rwb = _usrv;						\
									\
	c_rwb; /* return current state */				\
})

#endif

/**
 * @brief Service Register. Get current debug flag.
 *
 * @param s     -- statics table
 * @param uaddr -- data goes here
 *                 type is [ulong] in the user space
 * @param crwb  -- check r/w bounds (1 - check, 0 - don't)
 * @param r_rw  -- repeated r/w reg access (1 - yes, 0 - no). ignored
 *
 * @return SYSERR - if fails
 * @return OK     - otherwise
 */
int get___SRV__DEBUG_FLAG(register CVORBStatics_t *s,
			  char *uaddr, int crwb, int r_rw)
{
	char *uaddr_back; /* backup user-space address */

	DBG_BEG(__FUNCTION__);

	/* check (and prepare in case of Linux) user buffer */
	if (assert_wbounds(s, crwb, &uaddr, sizeof(long), &uaddr_back))
		return SYSERR;

	/* give data to the user */
	*(unsigned long*)uaddr = s->info->debugFlag;

	return end_read(s, uaddr, sizeof(long), uaddr_back);
}


/**
 * @brief Service Register. Set miscellaneous driver flags.
 *
 * @param s     -- statics table
 * @param uaddr -- new driver flags
 *                 type is [ulong] in the user space
 * @param crwb  -- check r/w bounds (1 - check, 0 - don't)
 * @param r_rw  -- repeated r/w reg access (1 - yes, 0 - no). ignored
 *
 * Enable/disable debug info printout, etc...
 * Previous flag register state returned to the user.
 *
 * @return SYSERR - if fails
 * @return OK     - otherwise
 */
int set___SRV__DEBUG_FLAG(register CVORBStatics_t *s,
			  char *uaddr, int crwb, int r_rw)
{
	char *uaddr_back; /* backup user-space address */
	dbgIPL dbg_prev = s->info->debugFlag; /* save current flag */

	DBG_BEG(__FUNCTION__);

	/* check (and prepare in case of Linux) usr buffer */
	if (assert_rbounds(s, crwb, &uaddr, sizeof(long), &uaddr_back))
		return SYSERR;

	/* set new dbg flag */
	s->info->debugFlag = *(unsigned long*)uaddr;

	/* give previous flag value to the user */
	*(unsigned long*)uaddr = dbg_prev;

	return end_read(s, uaddr, sizeof(long), uaddr_back);
}


/**
 * @brief Service Register. Get device info table
 *
 * @param s     -- statics table
 * @param uaddr -- data goes here
 *                 type is [DevInfo_t] in the user space
 * @param crwb  -- check r/w bounds (1 - check, 0 - don't)
 * @param r_rw  -- repeated r/w reg access (1 - yes, 0 - no). ignored
 *
 * @return SYSERR - if fails
 * @return OK     - otherwise
 */
int get___SRV__DEVINFO_T(register CVORBStatics_t *s,
			 char *uaddr, int crwb, int r_rw)
{
	char *uaddr_back; /* backup user-space address */

	DBG_BEG(__FUNCTION__);

	/* check (and prepare in case of Linux) usr buffer */
	if (assert_wbounds(s, crwb, &uaddr, sizeof(DevInfo_t), &uaddr_back))
		return SYSERR;

	/* give data to the user */
	memcpy((void*)uaddr, (void*)(s->info), sizeof(DevInfo_t));

	return end_read(s, uaddr, sizeof(DevInfo_t), uaddr_back);
}


/**
 * @brief Service Register. Give driver version info to the user.
 *
 * @param s        -- statics table
 * @param ioctlarg -- ioctl arguments are:
 *                    two usr-space addresses packed in unsigned long arg[3]
 *                    for ioctl call.
 *                    only arg[0] and arg[1] are valid. They are:
 *                    1. addr[0] -- address, where driver version in
 *                                  [int] format will go
 *                    2. addr[1] -- adsress, where driver version string
 *                                  in [char*] format will go
 * @param crwb     -- check r/w bounds (1 - check, 0 - don't)
 * @param r_rw     -- repeated r/w reg access (1 - yes, 0 - no). ignored
 *
 * @return SYSERR - if fails
 * @return OK     - otherwise
 */
int get___SRV__DRVR_VERS(register CVORBStatics_t *s,
			 char *ioctlarg, int crwb, int r_rw)
{
	char *vers_str;		/* version string */
	int drvr_vers;		/* current driver version */

	int *usr_drvr_vers;	/* user space */
	char *usr_drvr_vers_back; /* backup user-space address */

	char *usr_vers_str;	/* user space */
	char *usr_vers_str_back; /* backup user-space address */

	int dum;

	DBG_BEG(__FUNCTION__);

	drvr_vers = DisplayVersion(&vers_str); /* get version string */

	/* unpack arguments */
	UNPACK_ARGS(ioctlarg, usr_drvr_vers, usr_vers_str, dum);

	/* check (and prepare in case of Linux) user buffers */
	if (assert_wbounds(s, crwb, (char**)&usr_drvr_vers, sizeof(int),
			   (char**)&usr_drvr_vers_back))
		return SYSERR;
	if (assert_wbounds(s, crwb, (char**)&usr_vers_str, strlen(vers_str)+1,
			   (char**)&usr_vers_str_back)) {
#ifdef __linux__
		sysfree((char*)usr_drvr_vers, 0);
#endif
		return SYSERR;
	}

	/* give it to the user */
	memcpy((void*)usr_vers_str, (void*)vers_str, strlen(vers_str)+1);
	*(int*)usr_drvr_vers = drvr_vers;

#ifdef __linux__
	/* TODO Check copy_to_user completion code */
	dum = copy_to_user(usr_drvr_vers_back, usr_drvr_vers, sizeof(int));
	dum = copy_to_user(usr_vers_str_back, usr_vers_str, strlen(vers_str)+1);
	sysfree((char*)usr_drvr_vers, 0);
	sysfree((char*)usr_vers_str, 0);
#endif
	FINISH_TIME_STAT(); /* timing measurements */
	DBG_OK();
	return OK;
}


/**
 * @brief Service Register. Current driver/DAL consistency checking.
 *
 * @param s     -- statics table
 * @param uaddr -- driver generation time goes here
 *                 type is [time_t] in the user space
 * @param crwb  -- check r/w bounds (1 - check, 0 - don't)
 * @param r_rw  -- repeated r/w reg access (1 - yes, 0 - no). ignored
 *
 * If driver generation date is older than @e DAL generation date, then new
 * @e DAL features are not available for the user. In this case driver should
 * be regenerated using the newest version of the @b driverGen
 *
 * @return SYSERR - if fails
 * @return OK     - otherwise
 */
int get___SRV__DAL_CONSISTENT(register CVORBStatics_t *s,
			      char *uaddr, int crwb, int r_rw)
{
	char *uaddr_back; /* backup user-space address */

	DBG_BEG(__FUNCTION__);

	/* check (and prepare in case of Linux) usr buffer */
	if (assert_wbounds(s, crwb, &uaddr, 4, &uaddr_back))
		return SYSERR;

	/* all OK - put data in the user space */
	*(unsigned long*)uaddr = CVORB_GENERATION_TIME_HEX;

	return end_read(s, uaddr, 4, uaddr_back);
}
/*===================== end of service routines =============================*/


/**
 * @brief Interrupt sources
 *
 * This is a Normal register.
 */
RO_REG_SCAL(INT_SRC, long, 0, block00, l, 32);

/**
 * @brief Interrupt enable mask
 *
 * This is a Normal register.
 */
RW_REG_SCAL(INT_EN, long, 0, block00, l, 32);

/**
 * @brief Interrupt level
 *
 * This is a Normal register.
 */
RW_REG_SCAL(INT_L, long, 0, block00, l, 32);

/**
 * @brief Interrupt vector
 *
 * This is a Normal register.
 */
RW_REG_SCAL(INT_V, long, 0, block00, l, 32);

/**
 * @brief VHDL version
 *
 * This is a Normal register.
 */
RO_REG_SCAL(VHDL_V, long, 0, block00, l, 32);

/**
 * @brief PCB serial number (MSBs)
 *
 * This is a Normal register.
 */
RO_REG_SCAL(PCB_SN_H, long, 0, block00, l, 32);

/**
 * @brief PCB serial number (LSBs)
 *
 * This is a Normal register.
 */
RO_REG_SCAL(PCB_SN_L, long, 0, block00, l, 32);

/**
 * @brief Temperature
 *
 * This is a Normal register.
 */
RO_REG_SCAL(TEMP, long, 0, block00, l, 32);

/**
 * @brief ADC
 *
 * This is a Normal register.
 */
RO_REG_SCAL(ADC, long, 0, block00, l, 32);

/**
 * @brief Soft pulses
 *
 * This is a Normal register.
 */
WO_REG_SCAL(SOFT_PULSE, long, 0, block00, l, 32);

/**
 * @brief External clock frequency
 *
 * This is a Normal register.
 */
RO_REG_SCAL(EXT_CLK_FREQ, long, 0, block00, l, 32);

/**
 * @brief Clock generator control
 *
 * This is a Normal register.
 */
RW_REG_SCAL(CLK_GEN_CNTL, long, 0, block00, l, 32);

/**
 * @brief Module status
 *
 * This is a Normal register.
 */
RO_REG_SCAL(MOD_STAT, long, 0, block00, l, 32);

/**
 * @brief Module config
 *
 * This is a Normal register.
 */
RW_REG_SCAL(MOD_CFG, long, 0, block00, l, 32);

/**
 * @brief DAC value
 *
 * This is a Normal register.
 */
RW_REG_SCAL(DAC_VAL, long, 0, block00, l, 32);

/**
 * @brief SRAM start address
 *
 * This is a Normal register.
 */
RW_REG_SCAL(SRAM_SA, long, 0, block00, l, 32);

/**
 * @brief SRAM data
 *
 * This is a Normal register.
 */
RW_REG_SCAL(SRAM_DATA, long, 0, block00, l, 32);

/**
 * @brief Waveform length
 *
 * This is a Normal register.
 */
RW_REG_SCAL(WAVE_L, long, 0, block00, l, 32);

/**
 * @brief Waveform start address
 *
 * This is a Normal register.
 */
RW_REG_SCAL(WAVE_SA, long, 0, block00, l, 32);

/**
 * @brief Recurrent cycles
 *
 * This is a Normal register.
 */
RW_REG_SCAL(REC_CYC, long, 0, block00, l, 32);

/**
 * @brief DAC control
 *
 * This is a Normal register.
 */
RW_REG_SCAL(DAC_CNTL, long, 0, block00, l, 32);

/**
 * @brief Channel status
 *
 * This is a Normal register.
 */
RO_REG_SCAL(CH_STAT, long, 0, block01, l, 32);

/**
 * @brief Channel config
 *
 * This is a Normal register.
 */
RW_REG_SCAL(CH_CFG, long, 0, block01, l, 32);

/**
 * @brief Function selection
 *
 * This is a Normal register.
 */
RW_REG_SCAL(FUNC_SEL, long, 0, block01, l, 32);

/**
 * @brief Fct enable mask (63 to 32)
 *
 * This is a Normal register.
 */
RW_REG_SCAL(FCT_EM_H, long, 0, block01, l, 32);

/**
 * @brief Fct enable mask (31 to 0)
 *
 * This is a Normal register.
 */
RW_REG_SCAL(FCT_EM_L, long, 0, block01, l, 32);

/**
 * @brief Slope
 *
 * This is a Normal register.
 */
RW_REG_SCAL(SLOPE, long, 0, block01, l, 32);

/**
 * @brief Recurrent cycles
 *
 * This is a Normal register.
 */
RW_REG_SCAL(CH_REC_CYC, long, 0, block01, l, 32);
