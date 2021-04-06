
#define ARDUINOJSON_DECODE_UNICODE 1
#define ARDUINOJSON_ENABLE_PROGMEM 0
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_INFINITY 1
#define ARDUINOJSON_ENABLE_NAN 1
#include <stdint.h>
#include <inttypes.h>
#include "WiFi.h"
#include "mbedtls/md.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> //Async Webserver
#include <ESPmDNS.h>
#include "EEPROM.h"
#include <Button2.h>
#include <cstdlib>
#include <codecvt>
#include <string>
#include <cassert>
#include <locale>
#include <iterator>
#define ARRAYSIZE 10
const int BUTTON_down_PIN = 15; // 15 down time
const int BUTTON_up_PIN = 4;    // 4 up time

const int LEDPIN = 2;       // 2 led buildin
const int PushButton = 13;  // 18,13,12 quickstart test button
const int pausebutton = 12; // 18,13,12 pause button
const int highvoltage = 18; // 19 SSR relay
unsigned long milibuttonchange = 0;
// unsigned long milibuttondown = 0;
unsigned long up_time = 0;
unsigned long down_time = 0;

char SSIDchar[50];
char PWconnect[40];

byte scanstate = false;
String style1 PROGMEM = R"=====(<style>#file-input,input{width: 100%;height: 44px;border-radius: 4px;margin: 10px auto;font-size: 15px}input{background: #b0a9a9;border: 0;padding: 0 15px}body{background: #DFBE95;font-family: sans-serif;font-size: 14px; color: #777; text-align: center;}#file-input{padding: 0;border: 1px solid #ddd;line-height: 44px;text-align: left;display: block;cursor: pointer;}#bar,#prgbar{background-color: #f1f1f1;border-radius: 10px}#bar{background-color: #FF9B34;width: 0%;height: 10px}form{background: #fff;max-width: 400px;margin: 100px auto;padding: 10px;border-radius: 5px;text-align: center;}div{background: #fff;max-width: 400px;margin: 3px auto;border-radius: 10px; text-align: center; display: inline-block;}.divonoff{width: 400;}.w3-btn,.w3-button{border: none;display: inline-block;padding: 8px 16px;vertical-align: middle;overflow: hidden;text-decoration: none;color: inherit;background-color: inherit;text-align: center;cursor: pointer;white-space: nowrap;}.w3-btn,.w3-button{-webkit-touch-callout: none;-webkit-user-select: none;-khtml-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;}.w3-disabled,.w3-btn:disabled,.w3-button:disabled{cursor: not-allowed;opacity: 0.3;}.w3-disabled *,:disabled *{pointer-events: none;}.w3-dropdown-hover:hover>.w3-button:first-child,.w3-dropdown-click:hover>.w3-button:first-child{background-color: #ccc;color: #000;}.w3-bar-block .w3-dropdown-hover .w3-button,.w3-bar-block .w3-dropdown-click .w3-button{width: 100%;text-align: left;padding: 8px 16px;}.w3-bar .w3-button{white-space: normal;}.w3-dropdown-hover.w3-mobile,.w3-dropdown-hover.w3-mobile .w3-btn,.w3-dropdown-hover.w3-mobile .w3-button,.w3-dropdown-click.w3-mobile,.w3-dropdown-click.w3-mobile .w3-btn,.w3-dropdown-click.w3-mobile .w3-button{width: 100%;}.w3-button:hover{color: #000!important;background-color: #ccc!important;}.w3-deep-orange,.w3-hover-deep-orange:hover{color: #fff!important;background-color: #ff5722!important;}.btn{background: #3498db;color: #fff; cursor: pointer;}</style>)=====",
              htmljava PROGMEM = R"=====(<h1>Please waiting...ScanNetworks !!</h1><script>location.replace("/joinwifi")</script>)=====",
              script PROGMEM = R"=====(<script>setInterval(function(){var xhr=new XMLHttpRequest();xhr.withCredentials=!0;xhr.addEventListener('readystatechange',function(){if(this.readyState===4){document.getElementById("showtime").innerHTML = this.responseText}});xhr.open('GET','/apitimmer');xhr.send()},1000)</script>)=====";

////////////////////////////////////////////////// EEPROM
String Essid = ""; //EEPROM Network SSID
String Epass = ""; //EEPROM Network Password
String sssid = ""; //Read SSID From Web Page
String passs = ""; //Read Password From Web Page
String ssidconnect,
    inPass = "",
    findwifi[ARRAYSIZE],
    teststr[ARRAYSIZE];
//////////////////////////////////////////////////esp32 APmode
const char *ssidAP = "Randy timmer";
const char *passwordAP = "123456789";
IPAddress ip;

int timmer = 0;
int previousMillis = 0;
bool prestate = true;
bool prestate1 = true;

char disp_c[8];
const int SCLK_pin = 21;
const int RCLK_pin = 23;
const int DIO_pin = 22;
int disp[8];
bool state_start = false;
//time values for delay workaround
unsigned long prev = 0;
unsigned long waitMS = 0;
const char *host = "esprandy";
AsyncWebServer server(80);
TaskHandle_t controlhandle, timmerhandle, starthandle, asynchandle;

void handleRoot(AsyncWebServerRequest *request)
{
  String temp PROGMEM =
      "<body>"
      "<a href='/scanfon' class='w3-button w3-deep-orange'>SCAN WIFI</a>"
      "<p></p>"
      "<a href='/control' class='w3-button w3-deep-orange'>Control Timmer</a>"
      "<p></p>"
      "<a href='/ip' class='w3-button w3-deep-orange'>Check IP</a>"
      "</body>";
  request->send(200, "text/html", temp + style1);
}

void setup()
{
  Serial.begin(112500);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTON_up_PIN, INPUT);
  pinMode(BUTTON_up_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_down_PIN, INPUT);
  pinMode(BUTTON_down_PIN, INPUT_PULLDOWN);
  pinMode(PushButton, INPUT);
  pinMode(PushButton, INPUT_PULLDOWN);
  pinMode(pausebutton, INPUT);
  pinMode(pausebutton, INPUT_PULLDOWN);
  pinMode(highvoltage, OUTPUT);
  digitalWrite(highvoltage, LOW);
  pinMode(RCLK_pin, OUTPUT);
  pinMode(DIO_pin, OUTPUT);
  pinMode(SCLK_pin, OUTPUT);
  xTaskCreatePinnedToCore(
      asyncPV,      /* Task function. */
      "asyncN",     /* name of task. */
      (1024 * 4),   /* Stack size of task */
      NULL,         /* parameter of the task */
      8,            /* priority of the task */
      &asynchandle, /* Task handle to keep track of created task */
      0);           /* pin task to core 0 */
  // Add the task to the your scheduler

  xTaskCreatePinnedToCore(
      StartPV,      /* Task function. */
      "StartFunc",  /* name of task. */
      (1024 * 2),   /* Stack size of task */
      NULL,         /* parameter of the task */
      9,            /* priority of the task */
      &starthandle, /* Task handle to keep track of created task */
      1);           /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      timmerPV,      /* Task function. */
      "TimmerFunc",  /* name of task. */
      (1024 * 2),    /* Stack size of task */
      NULL,          /* parameter of the task */
      8,             /* priority of the task */
      &timmerhandle, /* Task handle to keep track of created task */
      0);            /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      ControlPV,      /* Task function. */
      "ControFunc",   /* name of task. */
      (1024 * 2),     /* Stack size of task */
      NULL,           /* parameter of the task */
      8,              /* priority of the task */
      &controlhandle, /* Task handle to keep track of created task */
      1);             /* pin task to core 0 */

  ////...Serial.println(F("============end APMODE==============="));
  /////////////////////////////////////////// Http /////////////////////////////////////
}

void asyncPV(void *pvParameters)
{
  EEPROM.begin(512);
  WiFi.mode(WIFI_AP_STA);
  EEPROMwifi();

  if (Essid.length() > 1)
  {
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      WiFi.disconnect(false);
      vTaskDelay(1000);
      WiFi.begin(Essid.c_str(), Epass.c_str());
    }
    //c_str()                       //Wait for IP to be assigned to Module by Router
    ip = WiFi.localIP(); //Get ESP8266 IP Adress
    Serial.print(F("EEPROMIP="));
    Serial.println(ip);
  }

  WiFi.softAP(ssidAP, passwordAP, 6); //////////////////////////////////// APwifi
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(F("AP IP address: "));
  Serial.println(myIP);

  if (!MDNS.begin(host))
  { //http://esp32.local
    ////...Serial.println(F("Error setting up MDNS responder!"));
    while (1)
    {
      vTaskDelay(1000);
    }
  }
  MDNS.addService("http", "tcp", 80);
  server.on("/", handleRoot);
  server.on("/a", vwifiscana);
  server.on("/joinwifi", vjoinwifi);
  server.on("/control", vControl);
  server.on("/apitimmer", vapitimmer);
  server.on("/apiquickstart", vquickstart);
  server.on("/apipause", vpause);
  server.on("/set", vset);
  server.on("/ip", vcheckip);
  server.on("/scanfon", HTTP_GET, [](AsyncWebServerRequest *request) {
    void clear();
    request->send(200, "text/html", style1 + htmljava);
  });

  server.begin();
  vTaskDelete(NULL);
}

void StartPV(void *pvParameters)
{
  while (1)
  {

    if (timmer > 0 && state_start == true)
    {
      digitalWrite(highvoltage, HIGH);
      digitalWrite(LEDPIN, HIGH);
      timmer = timmer - 1;
    }
    else
    {
      digitalWrite(highvoltage, LOW);
      digitalWrite(LEDPIN, LOW);
    }

    if (timmer <= 0)
    {
      timmer = 0;
      state_start = false;
      
    }
    vTaskDelay(1000);
  }
}

void click_quickstart()
{
  timmer = timmer + 30;
  state_start = true;
}

void click_pause()
{
  if (state_start)
  {
    state_start = false;
  }
  else
  {
    state_start = true;
  }
}

void timmerPV(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(60);
    int up_button_state = digitalRead(BUTTON_up_PIN);
    int down_button_state = digitalRead(BUTTON_down_PIN);
    if (up_button_state == HIGH)
    {
      timmer = timmer + 1;
      //show timer to lcd
      showText(((timmer / 60) / 10) % 60,
               (timmer / 60) % 10 + 10,
               (timmer % 60) / 10,
               (timmer % 60) % 10);

      up_time = millis() - milibuttonchange;
      if (up_time > 1000)
      {
        timmer = timmer + 12;
        //show timer to lcd
        showText(((timmer / 60) / 10) % 60,
                 (timmer / 60) % 10 + 10,
                 (timmer % 60) / 10,
                 (timmer % 60) % 10);
      }
    }
    else if (down_button_state == HIGH)
    {
      if (timmer > 0)
      {
        timmer = timmer - 1;
        //show timer to lcd
        showText(((timmer / 60) / 10) % 60,
                 (timmer / 60) % 10 + 10,
                 (timmer % 60) / 10,
                 (timmer % 60) % 10);

        down_time = millis() - milibuttonchange;
        if (down_time > 1000)
        {
          if (timmer >= 12)
          {
            timmer = timmer - 12;
            //show timer to lcd
            showText(((timmer / 60) / 10) % 60,
                     (timmer / 60) % 10 + 10,
                     (timmer % 60) / 10,
                     (timmer % 60) % 10);
          }
        }
      }
    }
    else
    {
      milibuttonchange = millis();
    }

    int Push_button_state = digitalRead(PushButton);
    if (Push_button_state == HIGH && prestate == false)
    {
      click_quickstart();
      prestate = true;
      vTaskDelay(100);
    }
    else
    {
      // digitalWrite(LEDPIN, LOW);
    }

    int pause_button_state = digitalRead(pausebutton);
    if (pause_button_state == HIGH && prestate1 == false)
    {
      click_pause();
      prestate1 = true;
      vTaskDelay(100);
    }
    else
    {
      // digitalWrite(LEDPIN, LOW);
    }

    if (pause_button_state == LOW)
    {
      prestate1 = false;
    }

    if (Push_button_state == LOW)
    {
      prestate = false;
    }
  }
}

void ControlPV(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(1000);
    // if (timmer > 0 && state_start == true)
    // {

    // }
    // else
    // {
    //   digitalWrite(highvoltage, LOW);
    // }

    showText(((timmer / 60) / 10) % 60,
             (timmer / 60) % 10 + 10,
             (timmer % 60) / 10,
             (timmer % 60) % 10);
  }
}

byte b = 0;
void loop()
{
  showDisplay();
}

void showText(char a, char b, char c, char d)
{
  disp_c[0] = d;
  disp_c[1] = c;
  disp_c[2] = b;
  disp_c[3] = a;
}

void showDisplay()
{
  setDisp();
  for (int i = 0; i < 8; i++)
  {
    setDigit(i, disp[i]);
  }
}

void setDigit(int dig, int character)
{
  int digits[] = {128, 64, 32, 16, 8, 4, 2, 1};

  //character set (0-9)0-9
  //            (10-19)0.-9.
  //            (20-45)A-Z
  //            (46-71)a-z
  //            (72)- (73) space
  int characters[] = {
      3, 159, 37, 13, 153, 73, 65, 31, 1, 9,
      2, 158, 36, 12, 152, 72, 64, 30, 0, 8,
      17, 1, 99, 3, 97, 113, 67, 145, 243, 135, 145, 227, 85, 19, 3, 49, 25, 115, 73, 31, 129, 129, 169, 145, 137, 37,
      5, 193, 229, 133, 33, 113, 9, 209, 247, 143, 81, 227, 85, 213, 197, 49, 25, 245, 73, 225, 199, 199, 169, 145, 137, 37,
      253, 255};

  digitalWrite(RCLK_pin, LOW);
  shiftOut(DIO_pin, SCLK_pin, LSBFIRST, characters[character]);
  shiftOut(DIO_pin, SCLK_pin, LSBFIRST, digits[dig]);
  digitalWrite(RCLK_pin, HIGH);
}

void setDisp()
{
  for (int i = 0; i < 8; i++)
  {
    int val = disp_c[i];
    if ((val >= 32) && (val <= 47))
    { // ! ถึง / ไม่ให้แสดง
      switch (val)
      {
      case 45:
        val = 72;
        break; // เครื่องหมาย -
      default:
        val = 73;
        break; // ค่าอื่นระหว่างนี้ ไม่ให้แสดง
      }
    }
    else if ((val >= 48) && (val <= 57)) //0-9
    {
      val -= 48;
    }
    else if ((val >= 65) && (val <= 90)) //A-Z
    {
      val -= 45;
    }
    else if ((val >= 97) && (val <= 122)) //a-z
    {
      val -= 51;
    }

    disp[i] = val;
  }
}

void EEPROMwifi()
{

  /////////// EEPROM  wifi ////////////////
  for (int i = 0; i < 32; ++i) //Reading SSID EEPROM
  {
    Essid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) //Reading Password EEPROM
  {
    Epass += char(EEPROM.read(i));
  }
  if (Essid.length() > 1)
  {
    Serial.println(Essid.c_str()); //Print SSID
    Serial.println(Epass.c_str()); //Print Password
  }
  return;
}

void vjoinwifi(AsyncWebServerRequest *request)
{

  int Tnetwork = 0, i = 0, len = 0;
  String st = "", out = "";
  if (scanstate == false)
  {
    scanstate = true;
    Tnetwork = WiFi.scanNetworks();
    st = "<select name=\"ssid\">";
    for (i = 0; i < Tnetwork && i < 10; ++i)
    {
      vTaskDelay(10);
      // st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
      String ssid_scan = WiFi.SSID(i);
      //  Serial.println(ssid_scan);
      if (ssid_scan != ssidAP)
      {
        st += "<option value=" + String(i) + ">" + ssid_scan + "( " + getPowerPercentage(WiFi.RSSI(i)) + "% )</option>";
        // st += "<option value=" + String(i) + ">####</option>";
        // Serial.println(String(i));
        findwifi[i] = ssid_scan;
      }
    }
    st += "</select>";
    //  ip = WiFi.softAPIP();
    out = "<div><h1>Scan Wifi!</h1> ";
    out += "<form method='post' action='a'><label>SSID: </label>" + st + "<p><label>PASSWORD : </label><input name='pass' length=64 placeholder='Ex : Pass1234'></p><input type='submit'></form>";
    out += "<a href =\"/\"><button>Back</button></a><div>";
    scanstate = false;
  }
  request->send(200, "text/html", out + style1);
}

int getPowerPercentage(int power)
{
  int quality;
  // dBm to Quality:
  if (power <= -100)
    quality = 0;
  else if (power >= -50)
    quality = 100;
  else
    quality = 2 * (power + 100);

  return quality;
}

void vControl(AsyncWebServerRequest *request)
{
  // String timhtml = "<h2>Timmer : </h2>";
  // timhtml += String("Free Memory is ") + String(ESP.getFreeHeap());
  // request->send(200, "text/html", timhtml + style1);
  String button0 = "<script>function function2(){var xhr=new XMLHttpRequest();xhr.withCredentials=!0;xhr.addEventListener('readystatechange',function(){if(this.readyState===4){console.log(this.responseText)}});xhr.open('GET','/apipause');xhr.send()};function function1(){var e,t;e=`0=${document.getElementById('Hourdropdown1').value}&1=${document.getElementById('Mindropdown').value}`,(t=new XMLHttpRequest).withCredentials=!0,t.addEventListener('readystatechange',function(){4===this.readyState&&console.log(this.responseText)}),t.open('POST','/set'),t.setRequestHeader('Content-Type','application/x-www-form-urlencoded'),t.send(e)};function funcbutton0() {var xhr=new XMLHttpRequest();xhr.withCredentials=!0;xhr.addEventListener('readystatechange',function(){if(this.readyState===4){}});xhr.open('GET','/apiquickstart');xhr.send()}</script>";
  String tex1web;
  // tex1web = "<meta http-equiv='refresh' content='1'/>";
  tex1web = "<body><h2>" + String("Cooking is = ") + "<span id='showtime'></span></h2>";
  tex1web += "<br><br>";
  tex1web += "<div id='Hourdropdown'><select id='Hourdropdown1' name='HourSelect' class='HourSelect'> <option value='0'>--Select Hour--</option> </select><select id='Mindropdown' name='MinSelect' class='MinSelect'> <option value='0'>--Select Minutes--</option> </select><br></div><script>window.onload=function(){var e,o,n,t;for(e=document.getElementById('Hourdropdown').getElementsByTagName('select')[0],o=0;o<=99;o++)(t=new Option).text=o+' MM',t.value=o,e.options[o]=t;for(n=document.getElementById('Mindropdown'),o=0;o<=59;o++)(t=new Option).text=o+' SS',t.value=o,n.options[o]=t}</script>";
  tex1web += "<p></p><br><center><a class='w3-button w3-deep-orange' onclick='funcbutton0()'>Quick Start</a><a onclick='function1()' class='w3-button w3-deep-orange'>Set time</a><a onclick='function2()' class='w3-button w3-deep-orange'>RUN / PAUSE</a></center>";
  tex1web += "</body>";
  request->send(200, "text/html", style1 + tex1web + script + button0);
}

void vapitimmer(AsyncWebServerRequest *request)
{
  String tex1web = String(timmer / 60) + " : " + String(timmer % 60);
  request->send(200, "text/html", tex1web);
}
void vquickstart(AsyncWebServerRequest *request)
{
  request->send(200, "text/html");
  click_quickstart();
}
void vpause(AsyncWebServerRequest *request)
{
  request->send(200, "text/html");
  click_pause();
}

void vset(AsyncWebServerRequest *request)
{
  request->send(200, "text/html");
  int params = request->params();
  int i;
  int hhh, mmm;

  for (i = 0; i < params; i++)
  {
    AsyncWebParameter *set_p = request->getParam(i);
    if (set_p->isPost())
    {
      String valuetime = String(set_p->value());

      if (i == 0)
      {
        hhh = valuetime.toInt();
      }
      else if (i == 1)
      {
        mmm = valuetime.toInt();
      }
    }
    Serial.println(String(hhh) + " : " + String(mmm));
    timmer = hhh * 60 + mmm;
  }
}

void vcheckip(AsyncWebServerRequest *request)
{
  String checkip = "", youip = "";
  checkip = "<h1>Check Your IP...</h1> ";
  checkip += "Your IP in home : ";
  checkip += " " + WiFi.softAPIP().toString() + " : " + WiFi.localIP().toString() + "<br><br>\n\n";
  // }
  checkip += "<a href =\"/\"><button>Back</button></a>";

  request->send(200, "text/html", checkip + style1);
}

void vwifiscana(AsyncWebServerRequest *request)
{
  String thtml = "<h2>Join new wifi success,Please reconnect....</h2>";
  request->send(200, "text/html", thtml + style1);
  int i;
  int selectwifi;
  int params = request->params();
  for (i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile())
    {
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    }
    else if (p->isPost())
    {
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      if (i == 0)
      {
        selectwifi = p->value().toInt();
        // Serial.println(selectwifi);
      }
      else if (i == 1)
      {
        inPass = String(p->value().c_str());
      }
    }
    else
    {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
  ssidconnect = findwifi[selectwifi];

  sssid = ssidconnect.c_str();
  passs = inPass.c_str();
  if (sssid.length() > 1 && passs.length() > 1)
  {
    ClearEeprom(); //First Clear Eeprom
    vTaskDelay(1000);
    for (int i = 0; i < sssid.length(); ++i)
    {
      EEPROM.write(i, sssid[i]);
    }

    for (int i = 0; i < passs.length(); ++i)
    {
      EEPROM.write(32 + i, passs[i]);
    }
    EEPROM.commit();
  }

  ESP.restart();
}

void ClearEeprom()
{
  for (int i = 0; i < 96; ++i)
  {
    EEPROM.write(i, 0);
  }
  return;
}
