/*
This is an ACIO helper class, just some static functions a few pieces of these drivers will use
*/

#ifndef ACIO_H
#define ACIO_H

#include "RageLog.h"
#include <string.h>
#include <stdint.h>
#include "arch/COM/serial.h"


class ACIO
{
public:
	static bool baudCheckWrapper(serial::Serial &acio_bus);
	static bool baudCheck(serial::Serial &acio_bus);
	static int prep_acio_packet_for_transmission(uint8_t* acio_request, int r_length, int init_escape_offset=1);
	static int unescape_acio_packet(uint8_t* acio_response2, int len, int data_start=-1);
	static int read_acio_packet(serial::Serial &acio_bus, uint8_t* acio_response);
	static int write_acio_packet(serial::Serial &acio_bus, uint8_t* acio_request,int r_length);
	static int assemble_init_packet_to_write(uint8_t* acio_request, uint8_t* init_line);

private:

};

#endif /* ACIO_H */