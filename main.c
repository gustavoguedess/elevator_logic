#include <stdio.h>
#include <stdint.h>
#include "gpio.h"
#include "elevador.h"

int main()
{
  init();
  
  esperarIniciar();
  
  sleep(1000);
  
  inicializar(ELEVADOR_ESQUERDO);
  
  
  while(1)
  {
    uint8_t elevador = ELEVADOR_ESQUERDO;
    
    if ( precisaParar(elevador) && estado(elevador)!=ELEVADOR_PARADO ){
      parar(elevador);
      printf("PARAR\n");
      
      ajustePosicao(elevador);
      uint8_t efetuado = abrirPorta(elevador);
      if(!efetuado) continue;
      
      if(!precisaSubir(elevador) && direcao(elevador) == ELEVADOR_SUBINDO 
         || !precisaDescer(elevador) && direcao(elevador) == ELEVADOR_DESCENDO )
        alterarDirecao(elevador, ELEVADOR_PARADO);
      
      uint8_t andar = getAndar(elevador);
      desligarBotao(elevador, andar);
      desmarcarFila(elevador, andar);
      
    }
    // Mandar Subir quando estiver parado, ou quando estiver parado temporariamente 
    else if( precisaSubir(elevador) && ( direcao(elevador) == ELEVADOR_PARADO 
                                          || ( direcao(elevador) == ELEVADOR_SUBINDO && porta(elevador) == PORTA_ABERTA ) ) ){
      printf("SUBIR\n");
      uint8_t efetuado = fecharPorta(elevador);
      if(porta(elevador) == PORTA_ABERTA) continue;
      
      subir(elevador);
      alterarDirecao(elevador, ELEVADOR_SUBINDO);
    }
    else if( precisaDescer(elevador) && (direcao(elevador) == ELEVADOR_PARADO 
                                          || ( direcao(elevador)==ELEVADOR_DESCENDO && porta(elevador) == PORTA_ABERTA ) ) ){
      printf("DESCER\n");
      uint8_t efetuado = fecharPorta(elevador);
      if(porta(elevador) == PORTA_ABERTA) continue;
      
      descer(elevador);
      alterarDirecao(elevador, ELEVADOR_DESCENDO);
    }
  }  
  
  return 0;
}

void sleep() {
      for(uint32_t i=0; i<4000000; i++);
  
}



