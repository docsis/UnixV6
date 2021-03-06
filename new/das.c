#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define OP_DUB 0x01
#define OP_SGL 0x02
#define OP_JSR 0x04
#define OP_BR  0x08
#define OP_JMP 0x10
#define OP_SOB 0x20

char *getsym(uint16_t addr);

struct insrtab {
	uint16_t op;
	uint16_t mask;
	char *str;
	uint16_t type;
};

struct symtab {
	uint16_t addr;
	char type;
	char *pname;
};

static struct symtab stab[400];
static uint16_t pc = 0466;
static uint16_t fbuf[30720/2];
static uint16_t fhead[8];

struct insrtab itab[] = {
/* double */
{0010000,0170000,"MOV", OP_DUB},
{0110000,0170000,"MOVB", OP_DUB},
{0020000,0170000,"CMP", OP_DUB},
{0120000,0170000,"CMPB", OP_DUB},
{0030000,0170000,"BIT", OP_DUB},
{0130000,0170000,"BITB", OP_DUB},
{0040000,0170000,"BIC", OP_DUB},
{0140000,0170000,"BICB", OP_DUB},
{0050000,0170000,"BIS", OP_DUB},
{0150000,0170000,"BISB", OP_DUB},
{0060000,0170000,"ADD", OP_DUB},
{0160000,0170000,"SUB", OP_DUB},
{0070000,0177000,"MUL", OP_DUB},
{0071000,0177000,"DIV", OP_DUB},
{0072000,0177000,"ASH", OP_DUB},
{0073000,0177000,"ASHC", OP_DUB},
{0074000,0177000,"XOR", OP_DUB},
{0077000,0177000,"SOB", OP_SOB},

/* single */
{0000300,0077700,"SWAB", OP_SGL},
{0004000,0177000,"JSR", OP_SGL | OP_JSR},
{0005000,0077700,"CLR", OP_SGL},
{0005100,0077700,"COM", OP_SGL},
{0005200,0077700,"INC", OP_SGL},
{0005300,0077700,"DEC", OP_SGL},
{0005400,0077700,"NEG", OP_SGL},
{0005500,0077700,"ADC", OP_SGL},
{0005600,0077700,"SBC", OP_SGL},
{0005700,0077700,"TST", OP_SGL},
{0006000,0077700,"ROR", OP_SGL},
{0006100,0077700,"ROL", OP_SGL},
{0006200,0077700,"ASR", OP_SGL},
{0006300,0077700,"ASL", OP_SGL},
{0006400,0077700,"MARK", OP_SGL},
{0106400,0077700,"MTPS", OP_SGL},
{0006500,0077700,"MFPI", OP_SGL},
{0106500,0077700,"MFPD", OP_SGL},
{0006600,0077700,"MTPI", OP_SGL},
{0106600,0077700,"MTPD", OP_SGL},
{0006700,0077700,"SXT", OP_SGL},
{0106700,0077700,"MFPS", OP_SGL},
// offset
{0000400,0007400,"BR", OP_BR},

{0001000,0007400,"BNE", OP_BR},
{0001400,0007400,"BEQ", OP_BR},

{0002000,0007400,"BGE", OP_BR},
{0002400,0007400,"BLT", OP_BR},

{0003000,0007400,"BGT", OP_BR},
{0003400,0007400,"BLE", OP_BR},

{0100000,0177400,"BPL", OP_BR},
{0100400,0177400,"BMI", OP_BR},
{0102000,0177400,"BVC", OP_BR},
{0102400,0177400,"BVS", OP_BR},
{0103000,0177400,"BCC", OP_BR},
{0103400,0177400,"BCS", OP_BR},

{0000100,0177700,"JMP", OP_SGL | OP_JMP},


{0000207,0077777,"RTS PC"},

{0000000,0077777,"HALT"},
{0000001,0077777,"WAIT"},
{0000002,0077777,"RTI"},
{0000003,0077777,"BPT"},
{0000004,0077777,"IOT"},
{0000005,0077777,"RESET"},
{0000006,0077777,"RTT"},

{0000000,0000000, "NNN"},
};

struct insrtab *getop(uint16_t ins)
{
	int i;
	for (i=0; i<sizeof(itab)/sizeof(itab[0]); i++) {
		if ((ins & itab[i].mask) == itab[i].op)
			return &itab[i];
	}
}

int inslen(struct insrtab *tab, uint16_t ins)
{
	int len = 1;

	if (tab->type & OP_DUB)	{
		if ((ins & 070) == 060 ||  (ins & 070) == 070)
			len++;
		if ((ins & 07000) == 06000 ||  (ins & 07000) == 07000)
			len++;
		/* pc */
		if ((ins & 077) == 027 || (ins & 077) == 037) 
			len++;
		if ((ins & 07700) == 02700 || (ins & 07700) == 03700)
			len++;
	} else if (tab->type & OP_SGL) {
		if ((ins & 070) == 060 ||  (ins & 070) == 070)
			len++;
		if ((ins & 077) == 027 || (ins & 077) == 037) 
			len++;
	}

	return len;
}

char *rname[] = {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"SP",
	"PC",
};

char *opd_str(uint16_t pc, uint16_t mode, uint16_t imm, char *buf, int jmp)
{
	uint16_t jmpaddr;
	char *p;

	buf[0] = '\0';

	/* 067 pc relative +2 word */
	if ((mode & 07) == 07) {
		switch (mode & 070) {
		case 020:
			sprintf(buf, "#%o", imm);
			break;
		case 030:
			sprintf(buf, "@#%o", imm);
			break;
		case 060:
			if (jmp) {
				if (imm & 0100000)
					jmpaddr = pc - (((~(imm & 077777)) & 077777) + 1) + 4;
				else
					jmpaddr = pc + imm + 4;
				if ((p=getsym(jmpaddr)) != "")
					return p;
			} else {
				jmpaddr = pc + imm + 4;
			}
			sprintf(buf, "%o", jmpaddr);
			break;
		case 070:
			sprintf(buf, "@%o", pc + imm + 4);
			break;
		}
		return buf;
	}

	switch (mode & 070) {
	case 000:
		sprintf(buf, "%s", rname[mode & 07]);
		break;
	case 010:
		sprintf(buf, "(%s)", rname[mode & 07]);
		break;
	case 020:
		sprintf(buf, "(%s)+", rname[mode & 07]);
		break;
	case 030:
		sprintf(buf, "@(%s)+", rname[mode & 07]);
		break;
	case 040:
		sprintf(buf, "-(%s)", rname[mode & 07]);
		break;
	case 050:
		sprintf(buf, "@-(%s)", rname[mode & 07]);
		break;
	case 060:
		sprintf(buf, "%o(%s)", imm, rname[mode & 07]);
		break;
	case 070:
		sprintf(buf, "@%o(%s)", imm, rname[mode & 07]);
		break;
	}
	return buf;
}

void loadfile(char *name)
{
	FILE *f;

	f = fopen(name, "r");
	if (!f) {
		perror("file");
		exit(1);
	}

	// skip 16byte header
	fread(fhead, 1, 16, f);
	fread(fbuf, 1, sizeof(fbuf), f);

	fclose(f);
}

char *getsym(uint16_t addr)
{
	struct symtab *p = stab;

	while (p->addr < addr) {
		p++;
	}
	if (addr == p->addr)
		return p->pname;

	return "";
}

char *getbroff(uint16_t ins, uint16_t pc, char *buf)
{
	uint16_t braddr;
	char *p;

	if (ins & 040) // ~ + 1 5bit offset 1bit sign
		braddr = pc + 2 - 2* (((~(ins & 037)) & 037) + 1);
	else
		braddr = pc + 2 + 2*(ins & 077);

	if ((p=getsym(braddr)) != "") {
		return p;
	}
	sprintf(buf, "%o", braddr);
	return buf;
}

void disa(void)
{
	struct insrtab *tab;
	uint16_t ins;
	int ilen;
	char opdbuf[128];

	for (; pc<061066; ) {
		ins = fbuf[pc/2];
		tab = getop(ins);
		ilen = inslen(tab, ins);

		printf("%-8s %o: %06o ", getsym(pc), pc, ins);
#if 0
		if (ilen >= 2)
			printf("%06o ", fbuf[pc/2 + 1]);
		if (ilen >= 3)
			printf("%06o ", fbuf[pc/2 + 2]);
#endif
		printf("%s ", tab->str);

		/* BR */
		if (tab->type & OP_BR) {
			printf("%s", getbroff(ins, pc, opdbuf));
		}

		if (tab->type & OP_SOB) {
			printf("%s,%o", rname[(ins&0700)>>6], pc+2-(ins&077)*2);
		}

		if (tab->type & OP_JSR) {
			printf("%s,", rname[(ins & 0700)>>6]);
		}
		if (tab->type & OP_DUB) {
			printf("%s,", opd_str(pc, (ins & 07700) >> 6, fbuf[pc/2 + 1], opdbuf, 0));
			printf("%s", opd_str(pc+2, ins & 077, fbuf[pc/2 + 2], opdbuf, 0));
		}
		if (tab->type & OP_SGL) {
			/* */
			if (tab->type & (OP_JMP | OP_JSR))
				printf("%s", opd_str(pc, ins & 077, fbuf[pc/2 + 1], opdbuf, 1));
			else
				printf("%s", opd_str(pc, ins & 077, fbuf[pc/2 + 1], opdbuf, 0));
		}
		printf("\n");
		pc +=  ilen * 2;
	}

}

void loadsym(char *fname)
{
	FILE *f;
	char line[128];
	struct symtab *p = stab;

	f = fopen(fname, "r");
	if (!f) {
		perror("file");
		exit(1);
	}

	while (fgets(line, sizeof(line), f) != NULL) {
		line[strlen(line)-1] = '\0';
		p->addr = strtol(line, NULL, 8);
		p->type = line[6];
		p->pname = strdup(&line[8]);
		p++;
	}

	fclose(f);
}

void dsym(void)
{
	struct symtab *p = stab;

	while (p->addr) {
		printf("%o %c %s\n", p->addr, p->type, p->pname);
		p++;
	}

}

int main(int argc, char *argv[])
{
	loadfile("dfile");
	loadsym("unix.nm");
	//dsym();
	disa();

	return 0;
}



