/*
 written by Jason Berger ,arduinoall, พ.ไพศาล
 reedited by Phaisarn Te. @060614
 
 8-Digit 7-Segment display driver.
 2x 74HC595 wired to 2x 4-digit 7-segment displays
 bits 0-7 select a digit
 bits 8-14 A-G
 
 */
char disp_c[8] ;

//กำหนด ขา pin
const int SCLK_pin=21;
const int RCLK_pin=23;
const int DIO_pin =22;


// 0-9   --> ใช้ค่า 0-9
// 0.-9. --> ใช้ค่า 10-19
// space --> ' '
// A-Z, a-z --> 'A' 'B' ... 'a' 'b'

int  disp[8];

//time values for delay workaround
unsigned long prev =0;
unsigned long waitMS=0;

void setup()
{

  pinMode(RCLK_pin,OUTPUT);
  pinMode(DIO_pin,OUTPUT);
  pinMode(SCLK_pin,OUTPUT);
  showText('O','P','E','N'); // แสดงข้อความต้อนรับ
} 

int n;
unsigned long start=millis();
byte b = 0;
void loop()
{
  showDisplay();
  if(b==0){
    b++;
    wait(3000);     // หน่วงเวลาแสดงข้อความต้อนรับ
  }
  else{
    if ( millis() > (prev + waitMS))
    {
      //code to loop in here


      //ระหว่าง loop แก้ไขแสดงค่าอื่นๆได้

      // แสดงผลแบบตัวอักษร ตัวอย่างแสดงคำว่า Ardu
     // showText('A','r','d','u'); //  แก้ค่าที่จะให้แสดงผลตรงนี้


      // แสดงผลแบบตัวเลข ตัวอย่างแสดงค่า 95.42
      // showText(9,15,4,2); //15 --> แสดงเลข 5.


       // แสดงผลตัวอย่าง การ run เลข
      showText((n/1000)%10,(n/100)%10+10,(n/10)%10,n%10);  //แก้ค่าที่จะให้แสดงผลตรงนี้
      n++;
      if(n>10000) {
        n=0;
      }
     
      // หน่วงเวลา
      wait(1000);
    }
  }
}

void showText(char a, char b , char c,char d){
  disp_c[0] = d;
  disp_c[1] = c;
  disp_c[2] = b;
  disp_c[3] = a;
}
void showDisplay()
{
  setDisp();
  for(int i=0; i<8; i++)
  {
    setDigit(i,disp[i]);
  }
}

void setDigit(int dig, int character)
{
  int digits[]= {
    128,64,32,16,8,4,2,1        };

  //character set (0-9)0-9
  //            (10-19)0.-9.
  //            (20-45)A-Z
  //            (46-71)a-z
  //            (72)- (73) space
  int characters[]= {
    3,159,37,13,153,73,65,31,1,9,
    2,158,36,12,152,72,64,30,0,8,
    17,1,99,3,97,113,67,145,243,135,145,227,85,19,3,49,25,115,73,31,129,129,169,145,137,37,
    5,193,229,133,33,113,9,209,247,143,81,227,85,213,197,49,25,245,73,225,199,199,169,145,137,37,
    253,255        };

  digitalWrite(RCLK_pin, LOW);
  shiftOut(DIO_pin, SCLK_pin, LSBFIRST, characters[character]);
  shiftOut(DIO_pin, SCLK_pin, LSBFIRST, digits[dig]);
  digitalWrite(RCLK_pin, HIGH);
}

void setDisp()
{
  for (int i=0; i<8;i++)
  {
    int val = disp_c[i];
    if((val >= 32)&&(val <= 47)){ // ! ถึง / ไม่ให้แสดง
      switch (val){
      case 45 :
        val = 72;
        break;  // เครื่องหมาย -
      default :
        val = 73;
        break;  // ค่าอื่นระหว่างนี้ ไม่ให้แสดง
      }
    }
    else if((val >= 48)&&(val <= 57)) //0-9
    {
      val -= 48;
    }
    else if((val >= 65)&&(val <= 90)) //A-Z
    {
      val -= 45;
    }
    else if((val >= 97)&&(val <= 122)) //a-z
    {
      val -= 51;
    }

    disp[i] = val;

  }
}  
void wait( unsigned long milsec)
{
  prev = millis();
  waitMS = milsec;
}
