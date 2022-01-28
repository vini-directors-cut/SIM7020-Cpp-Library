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
#define PIN_RX     26
#define PIN_TX     27
#define PWR_PIN    12

class SIM7020
{
  private:
  
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
  
  public:
    
	eNbiotStateMachine NetworkAttachHandler(void);
    eNbiotStateMachine StartTaskHandler(struct networkCredentials *p);
    eNbiotStateMachine BringUpGprsHandler(void);
    eNbiotStateMachine WaitGprsHandler(void);
    eNbiotStateMachine GetLocalIpHandler(void);
    eNbiotStateMachine SocketConnectHandler(struct networkCredentials *p);
    eNbiotStateMachine WaitSocketHandler(void);
    eNbiotStateMachine DataSendHandler(struct networkCredentials *p);
    eNbiotStateMachine WaitSocketCloseHandler(void);
};

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

struct networkCredentials{
  const std::string apn = "virtueyes.com.br";
  const std::string user = "virtu";
  const std::string psswd = "virtu";
  std::string socket_host;
  std::string socket_port;
  std::string transport_layer_protocol;
  std::string http_page;
  std::string app_layer_method;
};

extern struct networkCredentials credential;
extern struct networkCredentials *p_cred;

void at_command(String command, uint32_t timeout);
std::string at_CommandWithReturn(String command, uint16_t timeout);
void sim7020_HwInit(std::string rf_band);
void sim7020_NbiotManager(void);

eNbiotStateMachine NetworkAttachHandler(void);
eNbiotStateMachine StartTaskHandler(struct networkCredentials *p);
eNbiotStateMachine BringUpGprsHandler(void);
eNbiotStateMachine WaitGprsHandler(void);
eNbiotStateMachine GetLocalIpHandler(void);
eNbiotStateMachine SocketConnectHandler(struct networkCredentials *p);
eNbiotStateMachine WaitSocketHandler(void);
eNbiotStateMachine DataSendHandler(struct networkCredentials *p);
eNbiotStateMachine WaitSocketCloseHandler(void);

#endif