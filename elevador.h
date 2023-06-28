#define ELEVADOR_ESQUERDO 'e'
#define ELEVADOR_CENTRAL 'c'
#define ELEVADOR_DIREITO 'd'

#define QT_ANDARES 16

#define INICIALIZA 'r'
#define ABRE_PORTAS 'a'
#define FECHA_PORTAS 'f'
#define SOBE 's'
#define DESCE 'd'
#define PARA 'p'
#define CONSULTA 'x'
#define LIGAR_BOTAO 'L'
#define DESLIGAR_BOTAO 'D'

#define PORTA_ABERTA 'A'
#define PORTA_FECHADA 'F'
#define NUMERO 0x30

#define BOTAO_INTERNO_PRESS 'I'
#define BOTAO_EXTERNO_PRESS 'E'

#define BOTAO_CORREDOR_SOBE 's'
#define BOTAO_CORREDOR_DESCE 'd'

#define ELEVADOR_SUBINDO 1
#define ELEVADOR_DESCENDO -1
#define ELEVADOR_PARADO 0

#define TENTATIVAS_PORTA 3000


void esperarIniciar(); // Espera "initialized" do simulador

// OPERAÇÕES ELEVADOR
uint8_t getAndar(uint8_t elevador_cod); //Pega o andar

void inicializar(uint8_t elevador_cod);    //Inicializa o elevador

uint8_t fecharPorta(uint8_t elevador_cod); //Manda fechar elevador e aguarda mensagem
uint8_t abrirPorta(uint8_t elevador_cod);  //Manda abrir elevador e aguarda mensagem

int32_t consultar(uint8_t elevador_cod);   //Consulta posição
void atualizarPosicao(uint8_t elevador_cod);
void ajustePosicao(uint8_t elevador_cod); //Ajusta elevador no andar

void subir(uint8_t elevador_cod);          //Manda subir
void descer(uint8_t elevador_cod);         //Manda descer
void parar(uint8_t elevador_cod);          //Manda parar

void alterarEstado(uint8_t elevador_cod, int8_t estado); //Atualiza estado
void alterarDirecao(uint8_t elevador_cod, int8_t direcao); //Atualiza direcao
void desmarcarFila(uint8_t elevador_cod, uint8_t andar); //Retira andar da fila

uint8_t precisaSubir(uint8_t elevador_cod);    //Retorna se tem andar solicitado acima
uint8_t precisaDescer(uint8_t elevador_cod);   //Retorna se tem andar solicitado abaixo
uint8_t precisaParar(uint8_t elevador_cod);    //Retorna se precisa parar no andar atual

int8_t estado(uint8_t elevador_cod); //Retorna estado de movimentação (ELEVADOR_PARADO, ELEVADOR_SUBINDO, ELEVADOR_DESCENDO)
int8_t direcao(uint8_t elevador_cod);//Retorna direcao(ELEVADOR_PARADO, ELEVADOR_SUBINDO, ELEVADOR_DESCENDO)
int8_t porta(uint8_t elevador_cod);  //Retorna estado da porta (PORTA_ABERTA, PORTA_FECHADA)

// BOTÕES
void desligarBotao(uint8_t elevador_cod, uint8_t andar); //Desligar botão

// COMANDO
void decodificaComando(uint8_t *comando); //Decodifica comando


