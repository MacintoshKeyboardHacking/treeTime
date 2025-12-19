// treeTime: back around 2015, I bought a pre-lit GE christmas tree that advertised
// "GE Color Effects".  Unfortunately, it turned out to be the "2-Wire, Gemmy style" form of CE
// where the color patterns are stored in the bulbs, so, not nearly as cool as the
// popular hackable lights... I hadn't seen any hacks for this type and I was getting
// bored with the tree paterns, so...

// Using a Microbit v2 and an LED amplifier spliced into the 36vdc power cord,
// I was able to emulate the 6-bit protocol used by the stock controller.

// The resulting interface isn't perfect... either the DC offset or an issue
// with the slew rate of my amplifier, I don't know, but some of the LEDS just
// don't respond reliably when using the same timing as the stock controller.
// Therefore, a slower, flickery-er timing is employed.  MIT license.

// GE Color Effects "6 colors 20 functions" #0672836
// 803993-017334 (c)2015 Nicolas Holiday Inc.

// stock functions: solid r/g/p/b/y/w, pulse r/g/p/b/y/w/all, random flash/pulse, sparkle r/g/p/b/all

// timing defines:
const int D0 = 15000;    // stock=500, values as low as 100 work but not reliably, max ~ 100k and still functional
const int D1 = 7500;     // stock=5000
const int D2 = 15000;    // stock=15000
const int DlyD = 30000;  // stock=42000ish
const int buttonA = 5;
const int buttonB = 11;

const int SON = 0;  // might need to reverse these
const int SOFF = 1;

const int COL1 = 4;  // column #1 control- v2
const int COL2 = 7;
const int COL3 = 3;
const int COL4 = 6;
const int COL5 = 10;

const int LED = 25;  // row #1 led- v2
const int LED2 = 24;
const int SPKR = 27;

//const int COL1 = 3;     // column #1 control- v1
//const int LED = 26;     // row #1 led- v1

int mode = 0x01;  //38
int loops = 41;
int lastmode = 0;
int lsb = 0;
int txpin = 0;
int enable = 1;

int func = 2;  // MODE
int oldfunc = 0;
int doReset = 1;
int doTick = 0;


int newmode(int nowmode) {
  int lsb = (nowmode & 0x7);

  int newlsb = 1;
  if (lsb == 1) { newlsb = 5; }
  if (lsb == 5) { newlsb = 4; }
  if (lsb == 4) { newlsb = 6; }
  if (lsb == 6) { newlsb = 2; }
  if (lsb == 2) { newlsb = 3; }

  return ((nowmode & 0x38) + newlsb);
}


void txrst(int pin, int rstmode) {
  digitalWrite(pin, SOFF);
  // 750 = almost all colors syncd, 100 = ~ minimum
  delay(660);  // stock 660ms.
  if (rstmode == 0) {
    digitalWrite(pin, SON);
    delayMicroseconds(DlyD);
  } else if (rstmode == 1) {
    int count = 6;  // min=6 (+1 above)
    while (count) {
      count--;
      digitalWrite(pin, SOFF);  // 10000=long?
      delayMicroseconds(5000);
      digitalWrite(pin, SON);
      delayMicroseconds(5000);
    }
  } else if (rstmode == 2) {
    delayMicroseconds(30000);

    int count = 24;  // min=6 (+1 above)
    while (count < 32) {
      txmt(pin, count);
      count++;
    }
  }
}


void txmt(int pin, int txmode) {
  digitalWrite(pin, SOFF);
  delayMicroseconds(D0);

  int bit = 6;
  while (bit) {
    bit--;
    digitalWrite(pin, SON);

    if (txmode & (1 << bit)) {
      delayMicroseconds(D2);  // 1=long
    } else {
      delayMicroseconds(D1);  // 0=short
    }

    digitalWrite(pin, SOFF);
    delayMicroseconds(D0);
  }
  digitalWrite(pin, SON);
  delayMicroseconds(DlyD);
}


void setup() {
  Serial.begin(9600);
  Serial.println("treeTime");

  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  pinMode(COL1, OUTPUT);
  pinMode(COL2, OUTPUT);
  pinMode(COL3, OUTPUT);
  pinMode(COL4, OUTPUT);
  pinMode(COL5, OUTPUT);

  digitalWrite(COL1, LOW);
  digitalWrite(COL2, LOW);
  digitalWrite(COL3, LOW);
  digitalWrite(COL4, LOW);
  digitalWrite(COL5, LOW);

  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(SPKR, OUTPUT);
  digitalWrite(SPKR, LOW);

  pinMode(txpin, OUTPUT);
}


void loop() {
  digitalWrite(LED, HIGH);
  digitalWrite(LED2, HIGH);

  if (func != oldfunc) {
    doReset = 1;
    oldfunc = func;
  }

  if (doReset) {
    doReset--;
    digitalWrite(COL1, HIGH);
    digitalWrite(COL2, HIGH);
    digitalWrite(COL3, HIGH);
    digitalWrite(COL4, HIGH);
    digitalWrite(COL5, HIGH);
    switch (func) {
      case 0:
      case 1:
        digitalWrite(COL1, LOW);
        {
          txrst(txpin, 0);
          doTick = 0;
          break;
        }
      case 2:
        digitalWrite(COL2, LOW);
        {
          txrst(txpin, 0);
          doTick = 0;
          break;
        }
      case 3:
        digitalWrite(COL3, LOW);
        {
          txrst(txpin, 0);
          doTick = 1;
          break;
        }
      case 4:
        digitalWrite(COL4, LOW);
        {
          txrst(txpin, 2);
          doTick = 1;
          break;
        }
      case 5:
        digitalWrite(COL5, LOW);
        {
          txrst(txpin, 0);
          txmt(txpin, 10);  //15
          txmt(txpin, 10);  //15
          doTick = 1;
          break;
        }
    }
  }

  if (func == 1) {
    loops = 5;
    txmt(txpin, mode + 32);
    txmt(txpin, mode + 32);
  }

  if (func == 2) {
    loops = 68;
    txmt(txpin, mode + 8);
    txmt(txpin, mode + 8);
  }

  if (func == 3) {
    loops = -1;
    txmt(txpin, mode + 40);
    txmt(txpin, mode + 40);
  }

  if (func == 4) {
    loops = -1;
  }

  if (func == 5) {
    loops = 0;
    delay(100);
    int tmp = -1;
    while (tmp) {
      tmp--;
      digitalWrite(txpin, SOFF);
      digitalWrite(LED, HIGH);
      delayMicroseconds(5000);
      digitalWrite(txpin, SON);
      digitalWrite(LED, LOW);
      delayMicroseconds(30000);
    }
  }

  lastmode = mode;
  Serial.println(mode);
  digitalWrite(LED, LOW);


  while (loops) {
    loops--;

    if (!digitalRead(buttonA)) {
      func++;
      if (func > 4) { func = 1; }

      Serial.println(func);
      loops = 0;
      enable = 1;
      doReset = 1;
    }
    if (!digitalRead(buttonB)) {
      loops = 0;
      doTick = 0;
    }

    if (doTick) {
      digitalWrite(txpin, SOFF);
      digitalWrite(LED, HIGH);

      if ((func == 4) | (func == 3)) {
        delayMicroseconds(1000);
      } else {
        delayMicroseconds(D0);
      }

      digitalWrite(txpin, SON);
      digitalWrite(LED, LOW);

      if (func == 3) { delay(1500); }
      if (func == 4) { delay(1000); }
    }
    delay(100);
  }
  mode = newmode(mode);
}

// stock CMDs

// 00	SSSSSS	alloff
// 01	SSSSSL	red
// 02	SSSSLS	green
// 03	SSSSLL	yellow
// 04	SSSLSS	blue
// 05	SSSLSL	purple
// 06	SSSLLS	cyan
// 07	SSSLLL	white

// 08 	SSLSSS	?
// 09	SSLSSL	red pulse
// 10	SSLSLS	green pulse
// 11	SSLSLL	yellow pulse
// 12	SSLLSS	blue pulse
// 13	SSLLSL	purple pulse
// 14	SSLLLS	cyan pulse
// 15	SSLLLL	white pulse

// 16	SLSSSS	black/white
// 17	SLSSSL	red/white
// 18	SLSSLS	green/white
// 19	SLSSLL	yellow/white
// 20	SLSLSS	blue/white
// 21	SLSLSL	purple/white
// 22	SLSLLS	cyan/white
// 23	SLSLLL	all white

// 24-31	SLLSSS	allcjump
// single pulse to increment

// 32	LSSSSS	?
// 33	LSSSSL	black/red
// 34	LSSSLS	black/green
// 35	LSSSLL	? black/yellow
// 36	LSSLSS	black/blue
// 37	LSSLSL	? black/purple
// 38	LSSLLS	? black/cyan
// 39	LSSLLL	black/white

// 40-47	LSLSSS	allcfade
// 48-55  LLSxxx  disables bulb
// 56-63  LLLxxx  allcfade
