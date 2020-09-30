#include "memory.h"

void dump(unsigned char mem[65536][16], int start, int end) {
	/*
	 * prints out the contents of the memory from start to end
	 * Arguments:
	 *	char mem[][] - the virtual memory array
	 * 	int start - the start address to print
	 * 	int end - the end address to print
	 */
	int start_row, end_row;
	int mask=1; /* if set, memory content is masked */
	char str[17]; /* string to be printed for the ascii of the content */
	char ch;

	/* assert 0 <= start <= end <= 0xFFFFF */
	if(!(0<=start && start<=end && end <= 0xFFFFF)) {
		printf("invalid address range\n");
		return;
	}

	
	start_row = start / 16;
	end_row = end / 16;

	/* dump memory */
	for(int r=start_row; r<=end_row; r++) {
		printf("%05X  ", 16*r);
		for(int c = 0; c<16; c++) {
			/* determine the mask */
			mask = (start <= 16*r+c) && (16*r+c <= end) ? 0 : 1; 
			/* print content */
			if(!mask) {
				/* unmasked content */
				ch = mem[r][c];
				printf("  %02X", (unsigned char)ch);
				str[c] = (0x20 <= ch && ch <= 0x7E) ? ch : '.';
			} else {
				printf("    ");
				str[c] = '.';
			}
		}
		str[16] = '\0';
		printf("  ;  %s\n",str);
	}
}

void edit(unsigned char mem[65536][16], int address, int value) {
	/*
	 * updates the content in the given address to the given value
	 * Arguments:
	 * 	char mem[][] - the virtual memory array
	 * 	int address - where to update
	 * 	int value - value to be updated
	 */
	int row, col;

	row = address / 16;
	col = address % 16;

	mem[row][col] = value;
}

void fill(unsigned char mem[65536][16], int start, int end, int value) {
	/*
	 * fills the content to the given value within the whole range from start to end
	 * Arguments:
	 *	char mem[][] - the virtual memory array
	 *	int start - start address
	 * 	int end - end address
	 *	int value - value to be updated
	 */
	for(int address=start; address<=end; address++) edit(mem, address, value);
}

void reset(unsigned char mem[65536][16]) {
	/*
	 * resets the whole memory to 0
	 * Arguments:
	 *	char mem[][] - the virtual memory array
	 */
	fill(mem, 0, 0xFFFFF, 0);
}

unsigned int read_hb(unsigned char mem[65536][16], int address, int length) {
	/*
	 * read memory in half-bytes and wrap into unsigned int
	 * Arguments:
	 * 	char mem[][]
	 * 	int address - the smallest address containing at least one of the desired half-bytes
	 * 	int length - the number of half-bytes desired
	 */
	unsigned int buffer=0,ret=0;
	int n = length/2;
	int idx = 0;
	if(length%2) { /* if length is odd */
		buffer = mem[address/16][address%16]; /* read byte */
		buffer &= 0xF; /* get rid of upper halfbyte */
		buffer <<= 8*n; /* match the digit */
		ret |= buffer; /* put it to ret */
		//buffer = 0; /* clear buffer */
		address++; /* regard it as if it were even in the first place */
	}
	while(idx<n) { /* for each byte */
		buffer = mem[(address+idx)/16][(address+idx)%16]; /* read byte */
		buffer <<= 8*(n-1-idx); /* match the digit */
		ret |= buffer; /* put it to ret */
		//buffer = 0; /* clear buffer */
		idx++;
	}
	return ret;
} 

void write_hb(unsigned char mem[65536][16], int address, int length, unsigned int value) {
	/*
	 * inverse operation of read_hb()
	 * Arguments:
	 * 	char mem[][]
	 * 	int address - the smallest address containing at least one of the desired half-bytes
	 * 	int length - the number of half-bytes desired
	 * 	unsigned int value - value to write
	 */
	unsigned int mask,buffer;
	int n = length/2;
	int idx = 0;
	if(length%2) { /* if length is odd */
		mask = 0xF; /* mask for halfbyte */
		mask <<= 8*n; /* match the digit */
		buffer = value&mask; /* get the desired halfbyte */
		buffer >>= 8*n; /* put it in the rightmost */
		mem[address/16][address%16] &= 0xF0; /* clear mem */
		mem[address/16][address%16] |= (unsigned char)buffer; /* write halfbyte */
		address++; /* regard it as if it were even in the first place */
	}
	while(idx<n) { /* for each byte */
		mask = 0xFF; /* mask for byte */
		mask <<= 8*(n-1-idx); /* match the digit */
		buffer = value&mask; /* get the desired halfbyte */
		buffer >>= 8*(n-1-idx); /* put it in the rightmost */
		mem[(address+idx)/16][(address+idx)%16] = (unsigned char)buffer; /* write halfbyte */
		idx++;
	}
} 

void modify_hb(unsigned char mem[65536][16], int address, int length, int opmode, unsigned int operand) {
	/*
	 * the composite (write_hb) * (opmode) * (read_hb)
	 * Arguments:
	 * 	int opmode -- add if 1, sub if 0
	 */
	unsigned int content;
	content = read_hb(mem,address,length);
	content = (opmode ? (content+operand) : (content-operand));
	write_hb(mem,address,length,content);
}

