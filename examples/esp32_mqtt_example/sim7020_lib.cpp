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
        //eNextState = SIM7020::SocketConnectHandler();
        eNextState = SIM7020::SSL_ConnectHandler();
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

  else if(app_layer_protocol.find("mqtt") != std::string::npos)
  {
    aux_string = "AT+CMQNEW=\"" + socket_host + "\",\"" + socket_port + "\",\"5000\",\"256\"";
    at_command(aux_string.c_str(), 5000);
    aux_string.clear();

    //aux_string = "AT+CMQCON=\"0\"," + mqtt_version + ",\"myclient\",\"600\",\"1\", \"0\"";
    aux_string = "AT+CMQCON=\"0\",\"3\",\"deployment-NB\",\"60000\",\"1\",\"0\",\"gui\",\"123\"";
    command_response = at_CommandWithReturn(aux_string.c_str(), 5000);
    aux_string.clear();
    
    if((command_response.find("OK")) !=  std::string::npos)
    {
      command_response.clear();
      return CONNECT_OK;
    }
  }
}

SIM7020::eNbiotStateMachine SIM7020::SSL_ConnectHandler()
{
  std::string aux_string, root_ca, root_ca_b, root_total, serve_ca, private_key, broker, port, jwt, jwt_simple;

  broker = "mqtt.2030.ltsapis.goog";
  port = "8883";
  jwt = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NDQ0OTgyNjYsImV4cCI6MTY0NDQ5OTQ2NiwiYXVkIjoiaW90LXRlc3QtMzI4MTIwIn0.OINDi3cf8MBKX12SZzz_w9_9ssyKiaFlIzKhdYzvSFlKPhM12pIYGZ2dbqeHKykq_52gzqRLp0boHHXqDmsXdw";
  //jwt_simple = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9";
  jwt_simple = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJpb3QtdGVzdC0zMjgxMjAifQ.BqK7ryPBRCf7fAhxA3Gl77_SwaZL3IUBMOS80uyohIQ";
  //
  at_command("AT+CSETCA=?", 500);

  //serve_ca = "-----BEGIN CERTIFICATE-----MIIB4TCCAYegAwIBAgIRKjikHJYKBN5CsiilC+g0mAIwCgYIKoZIzj0EAwIwUDEkMCIGA1UECxMbR2xvYmFsU2lnbiBFQ0MgUm9vdCBDQSAtIFI0MRMwEQYDVQQKEwpHbG9iYWxTaWduMRMwEQYDVQQDEwpHbG9iYWxTaWduMB4XDTEyMTExMzAwMDAwMFoXDTM4MDExOTAzMTQwN1owUDEkMCIGA1UECxMbR2xvYmFsU2lnbiBFQ0MgUm9vdCBDQSAtIFI0MRMwEQYDVQQKEwpHbG9iYWxTaWduMRMwEQYDVQQDEwpHbG9iYWxTaWduMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEuMZ5049sJQ6fLjkZHAOkrprlOQcJFspjsbmG+IpXwVfOQvpzofdlQv8ewQCybnMO/8ch5RikqtlxP6jUuc6MHaNCMEAwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFFSwe61FuOJAf/sKbvu+M8k8o4TVMAoGCCqGSM49BAMCA0gAMEUCIQDckqGgE6bPA7DmxCGXkPoUVy0D7O48027KqGx2vKLeuwIgJ6iFJzWbVsaj8kfSt24bAgAXqmemFZHe+pTsewv4n4Q=-----END CERTIFICATE-----";

  aux_string = "AT+CSETCA=\"0\",\"0\",\"0\",\"0\",\"0\"";
  at_command(aux_string.c_str(), 500);
  aux_string.clear();

  aux_string = "AT+CSETCA=\"2\",\"0\",\"0\",\"0\",\"0\"";
  at_command(aux_string.c_str(), 500);
  aux_string.clear();

  at_command("AT+CSETCA=?", 500);

  root_ca = "-----BEGIN CERTIFICATE-----MIIC0TCCAnagAwIBAgINAfQKmcm3qFVwT0+3nTAKBggqhkjOPQQDAjBEMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzERMA8GA1UEAxMIR1RTIExUU1IwHhcNMTkwMTIzMDAwMDQyWhcNMjkwNDAxMDAwMDQyWjBEMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzERMA8GA1UEAxMIR1RTIExUU1gwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARr6/PTsGoOg9fXhJkj3CAk6C6DxHPnZ1I+ER40vEe290xgTp0gVplokojbN3pFx07fzYGYAX5EK7gDQYuhpQGIo4IBSzCCAUcwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSzK6ugSBx+E4rJCMRAQiKiNlHiCjAfBgNVHSMEGDAWgBQ+/v/MUuu/ND4980DQ5CWxX7i7UjBpBggrBgEFBQcBAQRdMFswKAYIKwYBBQUHMAGGHGh0dHA6Ly9vY3NwLnBraS5nb29nL2d0c2x0c3IwLwYIKwYBBQUHMAKGI2h0dHA6Ly9wa2kuZ29vZy9ndHNsdHNyL2d0c2x0c3IuY3J0MDgGA1UdHwQxMC8wLaAroCmGJ2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzbHRzc";
  
  root_ca_b = "i9ndHNsdHNyLmNybDAdBgNVHSAEFjAUMAgGBmeBDAECATAIBgZngQwBAgIwCgYIKoZIzj0EAwIDSQAwRgIhAPWeg2v4yeimG+lzmZACDJOlalpsiwJR0VOeapY8/7aQAiEAiwRsSQXUmfVUW+N643GgvuMH70o2Agz8w67fSX+k+Lc=-----END CERTIFICATE-----";

  private_key = "-----BEGIN EC PRIVATE KEY-----MHcCAQEEIEf7pG64E9BqMylniTrkm84zmCfO885apmpLHb5dybHmoAoGCCqGSM49AwEHoUQDQgAEQOVGC+fbMK45MCJscsIknKvV21bzNRCvP12Sf204pxhuf+iqa8zMuPFgPCb7d+bNd82uUlDMnvZYlKkrItXSiw==-----END EC PRIVATE KEY-----";

  aux_string = "AT+CSETCA=\"0\",\"1024\",\"0\",\"0\",\"" + root_ca + "\"";
  at_command(aux_string.c_str(), 500);
  aux_string.clear();

  aux_string = "AT+CSETCA=\"0\",\"1024\",\"1\",\"0\",\"" + root_ca_b + "\"";
  at_command(aux_string.c_str(), 500);
  aux_string.clear();

  aux_string = "AT+CSETCA=\"2\",\"224\",\"1\",\"0\",\"" + private_key + "\"";
  at_command(aux_string.c_str(), 500);
  aux_string.clear();

  at_command("AT+CMQTTSNEW=?", 500);
  at_command("AT+CMQTTSNEW?", 500);

  aux_string = "AT+CMQNEW=\"" + broker + "\",\"" + port + "\",\"12000\",\"1024\"";
  at_command(aux_string.c_str(), 2000);
  aux_string.clear();

  at_command("AT+CMQTTSNEW?", 500);
  
  //aux_string = "AT+CMQCON=\"0\"," + mqtt_version + ",\"myclient\",\"600\",\"1\", \"0\"";
  //aux_string = "AT+CMQCON=\"0\",\"3\",\"projects/iot-test-328120/locations/us-central1/registries/test-registry/devices/nb_iot\",\"60000\",\"1\",\"0\",\"unsed\",\"" + jwt + "\"";
  aux_string = "AT+CMQCON=\"0\",\"3\",\"projects/iot-test-328120/locations/us-central1/registries/test-registry/devices/nb_iot\",\"60000\",\"1\",\"0\",\"unsed\",\"" + jwt + "\"";
  command_response = at_CommandWithReturn(aux_string.c_str(), 5000);
  aux_string.clear();

  aux_string = "AT+CMQCON=\"0\",\"3\",\"projects/iot-test-328120/locations/us-central1/registries/test-registry/devices/nb_iot\",\"60000\",\"1\",\"0\",\"unsed\",\"" + jwt_simple + "\"";
  command_response = at_CommandWithReturn(aux_string.c_str(), 5000);
  aux_string.clear();

  aux_string = "AT+CMQDISCON=\"0\"";
  at_command(aux_string.c_str(), 5000);
  aux_string.clear();
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

  if(app_layer_protocol == "http")
  {
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

  else if(app_layer_protocol == "mqtt")
  {
    // aux_string = "AT+CMQSUB=\"0\",\""+ mqtt_topic + "\",\"" + mqtt_qos +  "\"";
    // at_command(aux_string.c_str(), 5000);
    // aux_string.clear();
    // aux_string = "AT+CMQPUB=\"0\",\"esp32/NbioT\",\"1\",\"0\",\"0\",\"8\",\"12345678\"";
    // at_command(aux_string.c_str(), 5000);

    aux_string = "AT+CMQSUB=\"0\",\"esp32/NbioT\",\"0\"";
    at_command(aux_string.c_str(), 5000);
    aux_string.clear();

    while(true)
    {
      aux_string = "AT+CMQPUB=\"0\",\"esp32/NbioT\",\"1\",\"0\",\"0\",\"10\",\"1644494166\"";
      at_command(aux_string.c_str(), 9000);
      aux_string.clear();
    }
    

    aux_string = "AT+CMQDISCON=\"0\"";
    at_command(aux_string.c_str(), 5000);
    aux_string.clear();
    
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