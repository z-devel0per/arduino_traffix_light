#include <EtherCard.h>

#undef SERIAL
#define SERIAL 1

static byte myip[] = { 192,168,1,206 };
static byte gwip[] = { 192,168,1,1 };
static byte ssdp[] = { 239,255,255,250 };
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[750]; // tcp ip send and receive buffer
unsigned long timer=9999;

const short pins[3] = {5, 6, 7};
static BufferFiller bfill;

const char okHeader[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
;

static void print_webpage(BufferFiller& buf)
{
  buf.emit_p(PSTR("$F\n\n"
    "<html><head><title>Trafix lights</title></head><body>"
    "<center><h1>Trafix lights</h1>"
    "<hr><h2><a href=\"/red\">RED</a>"
    "<br><a href=\"/yellow\">YELLOW</a>"
    "<br><a href=\"/green\">GREEN</a>"
    "<br></h2>"
    "</center><hr>"
    "</body></html>"
  ), okHeader);
}

void ToggleLight(short pinNum){
#if SERIAL
   Serial.print("TogglePin ");
   Serial.println(pins[pinNum]);
#endif
   for(short i=0; i<3; ++i){    
     if (i == pinNum) {
        digitalWrite(pins[i], HIGH);
     } else {
        digitalWrite(pins[i], LOW);       
     }
   }
}



void setup(){  
#if SERIAL
  Serial.begin(57600);
  Serial.println("Boot");
#endif
  
  
  ether.begin(sizeof Ethernet::buffer, mymac, SS);
  ether.staticSetup(myip, gwip);
/*

// Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0){
    #if SERIAL
      Serial.println( "Failed to access Ethernet controller");
    #endif
  }

  if (!ether.dhcpSetup()){
    #if SERIAL
      Serial.println("DHCP failed");
    #endif
  }

  ether.printIp("My IP: ", ether.myip);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
  */
  #if SERIAL
    Serial.println("Init");
  #endif

  for(int i=0; i<3; ++i){    
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  
}

void loop()
{
  word pos = ether.packetLoop(ether.packetReceive());

  if (pos) {
    
    char* data = (char *) Ethernet::buffer + pos;
    bfill = ether.tcpOffset();
    #if SERIAL
      Serial.println(data);
    #endif


    if (strncmp("GET ", data, 4) != 0){      // head, post and other methods:
      bfill.emit_p(PSTR("$F\r\n<h1>200 OK</h1>"), okHeader);
      
    }
    // just one web page in the "root directory" of the web server    
    if (strncmp("GET / ", data, 6) == 0){
      print_webpage(bfill);
     
    } else
    if (strncmp("GET /red", data, 8) == 0) {// Pin №1 On
      print_webpage(bfill);
      ToggleLight(0);
      
    } else
    if (strncmp("GET /green", data, 10) == 0) {// Pin №1 Off
      print_webpage(bfill);
      ToggleLight(2);
      
    }
    else
    if (strncmp("GET /yellow", data, 11) == 0) {  // Pin №2 On
      print_webpage(bfill);
      ToggleLight(1);
      
    }
    else{
      bfill.emit_p(PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
      
    }
    ether.httpServerReply(bfill.position()); // send web page data
  }

}


