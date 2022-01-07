#include <Stepper.h>
#include <DallasTemperature.h>
#include <OneWire.h>



//this class works like a container for the values registered from the arduino, with a specific function called "inserisciValori" it concatenates 
//each string inserted to form a unique string with all the values
class Pacchetto {
  private:

    String valori = "null" ;

  public:

    String getValori() {
      return this->valori;
    }

    //function to insert into the string
    void inserisciValori(String soggetto, String dato) {
      
      //check if space's character is present in the string 
      int ind_dato_err = dato.indexOf(' ');
      int ind_soggetto_err = soggetto.indexOf(' ');
      if(ind_dato_err >0){
        dato.remove(ind_dato_err,1);         
      }
      else if(ind_soggetto_err){
        soggetto.remove(ind_soggetto_err,1);        
      }
      //-------
      
      int i = 0;

      // 
      if ( valori == "null") {
        valori = soggetto + ':' + dato + ';';
      }
      else {
        //deve determinare se dato già presente
        int indexofsogg = valori.indexOf(soggetto);

        if (indexofsogg >= 0) {
         
          //il dato è già presente
          //deve individuare il ':' e sostituire il dato
          int indexofFine = valori.indexOf('-',indexofsogg);
          
          
          if (indexofFine > 0) {
            //if true then the value inserted is not the last
            //so it gets the substring
            String sub1 = valori.substring(indexofsogg, indexofFine);
        
            //and replaces the old with a new substring adding the new value
            valori.replace(sub1, soggetto + ':' + dato);
            
          }
          else {
            //altrimenti dato è ultimo
            indexofFine = valori.indexOf(';',indexofsogg);
            //ottiene substring            
            String sub1 = valori.substring(indexofsogg, indexofFine);
            //sostituisce intera substring con nuova            
            valori.replace(sub1, soggetto + ':' + dato);
          }

        }
        else {
          //it gets here if the value to insert is last in the string
          //so it must replace the ';' with '-'
          valori.replace(';','-');
          
          //and then adds the next value at the end
          valori = valori + soggetto+':' + dato + ';';
        }
      }
    }

    void Reset() {
      this->valori = "null";
    }
};

//GLOBAL VARIABLES
//pacchetto dati
Pacchetto pack;

//pin thermistor
#define ONE_WIRE_BUS 6

OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

//stringa input
String stringa = "";

//valore in input per dispositivi
int valore = 0;

//var per temperatura thermistor
int temp = 0;

//semaforo per elaborare la stringa valori dispositivi
bool dati = false;

//dispositivo
String soggetto = "";

//steps attuali della tenda
int steps = 0;

//multiplier tenda
float multiplier = 1.2;

//relay pin
#define relay  7

//motor's steps
int stepsPerRevolution = 2048;

//max speed 10
int motSpeed = 10;

//crea l'istanza steppermotor della classe Stepper
Stepper steppermotor(stepsPerRevolution, 2, 4, 3, 5);

void setup()
{
  //Sets the data rate in bits per second (baud) for serial data transmission
  Serial.begin(9600);

  //tiene spenta la corrente sul relay agli stepper
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  //inizializza pin thermistor
  sensors.begin();

  //imposta velocità
  steppermotor.setSpeed(motSpeed);

  //inizializza tenda
  pack.inserisciValori("seekbar(tenda)","0");

}
void loop()
{

 //rileva temperatura
  sensors.requestTemperatures();
  int varTemp = sensors.getTempCByIndex(0);

  //inserisce valori temperatura solo se c'è variazione
  if (varTemp != temp) {
    //entra solo se valora cambia
    temp = sensors.getTempCByIndex(0);
    pack.inserisciValori("temperatura", String(temp));

    //invia valori ad APP
    Serial.println( pack.getValori());
  }

  //se ci sono dati li legge
  if (Serial.available()) {
    stringa = Serial.readString();
    dati = true;
  }



  //SEMPLICE DIVISIONE DEI DATI IN INPUT IN SOGGETTO E VALORE
  //SE LA STRINGA CONTIENE ':'
  if (dati == true && stringa.indexOf(':') != -1) {
    int index = stringa.indexOf(':');
    soggetto = stringa.substring(0, index);
    valore = stringa.substring(index + 1).toInt();
    dati = false;
  }
  else {
    dati = false;
  }
  //_---------------------------------------------------

  //effettua controllo su input
  if (stringa == "connesso") {
   
    //invia valori ad APP
    Serial.println( pack.getValori());

    //resetstringa
    stringa = "";
    dati = false;
  }
  else if (soggetto == "tenda") {
    //Serial.println("entra in condizione tenda");

    //se soggetto comando è tenda allora imposta input per steps
    if (valore < steps) {
      digitalWrite(relay, HIGH);
      steppermotor.step( int(((valore - steps)*multiplier) * stepsPerRevolution) );
    }
    else if (valore > steps) {
      digitalWrite(relay, HIGH);
      steppermotor.step( int(((valore - steps)*multiplier) * stepsPerRevolution) );
    }
    else {
      //do nothing
    }
    //aggiorna gli step
    steps = valore;
    
    //aggiorna dati in stringa dati per BT
    pack.inserisciValori("seekbar(tenda)", String(steps));

    //invia valori ad APP
    Serial.println( pack.getValori());

    //delay per non far scivolare la tenda
    delay(1500);

    //chiude relay
    digitalWrite(relay, LOW);

    //reset valori
    soggetto = "";
    valore = "";
    dati = false;
    stringa = "";
  }
  else {
    //azzera la stringa in caso di comando non riconosciuto
    stringa = "";
  }

  
  

}
