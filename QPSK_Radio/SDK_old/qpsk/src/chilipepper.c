#include "chilipepper.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xuartps.h"
#include "xil_printf.h"
#include <math.h>
#include "xbasic_types.h"
#include <stdio.h>

#define MCU_UART
#define MCU_DRIVER
#define DC_OFFSET
#define TX_DRIVER
#define DAC_DRIVER
#define RX_DRIVER

#ifdef MCU_UART
#include "xuartlite.h"
XUartLite uartLite;
#endif
//#ifdef TX_DRIVER
//#include "tx_axiw.h"
//#endif
//#ifdef RX_DRIVER
//#include "rx_axiw.h"
//#endif
//#ifdef DC_OFFSET
//#include "dc_offset_axiw.h"
//#endif

#define TARGET_RSSI 800
#define TARGET_RSSI_MARGIN 50

unsigned char SendBuffer[3]; /* Buffer for Transmitting Data */
unsigned char RecvBuffer[3]; /* Buffer for Receiving Data */

//#ifdef RX_DRIVER
//xc_iface_t *chiliRxIface;
//xc_from_fifo_t *chiliRxFromFifo;
//#endif

// mcu registers
Xuint32 chili_init_done, chili_pa_en, chili_tr_sw, chili_mcu_reset,
		chili_rx_en, chili_tx_en, chili_mcu_latch, chili_mcu_reg_reset,
		chili_latch_done, chili_reg_reset_done;
// DAC registers
Xuint32 dac_en;
// DC Offset registers
Xuint32 chili_rssi, chili_dir, chili_rssi_high_goal, chili_rssi_low_goal,
		chili_agc_en, rx_en, chili_rssi_en, chili_dir_en, chili_get_next_byte,
		chili_byte_out, chili_byte_ready;
// RX registers
Xuint32 chili_mcu_rx_ready, chili_num_bytes_ready, chili_percent_full;
// tx registers
Xuint32 clear_fifo, tx_en, tx_done, tx_fifo, byte_in, store_byte, byte_received,
		reset_tx_fifo, clear_fifo_done, fifo_full;

/////////////////////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_Initialize(void) {
	int xStatus;
	int k = 0;

	// take care of the UART that goes to/from Chilipepper
#ifdef MCU_UART
	xStatus = XUartLite_Initialize(&uartLite, XPAR_AXI_UARTLITE_0_DEVICE_ID);
	if (xStatus != 0)
		return -1;
	XUartLite_ResetFifos(&uartLite);
#endif

#if defined (MCU_DRIVER)
	chili_pa_en = 			XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x108; // write
	chili_tr_sw = 			XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x104; // write
	chili_mcu_reset = 		XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x100; // write
	chili_rx_en = 			XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x110; // write
	chili_tx_en = 			XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x10C; // write
	chili_mcu_latch =		XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x114; // write
	chili_mcu_reg_reset =	XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x118; // write

	chili_init_done = 		XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x11C; // read
	chili_latch_done = 		XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x120; // read
	chili_reg_reset_done = 	XPAR_MCU_DRIVER_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x124; // read
#endif

#if defined (DAC_DRIVER)
	dac_en = 				XPAR_DAC_DRIVER_PCORE_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x100;
#endif

#if defined (RX_DRIVER)
	chili_num_bytes_ready = XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x120;
	chili_mcu_rx_ready = 	XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x100;
	chili_percent_full = 	XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x11C;
	chili_rssi = 			XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x128; // read
	chili_dir = 			XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x130; // read
	chili_rssi_en = 		XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x12C; // read
	chili_dir_en = 			XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x134; // read
	chili_rssi_high_goal = 	XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x110; // write
	chili_rssi_low_goal = 	XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x10C; // write
	chili_agc_en = 			XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x108; // write
	rx_en = 				XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x114; // write

	//added due to MATLAB --> Pcore
	chili_get_next_byte	=	XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x104; //write
	chili_byte_out = 		XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x118; //read
	chili_byte_ready = 		XPAR_QPSK_RX_FIXPT_PCORE_0_S_AXI_BASEADDR + 0x124; //read
#endif

#if defined (TX_DRIVER)

	clear_fifo = 			XPAR_QPSK_TX_S_AXI_BASEADDR + 0x104; // write
	tx_en = 				XPAR_QPSK_TX_S_AXI_BASEADDR + 0x108; // write

	tx_done = 				XPAR_QPSK_TX_S_AXI_BASEADDR + 0x110; // read
	clear_fifo_done = 		XPAR_QPSK_TX_S_AXI_BASEADDR + 0x11C; // read


	store_byte = 			XPAR_TX_FIFO_S_AXI_BASEADDR + 0x100; // write
	byte_in = 				XPAR_TX_FIFO_S_AXI_BASEADDR + 0x104; // write
	reset_tx_fifo = 		XPAR_TX_FIFO_S_AXI_BASEADDR + 0x108; // write

	byte_received = 		XPAR_TX_FIFO_S_AXI_BASEADDR + 0x10C; // read
	fifo_full = 			XPAR_TX_FIFO_S_AXI_BASEADDR + 0x110; // read
#endif

	chili_mcu_reg_reset = 1;
	for (k = 0; k++; k<1000){
		int j = 0;
	};
	chili_mcu_reg_reset = 0;

	// initialize rx driver
	Xil_Out32(chili_mcu_rx_ready, 1);
	mcu_latch_registers();
	Xil_Out32(chili_mcu_rx_ready, 0);
	mcu_latch_registers();
	Xil_Out32(chili_mcu_rx_ready, 1);
	mcu_latch_registers();

	// by default we enable Tx/Rx. User can disable later if they want
	Chilipepper_TxEnable(1);
	Chilipepper_RxEnable(1);
	// disable data going to DAC prior to reset
#if defined (DAC_DRIVER)
	Xil_Out32(dac_en, 0);
	mcu_latch_registers();
#endif
	Chilipepper_Reset();
	// go ahead and reenable DAC
#if defined (DAC_DRIVER)
	Xil_Out32(dac_en, 1);
	mcu_latch_registers();
#endif
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////

void mcu_latch_registers (void){
	Xil_Out32(chili_mcu_latch, 1);
	while(chili_latch_done == 0)
		;
	Xil_Out32(chili_mcu_latch, 0);
	while(chili_latch_done == 1)
		;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Check to see if onboard calibration has finished
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_InitDone(void) {
	int done;
	done = Xil_In32(chili_init_done);
	return done;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// reset the board
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_Reset(void) {
	// reset Chilipepper
	Xil_Out32(chili_mcu_reset, 1);
	mcu_latch_registers();
	Xil_Out32(chili_mcu_reset, 0);
	mcu_latch_registers();
	Xil_Out32(chili_mcu_reset, 1);
	mcu_latch_registers();
	// don't continue until we get a valid init back from Chilipepper.
	while (Chilipepper_InitDone() == 0)
		;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// set the transmit/receive switch
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SetTxRxSw(int zeroTxOneRx) {
	// 0- transmit, 1-receive - defaults to Rx if input not zero
	if (zeroTxOneRx == 0){
		Xil_Out32(chili_tr_sw, 0);
		mcu_latch_registers();
	}
	else{
		Xil_Out32(chili_tr_sw, 1);
		mcu_latch_registers();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// enable or disable the Tx PA
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SetPA(int onOff) {
	// if not 1 then some other value so for safety disable PA
	if (onOff == 1){
		Xil_Out32(chili_pa_en, 1);
		mcu_latch_registers();
	}
	else{
		Xil_Out32(chili_pa_en, 0);
		mcu_latch_registers();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// enable or disable the Tx portion of the transceiver
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_TxEnable(int onOff) {
	if (onOff == 1){
		Xil_Out32(chili_tx_en, 1);
		mcu_latch_registers();
	}
	else{
		Xil_Out32(chili_tx_en, 0);
		mcu_latch_registers();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// enable or disable the Rx portion of the transceiver
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_RxEnable(int onOff) {
	if (onOff == 1) {
		//*chili_rx_en = 1; // active high (enable rx in MCU)
		Xil_Out32(chili_rx_en, 1);
		Xil_Out32(rx_en, 1);
		mcu_latch_registers();
	} else {
		Xil_Out32(chili_rx_en, 0); // active high (disable rx in MCU)
		Xil_Out32(rx_en, 0);
		mcu_latch_registers();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// read RSSI off the hardware core
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_ReadRssi(void) {
	Xuint32 rssiValue;

	rssiValue = Xil_In32(chili_rssi);

	return (int) rssiValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// turn Chilipeppers onboard LED on/off
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SetLed(int onOff) {
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = 0x00;
	SendBuffer[2] = onOff;
	SendBuffer[3] = 0xFE;

	Chilipepper_SetRegister();
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Increment/Decrement the Chilipepper RX/TX center frequency
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_DeltaFreq(int RXTXswitch, int IncDecswitch){
	unsigned char myVar;
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = (RXTXswitch) ? 0x08:0x07;
	myVar = (IncDecswitch == '1') ? 0x01:0x00;
	SendBuffer[2] = myVar;
	SendBuffer[3] = 0xFE;

	Chilipepper_SetRegister();
	return;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// change the Chilipepper RX/TX center frequency
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SendFreq(char data[], int RXTXswitch) {
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = (RXTXswitch) ? 0x04:0x03;
//	unsigned char myData = hex_decimal(data);
	SendBuffer[2] = hex_decimal(data);
	SendBuffer[3] = 0xFE;
	Chilipepper_Delay();
	Chilipepper_SetRegister();
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// change the Chilipepper RX/TX bandpass filter bandwidth
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SendBand(char data[], int RXTXswitch) {
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = (RXTXswitch) ? 0x06:0x05;
	unsigned char myData = hex_decimal(data);
	SendBuffer[2] = myData;
	SendBuffer[3] = 0xFE;
	Chilipepper_Delay();
	Chilipepper_SetRegister();
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// control the AGC
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_ControlAgc(void) {
	static int gain = 14;
	//int rssi
	int dir;

	//rssi = *chili_rssi;
	dir = Xil_In32(chili_dir);
	// just to be safe clear the agc en if we have no dir instruction
	if (dir == 0)
		Xil_Out32(chili_agc_en, 0);
	if (dir == 1) // increase gain
	{
		gain += 1;
		if (gain > 56)
			gain = 56; // don't allow to grow beyond 60
		Chilipepper_SetRxGain(gain);
		Xil_Out32(chili_agc_en, 1);
		Xil_Out32(chili_agc_en, 0);
	}
	if (dir == 2) // decreases gain
	{
		gain -= 1;
		if (gain < 0)
			gain = 0; // don't allow to fall below 0
		Chilipepper_SetRxGain(gain);
		Xil_Out32(chili_agc_en, 1);
		Xil_Out32(chili_agc_en, 0);
	}
	//xil_printf("Dir %d, RSSI %d, Gain %d\r\n",dir,rssi,gain);
	return gain;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Set the transmitter gain
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SetTxGain(int dB) {
	int lg;

	if (dB < 0)
		lg = 0;
	else if (dB > 25)
		lg = 25;
	else
		lg = dB;
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = 0x01;
	SendBuffer[2] = (unsigned char) lg;
	SendBuffer[3] = 0xFE;

	Chilipepper_SetRegister();
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// control the AGC
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_SetRxGain(int dB) {
	SendBuffer[0] = 0xFF;
	SendBuffer[1] = 0x02;
	SendBuffer[2] = (unsigned char) dB;
	SendBuffer[3] = 0xFE;

	Chilipepper_SetRegister();
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// generic function to send and check a packet
/////////////////////////////////////////////////////////////////////////////////////////////

void Chilipepper_SetRegister(void) {
#ifdef MCU_UART
	int sentBytes, recvBytes, numTries, pass;
	int sentValue, recvValue;

	pass = 1;
	do {
		if (pass == 0) {
			//xil_printf("error setting register\r\n");
			XUartLite_ResetFifos(&uartLite);
		}
		// use the do/while loop to make sure we send all five bytes
		sentBytes = 0;
		numTries = 0;
		do {
			sentBytes += XUartLite_Send(&uartLite, &(SendBuffer[sentBytes]), 4);
			numTries++;
		} while (sentBytes != 4 && numTries < 1000000);
		if (sentBytes != 4)
			pass = 0;

		// make sure we get back the same thing we sent
		recvBytes = 0;
		numTries = 0;
		do {
			recvBytes += XUartLite_Recv(&uartLite, &(RecvBuffer[recvBytes]), 4);
			numTries++;
		} while (recvBytes != 4 && numTries < 1000000);

		if (recvBytes != 4)
			pass = 0;

		sentValue = SendBuffer[2];
		recvValue = RecvBuffer[2];
		if (sentValue != recvValue)
			pass = 0;
		if (sentValue == recvValue && sentBytes == 4 && recvBytes == 4)
			pass = 1;
	} while (pass == 0);
	pass = 1;
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Write packet to FIFO
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_WriteAckPacket(unsigned char *txBuf, unsigned char id) {
	txBuf[4] = id; // payload
	Chilipepper_WritePacket(txBuf, 1, 0);
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Write packet to FIFO
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_WritePacket(unsigned char *txBuf, int numPayloadBytes,
		unsigned char packetID) {
	int count;

	Xil_Out32(rx_en, 0);
	Chilipepper_SetTxRxSw(0); // 0- transmit, 1-receive
//	Chilipepper_FlushRxFifo();

	// 1.) set tx_en low, 2.) toggle clear_fifo, 3.) fill FIFO,
	// 4.) set tx_en high, 5.) wait for tx_done to go high
	Xil_Out32(tx_en, 0);
	Xil_Out32(clear_fifo, 0);
	Xil_Out32(clear_fifo, 1);
	while(clear_fifo_done == 0)
		;
	Xil_Out32(clear_fifo, 0);
	while(clear_fifo_done == 1)
		;
	txBuf[0] = numPayloadBytes % 256;
	txBuf[1] = (numPayloadBytes >> 8) % 256;
	txBuf[2] = (numPayloadBytes >> 16) % 256;
	txBuf[3] = packetID;
	Chilipepper_AppendCrc(txBuf, 4 + numPayloadBytes);
	for (count = 0; count < (numPayloadBytes + 6); count++) {
		Xil_Out32(tx_fifo, txBuf[count]);
		Xil_Out32(store_byte, 1);
		while (Xil_In32(byte_received) == 0)
			;
		Xil_Out32(store_byte, 0);
		while (Xil_In32(byte_received) == 1)
			;
	}

	Xil_Out32(tx_fifo, 55); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
		;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_fifo, 66); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
		;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_fifo, 77); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
		;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_en, 1);

	int i=0;
	while (Xil_In32(tx_done) == 0)
		i++;
		;
	// go back to receive
	Xil_Out32(rx_en, 1);
	Chilipepper_SetTxRxSw(1); // 0- transmit, 1-receive
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Write test "hello world!" packet to FIFO
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_WritePacketWithAck(unsigned char *txBuf, int numPayloadBytes,
		unsigned char *rxBuf) {
	int success, numSendTries, numRecvTries;
	unsigned char id;
	static unsigned char count = 1; // packet ID is a sequential counter, *not* equal to 0 which is ACK
	unsigned char retValue;
	int numBytes;

	success = 0;
	numSendTries = 0;
	do {
		// transmit a packet with the count index
		// count becomes the ID
		Chilipepper_WritePacket(txBuf, numPayloadBytes, count);

		// try to receive an ACK with the same count index
		numRecvTries = 0;
		do {
			numBytes = Chilipepper_ReadPacket(rxBuf, &id);

			if (numBytes > 0) {
				// id is zero for an ACK and first byte is previous id/count
				retValue = rxBuf[4]; // first payload byte
				if ((id == 0) && (retValue == count)) {
					success = 1;
					break; //we received the correct ACK
				}
			}

			numRecvTries += 1;
		} while (numRecvTries < 1000000);
		success = 1;
		if (success == 1)
			break; // Tx/ACK went ok

		numSendTries += 1;
	} while (numSendTries < 10);

	count += 1;
	if (count >= 255)
		count = 1;

	return success;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Write test "hello world!" packet to FIFO
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_WriteTestPacketWithAck(unsigned char *rxBuf) {
	int success, numSendTries, numRecvTries;
	unsigned char id;
	static unsigned char count = 1; // packet ID is a sequential counter, *not* equal to 0 which is ACK
	unsigned char retValue;
	int numBytes;

	success = 0;
	numSendTries = 0;
	do {
		// transmit a packet with the count index
		// count becomes the ID
		Chilipepper_WriteTestPacket(count);

		// try to receive an ACK with the same count index
		numRecvTries = 0;
		do {
			numBytes = Chilipepper_ReadPacket(rxBuf, &id);

			if (numBytes > 0) {
				// id is zero for an ACK and first byte is previous id/count
				retValue = rxBuf[4]; // first payload byte
				if ((id == 0) && (retValue == count)) {
					success = 1;
					break; //we received the correct ACK
				}
			}

			numRecvTries += 1;
		} while (numRecvTries < 1000000);

		if (success == 1)
			break; // Tx/ACK went ok

		numSendTries += 1;
	} while (numSendTries < 10);

	count += 1;
	if (count >= 255)
		count = 1;

	return success;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Write test "hello world!" packet to FIFO
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_WriteTestPacket(unsigned char count) {
	unsigned char testBuf[18];
	int i1;

	Xil_Out32(rx_en, 0);
	Chilipepper_SetTxRxSw(0); // 0- transmit, 1-receive
	Chilipepper_FlushRxFifo();

	// 1.) set tx_en low, 2.) toggle clear_fifo, 3.) fill FIFO,
	// 4.) set tx_en high, 5.) wait for tx_done to go high
	Xil_Out32(tx_en, 0);
	Xil_Out32(clear_fifo, 0);
	Xil_Out32(clear_fifo, 1);
	while(clear_fifo_done == 0)
		;
	Xil_Out32(clear_fifo, 0);
	while(clear_fifo_done == 1)
		;

	// first three bytes are number of bytes to write to FIFO
	testBuf[0] = 12;
	testBuf[1] = 0;
	testBuf[2] = 0;
	testBuf[3] = count;
	testBuf[4] = 104;
	testBuf[5] = 101;
	testBuf[6] = 108;
	testBuf[7] = 108;
	testBuf[8] = 111;
	testBuf[9] = 32;
	testBuf[10] = 119;
	testBuf[11] = 111;
	testBuf[12] = 114;
	testBuf[13] = 108;
	testBuf[14] = 100;
	testBuf[15] = 33;

	Chilipepper_AppendCrc(testBuf, 16);
	for (i1 = 0; i1 < 18; i1++){
		Xil_Out32(tx_fifo, testBuf[i1]);
		Xil_Out32(store_byte, 1);
		while (Xil_In32(byte_received) == 0)
		;
		Xil_Out32(store_byte, 0);
		while (Xil_In32(byte_received) == 1)
			;
	}

	Xil_Out32(tx_fifo, 55); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
	;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_fifo, 66); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
	;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_fifo, 77); // help with SRRC filtering at end
	Xil_Out32(store_byte, 1);
	while (Xil_In32(byte_received) == 0)
	;
	Xil_Out32(store_byte, 0);
	while (Xil_In32(byte_received) == 1)
		;


	Xil_Out32(tx_en, 1);

	while (Xil_In32(tx_done) == 0)
		;
	// go back to receive mode
	Chilipepper_SetTxRxSw(1); // 0- transmit, 1-receive
	Xil_Out32(rx_en, 1);
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// pull a packet off the receive FIFO
// #1 This begins by indicating that we are ready to receive a packet
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_ReadPacket(unsigned char *rxBuf, unsigned char *id) {
#ifdef RX_DRIVER
	int numBytes, i1;
	u32 value;
	int numBytesReady;
	int retValue;

	// put transceiver in receive mode
	Chilipepper_SetTxRxSw( 1 ); // 0- transmit, 1-receive
	Xil_Out32(rx_en, 1);
	Xil_Out32(chili_mcu_rx_ready, 1);

	numBytesReady = Xil_In32(chili_num_bytes_ready);

	if (numBytesReady == 0)
	return -1;

	// read the bytes and the CRC
	for (i1=0; i1<numBytesReady; i1++)
	{
		Xil_Out32(chili_get_next_byte, 1);
		int tries = 0;
		while (Xil_In32(chili_byte_ready) == 0 && tries < 100)
			tries++;
		;
		Xil_Out32(chili_get_next_byte, 0);
		if (tries < 100)
			rxBuf[i1] = Xil_In32(chili_byte_out);
		else
			rxBuf[i1] = 0;
	}

	*id = rxBuf[3];
	numBytes = rxBuf[0];

	Chilipepper_FlushRxFifo();

	if (Chilipepper_CheckCrc( rxBuf, numBytes+6 ) != 0)
	retValue = -5;
	else
	retValue = numBytes;

	// go ahead and queue up for new packet
	Xil_Out32(chili_mcu_rx_ready, 0);
	Xil_Out32(chili_mcu_rx_ready, 1);

	return retValue;
#endif
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// FlushRxFifo
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_FlushRxFifo(void) {
#ifdef RX_DRIVER
	u32 empty, value;

	// flush the read FIFO
//	while(1)
//	{
//		XC_Read(chiliRxIface, chiliRxFromFifo->empty, &empty);
//		if (empty == 1)
//		break;
//		XC_Read(chiliRxIface, chiliRxFromFifo->dout, &value);
//	}
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Check the CRC of the received packet
/////////////////////////////////////////////////////////////////////////////////////////////
int Chilipepper_CheckCrc(unsigned char *rxBuf, int numBytes) {
	unsigned short valueCRC, genPoly; // 16-bits
	unsigned short p, b, top;
	unsigned long valueCRCsh1, valueCRCadd1; // 32-bits
	unsigned char cv, d;
	int i1, i2;

	valueCRC = 65535;
	genPoly = 4129;
	for (i1 = 0; i1 < numBytes; i1++) // add header and CRC
	{
		cv = rxBuf[i1];
		for (i2 = 0; i2 < 8; i2++) {
			p = pow(2, 7 - i2);
			d = cv / p;
			b = d % 2; // mod of message
			valueCRCsh1 = valueCRC << 1; // shift left
			valueCRCadd1 = valueCRCsh1 | b; // bit or
			top = valueCRCadd1 / pow(2, 16);
			if (top == 1) {
				valueCRC = valueCRCadd1 ^ genPoly; // bit xor
			} else {
				valueCRC = valueCRCadd1;
			}
			valueCRC = valueCRC % 65535;
		}
	}
	return valueCRC;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Check the CRC of the received packet
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_AppendCrc(unsigned char *buf, int msgLength) {
	unsigned short valueCRC, genPoly; // 16-bits
	unsigned short p, b, top;
	unsigned long valueCRCsh1, valueCRCadd1; // 32-bits
	int i1, i2;
	unsigned char msb, lsb;
	unsigned char cv, d;

	valueCRC = 65535;
	genPoly = 4129;
	for (i1 = 0; i1 < (msgLength + 2); i1++) // add header and CRC
	{
		cv = buf[i1];
		for (i2 = 0; i2 < 8; i2++) {
			p = pow(2, 7 - i2);
			if (i1 < msgLength) {
				d = cv / p;
				b = d % 2; // mod of message
			} else
				b = 0;
			valueCRCsh1 = valueCRC << 1; // shift left
			valueCRCadd1 = valueCRCsh1 | b; // bit or
			top = valueCRCadd1 / pow(2, 16);
			if (top == 1) {
				valueCRC = valueCRCadd1 ^ genPoly; // bit xor
			} else {
				valueCRC = valueCRCadd1;
			}
			valueCRC = valueCRC % 65535;
		}
	}
	msb = valueCRC >> 8;
	lsb = valueCRC % 256;
	buf[msgLength] = msb;
	buf[msgLength + 1] = lsb;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Delay
/////////////////////////////////////////////////////////////////////////////////////////////
void Chilipepper_Delay() {
	int i = 0;
	for (i = 0; i < 5000000; i++) {
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////

void clear_uart_buffer(XUartPs *uart) {
	u8 Buf[256];
	int numReads = 0;
	unsigned char numUartRead;

	do {
		Chilipepper_Delay();
		numUartRead = XUartPs_Recv(uart, Buf, 1);
		numReads++;
	} while (numUartRead == 0 && numReads > 100);
}

int Chilipepper_cin(XUartPs *uart, unsigned char* myInput) {
	int i = 0, totalSent = 0;
	unsigned char numUartRead;
	u8 Buf[256];
	do {
		do {
			numUartRead = XUartPs_Recv(uart, Buf, 1);
		} while (numUartRead == 0);
		XUartPs_Send(uart, Buf, 1);
		totalSent += numUartRead;
		while (numUartRead-- != 0) {
			if (Buf[numUartRead] == '\r') {
				myInput[i] = '\0';
				return totalSent;
			}
			myInput[i] = Buf[numUartRead];
			i++;
		}
	} while (Buf[0] != 0x0D);
	myInput[i] = '\0';
	return totalSent;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Chilipepper_printf(XUartPs *uart, unsigned char *str) {
	int i, len, k, max;
	k = 0;
	len = (int) (strlen((char*) str));

	for (i = 0; i <= (len / 64); i++) {
		max = ((int) (i + 1) * 64);
		while (k < max) {
			XUartPs_Send(uart, str++, 1);
			if (k == len) {
				break;
			}
			k++;
		}
		Chilipepper_Delay();
	}
	return;
}

/* Function to convert hexadecimal to decimal. */
int hex_decimal(char hex[])
{
	int i, length, sum = 0;
	for (length = 0; hex[length] != '\0'; ++length)
		;
	for (i = 0; hex[i] != '\0'; ++i, --length) {
		if (hex[i] >= '0' && hex[i] <= '9')
			sum += (hex[i] - '0') * pow(16, length - 1);
		if (hex[i] >= 'A' && hex[i] <= 'F')
			sum += (hex[i] - 55) * pow(16, length - 1);
		if (hex[i] >= 'a' && hex[i] <= 'f')
			sum += (hex[i] - 87) * pow(16, length - 1);
	}
	return sum;
}

void Chilipepper_SetFreq(int total, int RXTXswitch) {
	char message[6];
	int i, MyTotal = total;
	char hexData[3];

	sprintf(message, "%X", MyTotal);

	switch (total < 1048575){
	case 0:
		for (i = 0; i < 3; i++) {
			hexData[0] = (char) message[2 * i];
			hexData[1] = (char) message[2 * i + 1];
			hexData[2] = (char) 0;
			if (checkEscapeValues(hexData, RXTXswitch, Chilipepper_SendFreq))
				Chilipepper_SendFreq(hexData, RXTXswitch);
		}
		break;
	case 1:
		hexData[0] = '0';
		hexData[1] = (char) message[0];
		hexData[2] = (char) 0;
		Chilipepper_SendFreq(hexData, RXTXswitch);

		hexData[0] = (char) message[1];
		hexData[1] = (char) message[2];
		hexData[2] = (char) 0;
		if (checkEscapeValues(hexData, RXTXswitch, Chilipepper_SendFreq))
			Chilipepper_SendFreq(hexData, RXTXswitch);

		hexData[0] = (char) message[3];
		hexData[1] = (char) message[4];
		hexData[2] = (char) 0;
		if (checkEscapeValues(hexData, RXTXswitch, Chilipepper_SendFreq))
			Chilipepper_SendFreq(hexData, RXTXswitch);
		break;
	default:
		break;
	}
	return;
}

void Chilipepper_SetBand(int total, int RXTXswitch) {
	char message[4];
	int i, MyTotal = total;
	char hexData[3];

	sprintf(message, "%X", MyTotal);

	switch (total < 4096) {
	case 0:
		for (i = 0; i < 2; i++) {
			hexData[0] = (char) message[2 * i];
			hexData[1] = (char) message[2 * i + 1];
			hexData[2] = (char) 0;
			if (checkEscapeValues(hexData, RXTXswitch, Chilipepper_SendBand))
				Chilipepper_SendBand(hexData, RXTXswitch);
		}
		break;
	case 1:
		hexData[0] = '0';
		hexData[1] = (char) message[0];
		hexData[2] = (char) 0;
		Chilipepper_SendBand(hexData, RXTXswitch);
		hexData[0] = (char) message[1];
		hexData[1] = (char) message[2];
		hexData[2] = (char) 0;
		if (checkEscapeValues(hexData, RXTXswitch, Chilipepper_SendBand))
			Chilipepper_SendBand(hexData, RXTXswitch);
		break;
	default:
		break;
	}
	return;
}

int checkEscapeValues(char hexData[3], int RXTXswitch, void (*sendEscape)(char[], int)){

	if (hexData[0] == 'F' && hexData[1] == 'F'){
		hexData[1] = 'D';
		(*sendEscape)(hexData, RXTXswitch);
		hexData[0] = '0';
		hexData[1] = '2';
		hexData[2] = (char) 0;
		(*sendEscape)(hexData, RXTXswitch);
		return 0;
	}

	else if (hexData[0] == 'F' && hexData[1] == 'E'){
		hexData[1] = 'D';
		(*sendEscape)(hexData, RXTXswitch);
		hexData[0] = '0';
		hexData[1] = '1';
		hexData[2] = (char) 0;
		(*sendEscape)(hexData, RXTXswitch);
		return 0;
	}

	else if (hexData[0] == 'F' && hexData[1] == 'D'){
		hexData[1] = 'D';
		(*sendEscape)(hexData, RXTXswitch);
		hexData[0] = '0';
		hexData[1] = '0';
		hexData[2] = (char) 0;
		(*sendEscape)(hexData, RXTXswitch);
		return 0;
	}
	return 1;
}
