#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>
#include <SoftwareSerial.h>
#include <SD.h>

//Rotate 0
//#define YP A1  // must be an analog pin, use "An" notation!
//#define XM A2  // must be an analog pin, use "An" notation!
//#define YM 5   // can be a digital pin, changed from 7
//#define XP 6   // can be a digital pin

//Rotate -90
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A1  // must be an analog pin, use "An" notation!
#define YM 6   // can be a digital pin, changed from 7
#define XP 5   // can be a digital pin

// Calibrate values
#define TS_MINX 90
#define TS_MAXX 888
#define TS_MINY 128
#define TS_MAXY 884

#define LCD_CS    A3
#define LCD_CD    A2
#define LCD_WR    A1
#define LCD_RD    A0
#define LCD_RESET A4
#define SD_CS 10

#define DEN       0x0000
#define LAM       0x001F
#define DO        0xF800
#define XANHLA    0x07E0
#define XANHDUONG 0x07FF
#define HONG      0xF81F
#define VANG      0xFFE0
#define TRANG     0xFFFF

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define BUFFPIXEL 20 // Thay doi de load nhanh hon

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
SoftwareSerial Zigbee(A5, 1); // RX, TX

int bandohientai=0;
int vitrihientai=1;
int vitridich=0;
int DAI, RONG;
unsigned long timer1, timer2, timer3;
unsigned char cuongdo[3] = {0, 0, 0};//Endd1, Endd2, Endd3

void drawBorder ();
void hienthibando(int bd);
void bmpDraw(char *filename, int x, int y);
uint32_t read32(File f);
uint16_t read16(File f);

void setup(void) {
  Serial.begin(9600);
  Zigbee.begin(9600);
  Serial.println(F("==DO AN TOT NGHIEP 2017=="));

  if (!SD.begin(SD_CS)) {
    Serial.println(F("Loi SDCard"));
    while(1);
  }
  
  tft.reset();
  tft.begin(0x9341); // SDFP5408
  tft.setRotation(3); //-90
  
  drawBorder();  
  
  tft.setCursor (70, 30);
  tft.setTextSize (2);
  tft.setTextColor(DO);
  tft.println(F("DO AN TOT NGHIEP"));
  tft.setCursor (130, 50);
  tft.println("2017");
  tft.setCursor (70, 100);
  tft.setTextSize (2);
  tft.setTextColor(DEN);
  tft.println(F("MSSV: 12141xxx"));
  tft.setCursor (70, 120);
  tft.println(F("MSSV: 12141xxx"));

  tft.setCursor (130, 160);
  tft.setTextSize (1);
  tft.setTextColor(DEN);
  tft.println(F("Ten do an"));

//  waitOneTouch();

//  tft.fillScreen(DEN);
  
//  RONG = tft.width()/3;
//  DAI = tft.height()/8;
  
//  tft.fillRect(0, 0, RONG, DAI, DO);
//  tft.setCursor (RONG/2-2, DAI/2-3);
//  tft.setTextSize (2);
//  tft.setTextColor(DEN);
//  tft.println(F("1"));
  
//  tft.fillRect(RONG, 0, RONG, DAI, XANHLA);
//  tft.setCursor (RONG*3/2-2, DAI/2-3);
//  tft.println(F("2"));
  
//  tft.fillRect(RONG*2, 0, RONG, DAI, HONG);
//  tft.setCursor (RONG*5/2-2, DAI/2-3);
//  tft.println(F("3"));

//  capnhatbando(vitrihientai*10 + vitridich); 
}


void loop()
{
  if(Zigbee.available() < 3) return;

  String dulieu = Zigbee.readString();
  Serial.print("\nCo thong bao:");
  Serial.println(dulieu);
  char i = 0, k = dulieu.length();
  int data;
  while(i < k){
    if(dulieu.charAt(i) == '#' && i+2 < k )
    {
      if(dulieu.charAt(i+1) == 1 || dulieu.charAt(i+1) == '1'){
        data = 11;
      }else if(dulieu.charAt(i+1) == 2  || dulieu.charAt(i+1) == '2'){
        data = 22;
      }else if(dulieu.charAt(i+1) == 3  || dulieu.charAt(i+1) == '3'){
        data = 33;
      }
      i += 3;
    }else
      i++;
  }    

  if(bandohientai != data) {    
    bandohientai = data;
    hienthibando(bandohientai);
  }
}


void hienthibando(int bd)
{    
    char s[7] = {'1','1','.','b','m','p',0};
    itoa(bandohientai, s, 10);
    s[2] = '.';
    bmpDraw(s,0, 0);
    Serial.print("Ban do hien tai ");
    Serial.println(bd);
  
}


void drawBorder () {
  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;
  uint8_t border = 10;
  tft.fillScreen(DO);
  tft.fillRect(border, border, (width - border * 2), (height - border * 2), TRANG);  
}

TSPoint waitOneTouch() {
  TSPoint p;  
  do {
    p= ts.getPoint();   
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);  
  } while((p.z < MINPRESSURE )|| (p.z > MAXPRESSURE));  
  return p;
}

void capnhatvitri()
{  
  if(millis() - timer1 > 5000) { timer1 = millis(); cuongdo[0] = 0; }
  if(millis() - timer2 > 5000) { timer2 = millis(); cuongdo[1] = 0; }
  if(millis() - timer3 > 5000) { timer3 = millis(); cuongdo[2] = 0; }
  
  if(Zigbee.available() < 3) return;

  String dulieu = Zigbee.readString();
  char i = 0, k = dulieu.length();
  bool codulieu = false;
  while(i < k){
    if(dulieu.charAt(i) == '#' && i+2 < k )
    {
      if(dulieu.charAt(i+1) == 1 || dulieu.charAt(i+1) == '1'){
        cuongdo[0] = dulieu.charAt(i+2);        
        timer1 = millis();
      }else if(dulieu.charAt(i+1) == 2  || dulieu.charAt(i+1) == '2'){
        cuongdo[1] = dulieu.charAt(i+2);
        timer2 = millis();
      }else if(dulieu.charAt(i+1) == 3  || dulieu.charAt(i+1) == '3'){
        cuongdo[2] = dulieu.charAt(i+2);
        timer3 = millis();
      }
      codulieu = true;
      i += 3;
    }else
      i++;
  }    
  
  if(!codulieu) return;
  
  if(cuongdo[0] >= max(cuongdo[1],cuongdo[2]))
  {
    vitrihientai = 1;    
  }
  else if(cuongdo[1] > cuongdo[2]){
      vitrihientai = 2;  
  }
  else vitrihientai = 3;  
  Serial.print("* Vi tri hien tai ");
  Serial.println(vitrihientai);
  capnhatbando(vitrihientai*10 + vitridich);
}

void capnhatbando(int bd)
{    
  static int bandohientai=0;
  int tam = bd;
  if(bd % 10 == 0) tam = bd + bd/10;//Tranh truong hop chua chon diem dich
  if(tam != bandohientai){
    bandohientai = tam;
    /*tft.fillRect(tft.width()/2, tft.height()/2, 100,100, DEN);
    tft.setCursor (tft.width()/2, tft.height()/2);
    tft.setTextSize (3);
    tft.setTextColor(TRANG);
    tft.println(bandohientai);   */
    char s[7] = {'1','1','.','b','m','p',0};
    itoa(bandohientai, s, 10);
    s[2] = '.';
    bmpDraw(s,0, DAI);
    Serial.print("Ban do hien tai ");
    Serial.println(bandohientai);
  }
}

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void bmpDraw(char *filename, int x, int y) {
  Serial.println(filename);
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;
  uint8_t  lcdidx = 0;
  boolean  first = true;

  if((x >= tft.width()) || (y >= tft.height())) return;

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.println(F("File khong tim thay"));
    //while(1);
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    (void)read32(bmpFile);
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    (void)read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { 
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if(lcdidx > 0) {
                tft.pushColors(lcdbuffer, lcdidx, first);
                lcdidx = 0;
                first  = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx++] = tft.color565(r,g,b);
          } // end pixel
        } // end scanline
        // Write any remaining data to LCD
        if(lcdidx > 0) {
          tft.pushColors(lcdbuffer, lcdidx, first);
        } 
        
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("Dinh dang anh khong duoc ho tro"));
}
