/*
*  @(#) ios.c 1.2 Tue Feb 13 15:10:20 CET 2001
*
*  Copyright (C) 1996 CNi
*
*  Da fare : 
*  - Controllare la dimensione dell'area richiesta per mmap : non dovrebbe
*    superare quella del dispositivo.
*  - Gestire l'E2 seriale.
*  - Aggiungere ioctl per manipolazione del segnali liberi sul latch.
*  - Aggiungere il caricamento dinamico del driver.
*
* 03/03/96 GG 0.0a Prima stesura.
* 02/04/96 GG 0.1a Corretto un errore nell'assegnamento di cpu2_size.
*             Corretto un errore nel metodo "open".
*             Corretto un errore nel calcolo di "nvram".
*             Aggiunta una "request_region" per evitare conflitti
*             con altri dispositivi.
* 09/04/96 GG 0.2a Corretto un errore nell'abilitazione della NVRAM nel
*             metodo "open". Provocava la disabilitazione della scheda.
*             Corretta un'imperfezione formale in "mmap".
* 28/06/00 GG 1.0 Modifiche e miglioramenti per kernel 2.2.x e caricamento
*             dinamico.
* 30/06/00 GG 1.1 Aggiunti controlli sui limiti di mmap. Rimosso codice
*             inutile. Aggiunta la possibilita` di configurare i parametri
*             del driver.
* 13/02/01 GG 1.2 Liberato il major number se non ci sono schede.
* 02/04/02 GG+RM 2.0 Modifiche varie per Kernel 2.4.x
*/

#include <linux/config.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/mtio.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/version.h>
 
#include <asm/dma.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#include "ioscni_priv.h"


static inline int remap_page_range(struct vm_area_struct *vma, unsigned long uvaddr, unsigned long paddr, unsigned long size, pgprot_t prot)
{
  return remap_pfn_range(vma, uvaddr, paddr >> PAGE_SHIFT, size, prot);
}




/* VMA_OFFSET macro, courtesy of Alessandro Rubini */
#define VMA_OFFSET(vma)  ((vma)->vm_pgoff << PAGE_SHIFT)

#define IOS_VER_SIZE 8
#define N_IOS_CPU 2
#define IOS_REGION_SIZE 2

/*
* Descrittore di IOS. Agire qui per riconfigurare.
*/
#define MAX_IOSCNI_INFO 4

static struct ios_info info[MAX_IOSCNI_INFO + 1] = {
/* Questa tabella e` sovrascritta se all'installatore dinamico si
 passa qualche parametro. */
	{ 0x300, 0xD0000 },
	{ 0x304, 0xD4000 },
	/* 0... */
};

static struct ios_statics ios_statics;
static struct ios_statics *statics = (struct ios_statics *) &ios_statics;
int ioscni_major =   IOSCNI_MAJOR;

static int ioscni_open(struct inode * inode, struct file * filp);
static int ioscni_release(struct inode * inode, struct file * filp);
static int ioscni_mmap(struct file * file, struct vm_area_struct * vma);
static int ioscni_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg);

static struct file_operations ioscni_fops = {
	mmap:    ioscni_mmap,      /* mmap */
	open:    ioscni_open,      /* open */
	release: ioscni_release,   /* release */
	ioctl:   ioscni_ioctl,     /* ioctl */
};

static void ios_enable(int ioaddr, unsigned long memaddr)
{
	outb(((memaddr >> 12) & 0xFE) | 0x01, ioaddr);
}

static void ios_disable(int ioaddr)
{
	outb(0x00, ioaddr);
}

static int ioscni_open(struct inode * inode, struct file * filp)
{
unsigned int minor;

	minor = MINOR(inode->i_rdev);
/* Ma c'e` ? */
	if (statics -> nb <= minor) {
		return -ENODEV;
	}
/* C'e` ! */
	ios_enable(statics -> board[minor].b.ioaddr,
	           statics -> board[minor].b.memaddr);

	return 0;
}

static int ioscni_release(struct inode * inode, struct file * filp)
{
    return 0;
}

/*
 * Common VMA ops.
 */

void ioscni_vma_open(struct vm_area_struct *vma)
{ 
}

void ioscni_vma_close(struct vm_area_struct *vma)
{ 
}

static struct vm_operations_struct ioscni_remap_vm_ops = {
    open:  ioscni_vma_open,
    close: ioscni_vma_close,
};

static int ioscni_mmap(struct file * file, struct vm_area_struct * vma)
{
unsigned int minor;
unsigned long physaddr,size;

	minor = MINOR(file -> f_dentry -> d_inode -> i_rdev);

    if (VMA_OFFSET(vma) & (PAGE_SIZE-1))                
        return -ENXIO; /* need aligned offsets */	

/* Rifiuta di mappare piu` di quanto competa alla scheda. */
	size = vma -> vm_end - vma -> vm_start;
	if (size + VMA_OFFSET(vma) > statics -> board[minor].tot_size)
		return -EINVAL;

	physaddr = VMA_OFFSET(vma) + statics -> board[minor].b.memaddr;

/* Fa di tutto perche' l'area non finisca in cache: la marca di
 I/O, cosi` il kernel non la tocca... */
	vma -> vm_flags = VM_IO;
/* ...e accende il bit di "non cache" della MMU. */
	pgprot_val(vma -> vm_page_prot) = pgprot_val(vma -> vm_page_prot)
	                                | _PAGE_PCD;

/* Ora si puo` procedere alla mappatura dell'indirizzo fisico. */
	if (remap_page_range(vma, vma -> vm_start, physaddr, size, vma -> vm_page_prot))
		return -EAGAIN;

    vma->vm_ops = &ioscni_remap_vm_ops;
    ioscni_vma_open(vma);

	return 0;
}

/* Ricerca di una stringa di versione stile IOS "sensata". */
static void ioscni_get_version(char *ver, volatile unsigned char *addr)
{
int j;

	for (j = 0; j < IOS_VER_SIZE; ++j) {
		ver[j] = addr[IOS_VER + j];
		if (ver[j] < ' ' || ver[j] >= 0x7F) {
			ver[j] = '?';
		}
	}
	if (!((ver[0] == '3' && ver[1] == '.')
	     || (ver[5] == 'C' && ver[6] == 'N' && ver[7] == 'i'))) {
		ver[0] = '\0';
	}
	ver[4] = '\0';
}

int ioscni_init(void)
{
int i,j,nvram_present;
char ver[N_IOS_CPU][IOS_VER_SIZE+1];
unsigned char c;
volatile unsigned char *addr;
volatile unsigned char *nvram_addr;
struct ios_board *b;
int result;

	result=register_chrdev(ioscni_major,IOSCNI_NAME,&ioscni_fops);
    if (result < 0) {
		printk("IOS: unable to get major %d\n", ioscni_major);
        return result;
	}
	else {
		if (ioscni_major == 0) ioscni_major = result; /* dynamic */

		printk("IOS driver %s Major %d\n",VER_IOSCNI,ioscni_major);

		memset(statics,0,sizeof(ios_statics));

		statics -> nb = 0;

		for (i = 0; i < MAX_IOSCNI_INFO; ++i) {

		/* Ignora gli elementi nulli. */
		    if (info[i].ioaddr == 0 && info[i].memaddr == 0)
				continue;

		/* Test di sensatezza dei dati. */
			if (info[i].ioaddr < 0x200
			 || info[i].ioaddr > 0x3EC
			 || (info[i].ioaddr & 0x013) != 0) {
				printk("IOS: bad I/O base address (0x%x)\n",info[i].ioaddr);
				continue;
			}
			if (info[i].memaddr < 0xA0000
			 || info[i].memaddr > 0xFC000
			 || (info[i].memaddr & 0x03FFF) != 0) {
				printk("IOS: bad memory address (0x%lx)\n",info[i].memaddr);
				continue;
			}
		/* Occupazione della regione di I/O. Sara` liberata se non
		 vi si trova niente. */
			if (check_region(info[i].ioaddr,IOS_REGION_SIZE)) {
				printk("IOS: I/O address conflict 0x%x\n",info[i].ioaddr);
				continue;
			}

			request_region(info[i].ioaddr,IOS_REGION_SIZE,IOSCNI_NAME);

		/* Abilitazione del selettore. */
			ios_enable(info[i].ioaddr,info[i].memaddr);

			addr = (unsigned char *)phys_to_virt(info[i].memaddr);

		/* Ricerca della versione del firmware. */
			ioscni_get_version(ver[0],addr + IOSCNI_OFF_CPU1);
			ioscni_get_version(ver[1],addr + IOSCNI_OFF_CPU2);

		/* Test NVRAM */
			nvram_addr = addr + IOSCNI_OFF_NVRAM;
			c = ~*nvram_addr;
			*nvram_addr = c;
			nvram_present = (c == *nvram_addr);
			c = ~c;
			*nvram_addr = c;
			nvram_present = nvram_present && (c == *nvram_addr);

		/* Disattivazione del dispositivo. */
			ios_disable(info[i].ioaddr);

		/* Registrazione della scheda e stampa della configurazione. */
			if (ver[0][0] == 0 && ver[1][0] == 0 && !nvram_present) {

			/* Non c'e` niente. Si libera la regione di I/O. */
				release_region(info[i].ioaddr,IOS_REGION_SIZE);

				printk("IOS board at io=0x%x, "
				       "mem=0x%x not found\n",
				       info[i].ioaddr, (int)info[i].memaddr);
			}
			else {
				b = &(statics -> board[statics -> nb]);
				b -> b.ioaddr = info[i].ioaddr;
				b -> b.memaddr = info[i].memaddr;
				b -> b.cpu_off[0] = IOSCNI_OFF_CPU1;
				b -> b.cpu_size[0] = 0;
				b -> b.cpu_off[1] = IOSCNI_OFF_CPU2;
				b -> b.cpu_size[1] = 0;
				b -> b.nvram_off = IOSCNI_OFF_NVRAM;
				b -> b.nvram_size = 0;
				b -> b.e2_size = 0;
				b -> virtaddr = (unsigned long)addr;
				b -> tot_size = IOS_SHMSIZE;

				printk("IOS board %d: io=0x%x, mem=0x%x, ",
					   statics -> nb, info[i].ioaddr, (int)info[i].memaddr);

				for (j = 0; j < N_IOS_CPU; ++j) {
					if (ver[j][0]) {
						b -> b.cpu_size[j] = IOSCNI_SIZE_CPU;
						printk("cpu%d %s, ",j,ver[j]);
					}
					else {
						b -> b.cpu_size[j] = 0;
						printk("no cpu%d, ",j);
					}
				}

			
				if (nvram_present) {
					b -> b.nvram_size = IOSCNI_SIZE_NVRAM;
					printk("nvram size=%d\n", b -> b.nvram_size);
				}
				else {
					printk("no nvram\n");
				}

				++statics -> nb;

			/* Non dovrebbe essere necessario, pero`... */
				if (statics -> nb >= MAX_IOSCNI_BOARD)
					break;
			}
		}

		if (statics -> nb == 0) {
	        	unregister_chrdev(ioscni_major,IOSCNI_NAME);
			return -ENODEV;
		}
	}
	return 0;
}

static int ioscni_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
int minor = MINOR(inode -> i_rdev);
struct iosboard *b = &(statics -> board[minor].b);
int rv;

	switch (cmd) {

		case IOSGET:
			if (copy_to_user((void *)arg,b,sizeof(*b)))
				rv = -EFAULT;
			else
				rv = 0;
			break;

		default:
			rv = -EINVAL;
			break;
	}
	return rv;
}


// #ifdef MODULE

static int io[MAX_IOSCNI_INFO] = { 0,0,0,0 };
static unsigned long mem[MAX_IOSCNI_INFO] = { 0,0,0,0 };

MODULE_AUTHOR("Giuseppe Guerrini");
MODULE_DESCRIPTION("Driver for CNi IOS I/O card.");
MODULE_LICENSE("GPL");
MODULE_PARM(io,"1-" __MODULE_STRING(MAX_IOSCNI_INFO) "i");
MODULE_PARM_DESC(io,"IOS I/O base address (0x200-0x3EC)");
MODULE_PARM(mem,"1-" __MODULE_STRING(MAX_IOSCNI_INFO) "l");
MODULE_PARM_DESC(mem,"IOS memory address (0xA0000-0xFC000)");

static int __init ioscni_init_module(void)
{
  int rv,i;

  /* Sostituisce in "info" tutti i campi dinamici diversi da zero.
  * Se per una scheda si indica "io= -1", significa che si vuole ignorare
  * il default. 
  */
  for (i = 0; i < MAX_IOSCNI_INFO; ++i) {
	  if (io[i] == -1) {
		  info[i].ioaddr = 0;
		  info[i].memaddr = 0;
	  }
	  else {
		  if (io[i] != 0)
			  info[i].ioaddr = io[i];
		  if (mem[i] != 0)
			  info[i].memaddr = mem[i];
	  }
  }

  rv = ioscni_init();
  return rv;
}

static void __exit ioscni_cleanup_module(void)
{
  int i;

	for (i = 0; i < statics -> nb; ++i) {
		ios_disable(statics -> board[i].b.ioaddr);
		release_region(statics -> board[i].b.ioaddr,IOS_REGION_SIZE);
	}

	unregister_chrdev(ioscni_major, IOSCNI_NAME);
}

// #endif

module_init(ioscni_init_module);
module_exit(ioscni_cleanup_module);

/*
vim:ts=4
*/

