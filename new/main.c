#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define BSIZ	512
#define	NINODE	100		/* number of in core inodes */

struct	filsys
{
	uint16_t	s_isize;	/* size in blocks of I list */
	uint16_t	s_fsize;	/* size in blocks of entire volume */
	uint16_t	s_nfree;	/* number of in core free blocks (0-100) */
	uint16_t	s_free[100];	/* in core free blocks */
	uint16_t	s_ninode;	/* number of in core I nodes (0-100) */
	uint16_t	s_inode[100];	/* in core free I nodes */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during I list manipulation */
	char	s_fmod;		/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	uint16_t	s_time[2];	/* current date of last update */
	uint16_t	pad[50];
};

/* flags */
#define	ILOCK	01		/* inode is locked */
#define	IUPD	02		/* inode has been modified */
#define	IACC	04		/* inode access time to be updated */
#define	IMOUNT	010		/* inode is mounted on */
#define	IWANT	020		/* some process waiting on lock */
#define	ITEXT	040		/* inode is pure text prototype */

/* modes */
#define	IALLOC	0100000		/* file is used */
#define	IFMT	060000		/* type of file */
#define		IFDIR	040000	/* directory */
#define		IFCHR	020000	/* character special */
#define		IFBLK	060000	/* block special, 0 is regular */
#define	ILARG	010000		/* large addressing algorithm */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* save swapped text even after use */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100

#if 0
struct	inode
{
	uint8_t	i_flag;
	char	i_count;	/* reference count */
	uint16_t	i_dev;		/* device where inode resides */
	uint16_t	i_number;	/* i number, 1-to-1 with device address */
	uint16_t	i_mode;
	char	i_nlink;	/* directory entries */
	char	i_uid;		/* owner */
	char	i_gid;		/* group of owner */
	char	i_size0;	/* most significant of size */
	uint16_t	i_size1;	/* least sig !!! char	*i_size1 */
	uint16_t	i_addr[8];	/* device addresses constituting file */
	uint16_t	i_lastr;	/* last logical block read (for read-ahead) */
} inode[NINODE];
#endif

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100

/*
 * Inode structure as it appears on
 * the disk. Not used by the system,
 * but by things like check, df, dump.
 */
struct	inode
{
	uint16_t i_mode;
	uint8_t	 i_nlink;
	uint8_t	 i_uid;
	uint8_t	 i_gid;
	uint8_t	 i_size0;
	uint16_t	i_size1;	//char	*i_size1; ?
	uint16_t	i_addr[8];	// block numbers
	uint16_t	i_atime[2];
	uint16_t	i_mtime[2];
};


static uint16_t rk[2077696/2];
static struct filsys *super;

int loadrk(char *name)
{
	FILE *fd = 0;
	
	fd = fopen(name, "r");
	if (!fd) {
		perror(name);
		exit(1);
	}

	if (fread(rk, 1, sizeof(rk), fd) != sizeof(rk)) {
		perror(name);
		exit(1);
	}
	super = (struct filsys *)&rk[BSIZ/2];

	return 0;
}

int dsuper(void)
{

	printf("s_isize: %d (I list size in blocks)\n", super->s_isize);
	printf("s_fsize: %d (disk size in blocks)\n", super->s_fsize);
	printf("s_nfree: %d (free blocks in core 0-100 ?)\n", super->s_nfree);
	printf("s_ninode: %d (I nodes in core 0-100 ?)\n", super->s_ninode);
	printf("s_time: %d \n", (super->s_time[1]<<16) +  super->s_time[0]);

	return 0;

}

struct inode *inodep(int num)
{
	struct inode *p;

	p = (struct inode *)&rk[BSIZ*2/2];
	p += num;
	if (num) // except 0, number -1
		p--;
	
	return p;
}

void *blockp(int num)
{
	void *p;
	
	p = (void *)&rk[BSIZ*num/2];
	return p;
}

void prn(uint8_t *p, int indent, uint8_t *pp)
{
	while (indent--)
		printf(" ");
	printf("%s%s", p, pp);
}

void prdir(int noden, int indent)
{
	struct inode *p = inodep(noden);
	uint16_t *pb = blockp(p->i_addr[0]);
	uint16_t entryn;
	uint8_t *pn;

	while (*pb) {
		entryn = *pb;
		p = inodep(entryn);
		pn = (uint8_t *)(pb+1);
		//printf(">> %d %s %o\n", entryn, pn, p->i_mode);
#if 1
		if (pn[0] == '.' && pn[1] == '\0' || pn[0] == '.' && pn[1] == '.') {
			//prn(pn, indent, "\n");
		} else if ((p->i_mode & IFMT) == IFDIR) {
			prn(pn, indent, "/\n");
			prdir(entryn, indent+1);
		} else {
			prn(pn, indent, "\n");
		}
#endif
		pb += 16/2;
	}
}

int dinode(void)
{
	int i,j;
	struct inode *p = inodep(0);

#if 1
	for (i=0; i<super->s_isize * 512/32; i++) {
		if (p->i_mode) {
			printf("%03d: mode %o, nlink %d, size0 %d, size1 %06d -- ", i, 
				p->i_mode, p->i_nlink, p->i_size0, p->i_size1);
			for (j=0; j<8; j++)
				printf("%02x ", p->i_addr[j]);
			printf("\n");
		}
		p++;
	}
#endif
}

int advance(int noden, char *dir)
{
	struct inode *p = inodep(noden);
	uint16_t *pb = blockp(p->i_addr[0]);
	
	uint16_t entryn;
	uint8_t *pn;

	while (*pb) {
		entryn = *pb;
		p = inodep(entryn);
		pn = (uint8_t *)(pb+1);
		if (!strcmp(dir, pn))
			return entryn;
		pb += 16/2;
	}
	return 0;
}

void dfile(char *path)
{
	char *p = &path[1];
	char *q;
	int noden = 0;

	do  {
		q = strchr(p, '/');
		if (q)
			*q = '\0';
		/*  */
		noden = advance(noden, p);
		if (!noden) {
			printf("No such file %s", p);
			exit(1);
		}

		if (q)
			p = q+1;
		else
			break;
	} while (1);

	printf("%s", blockp(inodep(noden)->i_addr[0]));
	
}

int main(int argc, char *argv[])
{
	char fname[] = "/usr/sys/conf/l.s";

	loadrk("rk0");
	printf("-----------\n");
	//dsuper();

	printf("-----------\n");
	//dinode();

	printf("-----------\n");
	prdir(0, 0);

	printf("----------- %s\n", fname);
	dfile(fname);

	return 0;
}



