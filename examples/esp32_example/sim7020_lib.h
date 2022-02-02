/*
  SIM7020 C++ PORTABLE LIBRARY
*/

#ifndef __SIM7020_CPP__
#define __SIM7020_CPP__

#include "arduino.h"

#define DEBUG_MODE
#define Serial_AT  Serial1
#define UART_BAUD  115200
#define PIN_DTR    25

void at_command(String command, uint32_t timeout);
std::string at_CommandWithReturn(String command, uint16_t timeout);

class SIM7020
{
  private:
  uint8_t pwr;
  
  std::string rf_band;
  std::string apn;
  std::string user;
  std::string psswd;
  std::string socket_host;
  std::string socket_port;
  std::string transport_layer_protocol;
  std::string app_layer_protocol;
  std::string app_layer_method;
  std::string app_payload;
  std::string http_page;
  
  typedef enum NbiotStateMachineEnum{
    IP_INITIAL,
    IP_START,
    IP_CONFIG,
    IP_GPRSACT,
    IP_STATUS,
    TCP_CONNECTING,
    CONNECT_OK,
    TCP_CLOSING,
    TCP_CLOSED,
    PDP_DEACT
  }eNbiotStateMachine;
  
  eNbiotStateMachine eNextState;
  public:
    SIM7020(uint8_t rx_pin, uint8_t tx_pin, uint8_t pwr_pin, std::string band){
	  Serial_AT.begin(UART_BAUD, SERIAL_8N1, rx_pin, tx_pin);
      delay(100);
	  
	  pwr = pwr_pin;
      pinMode(pwr_pin, OUTPUT);
      digitalWrite(pwr_pin, HIGH);
	  rf_band = band; 
	  
	  eNextState = PDP_DEACT;
	}
	
	void set_NetworkCredentials(std::string user_apn, std::string username, std::string user_psswd);
	void set_RFBand(std::string band);
  void set_Json(std::string app_payload);
	
	void HwInit(void);
	void NbiotManager(void);
	
    eNbiotStateMachine NetworkAttachHandler(void);
    eNbiotStateMachine StartTaskHandler(std::string apn, std::string user, std::string psswd);
    eNbiotStateMachine BringUpGprsHandler(void);
    eNbiotStateMachine WaitGprsHandler(void);
    eNbiotStateMachine GetLocalIpHandler(void);
    eNbiotStateMachine SocketConnectHandler(std::string app_protocol,std::string host, std::string port);
    eNbiotStateMachine WaitSocketHandler(void);
    eNbiotStateMachine DataSendHandler(std::string app_protocol,std::string app_method ,std::string host, std::string port);
    eNbiotStateMachine WaitSocketCloseHandler(void);
    eNbiotStateMachine MQTT_Connection(void);
};


#endif
