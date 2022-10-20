/** 
 * @author Higuera Consuegra Paula
 *         12508930-N
 * @author Manzanas Mogrovejo Francisco 
 *         71315131-C 
 * 
 * @version Domingo, 27 de Diciembre de 2020 a las 23:59 Horas
 * 
 * @file PRACTICA FINAL FUNDAMNETOS DE SISTEMAS OPERATIVOS
 * 
 */

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//buffer circular
struct dataBuffer{
    
    char m_tens; // decenas de la posicion
    char m_unit; // unidades de la posicion
    char m_char;  // caracter a colocar en la posicion P
    bool m_bEnd;  // bandera para saber si es o no el final del buffer
};

//datos lector
struct dataconsumidor
{
	int pthread; // hilo
	int tokens_read; // tokens leidos del hilo
	int tokens_Right; // tokens correctos del hilo
	int tokens_discarded ; // tokens descartados
	int maxIndex ; // indice maximo leido por ese hilo
	bool txt; // bandera para saber si es el final
};

//lista simple enlazada 
struct lista { 
  char letra;
  int posss;
  struct  lista *sig;
};

//semaforos
sem_t there_space, there_data; // semaforos del problema productor-consumidor
sem_t mutex, mutexl, mutexc;
sem_t leido, despierta;
sem_t mutex_solo_uno_vez; // semaforo para asegurar que cada consumidor se hace independientemente uno del resto

//ficheros a abrir
FILE *f;
FILE *fichero;

// difinimos las estructuras que usaremos mas adelante
struct dataBuffer* buffer1;
struct dataconsumidor* vector;
struct lista* lista;

//inicializamos algunas de las variables
int tamBuffer;	
int index_fill = 0, tamanio_buffer = 0, tmp = 0;

// variables para colocar caracter en la poscion
int tens, unit, position; // position == (tens * 10) + unit
char chars;
// contamos los tokens leidos, descartados... del archivo
int tokens_read = 0, read_total = 0;
int tokens_Right = 0, right_total = 0;
int tokens_discarded = 0, discar_total = 0;
int maxIndex = 0, index_total = 0;
int numCosumidores=0;

// distintos contadores
int cont=-1,longi=0;
int ccont=0;
bool fin=false;

/**
 * conocer la longitud del buffer 
 * 
 * @param buffer1 
 * @return longitud del buffer 
 */
int longitud(struct dataBuffer* buffer1){
    int cont = 0;
    
    while(true){
        if(buffer1[cont].m_bEnd != false){
            break;
        }else{
            cont++;
        }
    }

    return cont;
}

/**
 *  contar los caracteres que contiene la cadena
 * 
 * @param cadena 
 * @return numero de caracteres de la cadena pasada como argumento 
 */
int contaCaracters(char cadena[]){
    int counter = 0;
    for (int i = 0; i < strlen(cadena); i++){
        if (cadena[i] != '\0'){
            counter ++;
        }else{
			break;
		}
    }
    return  counter;
}

void recorre(){
	struct lista* index = lista;
	int i=0;
	bool fin=true;
	while(i<longi){
		index = lista;
		fin=true;
		while(fin){
			if(index->posss==i){
				fprintf(fichero,"%c",index->letra);
				fin=false;
			}
			index=index->sig;
		}
		i++;
		
	}

}

void inserta(int orden, char car){
	longi++;
	struct lista* index = lista;
	struct lista* ant = NULL;
	while(index != NULL){
		ant = index;
		index = index -> sig;
	}
	
	
	//reservamos memoria para el nuevo nodo
	struct lista* nodo = (struct lista*)malloc(sizeof(struct lista ));
	//alimentamos el nuevo nodo
	nodo ->posss= orden;
	nodo -> letra = car;
	
	if(ant == NULL){
		//no se ha metido en el bucle, hay que insertar en la primera posicion
		lista = nodo; //la cabeza de la lista apunta al nuevo nodo
	}else {
	
		ant ->sig = nodo;
		nodo -> sig = index;
		
	}
}

void *productor(void *arg) {
	struct dataBuffer data;
	/**
	 * variable donde iremos guardando cada linea del fichero
	 * NOTA: la linea mas larga del fichero contiene 10 caracteres
	 */
	char cadena [10]; 
	index_fill; 
	while (fscanf(f, "%s", cadena) != EOF) {
   		/**
   	     * EOF == variable que indica  final del fichero
         * comprobamos que la linea cogida tienen solo 3 caracteres
		 */

        if (contaCaracters(cadena) == 3){
            // vamos escribiendo en el buffer
            data.m_tens = cadena[0];
            data.m_unit = cadena[1];
            data.m_char = cadena[2];
            // there is space buffer size
            data.m_bEnd = false;
            sem_wait(&there_space); //Espera a que haya silla libre
            buffer1[index_fill] = data;
            index_fill = (index_fill + 1) % tamBuffer;
	        sem_post(&there_data); //Indica al semaforo que hay clientes
			tokens_read ++; // token con 3 caracteres leido
        }
	}
	// finalmente guardamos el caracter '\ 0' al final del String
	data.m_bEnd = true;
	sem_wait(&there_space); //Espera a que haya silla libre
	
    buffer1[index_fill] = data;
	sem_post(&there_data);
   
    pthread_exit(NULL);
}	

void *consumidor(void *arg){
	// empezamos con un mutex para que cada consumidor se haga de "forma independiente"
    sem_wait(&mutex_solo_uno_vez); 
    
    tamanio_buffer = longitud(buffer1); // para conocer la longitud del buffer1
    
    struct dataBuffer data;
	struct dataconsumidor datac;
	int index_empty = tmp; 
    int contamos = 0;

	// lo reinicializamos porque cada consumidor leere distinyos caracteres con distintas posiciones en el mensaje a decodificar
	maxIndex = 0; 

    while(contamos < tamanio_buffer){
        sem_wait(&there_data); // Espera a que haya algun dato
        sem_wait(&mutex);
        data = buffer1[index_empty];
		// si llegamos al final del buffer, salimos del bucle
        if(data.m_bEnd == true){
			sem_post(&mutex);
			sem_post(&there_data);
			fin=true;
            break;
        }else{ 
			index_empty = (index_empty + 1) % tamBuffer;
		}
        contamos++;
		sem_post(&mutex);
    	sem_post(&there_space); 
        // decodificamos
        if (100 <= (int)(data.m_tens ) && (int)(data.m_tens)<= 109){
            tens = (int)(data.m_tens - 100);
			tens = tens * 10;

            if (70 <= (int)(data.m_unit)&& (int)( data.m_unit) <= 79){
                unit = (int)(data.m_unit - 70);
                position = tens + unit; // posicion donde colocaremos el caracter
                chars = data.m_char - 1; // caracter que se colocara en la posicion calculada justo antes
                inserta(position, chars);
                tokens_Right++;// token leido y correcto
                if (position > maxIndex){
                    maxIndex = position;
                }
				
            }
        }
		tokens_discarded = tokens_read - tokens_Right; // tokens descartados
		// guardamos la informacion de cada hilo en una estructura
		datac.tokens_read=tokens_read;
		datac.tokens_Right=tokens_Right;
		datac.tokens_discarded =tokens_discarded;
		datac.maxIndex=maxIndex;
    }
	// sumamos los tokens de cada hilo
    discar_total += tokens_discarded;
    read_total += tokens_read;
    right_total += tokens_Right;

	// reinicializamos las variables
    tokens_read = 0;
    tokens_discarded = 0;
    tokens_Right = 0;
    contamos = 0;

	// guardamos en una temporal el indice del buffer1
    tmp = index_empty;

	sem_wait(&mutexc);
	cont++; // numero del hilo en el que estamos
	datac.pthread=cont;

	if(datac.pthread==(numCosumidores-1)){
		datac.txt=true;
	}else {
		datac.txt=false;
	}
	// vamos guardando la informacion de cada hilo en vector
	vector[datac.pthread]=datac;
	sem_post(&mutexc);
	sem_post(&despierta);
    sem_post(&mutex_solo_uno_vez); // senialamos que otro consumidor puede entrar en accion

    pthread_exit(NULL);
}

void *lector(void *arg) {
	struct dataconsumidor datac;
	while(true){
		sem_wait(&despierta);
		sem_wait(&mutexl);
		datac=vector[ccont];
		ccont++;
		
		// informacion que proporciona cada uno de los hilos
		fprintf(fichero,"Hilo %i.\n",datac.pthread);
		fprintf(fichero,"   Tokens procesados: %i\n",datac.tokens_read);
		fprintf(fichero,"   Tokens correctos: %i\n",datac.tokens_Right);
		fprintf(fichero,"   Tokens incorrectos: %i\n",datac.tokens_discarded);
		fprintf(fichero,"   MaxIndex: %i\n",datac.maxIndex);
		fprintf(fichero,"        \n");

		// para elegir el indice maximo
		if (datac.maxIndex > maxIndex){
			maxIndex = datac.maxIndex;
		}
		
		sem_post(&mutexl);
		if(ccont>=numCosumidores){
			break;
		}
	}
	
	// informacion que proprociona el hilo final (suma de todos los hilos)
	if(ccont>=numCosumidores){
		fprintf(fichero,"Resultado final(los que procesa el consumidor final).\n");
		fprintf(fichero,"   Tokens procesados: %i\n",read_total );
		fprintf(fichero,"   Tokens correctos: %i\n",right_total);
		fprintf(fichero,"   Tokens incorrectos: %i\n",discar_total);
		fprintf(fichero,"   MaxIndex: %i\n", maxIndex);
		if (maxIndex == right_total-1){		
			fprintf(fichero,"   Mensaje: Correcto \n");
			fprintf(fichero,"   Mensaje traducido: ");
            recorre();

		}else{
			fprintf(fichero,"   Mensaje: Incorrecto \n");
            fprintf(fichero, "\nNO se pudo transcribir el mensaje oculto(pruebe con más numero de consumidores).\n");
		}
	}
	pthread_exit(NULL);
}

// ./<program><inputFile><outputFile><tamBuffer><numCosumidores>
int main(int argc, char *argv[]){

	struct dataconsumidor datac;
    // validamos numero de argumentos de entrada
    if (argc != 5){
        printf("ERROR_1.\n");
        return -1;
    }
    
	tamBuffer = atoi(argv[3]); // tamanio del buffer, pasado como 3 argumento
	numCosumidores = atoi(argv[4]); // consumidores, pasado como 4 argumento
    // validamos que el 3 parametro pasado (tamanio del Buffer) sea distinto de 1
    if (tamBuffer < 1 || numCosumidores < 1){
        printf("ERROR_2.\n");
        return -1;
    }

    // reservamos memoria dinamica para las estructuras
    buffer1 = (struct dataBuffer *) malloc(tamBuffer* sizeof (struct dataBuffer));
	vector = (struct dataconsumidor *) malloc(numCosumidores* sizeof (struct dataconsumidor));
    if (buffer1 == NULL || vector == NULL){
        printf("IMPOSIBLE DAR MEMORIA.");
        return -1;
    }
    
	/**
	 * Inicializacion de los semaforos
	 * NOTA: se inicializan con el numero maximo de procesos  que pueden pasar por el sin bloquearse
	 * 
	 */
	sem_init(&there_space,0,tamBuffer); //Inicializado a tambuffer que son el numero de espacio que hay
	sem_init(&there_data,0,0);//Inicializado a 0 que se ira incrementando segun se creen datos
    sem_init(&mutex, 0, 1);
	sem_init(&mutexl, 0, 1);
	sem_init(&mutexc, 0, 1);
	sem_init(&leido, 0, 1);
    sem_init(&mutex_solo_uno_vez, 0, 1); // Para asegurar que cada consumidor se ejecute de forma independiente 
	sem_init(&despierta,0, 0);

	// abrimos los ficheros
	f = fopen(argv[1], "r+"); // lectura
	fichero = fopen(argv[2], "w+" ); /// escritura
	if(f==NULL || fichero == NULL) {
		fprintf(stderr, "ERROR_3.\n");
		return 1;
	}

    // definimos los hilos (PRODUCTOR Y CONSUMIDOR)
    pthread_t pProductor, pConsumidor,Lector;

    // creamos los hilos, llamando a las correspondientes funciones para cada hilo
	// lanzamos el hilo productor
    pthread_create(&pProductor, NULL, productor, NULL);
	// lanzamos los consumidor a la vez
	for(int i =0;i<numCosumidores;i++){
		pthread_create(&pConsumidor, NULL, consumidor, NULL);
	}
	for(int i =0;i<numCosumidores;i++){
        pthread_join(pConsumidor, NULL);
    }  
    
	// lanzamos el hilo lector
    pthread_create(&Lector, NULL, lector, NULL);

    //pthread_join(pProductor, NULL);
	pthread_join(Lector, NULL);
   
    // cerreamos los ficheros
    fclose( f );
  	fclose( fichero );
	
	// destruimos todos los semaforos
	sem_destroy(&there_data);
	sem_destroy(&there_space);
	sem_destroy(&mutex);
	sem_destroy(&mutexl);
	sem_destroy(&leido);
	sem_destroy(&despierta);
    sem_destroy(&mutex_solo_uno_vez);

	// limpiamos las estructuras buffer1, vector y lista
	free(buffer1);
    free(vector);
	free(lista);

    return 0;
}
