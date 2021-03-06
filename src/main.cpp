/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

/**
 * A simple example of sending data from 1 nRF24L01 transceiver to another
 * with Acknowledgement (ACK) payloads attached to ACK packets.
 *
 * This example was written to be used on 2 devices acting as "nodes".
 * Use the Serial Monitor to change each node's behavior.
 */
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

// instantiate an object for the nRF24L01 transceiver
RF24 radio(9, 10); // using pin 9 for the CE pin, and pin 10 for the CSN pin

// an identifying device destination
// Let these addresses be used for the pair
uint8_t address[][6] = {"2PC00", "2QC01"};
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
// to use different addresses on a pair of radios, we need a variable to

// uniquely identify which address this radio will use to transmit
bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

void setup()
{

	Serial.begin(57600);
	while (!Serial)
	{
		// some boards need to wait to ensure access to serial over USB
	}

	// initialize the transceiver on the SPI bus
	if (!radio.begin())
	{
		while (1) {
		} // hold in infinite loop
	}

	// Set the PA Level low to try preventing power supply related problems
	// because these examples are likely run with nodes in close proximity to
	// each other.
	radio.setPALevel(RF24_PA_MAX);     // RF24_PA_MAX is default.

	radio.setChannel(0x6f);

	radio.setDataRate(RF24_250KBPS);

	// below is to reduce time to send for each payload
	radio.setAddressWidth(3);
	radio.setCRCLength(RF24_CRC_8);

	// to use ACK payloads, we need to enable dynamic payload lengths (for all nodes)
	radio.enableDynamicPayloads();    // ACK payloads are dynamically sized

	// Acknowledgement packets have no payloads by default. We need to enable
	// this feature for all nodes (TX & RX) to use ACK payloads.
	radio.enableAckPayload();

	// set the TX address of the RX node into the TX pipe
	radio.openWritingPipe(address[radioNumber]);     // always uses pipe 0

	// set the RX address of the TX node into a RX pipe
	radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1

	radio.stopListening();                                      // put radio in TX mode

	// For debugging info
	// printf_begin();             // needed only once for printing details
	// radio.printDetails();       // (smaller) function that prints raw register values
	// radio.printPrettyDetails(); // (larger) function that prints human readable data

}

//unsigned long time = 0;
//unsigned long counter = 0;

void loop()
{
	// process outgoing data

	char payload[33];
	memset(payload, 0, sizeof(payload));
	int available = min(32, Serial.available());

	Serial.readBytes(payload, available);

	bool report = radio.write(&payload, available);

	if (report)
	{
		uint8_t pipe;
		if (radio.available(&pipe))
		{
			// process incoming data
			char received[33];
			memset(received, 0, sizeof(received));
			
			uint8_t bytes = radio.getDynamicPayloadSize();

			radio.read(&received, bytes);

			if (received[0] || bytes > 1) // if no data was sent, we will receive 1 byte '\0'
				Serial.write(received, bytes);
		}
		else
		{
			// empty ACK packet received
		}


	}
	else
	{
		// payload was not delivered
	}

	//delay(100);  // slow transmissions down to be readable in terminal
} // loop
