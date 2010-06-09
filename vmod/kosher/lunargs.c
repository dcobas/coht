/** Maximum number of VMOD-XXX mezzanine boards in a crate */
#define VMOD_MAX_BOARDS	64

struct vmod_dev {
	int             lun;            /** logical unit number */
	char            *carrier_name;  /** carrier name */
	int             carrier_lun;    /** supporting carrier */
	int             slot;           /** slot we're plugged in */
	unsigned long	address;	/** virtual address of mz a.s.  */
	int		is_big_endian;
};

struct vmod_devices {
	int			num_modules;
	struct vmod_dev		module[VMOD_MAX_BOARDS];
};

/* module parameters */
static int lun[VMOD_MAX_BOARDS];
static int num_lun;
module_param_array(lun, int, &num_lun, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical Unit Number of the VMOD-XXX board");

static char *carrier[CARRIER_NAME * VMOD_MAX_BOARDS];
static int num_carrier;
module_param_array(carrier, charp, &num_carrier, S_IRUGO);
MODULE_PARM_DESC(carrier, "Name of the carrier in which the VMOD-XXX is plugged in.");

static int carrier_number[VMOD_MAX_BOARDS];
static int num_carrier_number;
module_param_array(carrier_number, int, &num_carrier_number, S_IRUGO);
MODULE_PARM_DESC(carrier_number, "Logical Unit Number of the carrier");

static int slot[VMOD_MAX_BOARDS];
static int num_slot;
module_param_array(slot, int, &num_slot, S_IRUGO);
MODULE_PARM_DESC(slot, "Slot of the carrier in which the VMOD-XXX board is placed.");

/* module config table */
static struct vmod_layout args;

#define PFX "VMOD args: "

static int check_lun_args(char *driver_name, struct vmod_devices *devs)
{
	int i;

	if ((num_lun != num_carrier) || (num_lun != num_carrier_number) || (num_lun != num_slot) ||
		(num_lun > VMOD_MAX_BOARDS)) {
		printk(KERN_ERR PFX "the number of parameters doesn't match or is invalid\n");
		return -1;
	}
	
	for(i=0; i < num_lun ; i++){
		int lun_d 		= (int) lun[i];
		char *cname 		= carrier[i];
		int carrier_num 	= (int)carrier_number[i];
		int slot_board 		= (int)slot[i];

		struct vmod_dev 	*module = &(modules[i]);
		struct carrier_as 	as, *asp  = &as;
		gas_t 			get_address_space;

		if (lun_d < 0 || lun_d > VMOD_MAX_BOARDS){
			printk(KERN_ERR PFX 
				"%s, invalid lun: %d\n", driver_name, lun_d);
			module->lun = -1;
			continue;
		}
		
		get_address_space = modulbus_carrier_as_entry(cname);
		if (get_address_space == NULL) {
			printk(KERN_ERR PFX "No carrier %s\n", cname);
			return -1;
		} else
			printk(KERN_INFO PFX "Carrier %s a.s. entry point at %p\n",
					cname, get_address_space);
		if (get_address_space(asp, carrier_num, slot_board, 1) <= 0){
			printk(KERN_ERR PFX "VMODTTL Invalid carrier number: %d or slot: %d\n",
				  carrier_num, slot_board);
			module->lun = -1;
			continue;
		}

		module->lun           = lun_d;
		module->cname         = cname;
		module->carrier       = carrier_num;
		module->slot          = slot_board;
		module->address       = asp->address;

		module->is_big_endian = asp->is_big_endian;
		lun_to_dev[lun_d]       = i;

		printk(KERN_INFO PFX "VMODTTL handle is %lx. Big Endian %d\n", module->address, module->is_big_endian);
		printk(KERN_INFO PFX
				"    module<%d>: lun %d with "
				"carrier %s carrier_number = %d slot = %d\n",
				i, lun_d, cname, carrier_num, slot_board);
	}
        
        return 0;
}
