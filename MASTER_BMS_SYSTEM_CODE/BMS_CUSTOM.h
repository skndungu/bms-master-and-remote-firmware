#include <Arduino.h>
#include <stdint.h>
#include <CAN.h>

static char startByte = 0xDD;
static char stopByte = 0x77;

//command codes

static char getBasicInfo = 0x03;
static char getVolt      = 0x04;
static char getBMSVersion= 0x05;

//status types
static char STATUS_OK  = 0x00;

//status bits
static uint8_t read_status =  0xA53;

static uint8_t write_status = 0x5A;

//other variables
char recCommandType = 0x00;
//data structure of received data for getbasicinfo
/*
startbit|basicinfochar| status | data | checksum | endbit
0xDD    | 0X03      | ALWAY 00 | DATA | IGNORE 4 NOW| 0X77

*/
//ENUMS DEFINITION
enum fsm {
  find_start_byte,
  find_command_byte,
  find_status_byte,
  find_payload_len,
  find_payload,
  find_checksum,
  find_end_byte,
  in_processing
};

fsm state = find_start_byte; //define an object for enum

enum errors
{
  OKay,//0
  STATUS_ERROR,//1
  PAYLOAD_ERROR,//2
  CRC_ERROR,//3
  STOP_BYTE_ERROR,//4
  NEW_DATA//5
};
errors status = OKay;

//DATA HOLDING DEFINITIONS
const int MAX_PACKET_SIZE = 100;
char rxBuff[MAX_PACKET_SIZE] = {0};
int payIndex     = 0;
int bytesToRec   = 0;
int bytesRead    = 0;

#define RX_GPIO_NUM 26
#define TX_GPIO_NUM 25
//Finite state machine to run through the code reading process.
void initCAN()
{
    CAN.setPins (RX_GPIO_NUM, TX_GPIO_NUM);
    if (!CAN.begin (500E3)) {
    Serial.println ("Starting CAN failed!");
    while (1);
      }
      else {
        Serial.println ("CAN Initialized");
      }
}
void initSerialinterface()
{
  Serial2.begin(9600, SERIAL_8N1, RX_GPIO_NUM, TX_GPIO_NUM);
}
uint8_t getSerialData(HardwareSerial &thisSerial = Serial2)
{
	if (thisSerial.available())
	{
		while (thisSerial.available() && (state!=in_processing))
		{
			uint8_t recChar = thisSerial.read();

			switch (state)
			{
			case find_start_byte://///////////////////////////////////////
          {
            if (recChar == startByte)
              state = find_command_byte;
            break;
          }

			case find_command_byte://////////////////////////////////////
          {
            recCommandType = recChar;
            state = find_payload_len;
            break;
          }
      case find_status_byte:
          {
            if(recChar == STATUS_OK)
            {
              	state = find_payload_len;
            }
            else{
                bytesRead = 0;
                state = find_start_byte;
                status = STATUS_ERROR;
                return 0;
            }
            
          }

			case find_payload_len:////////////////////////////////////////
			{
				if (recChar <= MAX_PACKET_SIZE)
          {
            bytesToRec = recChar;
            state = find_payload;
          }
				else
          {
            bytesRead = 0;
            state = find_start_byte;
            status = PAYLOAD_ERROR;//INCREASE PACKET SIZE
            return 0;
          }
				break;
			}

			case find_payload:////////////////////////////////////////////
			{
				if (payIndex < bytesToRec)
				{
					rxBuff[payIndex] = recChar;
					payIndex++;

					if (payIndex == bytesToRec)
					{
						payIndex = 0;
						state = find_checksum; // ! TODO ADD CRC-CHECK CAPBILITY
					}
				}
				break;
			}

			case find_checksum:///////////////////////////////////////////
			{
				/*uint8_t calcCrc = crc.calculate(rxBuff, bytesToRec);

				if (calcCrc == recChar)
					state = find_end_byte;
				else
				{
					bytesRead = 0;
					state = find_start_byte;
					status = CRC_ERROR;
					return 0;
				}
       */
       state = find_end_byte;
				break;
			}

			case find_end_byte:///////////////////////////////////////////
			{
				
				if (recChar == stopByte)
				{
					//unpackPacket(rxBuff, bytesToRec);
          //do all processing here for the fuction we want
          state = find_start_byte;
					bytesRead = bytesToRec;
					status = NEW_DATA;
          state = in_processing;
					return bytesToRec;
				}
        else
        {
				bytesRead = 0;
				status = STOP_BYTE_ERROR;
        }
				return 0;
				break;
			}

			default:
			{
				Serial.print("ERROR: Undefined state: ");
				Serial.println(state);

				bytesRead = 0;
				state = find_start_byte;
				break;
			}
			}
		}
	}


//further prosessing

}

void sendCommandToRS485(char read_write, char command)
{
  //DD A5 03 00 FF FD 77
  char array [] = {startByte,read_write,command, STATUS_OK, 0xFF,0xFD, stopByte };
  int arraySize = sizeof(array) / sizeof(char);
   Serial2.write(array, arraySize);
   return;
}

