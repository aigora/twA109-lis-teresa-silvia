#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Biblioteca para la función sleep
#define n 10

typedef struct //Definimos la estructura necesaria para los leds
{
	char color[n];
	int salida;//Posicion que ocupara el led
}diodo;

FILE *guardar; 

void guardar_led(diodo);
int menu (void);

int main(void) 
{
	diodo led;
	int opcion;
  
	guardar=fopen("/dev/ttyACM0","w"); //Se abre el fichero del puerto serie
	
 	if (guardar==NULL)   
 		printf ("No se ha podido establecer la comunicacion\n"); 
 	else
 	{
	do
	{
	opcion=menu();//Menu de opciones para que el usuario ponga lo que quiere encender
	switch (opcion)//Se definen asi los colores para poder usar la funcion que compara cadenas sin que puedan surgir errores por no estar exactamente igual
	{
			case 1:
				{
				strcpy (led.color,"azul");
				}break;
			case 2:
				{
				strcpy (led.color,"verde");
				}break;
			case 3:	
				{
				strcpy (led.color,"rojo");
				}break;
			case 4:
				{
				strcpy (led.color,"amarillo");
				}break;
			case 5: break;
			default:printf("No existe led de ese color\n");break;
	}
	if (strcmp(led.color,"azul")==0)//Es igual el color a azul
		led.salida=1;//El led azul esta en la salida 1
		
	else 
	{
		if(strcmp(led.color,"verde")==0)
			led.salida=2;
			
		else
		{
			if(strcmp(led.color,"rojo")==0)
				led.salida=3;
			else
			{
				if(strcmp(led.color,"amarillo")==0)
					led.salida=0;
			}
		}
	}//Ponemos if para que sea mas facil la comprension del texto por parte de gente ajena		
	
	if (opcion>=1&&opcion<=4)
		guardar_led(led);//Guardar el numero de salida del led en un fichero aparte
	
	}while (opcion!=5);
	}
	fclose (guardar); /*Lo cerramos al final porque cada vez que se abre y cierra
						el puerto serie se resetea el arduino y no mantiene los leds
						encendidos*/
	
	system("PAUSE");
	return 0;
}


int menu (void)
{ 
	int opcion;
		printf("1_Azul\n");	
		printf("2_Verde\n");	
		printf("3_Rojo\n");	
		printf("4_Amarillo\n");
		printf("5_Salir\n");
		scanf("%d",&opcion);
		
	return (opcion);
}


void guardar_led(diodo l)
{  
	fprintf(guardar,"%d",l.salida); //Se escribe el numero de salida del led
	sleep(3);
	fflush(guardar); //Para que envíe el número que no se ha transmitido
} 
