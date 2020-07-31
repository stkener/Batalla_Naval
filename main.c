/*
BATALLA NAVAL

AUTOR: Kener, Sebastian
COLABORACION: Ing. Bassi, Matias

PRESENTACION: Se representan dos tableros clásicos del juego, radares enemigo y aliado, donde se coloca nuestra flota de barcos (tres).
Mediante una coneccion TCP se pasaran coordenadas para descubrir la localización de la flota enemiga y hundir todos los acorazados.


AGRADECIMIENTOS: Gracias al Ing. Matias Bassi por el acompañamiento, la guia y sus librerias de coneccion.
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define SOCKETCONNECTION_SINWARNING
#include "SocketConnection.h"
#include "SocketTrama.h"

#define maxcol 6
#define maxfil 7

/*MATRICES PARA TABLEROS*/
unsigned short matriz1 [maxfil][maxcol];
unsigned short matriz2 [maxfil][maxcol];// se crearon aca para que cualquier funcion las pueda agarrar este donde este

/*BARCOS*/
unsigned short *barcouno[3];
unsigned short *barcodos[3];
unsigned short *barcotres[3];
int salida;


/*ESTRUCTURAS PARA DATOS DE LOS JUGADORES*/
struct datos
{
  char nombre[51];
  long id_jugador;
  char port[8];
  char ip[18];
};
struct datos cliente;
struct datos servidor;// se crearon aca para que cualquier funcion las pueda agarrar este donde este

/*ELEMENTOS DE TABLERO*/
enum contenido
{
      vacio = 32,
     barco1 = 63,
     barco2 = 168,
     barco3 = 64,
     tocado = 88,
     agua   = 79,
yaingresado = 81,
    hundido = 45,
};

/*DATOS PARA COORDENADAS*/
struct coord
{
    char x;
    char y;
};struct coord BNrec; struct coord BNenv;

/*PARA LA TRAMA*/
enum trama
/*OBJETIVO:asignarle un codigo a la trama */
{
  iniciopartida = 123,
  enombre       = 110,
  eip           =105,
  epuerto       = 112,
  disparo       = 100,
  respuesta     = 114,
  finpartida    = 102,
};

/*FUNCIONES*/
void posicion (int x, int y);// agregar libreria windows.h
void funcion_titulo();
void menu_jugar();
void CD_Cliente(struct datos *servidor, struct datos *cliente);
void CD_servidor(struct datos *servidor);
int FX_mandarinfoCliente(struct datos dataCliente, struct datos *dataServidor);
void desserializarNombre(char *dataSerializada, struct datos *cliente );
void desserializarIP(char *dataSerializada, struct datos *cliente );
void desserializarPuerto(char *dataSerializada, struct datos *cliente );

int serializarInicioPartida( char *dataSerializada, struct datos dataJugador );
struct datos desserializarInicioPartida( char *dataSerializada );

int serializarNombreCli(char *dataSerializada, struct datos *cliente);
int serializarPuertoCli(char *dataSerializada, struct datos *cliente);
int serializarIPCli(char *dataSerializada, struct datos *cliente);
int socket_answerRequest(char *dataSerializada, int tramaenc, char *respuesta);
int desserializarresp (char *respuesta);
void tablero (void);
void tableromatriz1(unsigned short matriz1[][maxcol]);
void tableromatriz2(unsigned short matriz2[][maxcol]);
void campodebatalla(unsigned short matriz1[][maxcol], unsigned short matriz2[][maxcol]);
struct coord coordenadas();
void dibujarbarcos(unsigned short matriz1[][maxcol], unsigned short matriz2[][maxcol], unsigned short **barcouno, unsigned short **barcodos, unsigned short **barcotres);
void desserializarcoordenadas(char *dataSerializada, struct coord *BNrec);
int FXRespuesta(unsigned short matriz1[][maxcol], struct coord *BNrec);//revisar esto xq lo modifiq
int serializarRespcoord(char *respuesta, int respnum);
int funcion_hundido( unsigned short **ptr_barco1, unsigned short **ptr_barco2, unsigned short **ptr_barco3);
int serializarCoord(char *dataSerializada, struct coord *BNenv);//revisar esto xq lo modifiq
int FX_mandarcoord(struct coord *BNenv, struct datos datosOponente);//revisar esto
int funcionCheck(unsigned short matriz1[][maxcol]);
void FX_ganaste( struct datos datosOponente );
int serializarResfin(char *respuesta);
short entramarfin(char *dataSerializada);


int main(){
    finDprog:;
    char opcion;

do{
   system("cls");
   funcion_titulo();
   posicion(33,18);
   printf("1-JUGAR");
   posicion(33,19);
   printf("2-SALIR");
   posicion(33,21);
   opcion=getchar();
   switch(opcion){

            case '1':fflush( stdin ); menu_jugar(); break;

            case '2':fflush( stdin ); break;

}}while(opcion!='2');
 	return 0;}

void posicion (int x, int y){
/*
PROPOSITO: Posicionar el cursor en cualquier punto de la pantalla ingresando dos valores uno para 'x' e 'y'.
PRECONDICIONES: Nesecita tener incluida la librería <windows.h>.
*/
	HANDLE pantalla;

	pantalla= GetStdHandle(STD_OUTPUT_HANDLE);

	COORD POS;
	POS.X = x;
	POS.Y = y;

	SetConsoleCursorPosition(pantalla, POS);
}

void funcion_titulo(){
/*
PROPOSITO: Dibuja el titulo "BATALLA NAVAL" en pantalla
*/
	posicion(20,3);
	printf("XXX   XX  XXXX  XX  X    X     XX ");
	posicion(20,4);
	printf("X  X X  X  XX  X  X X    X    X  X");
	posicion(20,5);
	printf("XXX  X  X  XX  X  X X    X    X  X");
	posicion(20,6);
	printf("X X  XXXX  XX  XXXX X    X    XXXX");
	posicion(20,7);
	printf("X  X X  X  XX  X  X X    X    X  X");
	posicion(20,8);
	printf("XXX  X  X  XX  X  X XXXX XXXX X  X");
	posicion(25,10);
	printf("X  X  XX  X  X  XX  X");
	posicion(25,11);
	printf("X  X X  X X  X X  X X");
	posicion(25,12);
	printf("XX X X  X X  X X  X X");
	posicion(25,13);
	printf("X XX XXXX X  X XXXX X");
	posicion(25,14);
	printf("X  X X  X X  X X  X X");
	posicion(25,15);
	printf("X  X X  X  XX  X  X XXXX");
}

void CD_Cliente(struct datos *servidor, struct datos *cliente){
    char ip[20]="localhost"; // Tomara el ip de nuestra maquina
    char puerto[11]="27016"; // Este es un puerto poco utilizado

    /*
    "strcpy" es una funcion que sirve para copiar cadema de caracteres
     strcpy(destino, origen)
    */
        strcpy(cliente->ip,ip);
        strcpy(cliente->port,puerto);

        system("cls");
        posicion(25,9);
        printf("Ingrese su nombre: ");
        gets(cliente->nombre);
        posicion(25,10);
        printf("Ingrese su IP: %s", cliente->ip);
        posicion(25,11);
        printf("Ingrese su puerto: %s", cliente->port);
        posicion(25,12);
        printf("Ingrese IP del servidor: ");
        gets(servidor->ip);
        posicion(25,13);
        printf("Ingrese puerto del servidor: ");
        gets(servidor->port);
        getchar();
/*
	//jugador2.ip=localhost;
	//jugador2.port=DEFAULT_PORT;
    system("cls");
	posicion(25,9);
	printf("Ingrese su nombre:");
	gets(cliente->nombre);
	posicion(25,10);
	printf("Ingrese su ip:");
	gets(cliente->ip);
	posicion(25,11);
    printf("Ingrese su puerto:");
    gets(cliente->port);
	posicion(25,12);
	printf("Ingrese IP del servidor:");
	gets (servidor->ip);
	posicion(25,13);
	printf("Ingrese puerto del servidor:");
	gets (servidor->port);
    getchar();
    */
}

void CD_servidor(struct datos *servidor){
    char ip[20]="localhost"; // Tomara el ip de nuestra maquina
    char puerto[11]="27016"; // Este es un puerto poco utilizado

    /*
    "strcpy" es una funcion que sirve para copiar cadema de caracteres
     strcpy(destino, origen)
    */
    strcpy(servidor->ip,ip);
    strcpy(servidor->port,puerto);

    system("cls");
	posicion(25,9);
	printf("Ingrese su nombre: ");
	gets(servidor->nombre);
	posicion(25,10);
	printf("Ingrese su ip: %s", servidor->ip);
	posicion(25,11);
	printf("Ingrese su puerto: %s\n\n", servidor->port);
	posicion(25,12);
    printf("Ingrese IP cliente: ");
	gets (cliente.ip);
	posicion(25,13);
	printf("Ingrese puerto cliente: ");
	gets (cliente.port);
    getchar();
}

int FX_mandarinfoCliente(struct datos dataCliente, struct datos *dataServidor){
     int    tamanioenvio, tamaniorecepcion;
     char   dataSerializada[500];
     char   resp[100];
     struct datos servidor_aux = *dataServidor;

     tamanioenvio = serializarInicioPartida( dataSerializada, dataCliente );
     tamanioenvio = encabezarTrama( dataSerializada, tamanioenvio );

     tamaniorecepcion = socket_sendReceive( dataSerializada, tamanioenvio, servidor_aux.ip , servidor_aux.port, resp );

     if(tamanioenvio < 1 ) return 0; // indica error

     servidor_aux = desserializarInicioPartida( &resp[2] );

     if( servidor_aux.id_jugador < 0 ){return 0; /* indica error*/}
     else{
        (*dataServidor) = servidor_aux;
        return 1; /*indica partida iniciada Ok*/}
}

int serializarInicioPartida( char *dataSerializada, struct datos dataJugador ){
    union s{
        struct datos data;
        char dataSerial[ sizeof(dataJugador) ];
    };

    union s serializador;
    int tamanioData = sizeof(dataJugador);

    serializador.data = dataJugador;
    dataSerializada[0] = (char)iniciopartida;
    memcpy( &dataSerializada[1], serializador.dataSerial, tamanioData );

    return tamanioData+1;}

struct datos desserializarInicioPartida(char *dataSerializada){
    union s{
        struct datos data;
        char dataSerial[ sizeof(struct datos) ];
    };

    union s deserializador;
    struct datos dataJugador;

    if( dataSerializada[0] != (char)iniciopartida ){
        printf("Error al deserializar inicio de partida. Tipo de trama != iniciopartida\n");
        dataJugador.id_jugador = -1;
        return dataJugador;}

    memcpy( deserializador.dataSerial, &dataSerializada[1], sizeof(struct datos) );

    dataJugador = deserializador.data;

    return dataJugador;}

int serializarNombreCli(char *dataSerializada, struct datos *cliente){/*para iniciar partida*/
    int num;

        dataSerializada[0]=enombre;
        dataSerializada[1]=NULL;
        strcat(dataSerializada, cliente->nombre);
        num=strlen(dataSerializada);

    return num;} //devuelve el tamaño de dataSerializada


int serializarIPCli(char *dataSerializada, struct datos *cliente){
    int num;

        dataSerializada[0]=eip;
        dataSerializada[1]=NULL;
        strcat(dataSerializada, cliente->ip);
        num=strlen(dataSerializada);

    return num;}

int serializarPuertoCli(char *dataSerializada, struct datos *cliente){
    int num;

        dataSerializada[0]=epuerto;
        dataSerializada[1]=NULL;
        strcat(dataSerializada, cliente->port);
        num=strlen(dataSerializada);

    return num;}

int FX_mandarcoord(struct coord *BNenv, struct datos datosOponente){
   int casillero;
   char dataSerializada[10];
   short tamCS, tramCoordenc, tamanioresp;//coordenada serializada - tramaCoordenc -
   char respuesta[10];

        tamCS=serializarCoord(dataSerializada, BNenv);
        tramCoordenc=encabezarTrama(dataSerializada,tamCS);
        tamanioresp=socket_sendReceive(dataSerializada, tramCoordenc, datosOponente.ip, datosOponente.port, respuesta);
        casillero=desserializarresp(respuesta);

    return casillero;}

int desserializarresp (char *respuesta){
	int RESP;

        RESP=atoi(respuesta);

    return RESP;}

int serializarCoord(char *dataSerializada, struct coord *BNenv){
    int sizeData;

        dataSerializada[0]=(char)disparo;
        dataSerializada[1]=(char)BNenv->x;
        dataSerializada[2]=(char)BNenv->y;
        sizeData=3;

    return sizeData;}

void desserializarNombre(char *dataSerializada, struct datos *cliente )//esto para la parte de Servidor

    {strcpy(cliente->nombre, &dataSerializada[3]);};

void desserializarIP(char *dataSerializada, struct datos *cliente )

    {strcpy(cliente->ip, &dataSerializada[3]);}

void desserializarPuerto(char *dataSerializada, struct datos *cliente )

    {strcpy(cliente->port, &dataSerializada[3]);}

void desserializarcoordenadas(char *dataSerializada, struct coord *BNrec){

    BNrec->x=dataSerializada[3];
    BNrec->y=dataSerializada[4];}

int FXRespuesta(unsigned short matriz1[][maxcol], struct coord *BNrec){

    if (matriz1[BNrec->y][BNrec->x]==vacio){
            matriz1[BNrec->y][BNrec->x]=agua;
            printf("\nAGUA\n");
            getchar();
            return agua;}

    if (matriz1[BNrec->y][BNrec->x]==barco1||matriz1[BNrec->y][BNrec->x]==barco2||matriz1[BNrec->y][BNrec->x]==barco3){
            matriz1[BNrec->y][BNrec->x]=tocado;
            printf("\nTOCADO\n");
            getchar();
            return tocado;}

    if (matriz1[BNrec->y][BNrec->x]==agua||matriz1[BNrec->y][BNrec->x]==tocado) {return yaingresado;}
}

int serializarRespcoord(char *respuesta, int respcasillero){//prestar atencion y corregir donde se utiliza

    if(respcasillero==agua){
        respuesta[0]=6;
        respuesta[1]=7;
        respuesta[2]=9;
        return 3;}

    if(respcasillero==tocado){
        respuesta[0]=6;
        respuesta[1]=8;
        respuesta[2]=8;
        return 3;}

    if(respcasillero==yaingresado){
        respuesta[0]=6;
        respuesta[1]=8;
        respuesta[2]=1;
        return 3;}}

int funcionCheck(unsigned short matriz1[][maxcol]){
    short i,j, G=0;

        for (i=0; i<maxfil; i++){
            for(j=0; j<maxcol; j++){
                if(matriz1[i][j]==tocado)
                    {G++;}}}
    return G;}

int funcion_hundido( unsigned short **ptr_barco1, unsigned short **ptr_barco2, unsigned short **ptr_barco3 ){
    static int   i1=0, i2=0, i3=0;
    int i, contTocados, hundidos=0;

        if( i1 < 0 ){ // significaria que ya se hundió
            hundidos++;}
        else{
            contTocados=0;
                for(i1=0; i1<3; i1++){  //printf("%hd - %hd\n", *(ptr_barco1[i]), tocado );
                    if(*(ptr_barco1[i1])==tocado){ //printf("%hd\n", *(ptr_barco1[i]));
                        contTocados++;
                            if( contTocados == 3 ){
                                printf("BARCO 1 HUNDIDO \n");
                                        i1 = -5;
                                        hundidos++;
                    }}}}

    if( i2 < 0 ){ // significaria que ya se hundió
         hundidos++;}
    else{
        contTocados=0;
        for(i2=0; i2<3; i2++){
            if(*(ptr_barco2[i2])==tocado){
                contTocados++;
                if( contTocados == 3 ){
                    printf("BARCO 2 HUNDIDO \n");
                        i2 = -5;
                        hundidos++;
                }}}}

    if( i3 < 0 ){ // significaria que ya se hundió
           hundidos++;}
    else{
        contTocados=0;
            for(i3=0; i3<3; i3++){
                if(*(ptr_barco3[i3])==tocado){
                    contTocados++;
                        if( contTocados == 3 ){
                            printf("BARCO 3 HUNDIDO \n");
                                i3 = -5;
                                hundidos++;
                }}}}

    return hundidos;}

int serializarResfin(char *respuesta){

        respuesta[0]=(char)finpartida;
        respuesta[1]='o';
        respuesta[2]='k';

    return 3;}

int socket_answerRequest(char* dataSerializada, int tramaenc, char *respuesta){
        int VFT, TT, tamDS, tramenc, respnum, tamresp, fin;//Verif form trama - tipo de trama - tamanio data servidor - trama encabezada - respuesta numero(respuesta una vez visto la matriz) - tamanio respuesta(tamanio de respuesta serializada) -

    VFT=verificarFormatoTrama(dataSerializada);

        if(VFT==0){
            TT=obtenerTipoDeTrama(dataSerializada);
                if(TT==iniciopartida){
                    cliente = desserializarInicioPartida( &dataSerializada[2] );
                    tamresp = serializarInicioPartida( respuesta, servidor );
                    tamresp = encabezarTrama( respuesta, tamresp );
                return tamresp;}

                if(TT==enombre){
                    desserializarNombre(dataSerializada, &cliente );
                    strcpy(respuesta, servidor.nombre);
                return tamresp=strlen(servidor.nombre);}

                if(TT==eip){
                    desserializarIP(dataSerializada, &cliente );
                    strcpy(respuesta, "LLEGO");
                return tamresp=7;}

                if(TT==epuerto){
                    desserializarPuerto(dataSerializada, &cliente );
                    strcpy(respuesta, "LLEGO");
                return tamresp=7;}

                if(TT==disparo){
                     desserializarcoordenadas(dataSerializada, &BNrec);
                     respnum=FXRespuesta(matriz1, &BNrec);

                        if (respnum==agua){respuesta[0]='7'; respuesta[1]='9';}

                        if (respnum==tocado){respuesta[0]='8'; respuesta[1]='8';}
                return tamresp=2;}

                if(TT==finpartida){
                    system("cls");
                    fflush( stdin );
                    posicion(36, 10);
                    printf("%s", &dataSerializada[4]);
                    fin=serializarResfin(respuesta);
                    getchar();
                    salida=1;
               return fin;}
        }}

void tablero (void){
    char letratablero;
    short ypos;

        posicion(32,0);
        printf("BATTLE IN PROCESS");
        posicion(8,3);
        printf("RADAR ALIADO");
        posicion(56,3);
        printf("RADAR ENEMIGO");

        posicion(0,4);
        printf("! # ! 1 ! 2 ! 3 ! 4 ! 5 ! 6 \n");
        printf(" --- --- --- --- --- --- ---\n");

    for (letratablero=65; letratablero<71 && letratablero>64; letratablero++){

        printf("! %c \n ---\n", letratablero);}

        posicion(48,4);
        printf("! # ! 1 ! 2 ! 3 ! 4 ! 5 ! 6 \n");
        posicion(48,5);
        printf(" --- --- --- --- --- --- ---\n");

    for (letratablero=65, ypos=6; letratablero<71 && letratablero>64; letratablero++, ypos++){
        posicion(48,ypos);
        printf("! %c ", letratablero);
        ypos++;
        posicion(48,ypos);
        printf(" ---");}
}

void tableromatriz1(unsigned short matriz1[][maxcol]){
    short fila,columna,i,j, ypos=6;

    posicion (4,6);
    for (fila=1, i=0; fila<maxfil&&i<maxfil; fila++, ypos++, i++){
         posicion(4,ypos);
           for(columna=0, j=0; columna<maxcol&&j<maxcol; columna ++, j++){
                    printf("! %c ",matriz1[i][j]);}//si le pongo imprimir los numeros se corre todo

            printf("\n");
            ypos++;
            posicion(4,ypos);
            printf(" --- --- --- --- --- --- \n");}
}

void tableromatriz2(unsigned short matriz2[][maxcol]){
    short fila,columna,i,j, ypos=6, xpos=52;

    posicion (xpos,ypos);
    for (fila=1, i=0; fila<maxfil&&i<maxfil; fila++, ypos++, i++){
         posicion(xpos,ypos);
            for(columna=0, j=0; columna<maxcol&&j<maxcol; columna ++, j++){
                    printf("! %c ",matriz2[i][j]);}//si le pongo imprimir los numeros se corre todo
            printf("\n");
            ypos++;
            posicion(xpos,ypos);
            printf(" --- --- --- --- --- --- \n");}
}

void campodebatalla(unsigned short matriz1[][maxcol], unsigned short matriz2[][maxcol]){

            system("cls");
            tablero();
            tableromatriz1(matriz1);
            tableromatriz2(matriz2);
}

void dibujarbarcos(unsigned short matriz1[][maxcol], unsigned short matriz2[][maxcol], unsigned short **barcouno, unsigned short **barcodos, unsigned short **barcotres){
    struct coord barco;
    short i, j;

        for (i=1, j=0; i<4; i++, j++){
                printf("Ubicando Flota\n");
                barco=coordenadas();

            while(matriz1[(int)barco.y][(int)barco.x]!=vacio){
                    printf("Coordenada invalida\n");
                    system("cls");
                    campodebatalla(matriz1, matriz2);
                    barco=coordenadas();}

        matriz1[barco.y][barco.x]=barco1;
        barcouno[j]=&matriz1[barco.y][barco.x];
        system("cls");
        campodebatalla(matriz1, matriz2);}

        for (i=1, j=0; i<4; i++, j++){
                printf("Ubicando Flota\n");
                barco=coordenadas();

            while(matriz1[barco.y][barco.x]!=vacio){
                printf("Coordenada invalida\n");
                system("cls");
                campodebatalla(matriz1, matriz2);
                barco=coordenadas();}

        matriz1[barco.y][barco.x]=barco2;
        barcodos[j]=&matriz1[barco.y][barco.x];
        system("cls");
	    campodebatalla(matriz1, matriz2);}

        for (i=1, j=0; i<4; i++, j++){
                printf("Ubicando Flota\n");
                barco=coordenadas();

            while(matriz1[barco.y][barco.x]!=vacio){
                printf("Coordenada invalida\n");
                system("cls");
                campodebatalla(matriz1, matriz2);
                barco=coordenadas();}

        matriz1[barco.y][barco.x]=barco3;
        barcotres[j]=&matriz1[barco.y][barco.x];
        system("cls");
	    campodebatalla(matriz1, matriz2);}
}

struct coord coordenadas(){
    struct coord batnav;

        printf("Ingrese letra: ");
        batnav.y=getchar();
        fflush(stdin);

    while ((batnav.y<=64) || (batnav.y>=71)){
        printf("Letra incorrecta\nAsegurese de ingresar letras mayusculas y dentro de los parametros aceptados\n");
        printf("Ingrese letra: ");
        batnav.y=getchar();
        fflush(stdin);}

    batnav.y=batnav.y-65;
    printf("Ingrese numero: ");
    batnav.x=getchar();
    fflush(stdin);

    while ((batnav.x<=48) || (batnav.x>=55)){
         printf("Numero incorrect\nAsegurese de ingresar valores dentro de los parametros aceptados\n");
         printf("Ingrese numero: ");
         batnav.x=getchar();
         fflush(stdin);}

    batnav.x=batnav.x-49;

    return batnav;}

void menu_jugar(){
    int casillero, algo;
    char opcion;
    short i, j, ataques, cruces;
    int barcosHundidos=0;
    char flagOK;

            for (i=0; i<maxfil; i++){
                    for(j=0; j<maxcol; j++){
                        matriz1[i][j]=vacio;
                        matriz2[i][j]=vacio;}}

	system("cls");
	fflush( stdin );
	posicion(34,10);
	printf("1-Cliente");
	posicion(34,11);
	printf("2-Servidor");
	posicion(34,12);
	printf("3-Volver");
	posicion(34,13);
	opcion=getchar();

        switch(opcion){
            case '1':
            fflush( stdin );
                 do{
                    CD_Cliente(&servidor, &cliente);
                    printf("Presione una tecla para intentar conectar con el servidor\n");
                    getchar();
                    flagOK = FX_mandarinfoCliente( cliente, &servidor );

                        if( flagOK == 0 ){
                            printf("No se pudo conectar con el servidor\n");
                            getch();}}
                    while(flagOK == 0);

            system("cls");
            fflush( stdin );
            posicion(30,10);
            printf("Iniciando partida con %s", servidor.nombre);
            posicion(34,20);
            printf("Presione enter");
            getchar();
            system("cls");
            fflush( stdin );
            campodebatalla(matriz1, matriz2);
            dibujarbarcos(matriz1, matriz2, barcouno, barcodos, barcotres);
            system("cls");
            fflush( stdin );
            campodebatalla(matriz1, matriz2);

                    for(ataques=0;ataques<37||cruces==9;ataques++){//cruces, para no poner tocado y que se confunda con el enum
                         printf("Esperando ataque\n");
                         socket_serve(cliente.port);

                              if(salida==1){break;};

                         fflush( stdin );
                         system("cls");
                         campodebatalla(matriz1, matriz2);
                         fflush( stdin );
                                //barcosHundidos = funcion_hundido(barcouno, barcodos, barcotres);
                               // printf("BARCOS HUNDIDOS: %d\n", barcosHundidos);
                         cruces=funcionCheck(matriz1);

                                if(/*barcosHundidos==3 ||*/ cruces==9){
                                   FX_ganaste( servidor );
                                   system("cls");
                                   posicion(30,10);
                                   printf("PERDISTE...JAJAJAJAJA");
                                   fflush(stdin);
                                   getchar();
                                    break;}

                          fflush( stdin );
                          printf("Tu turno\n");
                          BNenv=coordenadas();
                          casillero=FX_mandarcoord(&BNenv, servidor);

                                 if(casillero==agua){printf("\nAGUA"); getchar();}
                                 if(casillero==tocado){printf("\nTOCADO"); getchar();}
                                 if(casillero==yaingresado){printf("\nYA INGRESADO"); getchar();}
                                 if(casillero!=yaingresado){matriz2[BNenv.y][BNenv.x]=casillero;}

                          fflush( stdin );
                          system("cls");
                          campodebatalla(matriz1, matriz2);
                  }

        break;

          case '2'://ak hay q hacer un struct datos DSC p/recibir datos mandados
          fflush( stdin );
          CD_servidor(&servidor);
          algo=socket_serve(servidor.port); // espera trama de inicio de partida
          getch();
          system("cls");
          fflush( stdin );
          posicion(30,10);
          printf("Iniciando partida con %s", cliente.nombre);
          posicion(34,20);
          printf("Presione enter");
          getchar();
          system("cls");
          fflush( stdin );
          campodebatalla(matriz1, matriz2);
          dibujarbarcos(matriz1, matriz2, barcouno, barcodos, barcotres);
          system("cls");
          fflush( stdin );
          campodebatalla(matriz1, matriz2);

                    for(ataques=0;ataques<37||cruces==9;ataques++){
                        fflush( stdin );
                        printf("Tu turno\n");
                        BNenv=coordenadas();
                        casillero=FX_mandarcoord(&BNenv, cliente);

                              if(casillero==agua){printf("\nAGUA"); getchar();}
                              if(casillero==tocado){printf("\nTOCADO"); getchar();}
                              if(casillero==yaingresado){printf("\nYA INGRESADO"); getchar();}
                              if(casillero!=yaingresado){matriz2[BNenv.y][BNenv.x]=casillero;}

                        system("cls");
                        fflush( stdin );
                        campodebatalla(matriz1, matriz2);
                        printf("Esperando ataque");
                        socket_serve(servidor.port);

                              if(algo==3){break;};

                        system("cls");
                        fflush( stdin );
                        campodebatalla(matriz1, matriz2);
                                //barcosHundidos=funcion_hundido(barcouno, barcodos, barcotres);
                                //printf("BARCOS HUNDIDOS: %d\n", barcosHundidos);
                        cruces=funcionCheck(matriz1);

                               if(/*barcosHundidos==3 ||*/ cruces==9){
                                   printf("FX_ganaste();");
                                   FX_ganaste( cliente );
                                   system("cls");
                                   posicion(30,10);
                                   printf("PERDISTE...JAJAJAJAJA");
                                   fflush(stdin);
                                   getchar();
                                    break;}
                                }

                     break;

            case '3': break;}
}


short entramarfin(char *dataSerializada){
    char    msj[20]="GANASTE";
    short tamDSerial;

        dataSerializada[0]=finpartida;
        dataSerializada[1]='0';
        strcat(dataSerializada, msj);
        tamDSerial=strlen(dataSerializada);

    return tamDSerial;
}


void FX_ganaste(struct datos datosOponente){
    char    dataSerializada[20];
    char    resp[20];
    int     tamanio, tamDSerial;

        tamDSerial=entramarfin(dataSerializada);

        tamanio = encabezarTrama(dataSerializada, tamDSerial);

        tamanio = socket_sendReceive( dataSerializada, tamanio, datosOponente.ip ,datosOponente.port, resp);
}
