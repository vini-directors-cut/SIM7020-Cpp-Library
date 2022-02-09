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
  while( (command_response.find("ERROR"||"CME ERROR")) !=  std::string::npos){  
    digitalWrite(pwr, LOW);
    delay(5000);
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
  
  while(1){
    switch(eNextState){
      case PDP_DEACT:
        eNextState = SIM7020::NetworkAttachHandler();
        break;
      
      case IP_INITIAL:
        eNextState = SIM7020::StartTaskHandler();
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
        eNextState = SIM7020::SocketConnectHandler();
        break;        

      case TCP_CONNECTING:
		eNextState = SIM7020::WaitSocketHandler();
        break;

      case CONNECT_OK:
        eNextState = SIM7020::DataSendHandler();
        break;

      case TCP_CLOSING:
        eNextState = SIM7020::WaitSocketCloseHandler();  
        break;

      case TCP_CLOSED:
      Serial.println("Estado ainda n implementado - flag");
      //at_command("AT+CSCLK=1", 1000);
	    return;
        break;
    }
  }
}


void SIM7020::set_NetworkCredentials(std::string user_apn, std::string username, std::string user_psswd){
  apn = user_apn;
  user = username;
  psswd = user_psswd;
}


void SIM7020::set_Host(std::string app_protocol, std::string host, std::string port){
  app_layer_protocol = app_protocol;
  socket_host = host;
  socket_port = port;
}


void SIM7020::set_HttpRequestOptions(std::string app_method, std::string page){
  app_layer_method = app_method;
  http_page = page;
}


void SIM7020::set_RFBand(std::string band){
	rf_band = band;
}


void SIM7020::set_HttpVersion(std::string version){
  http_version = version;
}


void SIM7020::set_HttpHeader(std::string header){
  http_header = header;
}


void SIM7020::set_MqttSubscriptionOptions(std::string topic, std::string qos){
  mqtt_topic = topic;
  mqtt_qos = qos;
}


void SIM7020::set_Packet(std::string packet){
  data_packet = packet;
}

/*
void SIM7020::set_SleepPin(uint8_t dtr_pin){
  dtr = dtr_pin;
  pinMode(dtr_pin, OUTPUT);
  digitalWrite(dtr_pin, LOW);
  at_command("AT+CSCLK=1", 1000);
}


void SIM7020::Sleep(bool will_sleep){
  if(will_sleep)
    digitalWrite(dtr, HIGH);
  else
    digitalWrite(dtr, LOW);
  delay(1000);
}
*/

SIM7020::eNbiotStateMachine SIM7020::NetworkAttachHandler(){
  //at_command("AT+CSCLK=0", 1000);
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


SIM7020::eNbiotStateMachine SIM7020::StartTaskHandler(){
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


SIM7020::eNbiotStateMachine SIM7020::SocketConnectHandler(){
  std::string aux_string;
    if(app_layer_protocol.find("http") != std::string::npos){
    aux_string = "AT+CIPSTART=\"TCP\",\"" + socket_host + "\"," + socket_port;
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
    aux_string = "AT+CMQNEW=\"" + socket_host + "\",\"" + socket_port + "\",\"5000\",\"256\"";
    at_command(aux_string.c_str(), 5000);
    aux_string.clear();

    aux_string = "AT+CMQCON=\"0\"," + mqtt_version + ",\"myclient\",\"600\",\"0\", \"0\"";
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


SIM7020::eNbiotStateMachine SIM7020::DataSendHandler(){
  std::string aux_string, cipsend_str;

  if(app_layer_protocol == "http"){
    std::string a;
    String d = (String) data_packet.size();
    for(uint8_t char_byte = 0; char_byte < d.length(); char_byte++)
      a.push_back(d[char_byte]);
    http_header += "Content-Length: "; 
    http_header += a.c_str();
    http_header += "\r\n";
    
    Serial.print(http_header.c_str());
    
    aux_string = app_layer_method + " " + http_page + " " + http_version + "\r\n" + http_header + "\r\n" + data_packet;
    at_command("AT+CIPSEND", 15000);
    Serial_AT.write(aux_string.c_str());
    Serial_AT.write(26);
    data_packet = "";
    at_command("AT+CIPCLOSE", 1000);
    command_response = at_CommandWithReturn("AT+CIPSTATUS", 500);
    if((command_response.find("TCP CLOSED")) !=  std::string::npos)
      return TCP_CLOSED;
    else if((command_response.find("CONNECT OK")) !=  std::string::npos){
        at_command("AT+CIPCLOSE", 1000);
        return TCP_CLOSED;
    }
  }

  else if(app_layer_protocol == "mqtt"){
    aux_string = "AT+CMQSUB=\"0\",\""+ mqtt_topic + "\",\"" + mqtt_qos +  "\"";
    at_command(aux_string.c_str(), 5000);
    aux_string.clear();
    aux_string = "AT+CMQPUB=\"0\",\"esp32/NbioT\",\"1\",\"0\",\"0\",\"8\",\"12345678\"";
    at_command(aux_string.c_str(), 5000);
    return TCP_CLOSED;
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