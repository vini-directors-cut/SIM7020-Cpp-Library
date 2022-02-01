/*
  SIM7020 C++ PORTABLE LIBRARY
*/

#include <string>
#include "arduino.h"
#include "sim7020_lib.h"


std::string command_response;


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


void SIM7020::HwInit(){
  std::string aux_string;

  #ifdef DEBUG_MODE
  at_command("AT", 500);
  at_command("ATE1", 500);
  #endif

  #ifndef DEBUG_MODE
  at_command("ATE0", 500);
  #endif
  
  command_response = at_CommandWithReturn("AT+CPIN?", 1000);
  while( (command_response.find("ERROR")) !=  std::string::npos){  
    digitalWrite(pwr, LOW);
    delay(10000);
    digitalWrite(pwr, HIGH);
    command_response = at_CommandWithReturn("AT+CPIN?", 1000);
  }

  command_response.clear();

  aux_string = "AT*MCGDEFCONT=\"IP\",\"" + apn + "\",\"" + user + "\",\"" + psswd + "\"";
  
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


void SIM7020::NbiotManager(){
  socket_host = "www.blinkenlichten.info";
  socket_port = "80";
  http_page = "/print/origin.html";
  app_layer_method = "GET";
  app_layer_protocol = "http";
  
  while(1){
    switch(eNextState){
      case PDP_DEACT:
        eNextState = SIM7020::NetworkAttachHandler();
        break;
      
      case IP_INITIAL:
        eNextState = SIM7020::StartTaskHandler(apn, user, psswd);
        break;

      case IP_START:
        eNextState = SIM7020::BringUpGprsHandler();
        break;
        
      case IP_CONFIG:
        eNextState = SIM7020::WaitGprsHandler();
        break;  
        
      case IP_GPRSACT:
        eNextState = SIM7020::GetLocalIpHandler();
        break;

      case IP_STATUS:
        eNextState = SIM7020::SocketConnectHandler(app_layer_protocol, socket_host, socket_port);
        break;        

      case TCP_CONNECTING:
		eNextState = SIM7020::WaitSocketHandler();
        break;

      case CONNECT_OK:
        eNextState = SIM7020::DataSendHandler(app_layer_method, http_page, socket_host);
        break;

      case TCP_CLOSING:
        eNextState = SIM7020::WaitSocketCloseHandler();  
        break;

      case TCP_CLOSED:
      Serial.println("Estado ainda n implementado - flag");
	    return;
        break;
    }
  }
}


SIM7020::eNbiotStateMachine SIM7020::NetworkAttachHandler(){
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


SIM7020::eNbiotStateMachine SIM7020::StartTaskHandler(std::string apn, std::string user, std::string psswd){
  std::string aux_string;
  aux_string = "AT+CSTT=\"" + apn + "\",\"" + user + "\",\"" + psswd + "\"";
  at_command(aux_string.c_str(), 2000);
  aux_string.clear();
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP START")) !=  std::string::npos)
    return IP_START;
  command_response.clear();   
}


SIM7020::eNbiotStateMachine SIM7020::BringUpGprsHandler(){
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


SIM7020::eNbiotStateMachine SIM7020::WaitGprsHandler(){
  delay(1000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP GPRSACT")) !=  std::string::npos){
    command_response.clear();
    return IP_GPRSACT;
  }
}


SIM7020::eNbiotStateMachine SIM7020::GetLocalIpHandler(){
  at_command("AT+CIFSR", 1000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("IP STATUS")) !=  std::string::npos)
    command_response.clear();
    return IP_STATUS;
}


SIM7020::eNbiotStateMachine SIM7020::SocketConnectHandler(std::string app_protocol, std::string host, std::string port){
  std::string aux_string;
    if(app_layer_protocol.find("http") != std::string::npos){
    aux_string = "AT+CIPSTART=\"TCP\",\"" + host + "\"," + port;
    at_command(aux_string.c_str(), 2000);
    aux_string.clear();
    command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
    if((command_response.find("CONNECT OK")) !=  std::string::npos)
      return CONNECT_OK;
    else if((command_response.find("TCP CONNECTING")) !=  std::string::npos){
      command_response.clear();
      return TCP_CONNECTING;
      }
  }

  else if(app_layer_protocol.find("mqtt") != std::string::npos){
    aux_string = "AT+CMQNEW=\"" + host + "\",\"" + port + "\",\"12000\",\"100\"";
    at_command(aux_string.c_str(), 12000);
    aux_string.clear();

    aux_string = "AT+CMQCON=\"0\",\"3\",\"myclient\",\"600\",\"0\", \"0\"";
    command_response = at_CommandWithReturn(aux_string.c_str(), 5000);
    if((command_response.find("OK")) !=  std::string::npos){
      command_response.clear();
      return CONNECT_OK;
    }
  }
}


SIM7020::eNbiotStateMachine SIM7020::WaitSocketHandler(){
  delay(5000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("CONNECT OK")) !=  std::string::npos){
    command_response.clear();
    return CONNECT_OK;
  }
}


SIM7020::eNbiotStateMachine SIM7020::DataSendHandler(std::string method, std::string page_file_topic, std::string host){
  std::string aux_string, cipsend_str;
  
  if(app_layer_protocol.find("http") != std::string::npos){
    aux_string = method + " " + page_file_topic + " HTTP/1.0\r\nHost: " + host + "\r\n\r\n";
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

  else if(app_layer_protocol.find("mqtt") != std::string::npos){
    aux_command_before = "AT+CMQSUB=\"0\",\"esp32/NbioT\",\"1\"";
    at_command(aux_command_before.c_str(), 5000);
    aux_command_pub = "AT+CMQPUB=\"0\",\"esp32/NbioT\",\"1\",\"0\",\"0\",\"8\",\"12345678\"";
    at_command(aux_command_pub.c_str(), 5000);
    }
  }
}


SIM7020::eNbiotStateMachine SIM7020::WaitSocketCloseHandler(){
  delay(5000);
  command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
  if((command_response.find("TCP CLOSED")) !=  std::string::npos){
    command_response.clear();
    return TCP_CLOSED;
    }
}


void SIM7020::set_NetworkCredentials(std::string user_apn, std::string username, std::string user_psswd){
  apn = user_apn;
  user = username;
  psswd = user_psswd;
}


void SIM7020::set_RFBand(std::string band){
	rf_band = band;
}