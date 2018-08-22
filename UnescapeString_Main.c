#include <stdio.h>
#include <stdlib.h>
#define PACKET_LENGTH 1500

#define START_SYMBOL 03
#define END_SYMBOL 02
// #define xil_printf printf
#define xil_printf

// IDLE
// START_CHAR_ESCAPE
// END_CHAR_ESCAPE
// COLLECT

enum State
{
	IDLE = 0,
	START_CHAR_ESCAPE,
	END_CHAR_ESCAPE,
	COLLECT,
};

static char *StateStrings[] = {
	"IDLE",
	"START_CHAR_ESCAPE",
	"END_CHAR_ESCAPE",
	"COLLECT"};

int Parse_Packet(char data, unsigned char **RxPtrAdd, enum State *currstate)
{
	unsigned char *rxdata = *RxPtrAdd;
	switch (*currstate)
	{
	case IDLE:
		if (data == START_SYMBOL)
			*currstate = START_CHAR_ESCAPE;
		break;
	case START_CHAR_ESCAPE:
		if (data == 0)
			*currstate = COLLECT;
		else
		{
			*currstate = COLLECT;
			*rxdata = data;
			(*RxPtrAdd)++;
		}
		break;
	case END_CHAR_ESCAPE:
		if (data == 0)
		{
			*currstate = IDLE;
			return 1;
		}
		else
		{
			*currstate = COLLECT;
			*rxdata = data;
			(*RxPtrAdd)++;
		}
		break;
	case COLLECT:
		if (data == START_SYMBOL)
			*currstate = START_CHAR_ESCAPE;
		else if (data == END_SYMBOL)
			*currstate = END_CHAR_ESCAPE;
		else
		{
			*rxdata = data;
			(*RxPtrAdd)++;
		}
		break;
	}
	xil_printf("%04X jumping To %s\r\n", data, StateStrings[*currstate]);
	return 0;
}

int main()
{
	int RxWord[PACKET_LENGTH];
	FILE *fp;
	FILE *out_text_fp;
	short int byte_cnt;
	int num;
	unsigned char data[1600];
	unsigned char RxUnescape[3200];
	unsigned char *RxUnescapepPtr;
	unsigned int datacnt;
	short int pktno = 0;
	unsigned int Rx_data;
	unsigned int Rx;
	enum State packetstate = IDLE;
	unsigned int pktdone;

	fp = fopen("FiFoEscapeSequenceWithZerosBin.bin", "rb");
	out_text_fp = fopen("CCODE_FIFOESCBIBin.txt", "w");
	if ((fp == 0) || (out_text_fp == 0))
	{
		printf("Cannot open file\n\r");
		return 0;
	}
	printf("Reading the File\n\r");
	RxUnescapepPtr = &RxUnescape[0];
	while (!feof(fp))
	{
		fread(&Rx_data, sizeof(char), 1, fp);
		pktdone = Parse_Packet(Rx_data, &RxUnescapepPtr, &packetstate);

		if (pktdone)
		{
			pktno = (RxUnescape[0] << 8) | RxUnescape[1];
			datacnt = (RxUnescape[2] << 8) | RxUnescape[3];
			printf("PacketFormed %04X %04X %04X\n", pktdone, datacnt, pktno);
			fprintf(out_text_fp, "%04X\n", pktno);
			fprintf(out_text_fp, "%04X\n", datacnt);
			RxUnescapepPtr = &RxUnescape[0];

			for (num = 0; num < datacnt; num++)
				fprintf(out_text_fp, "%02X ", RxUnescape[num + 4]);
			fprintf(out_text_fp, "\n");
		}
	}
	fclose(fp);
	fclose(out_text_fp);
	return 0;
}
