

struct fl {
	unsigned long firma;
	unsigned long size;
	unsigned long f0;
	unsigned long f1;
	unsigned long f2;
	struct fl * next;
	unsigned long f3;
	struct fl * list;
};

static void lx(unsigned long l)
{
int i;
static char hch[] = "0123456789abcdef";

	write(1," 0x",2);
	for (i = 0; i < 8; ++i)
		write(1,&hch[(l >> (7 - i)) & 0x0F],1);
}

void freewalk(struct fl *f)
{
struct fl *p,*q;

	write(1,"\n",1);
	for (p = f; p; p = p -> next) {
		lx(p -> firma);
		lx(p -> size);
		lx(p -> f0);
		lx(p -> f1);
		lx(p -> f2);
		lx(p -> next);
		lx(p -> f3);
		lx(p -> list);
		write(1,"\n",1);
		for (q = p -> list; q; q = q -> list) {
			write(1," ",1);
			lx(q -> firma);
			lx(q -> size);
			lx(q -> f0);
			lx(q -> f1);
			lx(q -> f2);
			lx(q -> next);
			lx(q -> f3);
			lx(q -> list);
			write(1,"\n",1);
		}
	}
}
