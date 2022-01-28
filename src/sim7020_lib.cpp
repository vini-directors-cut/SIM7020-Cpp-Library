/*
  SIM7020 C++ PORTABLE LIBRARY
*/

#include <string>
#include "arduino.h"
#include "sim7020_lib.h"


std::string command_response;
struct networkCredentials credential;
struct networkCredentials *p_cred;


std::string at_CommandWithReturn(String command, uint16_t timeout){
  std::string res;
  Serial_AT.println(command);
  while(Serial_AT.available() == 0);
  
  unsigned long lastRead = millis();
  while(millis() - lastRead < timeout){   
    while(Serial_AT.available()){
      res = Serial_AT.readString().c_str();
      #ifdef DEBUG_MODE
      printf(res.c_str());
      #endif
      lastRead = millis();
    }
  }
  return res;
}


void at_command(String command, uint32_t timeout) {
  Serial_AT.println(command);
  while (Serial_AT.available() == 0);

  unsigned long lastRead = millis();
  while(millis() - lastRead < timeout) {
    while(Serial_AT.available()) {
      Serial.println(Serial_AT.readString());
      lastRead = millis();
    }
  }
}


void sim7020_HwInit(std::string rf_band){
  std::string aux_string;
  
  Serial_AT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(100);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);

  p_cred = &credential;

  #ifdef DEBUG_MODE
  at_command("AT", 500);
  at_command("ATE1", 500);
  #endif

  #ifndef DEBUG_MODE
  at_command("ATE0", 500);
  #endif
  
  command_response = at_CommandWithReturn("AT+CPIN?", 1000);
  while( (command_response.find("ERROR")) !=  std::string::npos){  
    digitalWrite(PWR_PIN, LOW);
    delay(10000);
    digitalWrite(PWR_PIN, HIGH);
    command_response = at_CommandWithReturn("AT+CPIN?", 1000);
  }

  command_response.clear();

  aux_string = "AT*MCGDEFCONT=\"IP\",\"" + p_cred->apn + "\",\"" + p_cred->user + "\",\"" + p_cred->psswd + "\"";
  
  at_command("AT+CFUN=0", 1000); //turn-off rf
  at_command("AT+CREG=2", 500);
  at_command(aux_string.c_str(), 2000);
  at_command("AT+CFUN=1", 1000); //turn-on rf

  aux_string.clear();
  
  aux_string = "AT+CBAND=" + rf_band;
  at_command(aux_string.c_str(), 1000);
  
  at_command("AT+COPS=0,0", 1000);
  at_command("AT+CGCONTRDP", 1000);

  #ifdef DEBUG_MODE
  at_command("AT+CMEE=2",500);
  Serial.print("SIM7020 firmware version: ");
  at_command("AT+CGMR", 500);
  Serial.print("APN settings: ");
  at_command("AT*MCGDEFCONT?", 500);
  Serial.print("Banda: ");
  at_command("AT+CBAND?", 500);
  Serial.print("Internet register status: ");
  at_command("AT+CGREG?", 500);
  Serial.print("Network Information: ");
  at_command("AT+COPS?", 500);
  Serial.print("Signal quality: ");
  at_command("AT+CSQ", 500);
  Serial.print("GPRS service attachment: ");
  at_command("AT+CGATT?", 500);
  Serial.print("PDP context definition: ");
  at_command("AT+CGDCONT?", 500);
  Serial.println("\n\nDiagnostic completed!");
  #endif
}


void sim7020_NbiotManager(){
  eNbiotStateMachine eNextState = PDP_DEACT;

  credential.socket_host = "www.blinkenlichten.info";
  credential.socket_port = "80";
  credential.http_page = "/print/origin.html";
  credential.app_layer_method = "GET";
  
  
  while(1){
    switch(eNextState){
      case PDP_DEACT:
        eNextState = NetworkAttachHandler();
        break;
      
      case IP_INITIAL:
        eNextState = StartTaskHandler(p_cred);
        break;

      case IP_START:
        eNextState = BringUpGprsHandler();
        break;
        
      case IP_CONFIG:
        eNextState = WaitGprsHandler();
        break;  
        
      case IP_GPRSACT:
        eNextState = GetLocalIpHandler();
        break;

      case IP_STATUS:
        eNextState = SocketConnectHandler(p_cred);
        break;        

      case TCP_CONNECTING:
		eNextState = WaitSocketHandler();
        break;

      case CONNECT_OK:
        eNextState = DataSendHandler(p_cred);
        break;

      case TCP_CLOSING:
        eNextState = WaitSocketCloseHandler();  
        break;

      case TCP_CLOSED:
      Serial.println("Estado ainda n implementado - flag");
        break;
    }
  }
}


eNbiotStateMachine NetworkAttachHandler(){
  at_command("AT+CIPSHUT", 10000);
  at_command("AT+CIPMUX=0", 1000);

  command_response = at_CommandWithReturn("AT+CGATT=1", 20000);
  while( (command_response.find("ERROR")) !=  std::string::npos){  //procurar pela substring ERROR na resp de AT+CGATT
    at_command("AT+CGATT=0", 5000);
    command_response = at_CommandWithReturn("AT+CGATT=1", 20000);
    }

  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);

  if((command_response.find("IP INITIAL")) !=  std::string::npos){
    command_response.clear();
    return IP_INITIAL;
  }
}


eNbiotStateMachine StartTaskHandler(struct networkCredentials *p){
  std::string aux_string;
  p = &credential;
  aux_string = "AT+CSTT=\"" + p->apn + "\",\"" + p->user + "\",\"" + p->psswd + "\"";
  at_command(aux_string.c_str(), 2000);
  aux_string.clear();
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP START")) !=  std::string::npos)
    return IP_START;
  command_response.clear();   
}


eNbiotStateMachine BringUpGprsHandler(){
  at_command("AT+CIICR", 1000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP CONFIG")) !=  std::string::npos)
    return IP_CONFIG;
  else if((command_response.find("IP GPRSACT")) !=  std::string::npos)
    return IP_GPRSACT;
  else if((command_response.find("PDP DEACT")) !=  std::string::npos)
    return PDP_DEACT;
  command_response.clear();
}


eNbiotStateMachine WaitGprsHandler(){
  delay(1000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP GPRSACT")) !=  std::string::npos)
    return IP_GPRSACT;
  command_response.clear();
}


eNbiotStateMachine GetLocalIpHandler(){
  at_command("AT+CIFSR", 1000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP STATUS")) !=  std::string::npos)
    return IP_STATUS;
  command_response.clear();  
}


eNbiotStateMachine SocketConnectHandler(struct networkCredentials *p){
  std::string aux_string;
  p = &credential;
  aux_string = "AT+CIPSTART=\"TCP\",\"" +  p->socket_host + "\"," + p->socket_port;
  at_command(aux_string.c_str(), 2000);
  aux_string.clear();
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("CONNECT OK")) !=  std::string::npos)
    return CONNECT_OK;
  else if((command_response.find("TCP CONNECTING")) !=  std::string::npos)
    return TCP_CONNECTING;
  command_response.clear();
}


eNbiotStateMachine WaitSocketHandler(){
  delay(5000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("CONNECT OK")) !=  std::string::npos)
    return CONNECT_OK;
  command_response.clear();
}


eNbiotStateMachine DataSendHandler(struct networkCredentials *p){
  std::string aux_string, cipsend_str;
  p_cred = &credential;
  aux_string = p->app_layer_method + " " + p->http_page + " HTTP/1.0\r\nHost: " + p->socket_host + "\r\n\r\n";
  at_command("AT+CIPSEND", 10000);
  Serial_AT.write(aux_string.c_str());
  Serial_AT.write(26);

  at_command("AT+CIPCLOSE", 1000);
  
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("TCP CLOSED")) !=  std::string::npos)
    return TCP_CLOSED;
  else if((command_response.find("CONNECT OK")) !=  std::string::npos){
     at_command("AT+CIPCLOSE", 1000);
     return TCP_CLOSED;
  }
}


eNbiotStateMachine WaitSocketCloseHandler(){
  delay(5000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("TCP CLOSED")) !=  std::string::npos)
    return TCP_CLOSED;
  command_response.clear();
}