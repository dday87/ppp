#include <sys/types.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sun/vddrv.h>

extern struct streamtab pppinfo;
extern int ppp_count;
extern int nchrdev;

static struct vdldrv vd = {
    VDMAGIC_PSEUDO,
    "ppp"
};

extern int nodev();

static struct cdevsw ppp_cdevsw = {
    nodev, nodev, nodev, nodev, nodev, nodev, nodev, 0,
    &pppinfo
};

static struct cdevsw old_entry;

int
ppp_vdcmd(fun, vdp, vdi, vds)
    unsigned int fun;
    struct vddrv *vdp;
    addr_t vdi;
    struct vdstat *vds;
{
    int n, maj;

    switch (fun) {
    case VDLOAD:
	/*
	 * It seems like modload doesn't install the cdevsw entry
	 * for us.  Oh well...
	 */
	for (maj = 1; maj < nchrdev; ++maj)
	    if (cdevsw[maj].d_open == vd_unuseddev)
		break;
	if (maj >= nchrdev)
	    return ENODEV;
	vd.Drv_charmajor = maj;
	old_entry = cdevsw[maj];
	cdevsw[maj] = ppp_cdevsw;
	vd.Drv_cdevsw = &ppp_cdevsw;
	vdp->vdd_vdtab = (struct vdlinkage *) &vd;
	break;

    case VDUNLOAD:
	if (ppp_count > 0)
	    return EBUSY;
	if (vd.Drv_charmajor > 0)
	    cdevsw[vd.Drv_charmajor] = old_entry;
	break;

    case VDSTAT:
	break;

    default:
	return EIO;
    }
    return 0;
}
