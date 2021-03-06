// Copyright 2017 James Tate <thejamestate@gmail.com>
// Flash Chip Reader
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>

#include <Arduino.h>
#include "nandio.h"

void readIDNAND();
void readDATANAND();
int free_ram();

void printMenu() {
  Serial.println("Choose Option");
  Serial.println("1: Read ID");
  Serial.println("2: Read Contents");
}

void setup() {
  Serial.begin(250000);
  Serial.println("Welcome");
  initChip();
  printMenu();
}

void loop() {
  static int choice = 0;
  switch (Serial.read()) {
    case '1':
      choice = 1;
      break;
    case '2':
      choice = 2;
      break;
    case 'z':
      choice = 1000;
      break;
    case '\r':
    case '\n':
      switch (choice) {
        case 1: readIDNAND(); break;
        case 2: readDATANAND(); break;
        case 1000: Serial.println(free_ram()); break;
      }
      printMenu();
      choice = 0;
      break;
  }
}

static char * hex[256] = {
  "00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F",
  "10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F",
  "20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F",
  "30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F",
  "40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F",
  "50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F",
  "60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F",
  "70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F",
  "80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F",
  "90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F",
  "A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF",
  "B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF",
  "C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF",
  "D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF",
  "E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF",
  "F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF",
};

void readIDNAND() {
  Serial.println("====================================");
  setDataBusOut();
  prepChip();
  putDataBus(0x90);
  latchCommand();
  setAddress();
  putDataBus(0x00);
  latchAddress();
  setDataBusIn();
  int id = readDataBus();
  Serial.print("ID = 0x");
  Serial.print(hex[id]);
  Serial.print(hex[readDataBus()]);
  Serial.print(hex[readDataBus()]);
  Serial.print(hex[readDataBus()]);
  Serial.println(hex[readDataBus()]);
  switch (id) {
    case 0x01: Serial.println("Found myself attached to AMD");                    break;
    case 0x04: Serial.println("Found myself attached to Fujitsu");                break;
    case 0x07: Serial.println("Found myself attached to Renesas");                break;
    case 0x20: Serial.println("Found myself attached to ST Micro");               break;
    case 0x2C: Serial.println("Found myself attached to Micron");                 break;
    case 0x8F: Serial.println("Found myself attached to National Semiconductor"); break;
    case 0x98: Serial.println("Found myself attached to Toshiba");                break;
    case 0xAD: Serial.println("Found myself attached to Hynix");                  break;
    case 0xC2: Serial.println("Found myself attached to Macronix");               break;
    case 0xEC: Serial.println("Found myself attached to Samsung");                break;
    default  : Serial.println("Unknown chip ID");                                 break;
  }
  closeChip();
  Serial.println("====================================");
}

void readDATANAND() {
  Serial.println("====================================");
  // This is set up for Toshiba TC58NVG0S3ETA00
  // Change these parameters to correspond to your chip
  unsigned char buf[2112];  // Arduino has ~5K free; do not use buffer if page size is larger than that
  int pagesize = 2112;
  int pages = 64;
  int blocks = 1024;
  for (int block = 0; block < blocks; block++) {
    for (int page = 0; page < pages; page++) {
      setDataBusOut();
      prepChip();
      putDataBus(0x00);
      latchCommand();
      setAddress();
      // This is set up for Toshiba TC58NVG0S3ETA00
      // Change this addressing to correspond to your chip
      putDataBus(0);
      putDataBus(0);
      putDataBus((block << 6) | page);
      putDataBus(block >> 2);
      latchAddress();
      setCommand();
      putDataBus(0x30);
      latchCommand();
      setDataBusIn();
      delayMicroseconds(25);
      // for (int i = 0; i < pagesize; i++) Serial.print(hex[readDataBus()]);
      for (int i = 0; i < pagesize; i++) buf[i] = readDataBus();
      for (int i = 0; i < pagesize; i++) Serial.print(hex[buf[i]]);
      Serial.println();
    }
  }
  closeChip();
  Serial.println("====================================");
}

int free_ram() {
  extern int __heap_start, * __brkval; int v;
  return (int)&v - ((__brkval == 0) ? (int)&__heap_start : (int)__brkval);
}
