/*
 * Bargeldrechner und Kartensimulator für Monopoly
 *
 */

#define LED_RED 2
#define LED_GREEN 3

#define BTN_B 15
#define BTN_L 16
#define BTN_T 18
#define BTN_R 17

#define VOLTPIN A0

#define DIMLCD 19

#define START_GELD 1500

#define LOS_EINKOMMEN 200

#define INTRO_DURATION 5000

#define DEBUG false
#define LOG false
#define LCD true

#define ZS "00000"

#define BILDSCHIRMSCHONER (300000)

#include <LiquidCrystal.h>

LiquidCrystal lcd (13, 12, 11, 10, 9, 8, 7, 6, 5, 4);

short spielerzahl = 3;
int konto [8] = {0, 0, 0, 0, 0, 0, 0};

short spielerSender = 0;
short spielerEmpf = 0;

char * ereignisKarten [] = {
  "Bank-Irrtum         Ziehe +M200 ein"
  , "Lebensversicherung  faellig. +M100"
  , "Urlaubsgeld! Du     erhaelst +M100"
  , "Arztkosten.         Zahle -M50"
  , "Ruecke vor bis LOS. (+M200)"
  , "Zahle Schulgeld:    -M50"
  , "Stra\342enausbau: -M40 /Haus, -M115/Hotel"
  , "Schoenheitswettbew. +M10"
  , "Lagerverkaeufe      +M50"
  , "Du erbst +M100.       "
  , "Geburtstag. Jeder   schenkt +M10"
  , "Beratungsgebuehr von+M25"
  , "Gehe in's Gefaengnis(direkt)"
  , "KrankenhausgebuehrenZahle -M100."
  , "Du kommst aus dem   Gefaengnis frei."
  , "Steuerrueckzahlung. +M20."
};

char *  gemeinschaftsKarten [] = {
  "Renovieren:-M25  /Haus, -M100/Hotel."
  , "Dividende von M50.   "
  , "Ruecke vor bis LOS. (+M200)"
  , "Gehe in's Gefaengnis(direkt)"
  , "Vor bis Werk.       Zahle 2xW6 * 10"
  , "Vor bis Opernplatz. Bei LOS, +M200"
  , "Bausparvertrag      +M150"
  , "Vor bis Suedbahnhof.Bei LOS, +M200"
  , "Ruecke vor bis zur  Schlossallee"
  , "Vor bis naechst. BhfDoppelte Miete!"
  , "Vor bis naechst. BhfDoppelte Miete!"
  , "Strafzettel!        -M15."
  , "3 Felder zurueck.    "
  , "Vorstand gewaehlt   Zahle jedem -M50."
  , "Vor bis Seestraße.  Bei LOS, +M200"
  , "Du kommst aus dem   Gefaengnis frei."
};

char letzteTransaktion [4] [21] = {
  " Sp 0 \176 0000 \176 Sp 0\0",
  " Sp 0 \176 0000 \176 Sp 0\0",
  " Sp 0 \176 0000 \176 Sp 0\0",
  " Sp 0 \176 0000 \176 Sp 0\0"
};
short letzteTransaktionIndex = 0;
char* currentEntryBuffer = new char [20];

short aktGemeinschaftskarte = 0;
short aktEreigniskarte = 0;

long zeitSeitLetzterAktion = millis();
boolean activeDisplay = true;

void setup() {
  if (LOG)Serial.begin(9600);
  if (LOG)Serial.println("System start");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BTN_B, INPUT);
  pinMode(BTN_L, INPUT);
  pinMode(BTN_T, INPUT);
  pinMode(BTN_R, INPUT);
  pinMode(DIMLCD, OUTPUT);
  pinMode(VOLTPIN, INPUT);
  digitalWrite(DIMLCD, HIGH);
  if (LCD)lcd.begin(20, 4);
  resetGame ();
}



// Setzt alle Spielstände wieder auf Anfang.
void resetGame () {

  if (LCD)lcd.clear ();
  if (LCD)  lcd.setCursor (0, 0);
  if (LCD)  lcd.print("Unterwassermonopoly");
  if (LCD)  lcd.setCursor (1, 1);
  if (LCD)  lcd.print("-= Neues Spiel =-");
  if (LCD)  lcd.setCursor (2, 2);
  if (LCD) lcd.print("www.tief-dunkel-");
  if (LCD) lcd.setCursor (6, 3);
  if (LCD) lcd.print(" kalt.org");

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  if (!DEBUG) delay (INTRO_DURATION);
  while (checkForButton() == 0){
    digitalWrite(DIMLCD, LOW);
    delay (200);
  }
  
  digitalWrite(DIMLCD, HIGH);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  warte (INTRO_DURATION);

  for (int i = 1; i < 8; i++) {
    konto [i] = START_GELD;
  }
  konto [0] = 2147483647; //bank
  aktGemeinschaftskarte = 0;
  aktEreigniskarte = 0;

  if (!DEBUG) frageNachSpielerzahl();

  randomSeed(analogRead(VOLTPIN) * millis());

  shuffle (gemeinschaftsKarten, 16);
  shuffle (ereignisKarten, 16);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void frageNachSpielerzahl() {
  if (LCD) lcd.clear();
  if (LCD) lcd.setCursor (0, 0);
  if (LCD) lcd.print("Anzahl Spieler:");
  if (LCD) lcd.setCursor (0, 1);
  if (LCD) lcd.print("\177 (-)");
  if (LCD) lcd.setCursor (15, 1);
  if (LCD) lcd.print("(+) \176");
  if (LOG)Serial.println("Anz. Spieler?");

  if (LCD) lcd.setCursor (10, 2);
  if (LCD) lcd.print(spielerzahl);
  if (LOG)Serial.println (spielerzahl);

  short btnRes = 0;
  while (btnRes != BTN_B) {
    delay (5);
    btnRes = checkForButton ();
    if (btnRes == BTN_R) {
      if (spielerzahl < 6) {
        spielerzahl++;
        if (LCD) lcd.setCursor (10, 2);
        if (LCD) lcd.print(spielerzahl);
        if (LOG)Serial.println (spielerzahl);
      }
      warte (500);
    }
    if (btnRes == BTN_L) {
      if (spielerzahl > 2) {
        spielerzahl--;
        if (LCD) lcd.setCursor (10, 2);
        if (LCD) lcd.print(spielerzahl);
        if (LOG)Serial.println (spielerzahl);
      }
      warte (500);
    }
  }
  if (LCD) lcd.clear();
  if (LCD) lcd.setCursor (0, 0);
  if (LCD) lcd.print("Starte Spiel");
  if (LCD) lcd.setCursor (1, 1);
  if (LCD) lcd.print("mit " );
  if (LCD) lcd.print(spielerzahl);
  if (LCD) lcd.print(" Spielern" );
  if (LOG)Serial.println("Starte Spiel!");
  warte (1000);
  updateKontostand();
}

void loop() {

  short btnRes = checkForButton ();

  if (btnRes == BTN_B) {
    wuerfle();
  }
  if (btnRes == BTN_T) {
    if (spielerSender == spielerEmpf) {
      zieheKarte();
    } else {
      geldTransfer();
    }
  }
  if (btnRes == BTN_L || btnRes == BTN_R ) {
    if (btnRes == BTN_L) {
      warte (500);
      spielerSender = (spielerSender + 1) % (spielerzahl + 1);
    } else {
      warte (500);
      spielerEmpf = (spielerEmpf + 1) % (spielerzahl + 1);
    }
    updateGeldTransferRichtung();
  }
  if (btnRes == 5) {
    zeigeLetzteTransaktionen();
  }

  checkBildschirmschoner();   
  delay(2);
}

void checkBildschirmschoner (){
  if (zeitSeitLetzterAktion + BILDSCHIRMSCHONER < millis() && activeDisplay) {
    digitalWrite(DIMLCD, LOW);
    warte (300);
    digitalWrite(DIMLCD, HIGH);
    warte (300);
    digitalWrite(DIMLCD, LOW);
    warte (300);
    digitalWrite(DIMLCD, HIGH);
    warte (300);
    digitalWrite(DIMLCD, LOW);
    activeDisplay = false;
    lcd.noDisplay();
  } 
  digitalWrite(DIMLCD, activeDisplay);
  lcd.display();
}

//Testet, welcher Knopf gedrückt wurde. Wenn TOP und BOTTOM gleichzeitig, dann wartet
// die Methode ob in 5 Sekunden immer noch beide gedrückt sind und started dann das Spiel neu.
short checkForButton () {
  if (digitalRead(BTN_T) || digitalRead(BTN_B) || digitalRead(BTN_L) || digitalRead(BTN_R)) {
    zeitSeitLetzterAktion = millis ();
    activeDisplay = true;
  }


  if (digitalRead(BTN_T) == HIGH && digitalRead(BTN_B) == HIGH) {
    if (LCD) lcd.clear();
    if (LCD) lcd.print("Halten fuer Neustart");
    if (LOG)Serial.println("Halten um neues Spiel zu starten");
    warte (5000);
    if (digitalRead(BTN_T) == HIGH && digitalRead(BTN_B) == HIGH) {
      resetGame();
    }
  }
  if (digitalRead(BTN_R) == HIGH && digitalRead(BTN_L) == HIGH) {
    return 5;
  }
  if (digitalRead(BTN_T) == HIGH) {
    return BTN_T;
  }
  if (digitalRead(BTN_R) == HIGH) {
    return BTN_R;
  }
  if (digitalRead(BTN_L) == HIGH) {
    return BTN_L;
  }
  if (digitalRead(BTN_B) == HIGH) {
    return BTN_B;
  }
  return 0;
}

void wuerfle () {

  for (short i = 0; i < 10; i++) {
    short w1 = random(1, 7);
    short w2 = random(1, 7);
    zeigeWuerfel (w1, 5);
    zeigeWuerfel (w2, 9);
    delay (i * 35);
  }
  updateBatterieanzeige();
  warte (1500);
}

void zieheKarte() {
  lcd.clear();
  if (LCD) lcd.setCursor (0, 0);
  if (LCD) lcd.print("Ziehe Karte!");
  warte (1000);
  if (LCD) lcd.setCursor (0, 0);
  if (LCD) lcd.print("^^ Ereigniskarte ^^");
  if (LOG) Serial.println( "^^ Ereigniskarte ^^" );
  if (LCD) lcd.setCursor (0, 2);
  if (LCD) lcd.print("\177    Abbruch    \176");
  if (LOG)Serial.println( "<<    Abbruch    >>" );
  if (LCD) lcd.setCursor (0, 3);
  if (LCD) lcd.print("vv Gemeinschaft  vv");
  if (LOG)Serial.println( "vv Gemeinschaft  vv" );
  short btnRes = 0;
  while (btnRes == 0) {
    delay (25);
    btnRes = checkForButton ();
  }

  if (btnRes == BTN_L || btnRes == BTN_R) {
    if (LCD) lcd.clear();
    updateKontostand();
    return;
  }

  if (btnRes == BTN_T) {
    aktEreigniskarte = zeigeKarte (ereignisKarten, aktEreigniskarte, "Ereignis:");
  } else {
    aktGemeinschaftskarte = zeigeKarte (gemeinschaftsKarten, aktGemeinschaftskarte, "Gemeinschaft:");
  }
  warte(1000);
  btnRes = 0;
  while (btnRes == 0) {
    delay (25);
    btnRes = checkForButton ();
  }
  warte(1000);
  if (LCD) lcd.clear();
  updateKontostand();
}

void warte (int zeit) {
  short ledOn = 0;
  for (int i = 0; i < zeit; i += 10) {
    if (ledOn++ >= 50) {
      ledOn = 0;
    }
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);

    if (ledOn < 25) {
      if (LCD) lcd.setCursor (19, 4);
      if (LCD) lcd.print (" ");
      if (LCD) lcd.setCursor (0, 4);
      if (LCD) lcd.print (" ");
    } else {
      if (LCD) lcd.setCursor ( 19, 4);
      if (LCD) lcd.print ("\364");
      if (LCD) lcd.setCursor (0, 4);
      if (LCD) lcd.print ("\364");
    }
    if (i % 300 == 0) {
      digitalWrite(LED_GREEN, HIGH);
    }
    if ((i + 150) % 300 == 0) {
      digitalWrite(LED_RED, HIGH);
    }
    delay (10);
  }
  if (LCD) lcd.setCursor ( 19, 4);
  if (LCD) lcd.print (" ");
  if (LCD) lcd.setCursor (0, 4);
  if (LCD) lcd.print (" ");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

}

void geldTransfer() {
  if (LCD) lcd.clear();
  if (LCD) lcd.setCursor (0, 0);
  if (LOG) Serial.print( "Ueberweise von ");
  if (LCD) lcd.print( "Ueberweise von ");
  if (LCD) lcd.setCursor (0, 1);
  if (spielerSender == 0) {
    if (LCD) lcd.print( "Bank");
    if (LOG) Serial.print( "Bank");
  } else {
    if (LCD) lcd.print("Sp. ");
    if (LCD) lcd.print(spielerSender);
    if (LOG) Serial.print( spielerSender);
  }
  if (LCD) lcd.print(" an ");
  if (LOG) Serial.print( " an ");
  if (spielerEmpf == 0) {
    if (LCD) lcd.print("Bank");
    if (LOG) Serial.println( "Bank");
  } else {
    if (LCD) lcd.print("Sp. ");
    if (LCD) lcd.print(spielerEmpf);
    if (LOG) Serial.println( spielerEmpf);
  }
  if (LCD) lcd.setCursor (0, 2);
  if (LCD) lcd.print("<< Abr. 0000   OK >>");
  if (LOG) Serial.println("<< Abr. 0000   OK >>  (1)");
  short currEdit = 2;
  int money = 0;
  warte (1500);

  renderSelector(currEdit, money);
  while (true) {
    short btnRes = checkForButton ();
    if (btnRes == BTN_R) {
      if (--currEdit < 0) {
        currEdit = 0;
      }
      renderSelector (currEdit, money);
      warte (500);
    } else if (btnRes == BTN_L) {
      if (++currEdit > 5) {
        currEdit = 5;
      }
      renderSelector (currEdit, money);
      warte (500);
    }
    if (btnRes == BTN_T || btnRes == BTN_B) {
      if (currEdit == 5) {
        //abbruch
        if (LCD) lcd.clear();
        if (LCD) lcd.setCursor (3, 3);
        if (LCD) lcd.print ("Abbruch");
        warte (1000);
        updateKontostand();
        return;
      } else if (currEdit == 0) {
        //ok
        sprintf(letzteTransaktion[letzteTransaktionIndex], " Sp %d \176 %d \176 Sp %d \0", spielerSender, money, spielerEmpf);
        letzteTransaktionIndex = (letzteTransaktionIndex + 1) % 4;
        konto[spielerSender] -= money;
        if (spielerEmpf != 0) {
          konto[spielerEmpf] += money;
        }
        if (LCD) lcd.clear();
        if (LCD) lcd.setCursor (3, 3);
        if (LCD) lcd.print ("Ueberwiesen!");
        if (LOG) Serial.println ("Ueberwiesen!");
        warte(1500);
        if (LCD) lcd.clear();
        updateKontostand();
        return;
      } else {
        if (btnRes == BTN_T) {
          money = money + pow(10, currEdit - 1);
          if (money > 9999) {
            money = 9999;
          }
        } else if (btnRes == BTN_B) {
          money = money - pow(10, currEdit - 1);
          if (money < 0) {
            money = 0;
          }
        }
        if (LCD) lcd.setCursor (8, 2);
        if (LCD) lcd.print("0000");
        short offset = 0;

        if (money < 10) {
          offset = 3;
        } else        if (money < 100) {
          offset = 2;
        } else  if (money < 1000) {
          offset = 1;
        }

        if (LCD) lcd.setCursor (8 + offset, 2);
        if (LCD) lcd.print(money);
        renderSelector(currEdit, money);
        warte (300);
      }
    }
    delay (4);
  }
}


void renderSelector (short currentEdit, int money) {
  if (LOG) Serial.print("\176 Abr. ");
  if (LOG) Serial.print(money);
  if (LOG) Serial.print(" OK \177 (");
  if (LOG) Serial.print(currentEdit);
  if (LOG) Serial.println(")");
  if (LCD) lcd.setCursor (0, 3);
  if (LCD) lcd.print ("                    ");
  if (currentEdit > 0 && currentEdit < 5) {
    if (LCD) lcd.setCursor (12 - currentEdit, 3);
    if (LCD) lcd.print("^");
  } else if (currentEdit == 0) {
    if (LCD) lcd.setCursor (15, 3);
    if (LCD) lcd.print("^^");
  } else {
    if (LCD) lcd.setCursor (3, 3);
    if (LCD) lcd.print("^^^^");
  }
}

void updateKontostand () {
  if (LCD) lcd.clear();
  for (short i = 1; i <= spielerzahl; i++) {
    if (i % 2 == 1) {
      if (LCD) lcd.setCursor (0, ((i - 1) / 2));
      if (LCD) lcd.print (konto[i]);
      if (LOG) Serial.print (konto[i]);
      if (spielerSender == spielerEmpf) {
        if (LOG) Serial.print ('\'');
      } else if (spielerSender == i) {
        if (LOG) Serial.print ('\177');
      } else if (spielerEmpf == i) {
        if (LOG) Serial.print ('\176');
      }
      if (LOG) Serial.print ("     ");
    } else {

      if (spielerSender == spielerEmpf) {
        if (LOG) Serial.print ('\'');
      } else if (spielerSender == i) {
        if (LOG) Serial.print ('\176');
      } else if (spielerEmpf == i) {
        if (LOG) Serial.print ('\177');
      }
      if (LCD) lcd.setCursor (15, (i / 2) - 1);
      if (LCD) lcd.print (konto[i]);
    }
  }
  if (LOG) Serial.println ("");
  updateBatterieanzeige();
  updateGeldTransferRichtung();
}

void updateBatterieanzeige() {
  short inputVoltage = analogRead(VOLTPIN);
  if (inputVoltage < 512) {
    if (LCD) lcd.setCursor (15, 3);
    if (LCD) lcd.print("usb");
  } else {
    float volt = (inputVoltage - 25.0) / 100.0;
    if (LCD) lcd.setCursor (15, 3);
    if (LCD) lcd.print(volt);
    if (LCD) lcd.setCursor (18, 3);
    if (LCD) lcd.print("V");
  }
}

String zeigeEinzelnerKontostand (short posY, short posX, short index) {
  if (LCD) {
    lcd.setCursor (posX, posY);
    lcd.print (ZS);
    short offsetX = 1 ;
    lcd.setCursor ( offsetX, posY);
    lcd.print (konto[index]);

  }
}

void updateGeldTransferRichtung() {
  if (LCD) lcd.setCursor (5, 4);
  if (spielerSender == 0) {
    if (LOG) Serial.print( 'B');
    if (LCD) lcd.print('B');
  } else {
    if (LOG) Serial.print(spielerSender);
    if (LCD) lcd.print(spielerSender);
  }

  if (spielerSender == spielerEmpf) {
    if (LOG) Serial.print("-card-" );
    if (LCD) lcd.print( "-card-" );
  } else {
    if (LOG) Serial.print("\176 $$ \176" );
    if (LCD) lcd.print( "\176 $$ \176" );
  }
  if (spielerEmpf == 0) {
    if (LOG) Serial.println( 'B');
    if (LCD) lcd.print('B');
  } else {
    if (LOG) Serial.println(spielerEmpf);
    if (LCD) lcd.print(spielerEmpf);
  }

}

short zeigeKarte (char ** karten, short aktuelleKarte, char * title) {
  if (LCD) lcd.clear();
  if (LOG)Serial.println( title );
  if (LCD) lcd.setCursor (0, 0);
  if (LCD) lcd.print (title);
  char* buff = karten [aktuelleKarte];
  char subbuff[21];
  memcpy( subbuff, &buff[0], 20 );
  subbuff[20] = '\0';
  if (LCD) lcd.setCursor (0, 1);
  if (LCD) lcd.print (subbuff);
  if (LOG)Serial.println( subbuff );
  if (LCD) lcd.setCursor (0, 2);
  if (LCD) lcd.print (&buff[20]);
  if (LOG)Serial.println( &buff[20] );
  if (LCD) lcd.setCursor (1, 3);
  if (LCD) lcd.print ("\177\177   Weiter   \176\176");
  return (aktuelleKarte + 1) % 16;
}


void zeigeWuerfel (short wert, short offset) {

  char * wg [3][6];
  switch ( wert ) {
    case 1:
      *wg [0] = "|   |";
      *wg [1] = "| o |";
      *wg [2] = "|   |";
      break;
    case 2:
      *wg [0] = "|  o|";
      *wg [1] = "|   |";
      *wg [2] = "|o  |";
      break;
    case 3:
      *wg [0] = "|  o|";
      *wg [1] = "| o |";
      *wg [2] = "|o  |";
      break;
    case 4:
      *wg [0] = "|o o|";
      *wg [1] = "|   |";
      *wg [2] = "|o o|";
      break;
    case 5:
      *wg [0] = "|o o|";
      *wg [1] = "| o |";
      *wg [2] = "|o o|";
      break;
    default:
      *wg [0] = "|o o|";
      *wg [1] = "|o o|";
      *wg [2] = "|o o|";
      break;
  }

  if (LCD) lcd.setCursor ( offset, 0);
  if (LCD) lcd.print (*wg[0]);
  if (LCD) lcd.setCursor (offset, 1);
  if (LCD) lcd.print (*wg[1]);
  if (LCD) lcd.setCursor (offset, 2);
  if (LCD) lcd.print (*wg[2]);
  if (LOG)Serial.println ("---");
  if (LOG)Serial.println (*wg[0] );
  if (LOG)Serial.println (*wg[1] );
  if (LOG)Serial.println (*wg[2] );
  if (LOG)Serial.println ("---");

}

void zeigeLetzteTransaktionen() {
  if (LCD) lcd.clear();
  for (short i = 0; i < 4; i++) {
    if (LCD) lcd.setCursor(0, i);
    if (LCD) lcd.print(letzteTransaktion [(letzteTransaktionIndex + i) % 4]);
  }
  warte(3000);
  while (checkForButton () == 0) {
    delay (25);
  }
  warte(1000);
  updateKontostand();
}

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(char * *array, size_t n)
{
  size_t i;
  for (i = 0; i < n - 1; i++)
  {
    size_t j = i + random(9999999) / (9999999 / (n - i) + 1);
    char * t = array[j];
    array[j] = array[i];
    array[i] = t;
  }

}

