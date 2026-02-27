import sys
import serial
from time import sleep
import importlib.util
import os

# Path to CRC16.py
current_dir = os.path.dirname(__file__)
crc_path = os.path.join(current_dir, '..', 'MODBUS-CRC16', 'CRC16.py')
crc_path = os.path.abspath(crc_path)

# Load module dynamically
spec = importlib.util.spec_from_file_location("CRC16", crc_path)
crc_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(crc_module)

# Now access the function
MODBUS_CRC16_v1 = crc_module.MODBUS_CRC16_v1

def read_holding_regsiters_payload(start_addr: int, num_regs: int):
	ret = bytearray(5)
	# this function returns fuction code + starting address + number of registers
	ret[0] = 0x03
	ret[1] = (start_addr >> 8) & 0xff
	ret[2] = start_addr & 0xff
	ret[3] = (num_regs >> 8) & 0xff
	ret[4] = num_regs & 0xff
	return ret

def read_input_regsiters_payload(start_addr: int, num_regs: int):
	ret = bytearray(5)
	# this function returns fuction code + starting address + number of registers
	ret[0] = 0x04
	ret[1] = (start_addr >> 8) & 0xff
	ret[2] = start_addr & 0xff
	ret[3] = (num_regs >> 8) & 0xff
	ret[4] = num_regs & 0xff
	return ret

def write_single_register_payload(start_addr: int, reg_values: list):
	ret = bytearray(5)
	# this function returns fuction code + starting address + register value
	ret[0] = 0x06
	ret[1] = (start_addr >> 8) & 0xff
	ret[2] = start_addr & 0xff
	ret[3] = (reg_values[0] >> 8) & 0xff
	ret[4] = reg_values[0] & 0xff
	return ret

def write_multiple_registers_payload(start_addr: int, num_regs: int, reg_values: list):
	ret = bytearray(6 + num_regs * 2)
	# this function returns fuction code + starting address + number of registers - number of bytes + register value
	ret[0] = 0x10
	ret[1] = (start_addr >> 8) & 0xff
	ret[2] = start_addr & 0xff
	ret[3] = (num_regs >> 8) & 0xff
	ret[4] = num_regs & 0xff
	ret[5] = num_regs << 1
	
	for i in range (0, num_regs):
		
		ret[6 + i * 2] = (reg_values[i] >> 8) & 0xff
		ret[6 + i * 2 + 1] = reg_values[i] & 0xff
		
	return ret

def print_frame(buf: bytearray):
	
	for i in range(0, len(buf)):
		
		print(hex(buf[i]), end="")
		
		if i < len(buf) - 1:
			
			print(" ", end="")
			
		else:
			
			print("")

slave_address = 0x10
target_register_address = 0x0004
number_of_regsiters = 1
write_values = [2, 11, 1000]

 # Build frame

frame = bytearray(1)
frame[0] = slave_address

# frame += read_holding_regsiters_payload(target_register_address, number_of_regsiters)
# frame += read_input_regsiters_payload(target_register_address, number_of_regsiters)
# frame += write_single_register_payload(target_register_address, write_values)
frame += write_multiple_registers_payload(target_register_address, number_of_regsiters, write_values)

crc = MODBUS_CRC16_v1(frame, len(frame))

frame += bytearray(1)
frame[len(frame) - 1] = crc & 0xff
frame += bytearray(1)
frame[len(frame) - 1] = crc >> 8 & 0xff

print_frame(frame)

 # Send frame

# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=19200,
    parity=serial.PARITY_EVEN,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
	timeout=1
)

# ser.reset_input_buffer()
ser.flush()

ser.write(frame)

response = ser.read(100)

if response[0] == 0xff:
	
	response_bytearray = response[1: len(response)]
	
else:
	
	response_bytearray = response[0: len(response)]

print_frame(response_bytearray)

crc = MODBUS_CRC16_v1(response_bytearray, len(response_bytearray) - 2)

if ((crc & 0xff) == response_bytearray[len(response_bytearray) - 2]) and ((crc >> 8 & 0xff) == response_bytearray[len(response_bytearray) - 1]):
	
	print("CRC match!")
	
else:
	
	print("CRC mismatch!")



# print(hex(MODBUS_CRC16_v1(bytes("Hello World!".encode("utf-8")), len("Hello World!"))))
