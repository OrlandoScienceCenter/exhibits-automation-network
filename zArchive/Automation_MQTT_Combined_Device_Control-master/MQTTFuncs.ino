extern unsigned long OTAUntilMillis;
extern unsigned long now;
extern unsigned long computerPowerOffTimeout;

extern char msg[50];

extern const int numReadings;

extern int value;
extern int curQueryStat;
extern int curState;
extern int getStatusPowerUsage[numReadings];

extern unsigned int infraredTimeoutCtr;

extern bool OTARdyFlag;
extern bool startupFlag;
extern bool computerPowerOffCheckingFlag;
extern bool computerNeedsToTurnBackOnFlag;
extern bool OTAReportFlag;

void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += ((char)payload[i]);
  }

  Serial.println(payloadStr);

  // powerOff, powerOn, getStatus, startOTA, powerReset, resetESP

  // powerOff
  if (payloadStr.equalsIgnoreCase(F("powerOff"))) {
    digitalWrite(BUILTIN_LED, HIGH);   // Turn the LED off (HIGH is off)
    if (deviceIsRelay) {
      powerOffRelay();
    }
    
    if (deviceIsComputer) {
      powerOffComputer();
      
      computerPowerOffByTimeout = now + (120 * SECONDS);
      computerPowerOffCheckingFlag = 1;
      computerNeedsToTurnBackOnFlag = 0;
    }
    
    if(deviceIsInfrared){
      powerOffInfrared();

      infraredPowerOffByTimeout = now + (45 * SECONDS);
      infraredPowerOffCheckingFlag = 1;
    }

    // powerOn
  } else if (payloadStr.equalsIgnoreCase(F("powerOn"))) {
    digitalWrite(BUILTIN_LED, LOW);  // Turn the LED on
    if (deviceIsRelay) {
      powerOnRelay();
    }
    if (deviceIsComputer) {
      powerOnComputer();
    }
    if(deviceIsInfrared){
      powerOnInfrared();
    }

    // getStatus
  } else if (payloadStr.equalsIgnoreCase(F("getStatus"))) {
    if (deviceIsComputer) {
      if (analogRead(A0) > 900) { 
        curState = 1;
      } else {
        curState = 0;
      }
    }
    if (deviceIsRelay) {
      
    }
    
    if(deviceIsInfrared){
   
    }

    // These get sent no matter what device type is
    snprintf (msg, 20, "PowerState: %i", curState);
    client.publish(TOPIC_T, msg);
    
    snprintf (msg, 30, "Up for: %i seconds", now / 1000);
    client.publish(TOPIC_T, msg);
    IPAddress myIP = WiFi.localIP();
    snprintf (msg, 30, "My IP is: %i.%i.%i.%i", myIP[0], myIP[1], myIP[2], myIP[3]);
    client.publish(TOPIC_T, msg);

    // powerDump
  } else if (payloadStr.equalsIgnoreCase(F("PowerDump"))) {
    if(deviceIsInfrared){
        for (int i = 0; i < numReadings; i++) {
          snprintf (msg, 40, "Power usage %d: %d", i, getStatusPowerUsage[i]);
          client.publish(TOPIC_T, msg);
          
        }

          snprintf (msg, 40, "infraredTimeoutCtr: %d", infraredTimeoutCtr);
          client.publish(TOPIC_T, msg);      
      }
    
    // startOTA
    } else if (payloadStr.equalsIgnoreCase(F("startOTA")) || payloadStr.equalsIgnoreCase(F("OTAstart"))) {
    client.publish(TOPIC_T, "OTA for 10m");
    OTAUntilMillis = now + 600 * SECONDS; // 10 minutes
    
    IPAddress myIP = WiFi.localIP();
    snprintf (msg, 50, "OTA will be available at IP: %i.%i.%i.%i", myIP[0], myIP[1], myIP[2], myIP[3]);
    client.publish(TOPIC_T, msg);

    // powerReset
  } else if (payloadStr.equalsIgnoreCase(F("powerReset"))) {
    client.publish(TOPIC_T, "Power-cycling.");
    delay(500);

    if (deviceIsRelay) {
      powerOffRelay();
      // Then these two lines make it do the auto startup after delay in the main loop.
      delayTime = STARTUP_DELAY_SECONDS * SECONDS + now;
      startupFlag = 0;
    }

    if (deviceIsComputer) {
      powerOffComputer();

      computerPowerOffByTimeout = now + (120 * SECONDS);
      computerPowerOffCheckingFlag = 1;
      computerNeedsToTurnBackOnFlag = 1;
    }

    // resetESP
  } else if (payloadStr.equalsIgnoreCase(F("resetesp")) || payloadStr.equalsIgnoreCase(F("espreset"))) {
    client.publish(TOPIC_T, "ESP8266 resetting");
    delay(500);
    ESP.restart();
  } else if (payloadStr.equalsIgnoreCase(F("custom_action1"))) {
    if (deviceIsRelay) {
       Serial.println(F("custom_action1CommandRecieved"));
    }
    if (deviceIsComputer) {

    }
    if(deviceIsInfrared){

    }
  } else if (payloadStr.equalsIgnoreCase(F("custom_action2"))) {
    if (deviceIsRelay) {
       Serial.println(F("custom_action2CommandRecieved"));
    }
    if (deviceIsComputer) {

    }
    if(deviceIsInfrared){

    }
  } else if (payloadStr.equalsIgnoreCase(F("custom_action3"))) {
    if (deviceIsRelay) {
       Serial.println(F("custom_action3CommandRecieved"));
    }
    if (deviceIsComputer) {

    }
    if(deviceIsInfrared){

    }
  } else if (payloadStr.equalsIgnoreCase(F("custom_action4"))) {
    if (deviceIsRelay) {
       Serial.println(F("custom_action4CommandRecieved"));
    }
    if (deviceIsComputer) {
      
    }
    if(deviceIsInfrared){
      
    }
  } else if (payloadStr.equalsIgnoreCase(F("custom_action5"))) {
    if (deviceIsRelay) {
       Serial.println(F("custom_action5CommandRecieved"));
    }
    if (deviceIsComputer) {
      
    }
    if(deviceIsInfrared){
      
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Create a random client ID
    String clientId = F("ESPCli-");
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(F("connected"));
      // And resubscribe
      client.subscribe(TOPIC_T);

      // Check the OTA report flag
      if (OTAReportFlag == 1){
        OTAReportFlag = 0;
        
        yield();
        ESP.wdtFeed();  
              
        client.publish(TOPIC_T, "OTA upload done, restarting ESP now.");
        client.loop();  
        
        Serial.println(F("OTA Upload done"));
        delay(1000); // Give time to read the serial message.
        ESP.restart();
      }
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
