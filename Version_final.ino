/* Sistema de iluminación leds a través de una contraseña introducida por teclado y mostrada en una pantalla lcd.
   Primera versión operativa, 2/5/2019 
   Grupo Lis Muñoz, Teresa Ramírez, Silvia Saeta */


//Librerías utilizadas

#define N 5 //Número de caracteres de la contraseña
#include <TimeLib.h> //Librería para calcular intervalos de tiempo transcurridos
#include <Keypad.h> //Librería para manejar el teclado matricial
#include <LiquidCrystal_I2C.h> //Librería para usar la pantalla lcd


//Definición de la estructura tipo tiempo

typedef struct
{
float tiempo_inicial, tiempo_final;
float intervalo_de_tiempo;
}tiempo;


//Funciones prototipo

int medir_distancias (int, int);
int elegir_pin (char);
void encender (int);
void apagar (int);
void escribir_en_lcd (char, int *, int *);
int comprobacion_clave (char [], char []);
tiempo inicializar_tiempo (tiempo);
tiempo tiempo_transcurrido (tiempo);
char borrar_caracter (char [], int *, int *);
void borrar_todo (char [], int *, int *);


//Declaración de variables

int distancia, correcto, contador_de_claves, salida, i;
int c_filas, c_columnas;
int posicion_x, posicion_y=1;
int posicion_en_cadena;
int LedPin, LedPinAnterior=-1;
tiempo t; //Sirve para contar el tiempo transcurrido sin pulsar una tecla
char cadena[N], aux='\0', input='\0';
char codigo[N]={'1','2','3','4','5'};  //Se define el código correcto.


//Declaración de pines

const int LedPinAmarillo=A0;
const int LedPinAzul=A1;
const int LedPinVerde=A2;
const int LedPinRojo=A3;

const int pinBuzzer=12;

const int TriggerPin=11;
const int EchoPin=10;

const byte filas=4, columnas=4;
char matriz[filas][columnas]={
				     {'1','2','3','A'},
				     {'4','5','6','B'},
				     {'7','8','9','C'},
				     {'*','0','#','D'},
					         };
byte pinesFilas[filas]={9,8,7,6};
byte pinesColumnas[columnas]={5,4,3,2};
Keypad teclado = Keypad(makeKeymap(matriz), pinesFilas, pinesColumnas, filas, columnas);

LiquidCrystal_I2C lcd(0x3F,16,2); //Está por defecto configurado en A4 (SDA) y A5 (SCL).



void setup ()
{
	Serial.begin(9600);
	pinMode (LedPinAmarillo, OUTPUT);
  pinMode (LedPinVerde, OUTPUT);
  pinMode (LedPinAzul, OUTPUT);
  pinMode (LedPinRojo, OUTPUT);
	pinMode (TriggerPin, OUTPUT);
	pinMode (EchoPin, INPUT);
	lcd.init();
  lcd.noBacklight();
  lcd.clear();
}



void loop ()
{

//Espera hasta que reciba un valor del código en C por el puerto serie
while (input=='\0')
{
  if(Serial.available()>0)
  {
    input=Serial.read(); //Guarda el valor enviado en la variable input
  }
}
LedPin = elegir_pin (input); //Asigna un valor a LedPin dependiendo del que ha llegado del puerto serie

posicion_x=0;
contador_de_claves=0;
salida=0;

distancia = medir_distancias (TriggerPin , EchoPin);

if (distancia < 10) //El ultrasonidos ha detectado algo a menos de 10 centímetros
{
  //Se enciende la pantalla, se coloca el cursor en la posicion (0,0) y se solicita el código
	lcd.clear ();
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print ("Introduce codigo");

  t=inicializar_tiempo(t);


	while (t.intervalo_de_tiempo<=10&&salida==0) //Espera la pulsación de una tecla durante los primeros 10 segundos
	{
		t=tiempo_transcurrido(t);
    aux =teclado.getKey();
    if (aux)
    {
      escribir_en_lcd (aux, &posicion_x, &posicion_y); //Imprime en la pantalla la tecla pulsada
      cadena[0]=aux;
      salida=1;
    }
	}

	if (salida==1) //Se ha pulsado un botón durante los primeros 10 segundos
	{
    //Se pide y se comprueba la clave, el bucle termina si se ha introducido la clave correcta o si se han escrito 5 claves erróneas
		do
		{
      posicion_en_cadena=1;
			for (i=1;i<N;i++)
			{
        aux='\0';
        while (!aux)
        {
				  aux=teclado.getKey();
        }
        if (aux)
        {
          //Se comprueba si la tecla pulsada es un asterisco para borrar el caracter anterior
          if (aux=='*')
          {
            aux = borrar_caracter (cadena,&posicion_en_cadena,&posicion_x);
            escribir_en_lcd (aux, &posicion_x, &posicion_y);
            cadena[posicion_en_cadena]=aux;
            i=i-2; //Se disminuye la i para poder continuar con el bucle en la función correcta
            posicion_x--;
          }
          else{
          //Se comprueba si la tecla pulsada es una almohadilla para borrar los caracteres escritos hasta el momento
          if (aux=='#')
          {
            borrar_todo (cadena,&posicion_en_cadena,&posicion_x);
            i=0; //Se pone i a 0 para que el for comience con el 1
            aux='\0';
            //Se lee el primer caracter de la cadena aquí porque el for está hecho empezando en 1
            while (!aux)
            {
            aux=teclado.getKey();
            }
            if (aux)
            {
              escribir_en_lcd (aux, &posicion_x, &posicion_y);
              cadena[posicion_en_cadena]=aux;
              posicion_en_cadena++;
            }
          }
          else
          {
				    escribir_en_lcd (aux, &posicion_x, &posicion_y);
            if (i==4)
              delay(500);
            cadena[posicion_en_cadena]=aux;
            posicion_en_cadena++;
          }
          }
        }
			}
			correcto = comprobacion_clave (cadena, codigo); //Se comprueba la clave mediante la función comprobacion_clave
			if (correcto == 0) //Se ha introducido una clave incorrecta
			{
        //Suena el buzzer durante 1000 ms, borra la clave introducida y solicita el código de nuevo
				tone (pinBuzzer, 600, 1000);
				lcd.clear ();
        posicion_x=0;
        lcd.setCursor(0,0);
        lcd.print ("Introduce codigo");


        //Guarda la siguiente tecla pulsada en la primera posición de la cadena        
        aux='\0';
        while (!aux)
        {
          aux=teclado.getKey();
        }
        if (aux)
        { 
          escribir_en_lcd (aux, &posicion_x, &posicion_y);
          cadena[0]=aux;
         }
			 }
       
			contador_de_claves++; //Aumenta el contador del número de claves introducidas
     
		} while (correcto==0 && contador_de_claves <5);

    //Se han introducido 5 claves incorrectas
		if (correcto == 0)
		{
			lcd.noBacklight(); //Se apaga la luz de la pantalla
      lcd.clear();
		}

    //Se ha introducido la clave correcta
		if (correcto == 1)
		{
      tone (pinBuzzer, 1500, 100);
      delay (200);
      tone (pinBuzzer, 1500, 100);
      lcd.clear();
      lcd.print("Codigo correcto");

      if (LedPin!=LedPinAnterior)
      {
			  encender(LedPin); //Se enciende el led elegido
			  apagar (LedPinAnterior);
        LedPinAnterior = LedPin;
      }
      else
			  apagar(LedPin); //Se apaga el led elegido
     
      lcd.noBacklight();
      lcd.clear();
		}
	}
	else //No se ha pulsado un botón durante los primeros 10 segundos y la pantalla se apaga
	{
		lcd.noBacklight();
    lcd.clear();
	}
  input='\0'; //Se vuelve a poner input a '\0' para poder volver a comparar la próxima vez
}
}



//Definición de funciones


int medir_distancias (int TriggerPin, int EchoPin) //Mide la distancia con el sensor ultrasonidos
{
	long tiempo, distancia_cm;
	digitalWrite (TriggerPin, LOW);
	delayMicroseconds (4);
	digitalWrite (TriggerPin, HIGH);
	delayMicroseconds (10); //El Trigger envía una señal durante 10 microsegundos.
	digitalWrite (TriggerPin, LOW);

	tiempo = pulseIn (EchoPin, HIGH); //Mide el tiempo que tarda el EchoPin en pasar de LOW a HIGH. Se mide en microsegundos.
	distancia_cm =  (tiempo * 1 / 29.2)/ 2; //Se calcula la distancia a partir del tiempo y la velocidad del sonido y se pasa a centímetros.
  /* La velocidad del sonido es 343 m/s. Para ver los centímetros que recorre cada microsegundo: 343 m/s * 100 cm/m * 1/1000000 s/microsegundos.
     Así queda que el sonido recorre 1/29.2 cm cada microsegundo. 
     Para saber la distancia que recorre multiplicamos el tiempo del pulseIn por los centímetros recorridos cada microsegundo. Este valor hay que dividirlo entre dos
     porque la distancia que mide es el doble que la real ya que la señal tiene que ir y volver*/
	return (distancia_cm);
}

int elegir_pin (char c)
{
  int resultado;

  switch (c)
  {
    case '0':resultado=LedPinAmarillo; break;
    case '1':resultado=LedPinAzul; break;
    case '2':resultado=LedPinVerde; break;
    case '3':resultado=LedPinRojo; break;
  }
  return (resultado);
}

void encender (int LedPin) //Enciende el led elegido
{
	digitalWrite (LedPin, HIGH);
}

void apagar (int LedPin) //Apaga el led elegido
{
	digitalWrite (LedPin, LOW);
}

void escribir_en_lcd (char caracter, int *posicion_x, int *posicion_y) //Se escribe el caracter que llega como entrada en la pantalla
{
	int i;

	lcd.setCursor (*posicion_x, *posicion_y); //Se coloca el cursor en el (posicion_x,1).
	lcd.print (caracter); //Se imprime el caracter en la pantalla.
	(*posicion_x)++; //Se aumenta la posición de las x para continuar escribiendo.
}

int comprobacion_clave (char cadena[], char codigo[]) //Se compara la clave introducida con la contraseña real
{
  int resultado=1, i;

  for (i=0;i<N;i++)
  {
    if (cadena[i]!=codigo[i])
      resultado=0;
  }
  return (resultado); //Devuelve un 0 si la contraseña es incorrecta y un 1 si es correcta
}

tiempo inicializar_tiempo (tiempo t) //Inicializa el tiempo inicial y el final a 0
{
t.tiempo_inicial=now();
t.tiempo_final=now();
t.intervalo_de_tiempo=(float) (t.tiempo_final-t.tiempo_inicial);
return t;
}

tiempo tiempo_transcurrido (tiempo t) //Guarda el tiempo actual en tiempo_final y calcula el intervalo de tiempo
{
t.tiempo_final=now();
t.intervalo_de_tiempo= (float) (t.tiempo_final-t.tiempo_inicial);
return t;
}

char borrar_caracter (char cadena[], int *n, int *pos_x) //Borra el último caracter escrito
{
char aux;
cadena[(*n)-1]= ' ';
aux=cadena[(*n)-1];
(*n)--;
(*pos_x)=(*pos_x)-1;
return aux;
}

void borrar_todo (char cadena[], int *n, int *pos_x) //Borra todos los caracteres escritos hasta el momento
{
int i, posicion_borrado_x=0;
for (i=0;i<(*n);i++)
{
  cadena[i]=' ';
  escribir_en_lcd (cadena[i], &posicion_borrado_x, &posicion_y);
}
(*n)=0;
(*pos_x)=0;
}
