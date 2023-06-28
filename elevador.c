#include <stdint.h>
#include "elevador.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>


int limite_inferior[16] = {0,   4860, 9852,14867,19858,24851,29865,34857,39871,44863,49855,54869,59861,64853,69867,74558};
int limite_superior[16] = {300, 5147,10139,15130,20140,25136,30129,35143,40135,45149,50141,55133,60147,65139,70131,74658};

// **** ELEVADOR ESQUERDO ****
typedef struct elevador{
  uint8_t codigo;
  uint8_t fila_subindo[QT_ANDARES];
  uint8_t fila_descendo[QT_ANDARES];
  uint8_t andar;
  int8_t estado;  //Estado atual (PARADO, SUBINDO, DESCENDO)
  int8_t direcao; //Direcao do elevador (PARADO, SUBINDO, DESCENDO)
  int8_t porta;
  int32_t ultima_posicao;
  
}elevador_t;

elevador_t elevador_esquerdo;
elevador_t elevador_direito;
elevador_t elevador_central;

// ***************************

int32_t posicao_elevador;
  
uint8_t inicializado;

void esperarIniciar(){
  inicializado = 0;
  while(inicializado==0){
    sleep(300);
  }
}

// ************ OPERAÇÕES ELEVADOR ****************
elevador_t* getElevador(uint8_t elevador){
  if(elevador == ELEVADOR_ESQUERDO)
    return &elevador_esquerdo;
  if(elevador == ELEVADOR_DIREITO)
    return &elevador_direito;
  if(elevador == ELEVADOR_CENTRAL)
    return &elevador_central;
  return 0;
}
uint8_t getAndar(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador==0) return -1;
  
  return elevador->andar;
}

void inicializar(uint8_t elevador_cod){
  uint8_t comando[3] = {elevador_cod, INICIALIZA, '\r'};
  sendString(comando, 3);
  
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return;
  
  elevador->codigo=elevador_cod;
  elevador->andar=getAndar(elevador->codigo);
  elevador->direcao=ELEVADOR_PARADO;
  elevador->direcao=ELEVADOR_PARADO;
  elevador->porta=PORTA_ABERTA;
  for(uint8_t i=0; i<QT_ANDARES;i++){
    elevador->fila_subindo[i]=0;
    elevador->fila_descendo[i]=0;
  }  
  atualizarPosicao(elevador->codigo);
}
uint8_t fecharPorta(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
   
  if(elevador->porta==PORTA_FECHADA) return 0; //Já está fechada
  
  uint8_t comando[3] = {elevador->codigo, FECHA_PORTAS, '\r'};
  sendString(comando, 3);
  
  //Espera fechar
  int16_t tentativas = TENTATIVAS_PORTA;
  while(tentativas-- && elevador->porta==PORTA_ABERTA){ 
    sleep(1);
  }
  if(tentativas<=0) return 0;
  
  return 1;
}

uint8_t abrirPorta(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  if(elevador->porta==PORTA_ABERTA) return 0; //Já está aberta
  
  uint8_t comando[3] = {elevador_cod, ABRE_PORTAS, '\r'};
  sendString(comando, 3);
  
  //Espera abrir
  int16_t tentativas = TENTATIVAS_PORTA;
  while(tentativas-- && elevador->porta==PORTA_FECHADA){
    sleep(1);
  }
  if(tentativas<=0) return 0;
  
  return 1;
}

void ajustePosicao(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  atualizarPosicao(elevador_cod);
  while(elevador->ultima_posicao < limite_inferior[elevador->andar]){
    int32_t diff = elevador->ultima_posicao - limite_inferior[elevador->andar]; //Sempre negativo
    int tempo = -diff/2;
    subir(elevador_cod);
    sleep(tempo);
    parar(elevador_cod);
    atualizarPosicao(elevador_cod);
  }
  while(elevador->ultima_posicao > limite_superior[elevador->andar]){
    int32_t diff = elevador->ultima_posicao - limite_superior[elevador->andar]; //Sempre positivo
    int tempo = diff/2;
    
    descer(elevador_cod);
    sleep(tempo);
    parar(elevador_cod);
    atualizarPosicao(elevador_cod);
  }
}

void subir(uint8_t elevador_cod){
  uint8_t comando[3] = {elevador_cod, SOBE, '\r'};
  sendString(comando, 3);
  
  alterarEstado(elevador_cod, ELEVADOR_SUBINDO);
}

void descer(uint8_t elevador_cod){
  uint8_t comando[3] = {elevador_cod, DESCE, '\r'};
  sendString(comando, 3);
  
  alterarEstado(elevador_cod, ELEVADOR_DESCENDO);
}

void parar(uint8_t elevador_cod){
  uint8_t comando[3] = {elevador_cod, PARA, '\r'};
  sendString(comando, 3);
  
  alterarEstado(elevador_cod, ELEVADOR_PARADO);
}

int32_t consultar(uint8_t elevador_cod){
  uint8_t comando[3] = {elevador_cod, CONSULTA, '\r'};
  posicao_elevador = -1;
  sendString(comando, 3);
  
  uint16_t tentativas = 3000;
  while(tentativas-- && posicao_elevador == -1){
    sleep(1);
  }
  int32_t pos = posicao_elevador;
  posicao_elevador = -1;
  return pos;
}

void atualizarPosicao(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  elevador->ultima_posicao = consultar(elevador_cod);
  printf("%d\n", elevador->ultima_posicao);
}

void alterarEstado(uint8_t elevador_cod, int8_t estado) {
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  if(estado == ELEVADOR_PARADO || estado == ELEVADOR_SUBINDO || estado == ELEVADOR_DESCENDO){
    elevador->estado = estado;
  }
}
void alterarDirecao(uint8_t elevador_cod, int8_t direcao){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  if(direcao == ELEVADOR_SUBINDO || direcao == ELEVADOR_DESCENDO || direcao == ELEVADOR_PARADO){
    elevador->direcao = direcao;
  }
}

void desmarcarFila(uint8_t elevador_cod, uint8_t andar){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  if(elevador->direcao == ELEVADOR_SUBINDO || elevador->direcao==ELEVADOR_PARADO)
    elevador->fila_subindo[andar]=0;
  if(elevador->direcao == ELEVADOR_DESCENDO || elevador->direcao==ELEVADOR_PARADO)
    elevador->fila_descendo[andar]=0;
}

void atualizarAndar(uint8_t elevador_cod, uint8_t andar){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  elevador->andar=andar;
}

uint8_t precisaSubir(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  for(uint8_t i = elevador->andar+1; i<QT_ANDARES; i++){
    if (elevador->fila_subindo[i]) return 1;
    if (elevador->fila_descendo[i]) return 1;
  }
  return 0;
}

uint8_t precisaDescer(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  for(int8_t i = elevador->andar-1; i>=0; i--){
    printf("%d %d\n", elevador->andar, elevador->andar-1);
    printf("i: %d\n", i);
    if (elevador->fila_subindo[i]) return 1;
    if (elevador->fila_descendo[i]) return 1;
  }
  return 0;
}

uint8_t precisaParar(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  if(elevador->direcao==ELEVADOR_SUBINDO && elevador->fila_subindo[elevador->andar] )
    return 1;
  if(elevador->direcao==ELEVADOR_DESCENDO && elevador->fila_descendo[elevador->andar])
    return 1;
  
  if(elevador->direcao==ELEVADOR_SUBINDO && !precisaSubir(elevador_cod) )
    return elevador->fila_descendo[elevador->andar];
  if(elevador->direcao==ELEVADOR_DESCENDO && !precisaDescer(elevador_cod) )
    return elevador->fila_subindo[elevador->andar];
  
  
  return 0;
}

int8_t estado(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  return elevador->estado;
}
int8_t direcao(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  return elevador->direcao;
}
int8_t porta(uint8_t elevador_cod){
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador==0) return 0;
  
  return elevador->porta;
}

// **********************************************


int32_t atoi(uint8_t *str){
  int i;
  int32_t val=0;
  for(i=0; str[i]&&str[i]!='\r';i++){
    val+=str[i]-'0';
    val*=10;
  }
  val/=10;
  return val;
}


//*********** OPERAÇÕES BOTÕES ************
void ligarBotao(uint8_t elevador, uint8_t botao){
  uint8_t comando[4] = {elevador, LIGAR_BOTAO, botao, '\r'};
  sendString(comando, 4);
}

void desligarBotao(uint8_t elevador_cod, uint8_t andar) {  
  uint8_t botao = andar+'a';
  uint8_t comando[4] = {elevador_cod, DESLIGAR_BOTAO, botao, '\r'};
  sendString(comando, 4);
}

void marcarAndarInterno(uint8_t elevador_cod, uint8_t andar){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  //printf("MAI: %d\n", andar);
  if( andar > elevador->andar){
    elevador->fila_subindo[andar]=1;
  }
  else if ( andar < elevador->andar ) {
    elevador->fila_descendo[andar]=1;
  }
}

void marcarAndarExterno(uint8_t elevador_cod, uint8_t andar, uint8_t sentido_botao){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  if(sentido_botao == BOTAO_CORREDOR_SOBE){
    elevador->fila_subindo[andar]=1;
  }
  if(sentido_botao == BOTAO_CORREDOR_DESCE){
    elevador->fila_descendo[andar]=1;
  }
}

void botaoInternoPressionado(uint8_t elevador_cod, uint8_t botao){
  elevador_t* elevador = getElevador(elevador_cod);
  if(elevador == 0) return;
  
  uint8_t andar = botao-'a'; //Converte botao em andar
  marcarAndarInterno(elevador_cod, andar);
  
  if(andar!=elevador->andar){
    ligarBotao(elevador_cod, botao);
  }
}

//******************************************

void decodificaComando(uint8_t *comando){
  if (comando[0]>='0' && comando[0]<='9'){
    posicao_elevador = atoi(comando);
    return;
  }
  if( strcmp((const char*) comando, "initialized\r")==0 ){
    inicializado=1;
    return;
  }
  
  
  uint8_t elevador_cod = comando[0];
  uint8_t operacao = comando[1];
  
  elevador_t* elevador = getElevador(elevador_cod);
  if (elevador == 0) return;
  
  if(operacao == PORTA_ABERTA || operacao == PORTA_FECHADA)
    elevador->porta=operacao;
  
  if(operacao & NUMERO){
    uint8_t andar_string[3] {operacao,comando[2],'\r'};
    uint8_t andar = atoi(andar_string);
    
    atualizarAndar(elevador_cod, andar); 
  }
  
  if(operacao==BOTAO_INTERNO_PRESS){
    uint8_t botao = comando[2];
    botaoInternoPressionado(elevador_cod, botao);
  }
  if(operacao==BOTAO_EXTERNO_PRESS){
    uint8_t andar_string[3] = {comando[2], comando[3], '\r'};
    uint8_t andar = atoi(andar_string);
    uint8_t sentido_botao = comando[4];
    
    marcarAndarExterno(elevador_cod, andar, sentido_botao);
  }
  
}
