// programme de commande du réseau de Sol'Ex
// version 1.2b
// Jean Brunet  le 17/01/2023
// Modifié par Stéphane Ferier pour poulie 60 dents GT2 le 03/03/2023 avec l'accord de Jean.

#include <Wire.h>
#include <WiFi.h>

#define MOTOR_PIN_1  19   // Blue   - 28BYJ48 pin 1
#define  MOTOR_PIN_3  18  // Pink   - 28BYJ48 pin 2
#define  MOTOR_PIN_2   5  // Yellow - 28BYJ48 pin 3
#define MOTOR_PIN_4  17   //  Orange - 28BYJ48 pin 4

//#define EEPROM_SDA 23
//#define EEPROM_SCL 19
#define adresse_EEPROM 0x50   // eeprom externe 24LC01B
#define Adr1_RW_EEPROM 0
#define Adr2_RW_EEPROM 1
#define iNDICE_TABLE_RAY 30

// raies programmées
#define RAIEHalpha 1230
#define RAIENa 1060
#define RAIEMg 910
#define RAIEHb 850
#define RAIECa 670

 // rapport entre les roues 3.75 poulie 16, roue 60 en GT2
 // 2048 * 3.75 = 7680 1 tour roue
 // 1 ° = 21.333 pas, 70 ° = 1493 pas
 // conversion angströms par  pas. 1 nanomètre = 10 angströms
 // Ha à 6562.81
 // Hb à 4861.34
 // ca à 4307.74
 // sodium  5 895,924 et 5 889,950
 // Mg à 5183.62 et 5167.22

const char* ssid     = "SolEx";
const char* password = "solex1234";  // A MODIFIER si plusieurs Sol'Ex proches
WiFiServer server(80);

String header;

const unsigned int MAX_STEPS = 4000;  // limite de course du réseau
const unsigned int LITTLE_STEP = 2;  // petit déplacement
const unsigned int MIDDLE_STEP = 10;  // moyen
const unsigned int FAST_STEP = 25;    // par écran

const unsigned long STEP_DELAY_MICROSEC = 4000;  // vitesse moteur

// tableau des raies qui seront éditées dans la page web
String raies[iNDICE_TABLE_RAY] = {"y O2 898.765","Z O2 822.696","A O2 759.370","B O2 686.719","C H&#945; 656.281","a O2 627.661","D1 Na 589.592","D2 Na 588.995","D3 ou d He 587.562","e Hg 546.073","E2 Fe 527.039","b1 Mg 518.362","b2 Mg 517.270","b3 Fe 516.891","b4 Mg 516.722","c Fe 495.761","F H&#946; 486.134","d Fe 466.814","e Fe 438.355","G' ou f H&#947; 434.047","G Fe 430.790","G Ca 430.774","h H&#948; 410.175","H Ca+ 396.847","K Ca+ 393.366","L Fe 382.044","N Fe 358.121","P Ti+ 336.112","T Fe 302.108","t Ni 299.444"};
int cycle = 0;
int corrPas = 20; // correction au changement de sens, jeu des pignons
int memoSelectRaie = 4;  // indice du sélecteur de raies sur hydrogène
String fixNano = "0";
String raieString;
bool MotorFinish = true;

enum SetMotor {
  stopM = 0,
  slow = 1,
  fast = 2,
  screen = 3,
  fix = 4
} setMotor;

enum Direction {
    forward = 1,
    backward = -1
} direction;

int position = 0;
int setMove = 0;
int fixPos = 0;
unsigned long last_step_time;
int nano = 0;
bool zerofix = false;
int address = 0x50;

/* ---------------------------
setup
-------------------------------*/
void setup() 
{
    // Initialize serial port I/O.
    // Serial.begin(115200); // seulement pour déboggage
    // Initialize eeprom
   // Wire.begin(EEPROM_SDA, EEPROM_SCL);
     Wire.begin();
    // Initialize motor control pins...
    pinMode(MOTOR_PIN_1, OUTPUT);
    pinMode(MOTOR_PIN_2, OUTPUT);
    pinMode(MOTOR_PIN_3, OUTPUT);
    pinMode(MOTOR_PIN_4, OUTPUT);

    direction = forward;
    position = 0;
    last_step_time = 0L;
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    server.begin();
     // lecture en mémoire EEPROM de la dernière position enregistrée.
    byte val1 = readbyte(adresse_EEPROM, Adr1_RW_EEPROM);
    byte val2 = readbyte(adresse_EEPROM, Adr2_RW_EEPROM);
    position = word(val1, val2);
}

/* --------------------------------
Boucle de traitement
-------------------------------------*/
void loop() 
{
	Wificommmand();  // affichage de la page web et traitement des retours utilisateur
	 // Lors de la recherche de l'ordre zéro du réseau (réglage coché dans le formulaire web), la position n'est pas actualisée. Seul le nombre de pas est modifié

}
void setupMotor()
{
	if (setMotor > stopM){  // si mouvement demandé
		if(setMotor == slow){    // si petit décalage
			setMove = LITTLE_STEP;
		}
		if(setMotor == fast){    // si moyen décalage
			setMove = MIDDLE_STEP;
		}
	   if(setMotor == screen){     // si par page d'écran 
			setMove = FAST_STEP;
		}
		if(setMotor == fix){       // nombre de pas demandé
			if(fixPos > position){  // si mouvement vers la gauche
				correctionPas(forward);  // correction du jeu des pignons
				direction = forward;
				setMove = fixPos - position;    
			}else if(fixPos < position){
				correctionPas(backward);  // correction du jeu des pignons      
				direction = backward;
				setMove = position -  fixPos;
			}
		}

		if(zerofix == false){  // réglage n'est pas coché dans le formulaire web, la position est actualisée
			if(direction == forward){
				if( position + setMove > MAX_STEPS){  // limite de course max
					setMove =  MAX_STEPS - position;
				}    
			position += setMove;   // nouvelle position
		 
			}else{
				if(position - setMove < 0){   // limite de course zéro
					setMove = position;       
				}
			position -= setMove;    // nouvelle position     
			}
		}

		commandMotor(setMove);  //  moteur actif
    raieString = convertPasNano(position);
    if(zerofix == true || position == 0 || raieString.substring(0,1) == "-"){
      raieString = "";
    }
		byte hi  = highByte(position);  // sauvegarde en eeprom de la dernière position
		byte low = lowByte(position);
		writebyte(adresse_EEPROM, Adr1_RW_EEPROM, hi);
		writebyte(adresse_EEPROM, Adr2_RW_EEPROM, low);
		setMotor = stopM;        // arrêt moteur
	}
}
/*---------------------------------------------------------------------------------------------
 Page web et traitement des commandes envoyées par l'utilisateur
------------------------------------------------------------------------------------------------*/
void Wificommmand()
{
	zerofix = false;
	WiFiClient client = server.available();   // Listen for incoming clients
   
	if (client){ 
		//Serial.println("New Client.");          // print a message out in the serial port
    while (client.connected()){            // loop while the client's connected
		if (client.available()) {             // if there's bytes to read from the client,
			char c = client.read();             // read a byte, then
			header += c;
			if (c == '\n') // si c'est la fin de la commande reçue
			{          
				client.println("HTTP/1.1 200 OK");
				client.println("Content-type:text/html");
				client.println("Connection: close");
				client.println();
        if (header.indexOf("setzero") >= 0) {  // réglage est coché, la position n'est pas actualisée
					zerofix = true;
				}
				if (header.indexOf("Left") >= 0) {
					correctionPas(backward);
					direction = backward;
					setMotor = slow;
				}else if (header.indexOf("Right") >= 0) {   // corrections manuelles de déplacement
					correctionPas(forward);
					direction = forward;
					setMotor = slow;
				 }else if (header.indexOf("fastRt") >= 0) {
				   correctionPas(forward);
					direction = forward;
					setMotor = fast;				 
				 }else if (header.indexOf("fastLt") >= 0) {
				   correctionPas(backward);
					direction = backward;
					setMotor = fast;
				 }else if (header.indexOf("screenR") >= 0) {
				   correctionPas(forward);
					direction = forward;
					setMotor = screen; 
				 }else if (header.indexOf("screenL") >= 0) {
				  correctionPas(backward);
					direction = backward;
					setMotor = screen; 
				 }else if (header.indexOf("envoi") >= 0) {   // position sur une raie donnée
         // teste les caractères, doivent être numériques et positifs
					fixNano = String(headerReturn(header,"pos"));
          
					fixPos = convertNanoPas(headerReturn(header,"pos"));
					if(fixPos >= 0){
						setMotor = fix;            
					}          
				 }else if (header.indexOf("rHa")>= 0) {  // raie H alpha
					fixPos = RAIEHalpha;  
					setMotor = fix; 
				 }else if (header.indexOf("rNa")>= 0) {  // raie Na
					fixPos = RAIENa;
					setMotor = fix; 
				 }else if (header.indexOf("rMg")>= 0) {   // raie Mg
					fixPos = RAIEMg;
					setMotor = fix; 
				 }else if (header.indexOf("rHb")>= 0) {    // raie H beta
					fixPos = RAIEHb;
					setMotor = fix; 
				 }else if (header.indexOf("rCa")>= 0) {    // raie Ca
					fixPos = RAIECa;
					setMotor = fix; 
				 }else if (header.indexOf("ordreZ")>= 0) {  // pour fixer l'ordre 0
            if (zerofix == false){   // retour sur pas 0, soit ordre zero, réglage est décoché
              fixPos = 0;  // retour sur zéro
              setMotor = fix;
            } else{
              fixPos = 0;  // on fixe la position à l'ordre zéro.
              position = 0;
            }         				
				 }else if (header.indexOf("select")>= 0) {  // raie sélectionnée dans la liste de raies
					// extrait longueur d'onde en nm
				   String nom = "tab";
				   int addr_start = header.indexOf(nom + "=")+ nom.length() + 1;
				   int addr_end = header.indexOf("&",addr_start);
				   if(addr_end == -1){
						addr_end = header.length();
				   }
				   String id = header.substring(addr_start, addr_end) ;  // indice du tableau raies retourné par le formulaire
				   int idInt = id.toInt();
				   memoSelectRaie = idInt;  // pour affichage dans formulaire le SELECT
				   int lg = raies[idInt].length();  
				   String mem = raies[idInt].substring(lg - 7,lg);
				   nano = mem.toFloat();   
				   fixPos = convertNanoPas(nano);    // donnée converties du nano en pas moteur.
				   setMotor = fix; 
				}
        //correctionPas(direction);
       // while(MotorFinish == false){}   // attend que la course moteur soit terminée     
	      setupMotor();
       	client.println(webPage());  // page web

				// The HTTP response ends with another blank line
			 
      	client.println();
				client.stop();
				header = "";
				}
			}
		}
	}
}

/* --------------------
Affichage page web
----------------------*/
String webPage()
{
  String Ch;
  Ch = "<!DOCTYPE html><html>";
  Ch +="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  Ch +="<style>.bouton {background-color: #504caf; border: none; color: white; width:105px; height:50px; margin: 2px;  padding: 10px; text-decoration: none; text-align: center; display: inline-block; font-size: 15px; cursor: pointer; }"; 
  Ch +=".boutonLite {background-color: #504caf; border: none; color: white; width:65px; height:65px; margin: 2px;  padding: 10px; text-decoration: none; text-align: center; display: inline-block; font-size: 22px; cursor: pointer; }"; 
  Ch +=".texteEntree {background-color: #EEEEEE; border: none; color: black; width:165px; height:30px;  padding: 10px; text-decoration: none; text-align: left; display: inline-block; font-size: 22px; cursor: pointer; }</style>"; 
  Ch +="</head><body>";    
  
  Ch +="<div style='text-align: center '><H2>Sol'Ex</H2><br><form enctype='multipart/form data' method=GET>"; 
  Ch +="<input button class='boutonLite' type='submit' name='fastLt' value = '<<' />"; 
  Ch +="<input button class='boutonLite' type='submit' name='Left' value = '<' />"; 
  Ch +="<input button class='boutonLite' type='submit' name='Right' value = '>' />";   
  Ch +="<input button class='boutonLite' type='submit' name='fastRt' value = '>>' />"; 
  Ch +="<br><input button class='boutonLite' type='submit' name='screenL' value = '<<<' />";    
  Ch +="<input button class='boutonLite' type='submit' name='screenR' value = '>>>' /></center><br>";    
  Ch +="<H3>Raie (nm) " + raieString + "&ensp;&ensp;&ensp;pas " + String(position) + "</H3></center><br>"; 
  
  Ch +="<input type = 'text' class ='texteEntree' id='donnee' name='pos' maxlength='7' placeholder='RAIE' spellcheck='false' oninput='this.value = this.value.replace(/[^0-9.]/g, '').replace(/(\..*)\./g, '$1');' /> ";
  
  Ch +="<input button class='bouton' type='submit' name='envoi' value = 'Envoi' />";     
  Ch +="<br><br><input button class='boutonLite' type='submit' name='rHa' value = 'H &#945 ' />"; 
  Ch +="<input button class='boutonLite' type='submit' name='rNa' value = 'Na' />"; 
  Ch +="<input button class='boutonLite' type='submit' name='rMg' value = 'Mg' />"; 
  Ch +="<br><input button class='boutonLite' type='submit' name='rHb' value = 'H &#946 ' />"; 
  Ch +="<input button class='boutonLite' type='submit' name='rCa' value = 'Ca' />"; 
  Ch +="<br><br><select class ='texteEntree' id='tabraies' name='tab' style='height:50px ' />"; 
       // affiche index déjà choisi 
      for (int i=0; i<iNDICE_TABLE_RAY; i++){
       if (i == memoSelectRaie){
  Ch +="<option value='" + String(i) + "' selected>" + raies[i] + "</option>";
       }else{
  Ch +="<option value='" + String(i)+ "'>" + raies[i] + "</option>";
       }
      }
  Ch +="</select>&ensp; <input button class='bouton' type='submit' name='selection' value = 'S&eacute;lection' />"; 
      String op = ""; 
      if (zerofix == true){
       op = "checked= 'yes'" ;
      }  
  Ch +="<br><br>R&eacute;glage <input type='checkbox' name='setzero' " + op + " value='setzero'/>";  
  Ch +="<input button class='bouton' type='submit' name='ordreZ' value = 'Ordre 0' />";  
  Ch +="</form> </div></body></html>"; 
  return Ch;
}

/*---------------------------------------------------------
Polynome de correction de la courbe
------------------------------------------------------------*/
int convertNanoPas(float nano)
{
// polynome du 2nd degré. y = a + bx + cx au carré
	float y;
//	int res;
	float a = 154.9	;
	float b = 0.7978;
	float c = 0.001267;

	float k = pow(nano,2);
	y = a + (b * nano) + (c * k);
//	res = int(y);
	return int(y);
}

String convertPasNano(int pas)
{
  float y;
	float a = -24.573;
	float b = 0.7153;
	float c = -0.00013;

  float k = pow(pas,2);
  y = a + (b * pas) + (c * k);
  return String(y);
}

/* ----------------------------------------------------------------------------------------------------------
Routine de correction du jeu des pignons du moteur au changement de sens
--------------------------------------------------------------------------------------------------------------*/
void correctionPas(int sens)
{ 
return;   
int memdirection = direction;
	if(sens == direction){  // même direction , pas de correction
		return;
	}
	if(direction == forward){ 
		direction =  backward;  // changement de sens
		commandMotor(corrPas); // rattrapage du jeu des pignons
	}else{
		direction = forward;  // changement de sens
		commandMotor(corrPas); // rattrapage du jeu des pignons
	}
	if(memdirection == 1){  // restaure sens
		direction = forward;    
	} else{
		direction = backward;
	}              
}

/* ----------------------------------------------------------------------
Retourne un float avec le nombre de la raie choisie
---------------------------------------------------------------------------*/
float headerReturn(String head, String nom)
{
	int addr_start = header.indexOf(nom + "=")+ nom.length() + 1;
	int addr_end = header.indexOf("&",addr_start);
	return header.substring(addr_start, addr_end).toFloat();
}

/*-----------------------------------------
 Routines moteur 28BYJ48
 -------------------------------------------*/
void commandMotor(unsigned int st)
{
  MotorFinish = false;
	for(int i=0;i<st;i++){
		Motor();
		// tempo pour fixer la vitesse du moteur
		last_step_time = micros();
		while(micros() - last_step_time < STEP_DELAY_MICROSEC){}
	}
	stop();  // arrêt du moteur
   MotorFinish = true;
}

void Motor()
{
	if (direction == backward){
		stepMotor();           
		cycle++;
		if (cycle == 4){ 
			cycle = 0;
		}
	} else{
		stepMotor();
		if (cycle == 0){ 
			cycle = 4 ;
		}
		cycle--;
	}
}

void stepMotor()
{
	switch (cycle){
		case 0: // 1010
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
			break;
		case 1: // 0110
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, HIGH);
			digitalWrite(MOTOR_PIN_4, LOW);
			break;
		case 2: // 0101
			digitalWrite(MOTOR_PIN_1, LOW);
			digitalWrite(MOTOR_PIN_2, HIGH);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
			break;
		case 3: // 1001
			digitalWrite(MOTOR_PIN_1, HIGH);
			digitalWrite(MOTOR_PIN_2, LOW);
			digitalWrite(MOTOR_PIN_3, LOW);
			digitalWrite(MOTOR_PIN_4, HIGH);
			break;
	}
}

void stop() 
{
// met le moteur à l'arrêt, sans alimentation. (limite la consommation)
	digitalWrite(MOTOR_PIN_1, LOW);
	digitalWrite(MOTOR_PIN_2, LOW);
	digitalWrite(MOTOR_PIN_3, LOW);
	digitalWrite(MOTOR_PIN_4, LOW);
}

/* -------------------------------------------
Lecture de l'EEPROM 24LC01B
-----------------------------------------------*/
byte readbyte(int adressei2c, unsigned int adresseMem )
{
	byte lecture = 0;

	Wire.beginTransmission(adressei2c);  // adresse i2c du 24LC01B
	Wire.write(adresseMem); 		// adresse de l'octet qu'on veut lire
	Wire.endTransmission();
	Wire.requestFrom(adressei2c, 1); // demande lecture de l'octet
	delay(5);
	if (Wire.available()) {  
		lecture = Wire.read(); // lecture de l'info
	}
  return lecture;
}

/* -------------------------------------------
Ecriture de l'EEPROM 24LC01B
-----------------------------------------------*/
void writebyte(int adressei2c, unsigned int adresseMem, byte data )
{
	 Wire.beginTransmission(adressei2c); //adresse I2C de l'EEPROM
	 Wire.write(adresseMem); // adresse de l'octet à modifier
	 Wire.write(data);		 // écrture de l'octet
	 Wire.endTransmission(); // fin de la transmission I2C
	 delay(5);
}
