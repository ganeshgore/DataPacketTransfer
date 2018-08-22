#include <stdio.h>
#include <stdlib.h>
#define PACKET_LENGTH 1500

#define START_PACKET 03
#define END_PACKET 02

#define PRINT_DATA
#define PRINT_ESCAPED_DATA

int EscapeString(char *Fifobuff, char *ethernetbuff, short int datalength, unsigned int pktno)
{
	int ethcnt, fifocnt;
	fifocnt = 0;

	// Add Start Frame into Sequence
	Fifobuff[fifocnt++] = START_PACKET;
	Fifobuff[fifocnt++] = 0;

	// Add Packet Number into Sequence
	Fifobuff[fifocnt++] = (char)(pktno >> 8) & 0x00FF;
	if ((Fifobuff[fifocnt - 1] == START_PACKET) || (Fifobuff[fifocnt - 1] == END_PACKET))
		Fifobuff[fifocnt++] = (char)(pktno >> 8) & 0x00FF; //repeat it again

	Fifobuff[fifocnt++] = (char)(pktno & 0x00FF);
	if ((Fifobuff[fifocnt - 1] == START_PACKET) || (Fifobuff[fifocnt - 1] == END_PACKET))
		Fifobuff[fifocnt++] = (char)(pktno & 0x00FF); //repeat it again

	// Add Packate DataLength into Sequence
	Fifobuff[fifocnt++] = (char)(datalength >> 8) & 0x00FF;
	if ((Fifobuff[fifocnt - 1] == START_PACKET) || (Fifobuff[fifocnt - 1] == END_PACKET))
		Fifobuff[fifocnt++] = (char)(datalength >> 8) & 0x00FF; //repeat it again

	Fifobuff[fifocnt++] = (char)(datalength & 0x00FF);
	if ((Fifobuff[fifocnt - 1] == START_PACKET) || (Fifobuff[fifocnt - 1] == END_PACKET))
		Fifobuff[fifocnt++] = (char)(datalength & 0x00FF); //repeat it again

	// Added Sequence with Escape Characters
	for (ethcnt = 0; ethcnt < datalength; ethcnt++)
	{
		Fifobuff[fifocnt++] = ethernetbuff[ethcnt];

		// Repreat in case of Start and End Packets
		if ((ethernetbuff[ethcnt] == START_PACKET) || (ethernetbuff[ethcnt] == END_PACKET))
			Fifobuff[fifocnt++] = ethernetbuff[ethcnt];
	}

	// Add Final Packet in Sequence
	Fifobuff[fifocnt++] = END_PACKET;
	Fifobuff[fifocnt++] = 0;

	// Fill up buffer with zeros to align word boundary
	while (fifocnt % 4)
		Fifobuff[fifocnt++] = 0;
	return fifocnt;
}

int main()
{
	int RxWord[PACKET_LENGTH];
	FILE *fp;
	FILE *out_text_fp;
	short int byte_cnt;
	int num;
	unsigned char data[1600];
	unsigned char EscapedData[3200];
	unsigned int datacnt;
	short int pktno = 0;

	fp = fopen("InputPacketsBin.bin", "rb");
	out_text_fp = fopen("CCODE_FIFO_ESC.txt", "w");
	if (
		(fp == 0) ||
		(out_text_fp == 0))
	{
		printf("Cannot open file\n\r");
		return 0;
	}
	printf("Reading the File\n\r");
	while (!feof(fp))
	{
		fread(&pktno, sizeof(short int), 1, fp);
		fread(&byte_cnt, sizeof(short int), 1, fp);
		byte_cnt = ((byte_cnt >> 8) & 0xFF) | ((byte_cnt << 8) & 0xFF00);
		pktno = ((pktno >> 8) & 0xFF) | ((pktno << 8) & 0xFF00);
		printf("\r\n%04X Size of load is %04X", pktno, byte_cnt);
		if (feof(fp))
			break;

		fread(&data, sizeof(char), byte_cnt, fp);

		datacnt = EscapeString(EscapedData, data, byte_cnt, pktno);
		printf(" AfterEscape length %04X", datacnt);
		for (num = 0; num < datacnt; num++)
			fprintf(out_text_fp, "%02X\r\n", EscapedData[num]);
	}
	printf("\r\n");
	fclose(fp);
	fclose(out_text_fp);
	return 0;
}
