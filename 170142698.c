/* Gabriel Fernandes Carvalho */
/* 170142698 */
/* APC(Algoritmos e Programacao de Computadores)*/
/* Trabalho 3 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define MAX_LINHAS_MAPA 30
#define MAX_COLUNAS_MAPA 80
#define MAX_VAMPIROS 50

#define TRUE 1
#define FALSE 0

typedef enum Tipo_vampiro {DRACULA, VAMPIRO} Tipo_vampiro;
typedef enum resultado_batalha {JOGADOR_GANHOU, JOGADOR_PERDEU, JOGADOR_FUGIU} resultado_batalha;

/* Usada para representar os elementos no mapa */
typedef struct coordenada
{
	int linha, coluna;
} coordenada;

typedef struct Combate_info
{
	int hp_max;
	int hp;
	int pocoes;
	int ataque;
	int precisao;
	int atordoamento;
} Combate_info;

/* Armazena informacoes dos vampiros*/
typedef struct Vampiro_info
{
	Tipo_vampiro tipo;
	int level;
	int turnos_para_reviver;
	int esta_vivo;
	coordenada posicao;
} Vampiro_info;

/* Armazena as informacoes do jogador */
typedef struct Usuario
{
	coordenada posicao;
	int vidas;
	int esta_vivo;
	int level;
	Combate_info status;

	int em_cima_de_objeto;
	char objeto;
} Usuario;

typedef struct Jogo_info
{
	int n_vampiros, n_linhas, n_colunas;
	Usuario jogador;

	char mapa[MAX_LINHAS_MAPA][MAX_COLUNAS_MAPA];
	Vampiro_info vampiros[MAX_VAMPIROS];
} Jogo_info;




void salva(Jogo_info *jogo)
{
	FILE *save_game_file = fopen("jogo.bin", "wb");
	if (save_game_file == NULL)
	{
		printf("aqui\n");
		exit(EXIT_FAILURE);
	}

	fwrite(jogo, sizeof(Jogo_info), 1, save_game_file);

	printf("Jogo Salvo!!!\n");

	fclose(save_game_file);
}


void carrega_mapa(Jogo_info *jogo)
{
	int i, j, c;
	FILE *mapa_file = fopen("mapa.txt", "r");
	if (mapa_file == NULL)
	{
		printf("aqui\n");
		exit(EXIT_FAILURE);
	}

	if (fscanf(mapa_file, "%d", &jogo->n_colunas) != 1)
	{
		printf("Nao foi possivel ler o numero de colunas do mapa\n");
		exit(EXIT_FAILURE);
	}

	if (fscanf(mapa_file, "%d", &jogo->n_linhas) != 1)
	{
		printf("Nao foi possivel ler o numero de linhas do mapa\n");
		exit(EXIT_FAILURE);
	}

	if (jogo->n_linhas < 20 || jogo->n_linhas > MAX_LINHAS_MAPA ||
		jogo->n_colunas < 60 ||jogo->n_colunas > MAX_COLUNAS_MAPA)
	{
		printf("Tamanho do mapa invalido\n");
		exit(EXIT_FAILURE);
	}

	/* Le mapa */
	for (i = 0; i < jogo->n_linhas; i++)
	{
		for (j = 0; j < jogo->n_colunas;)
		{
			c = fgetc(mapa_file);
			if (c == EOF)
			{
				printf("aqui\n");
				exit(EXIT_FAILURE);
			}

			if (c != '\n' && c != '\r')
			{
				if (c != '#' && c != ' ' && c != '+' && c != '-')
				{
					printf("Caracter invalido no mapa\n");
					exit(EXIT_FAILURE);
				}

				jogo->mapa[i][j++] = c;
			}
		}
	}

	/* Le posicao do jogador */
	if (fscanf(mapa_file, "%d", &jogo->jogador.posicao.linha) != 1 ||
		fscanf(mapa_file, "%d", &jogo->jogador.posicao.coluna) != 1)
	{
		printf("Nao foi possivel ler a posicao do jogador\n");
		exit(EXIT_FAILURE);
	}
	jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = '@';


	/* Le vampiros */
	if (fscanf(mapa_file, "%d", &jogo->n_vampiros) != 1)
	{
		printf("Nao foi possivel ler o numero de vampiros\n");
		exit(EXIT_FAILURE);
	}
	jogo->n_vampiros++; /* Adiciona o Dracula */

	if (jogo->n_vampiros >= MAX_VAMPIROS)
	{
		printf("Limite de vampiros ultrapassado\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < jogo->n_vampiros; i++)
	{
		if (fscanf(mapa_file, "%d", &jogo->vampiros[i].posicao.coluna) != 1 ||
			fscanf(mapa_file, "%d", &jogo->vampiros[i].posicao.linha) != 1 ||
			fscanf(mapa_file, "%d", &jogo->vampiros[i].level) != 1)
		{
			printf("Nao foi possivel ler as informacoes do vampiro %d\n", i);
			exit(EXIT_FAILURE);
		}

		if (i == jogo->n_vampiros - 1)
		{
			jogo->vampiros[i].tipo = DRACULA;
			jogo->vampiros[i].esta_vivo = TRUE;
			jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] = 'D';
		}
		else
		{
			jogo->vampiros[i].tipo = VAMPIRO;
			jogo->vampiros[i].esta_vivo = TRUE;
			jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] = 'V';
		}
	}


	fclose(mapa_file);
}


void imprimir_mapa(Jogo_info *jogo)
{
	int i,j;
	
	printf("Vidas: %d\t Hp: %d/%d\t Level:%d\t Pocoes:%d\n", jogo->jogador.vidas, jogo->jogador.status.hp, jogo->jogador.status.hp_max, jogo->jogador.level, jogo->jogador.status.pocoes);

	for(i = 0; i < jogo->n_linhas; i++)
	{
		for(j = 0; j < jogo->n_colunas; j++)
		{
			printf("%c",jogo->mapa[i][j]);
		}
		putchar('\n');
	}
}


void carrega_itens(Jogo_info *jogo)
{
	FILE *itens_file = fopen("mapa.txt", "r");
	if (itens_file == NULL)
	{
		printf("aqui\n");
		exit(EXIT_FAILURE);
	}
}


char getch()/*le um caracter da entrada padrão sem o bloqueio de entrada(nao necessita apertar enter) */
{
	int ch;
	struct termios oldt;
	struct termios newt;
	tcgetattr(STDIN_FILENO,&oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

int e_divisoria(char c)
{
	return c == '#' || c == '+' || c == '-';
}

void abre_a_porta(Jogo_info *jogo, int indice)
{
	int i, coluna_min, coluna_max, linha;

	for (coluna_min = jogo->vampiros[indice].posicao.coluna;
		 !e_divisoria(jogo->mapa[jogo->vampiros[indice].posicao.linha][coluna_min]);
		 coluna_min--);

	for (coluna_max = jogo->vampiros[indice].posicao.coluna;
		 !e_divisoria(jogo->mapa[jogo->vampiros[indice].posicao.linha][coluna_max]);
		 coluna_max++);

	linha = jogo->vampiros[indice].posicao.linha;
	do
	{
		linha--;
		for (i = coluna_min; i <= coluna_max; i++)
		{
			if(jogo->mapa[linha][i] == '+')
			{
				jogo->mapa[linha][i] = '-';
			}
		}
	} while(!e_divisoria(jogo->mapa[linha][jogo->vampiros[indice].posicao.coluna]));

	linha = jogo->vampiros[indice].posicao.linha;
	do
	{
		linha++;
		for (i = coluna_min; i <= coluna_max; i++)
		{
			if(jogo->mapa[linha][i] == '+')
			{
				jogo->mapa[linha][i] = '-';
			}
		}
	} while(!e_divisoria(jogo->mapa[linha][jogo->vampiros[indice].posicao.coluna]));

	linha = jogo->vampiros[indice].posicao.linha;
	for (i = coluna_min; i <= coluna_max; i++)
	{
		if(jogo->mapa[linha][i] == '+')
		{
			jogo->mapa[linha][i] = '-';
		}
	}
}


/* Fornece os atributos iniciais do jogador */
void inicia_jogador(Usuario *jogador)
{
	jogador->level = 1;
	jogador->esta_vivo = TRUE;
	jogador->em_cima_de_objeto = FALSE;

	jogador->status.hp_max = 100;
	jogador->status.hp = 100;
	jogador->status.pocoes = 0;
	jogador->status.ataque = 5;
	jogador->status.precisao = 50;
	jogador->status.atordoamento = 20;
}


void inicia_jogo(Jogo_info *jogo)
{
	int opcao;
	FILE *save_game_file = fopen("jogo.bin", "rb");

	jogo->jogador.vidas = 5;

	do
	{
		printf("Escolha uma opcao:\n");
		printf("1 - Novo jogo\n");
		if (save_game_file != NULL)
			printf("2 - Carregar jogo\n");
		scanf("%d", &opcao);

		while(getchar() != '\n');
	} while (opcao != 1 && (save_game_file == NULL || opcao != 2));

	system("clear");

	if (opcao == 1)
	{
		carrega_mapa(jogo);
		carrega_itens(jogo);
		inicia_jogador(&jogo->jogador);
	}
	else if (opcao == 2)
	{
		/* Carrega jogo salvo */
		fread(jogo, sizeof(Jogo_info), 1, save_game_file);
	}
	else
	{
		printf("aqui\n");
		exit(EXIT_FAILURE);
	}


	if (save_game_file != NULL)
		fclose(save_game_file);
}


void movimentacao_usuario(char letra, Jogo_info *jogo)
{
	int proximo_x, proximo_y;

	switch (letra)
	{
		case 'w':
		case 'W':
			proximo_x = jogo->jogador.posicao.linha - 1;
			proximo_y = jogo->jogador.posicao.coluna;
			break;

		case 'a':
		case 'A':
			proximo_x = jogo->jogador.posicao.linha;
			proximo_y = jogo->jogador.posicao.coluna - 1;
			break;

		case 's':
		case 'S':
			proximo_x = jogo->jogador.posicao.linha + 1;
			proximo_y = jogo->jogador.posicao.coluna;
			break;

		case 'd':
		case 'D':
			proximo_x = jogo->jogador.posicao.linha;
			proximo_y = jogo->jogador.posicao.coluna + 1;
			break;
	}

	switch (jogo->mapa[proximo_x][proximo_y])
	{
		case 'V':
		case '+':
		case '#':
		printf("CAMINHO BLOQUEADO!!!!!!\n");
		break;

		case '-':
		jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = jogo->jogador.em_cima_de_objeto ? jogo->jogador.objeto : ' ';
		jogo->jogador.em_cima_de_objeto = TRUE;
		jogo->jogador.objeto = '-';
		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

		case 'M':
		jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = jogo->jogador.em_cima_de_objeto ? jogo->jogador.objeto : ' ';
		jogo->jogador.em_cima_de_objeto = TRUE;
		jogo->jogador.objeto = 'M';
		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

		case ' ':
		if (jogo->jogador.em_cima_de_objeto)
		{
			jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = jogo->jogador.objeto;
			jogo->jogador.em_cima_de_objeto = FALSE;
		}
		else
			jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = ' ';

		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

		case 'p':
		jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = ' ';
		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.status.pocoes++;
		printf("voce ganhou mais uma pocao!\n");
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

		case 'w':
		jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = ' ';
		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.status.ataque += 10;
		printf("voce ganhou mais 10 pontos de ataque!\n");
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

		case 'a':
		jogo->mapa[jogo->jogador.posicao.linha][jogo->jogador.posicao.coluna] = ' ';
		jogo->mapa[proximo_x][proximo_y] = '@';
		jogo->jogador.status.hp_max += 20;
		jogo->jogador.status.hp += 20;
		printf("voce ganhou mais 20 pontos de hp_max e recuperou 20 pontos de hp!\n");
		jogo->jogador.posicao.linha = proximo_x;
		jogo->jogador.posicao.coluna = proximo_y;
		break;

	}
}


/* Promove uma movimentacao aleatoria dos vampiros */
void movimentacao_vampiros(Jogo_info *jogo)
{
	int i, proximo_x, proximo_y, movimento;

	for (i = 0; i < jogo->n_vampiros; i++)
	{
		if(!jogo->vampiros[i].esta_vivo)
		{
			continue;
		}

		movimento = rand() % 4;

		switch (movimento)
		{
			case 0:
				proximo_x = jogo->vampiros[i].posicao.linha - 1;
				proximo_y = jogo->vampiros[i].posicao.coluna;
				break;

			case 1:
				proximo_x = jogo->vampiros[i].posicao.linha;
				proximo_y = jogo->vampiros[i].posicao.coluna - 1;
				break;

			case 2:
				proximo_x = jogo->vampiros[i].posicao.linha + 1;
				proximo_y = jogo->vampiros[i].posicao.coluna;
				break;

			case 3:
				proximo_x = jogo->vampiros[i].posicao.linha;
				proximo_y = jogo->vampiros[i].posicao.coluna + 1;
				break;
		}

		if (jogo->mapa[proximo_x][proximo_y] == ' ')
		{
			jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] = ' ';

			if (jogo->vampiros[i].tipo == VAMPIRO)
				jogo->mapa[proximo_x][proximo_y] = 'V';
			else
				jogo->mapa[proximo_x][proximo_y] = 'D';

			jogo->vampiros[i].posicao.linha = proximo_x;
			jogo->vampiros[i].posicao.coluna = proximo_y;
		}
	}
}


/* Verifica o caractere colocado pelo usuario e chama as outras funcoes de movimentacao*/
void movimentacao(Jogo_info *jogo)
{
	char letra;


	printf("Digite W ou w para se mover para cima,\n");
	printf("Digite A ou a para se mover para esquerda,\n");
	printf("Digite S ou s para se mover para baixo,\n");
	printf("Digite D ou d para se mover para direita\n");
	printf("Digite Z ou z para salvar o jogo.\n");
	printf("Digite X ou x para salvar o jogo e sair.\n");

	do
	{
		letra = getch();

		if(letra == 'X' || letra == 'x')
		{
			salva(jogo);
			exit(EXIT_SUCCESS);
		}
		else if(letra == 'Z' || letra == 'z')
		{
			salva(jogo);
		}
		else if(letra != 'W' && letra != 'w' &&
			letra != 'A' && letra != 'a' &&
			letra != 'S' && letra != 's' &&
			letra != 'D' && letra != 'd')
		{
			printf("\nComando invalido!!!!\n");
		}
		else
		{
			break;
		}
	} while(1);

	system("clear");

	movimentacao_usuario(letra, jogo);
	movimentacao_vampiros(jogo);
}


Combate_info novo_vampiro(int level)
{
	Combate_info vampiro;

	vampiro.hp_max = 100 + (10 * level);
	vampiro.hp = vampiro.hp_max;
	vampiro.pocoes = 0;
	vampiro.ataque = 5 + level;
	vampiro.precisao = 40 + (2 * level);
	vampiro.atordoamento = 20 + (3 * level);

	return vampiro;
}

/*Funcao responsavel por apresentar as possiveis formas de acao do usuario*/
int EscolhaDoComando_usuario()
{ 
	int comando_usuario;

	printf("Escolha alguma das opcoes abaixo e aperte <ENTER>:\n");
	putchar('\n');
	printf("1) Ataque Rapido\n");
	printf("2) Ataque Forte\n");
	printf("3) Defender\n");
	printf("4) Usar Pocao\n");
	printf("5) Fugir\n");
	printf("opcao:");
	do{
		scanf("%d", &comando_usuario);
		while(getchar() != '\n'); /*Limpa o buffer do teclado para nao aparecer varias mensagens de erro ao colocar varias letras*/
		if(comando_usuario<1 || comando_usuario>5)
			printf("Erro! Coloque o numero da opcao e aperte <ENTER>:\n");
	}while(comando_usuario<1 || comando_usuario>5);

	return comando_usuario;
}

/*Essa funcao faz com que, aleatoriamente, o computador	escolha alguma das acoes*/
int EscolhaDoComando_inimigo()
{ 
	int comando_inimigo;
	srand((unsigned)time(NULL));
	comando_inimigo= 1 + rand()% 4;

	return comando_inimigo;
}

/*Funcao que mostra se os ataques foram eficazes ou nao*/
int precisao()
{ 
	int precisao;
	srand((unsigned)time(NULL));
	precisao = 1 + rand()% 100;

	return precisao;
}

/* Funcao responsavel por decidir se o vampiro sera ou nao atordoado */
int atordoamento()
{
	int atordoamento;
	srand((unsigned)time(NULL));
	atordoamento = 1 + rand()% 100;

	return atordoamento;
}

resultado_batalha fight(Usuario *jogador, int vampiro_level)
{
	int comando_usuario, comando_inimigo;
	Combate_info vampiro = novo_vampiro(vampiro_level);

	printf("*****************************************\n");
	printf("           BATALHA DE VAMPIROS           \n");
	printf("_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_\n");


	printf("O seu personagem esta representado na esquerda e o seu oponente a direita.\n\n");
	printf("*******************************************************************************\n");
	printf("            Level:%d                     :|:              Level:%d\n", jogador->level, vampiro_level);
	printf("			ALIADO		:|:             INIMIGO\n");
	printf("HIT POINTS:	%d/%d			:|:       HIT POINTS:  %d/%d\n", jogador->status.hp, jogador->status.hp_max, vampiro.hp, vampiro.hp_max);
	printf("POCOES:		0%d			:|:       POCOES:      0%d \n", jogador->status.pocoes, vampiro.pocoes);
	printf("ATAQUE:		%d			:|:       ATAQUE:      %d\n", jogador->status.ataque, vampiro.ataque);
	printf("PRECISAO:	0%d%%			:|:       PRECISAO:    0%d%%\n\n\n", jogador->status.precisao, vampiro.precisao);
	putchar('\n');
	while(jogador->status.hp > 0 || vampiro.hp > 0)
	{
		if (vampiro.hp < 5)
		{
			comando_inimigo = 5;
		}
		else
		{
			comando_usuario = EscolhaDoComando_usuario();
			comando_inimigo = EscolhaDoComando_inimigo();
		}

		/*Aqui estao descritas as possibilidades de escolha do usuario e do computador*/
		if(comando_usuario == 1 && comando_inimigo == 1)
		{ 
		  	/*Assim como as consequencias de cada escolha*/
			if(jogador->status.precisao >= precisao())
			{				
				vampiro.hp = vampiro.hp - jogador->status.ataque;
				printf("Ataque bem sucedido!\n");
			}	
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - vampiro.ataque;
				printf("Voce foi atacado!\n");
			}
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Inimigo errou o ataque\n");
			}
		}
		else if(comando_usuario == 1 && comando_inimigo == 2)
		{
			if(jogador->status.precisao >= precisao())
			{
				vampiro.hp = vampiro.hp - jogador->status.ataque;
				printf("Ataque bem sucedido!\n");
			}
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - (vampiro.ataque*2);
				printf("Voce foi atacado fortemente! Inimigo passivel de atordoamento\n");
				
			}
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Inimigo errou o ataque\n");
			}
			if(vampiro.atordoamento >= atordoamento())
			{
				printf("O inimigo foi Atordoado!\n");
			}

		}
		else if(comando_usuario == 2 && comando_inimigo == 1)
		{
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - vampiro.ataque;
				printf("Voce foi atacado\n");
			}
			if(jogador->status.precisao >= precisao())
			{
				vampiro.hp = vampiro.hp - (jogador->status.ataque*2);
				printf("Voce atacou fortemente o inimigo! Voce esta passivel de atordoamento\n");
				
			}
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Inimigo errou o ataque\n");
			}
			if(jogador->status.atordoamento >= atordoamento())
			{
				printf("Voce foi Atordoado!\n");
			}

		}
		else if(comando_usuario == 2 && comando_inimigo == 2)
		{
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - (vampiro.ataque*2);
				printf("Voce foi atacado fortemente!\n");
			}
			if(jogador->status.precisao >= precisao())
			{
				vampiro.hp = vampiro.hp - (jogador->status.ataque*2);
				printf("Voce atacou fortemente!\n");
				
			}
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Inimigo errou o ataque\n");
			}
		}
		else if(comando_usuario == 1 && comando_inimigo == 3)
		{
			if(jogador->status.precisao >= precisao())
			{
				vampiro.hp = vampiro.hp - (jogador->status.ataque/2);
				printf("Ataque bem sucedido mas defendido!\n");
			}	
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido!\n");
			}
			
		}
		else if(comando_usuario == 3 && comando_inimigo == 1)
		{
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - (vampiro.ataque/2);
				printf("Voce foi atacado mas se defendeu!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Ataque inimigo mal sucedido!\n");
			}
		}
		else if(comando_usuario == 2 && comando_inimigo == 3)
		{
			if(jogador->status.precisao >= precisao())
			{
				vampiro.hp = vampiro.hp - (jogador->status.ataque/2);
				printf("Ataque forte bem sucedido mas defendido!\n");
			}	
			if(jogador->status.precisao < precisao())
			{
				printf("Ataque mal sucedido e o inimigo se defendeu!\n");
			}
			printf("Voce ficou atordoado porque o inimigo se defendeu!\n");
		}
		else if(comando_usuario == 3 && comando_inimigo == 2)
		{
			if(vampiro.precisao >= precisao())
			{
				jogador->status.hp = jogador->status.hp - (vampiro.ataque/2);
				printf("Voce foi atacado fortemente mas se defendeu!\n");
			}
			if(vampiro.precisao < precisao())
			{
				printf("Ataque inimigo mal sucedido e voce se defendeu!\n");
			}
			printf("O inimigo ficou atordoado porque voce se defendeu!\n");
		}
		else if(comando_usuario == 3 && comando_inimigo == 3)
		{
			jogador->status.hp += 0.1*jogador->status.hp;
			vampiro.hp += 0.1*vampiro.hp;
			printf("Ambos se defenderam e ganharam 10%% a mais de hp!\n");
		}
		else if((comando_usuario == 1 || comando_usuario == 2)&& comando_inimigo == 4)
		{
			if(vampiro.pocoes >= 1)
			{
				vampiro.hp += 30;
				vampiro.pocoes--;
				printf("O inimigo tomou uma pocao e voce o atacou!\n");
			}
			vampiro.hp -= jogador->status.ataque;
			printf("O inimigo nao tem nenhuma pocao e voce o atacou!\n");
		}
		else if((comando_inimigo == 1 || comando_inimigo == 2)&& comando_usuario == 4)
		{
			if(jogador->status.pocoes >= 1)
			{
				jogador->status.hp +=30;
				jogador->status.pocoes--;
				printf("Voce tomou uma pocao e foi atacado!\n");
			}
			else if (jogador->status.pocoes == 0)
			{ 
				printf("Voce nao possui mais pocoes e foi atacado!\n");
				jogador->status.hp -= vampiro.ataque;
				jogador->status.pocoes--;
			}
		}
		else if(comando_usuario == 3 && comando_inimigo == 4)
		{
			if(vampiro.pocoes >= 1)
			{
				vampiro.hp += 30;
				vampiro.pocoes--;
				printf("O inimigo tomou uma pocao enquanto voce se defendia!\n");
			}
			else if(vampiro.pocoes == 0)
			{
			printf("O inimigo nao tem nenhuma pocao e voce se defendeu!\n");
			}
		}
		else if(comando_usuario == 4 && comando_inimigo == 3)
		{
			if(jogador->status.pocoes >= 1)
			{
				jogador->status.hp += 30;
				jogador->status.pocoes--;
				printf("Voce tomou uma pocao enquanto o inimigo se defendia!\n");
			}
			else
				printf("Voce nao tem pocoes, o inimigo se defendeu\n");
		}
		else if(comando_usuario == 4 && comando_inimigo == 4)
		{
			if(jogador->status.pocoes >= 1 && vampiro.pocoes >= 1)
			{
				jogador->status.hp += 30;
				jogador->status.pocoes--;
			
				vampiro.hp += 30;
				vampiro.pocoes--;
				printf("Voce e o inimigo tomaram uma pocao!\n");
			}
			else if (jogador->status.pocoes>= 1 && vampiro.pocoes == 0)
			{
				jogador->status.hp += 30;
				jogador->status.pocoes--;
				printf("Voce tomou uma pocao e o inimigo nao tinha nenhuma para tomar.\n");
			}
			else if(jogador->status.pocoes == 0 && vampiro.pocoes>= 1)
			{
				vampiro.hp += 30;
				vampiro.pocoes--;
				printf("Voce nao possui mais pocoes e o inimigo tomou uma pocao!\n");
			}
			else if(jogador->status.pocoes == 0 && vampiro.pocoes == 0)
			{
				printf("Nenhum dos dois possuem pocoes!\n");
			}
			
		}
		else if(comando_usuario == 5)
		{
			printf("Fugiu eh?!!hahahaha\n");
			printf("PeRdEu!!\n");
			return JOGADOR_FUGIU;
			
		}
		else if(comando_inimigo == 5)
		{
			if (vampiro.hp > 0)
				printf("O inimigo fugiu!\n");
			printf("Parabens!! Voce ganhou!\n");
			return JOGADOR_GANHOU;
			
		}
		if(jogador->status.hp > jogador->status.hp_max)
		{
			jogador->status.hp = jogador->status.hp_max;
		}
		if(vampiro.hp > vampiro.hp_max)
		{
			vampiro.hp = vampiro.hp_max;
		}
		if(jogador->status.hp <= 0)
		{
			break;
		}
		if(vampiro.hp <= 0)
		{
			break;
		}
		if(jogador->status.pocoes < 0)
			jogador->status.pocoes = 0;
		if(vampiro.pocoes < 0)
			vampiro.pocoes = 0;
		printf("\n\n\n\nO seu personagem esta representado na esquerda e o seu oponente a direita.\n\n");
		printf("*******************************************************************************\n");
		printf("            Level:%d                     :|:              Level:%d\n", jogador->level, vampiro_level);
		printf("			ALIADO		:|:             INIMIGO\n");
		printf("HIT POINTS:	%d/%d			:|:       HIT POINTS:  %d/%d\n", jogador->status.hp, jogador->status.hp_max, vampiro.hp, vampiro.hp_max);
		printf("POCOES:		0%d			:|:       POCOES:      0%d \n", jogador->status.pocoes, vampiro.pocoes);
		printf("ATAQUE:		%d			:|:       ATAQUE:      %d\n", jogador->status.ataque, vampiro.ataque);
		printf("PRECISAO:	0%d%%			:|:       PRECISAO:    0%d%%\n\n\n", jogador->status.precisao, vampiro.precisao);
		putchar('\n');
	}

	if(jogador->status.hp <= 0)
	{
		printf("\n\n\nDepois desse ataque voce ja era!Perdedor!\n");
		return JOGADOR_PERDEU;
	}
	else {
		printf("\n\n\nVoce destruiu ele com o ultimo ataque!Parabens pela vitoria!!\n");
		return JOGADOR_GANHOU;                                                       
	}
}


/* Atualiza os atributos do usuario conforme o level*/
void aumenta_level_usuario(Usuario *jogador)
{
	jogador->level++;
	jogador->status.hp_max += 10;
	jogador->status.hp += 10;
	jogador->status.ataque += 1;
	jogador->status.precisao += 5;
	jogador->status.atordoamento += 5;
	if (jogador->status.precisao > 100)
		jogador->status.precisao = 100;
	if (jogador->status.atordoamento > 100)
		jogador->status.atordoamento = 100;
}


void verifica_combate(Jogo_info *jogo)
{
	resultado_batalha resultado;
	int i, k, l;

	for(i = 0; i < jogo->n_vampiros; i++)
	{
		if (!jogo->vampiros[i].esta_vivo)
			continue;

		for (k = -2; k <= 2; k++)
		{
			for (l = -2; l <= 2; l++)
			{
				if (jogo->jogador.posicao.linha + k == jogo->vampiros[i].posicao.linha &&
					jogo->jogador.posicao.coluna + l == jogo->vampiros[i].posicao.coluna)
				{
					if ((k > 0 && (jogo->mapa[jogo->jogador.posicao.linha + 1][jogo->jogador.posicao.coluna + l] == '#' || jogo->mapa[jogo->jogador.posicao.linha + 1][jogo->jogador.posicao.coluna + l] == '-' || jogo->mapa[jogo->jogador.posicao.linha + 1][jogo->jogador.posicao.coluna + l] == '+')) ||
						(k < 0 && (jogo->mapa[jogo->jogador.posicao.linha - 1][jogo->jogador.posicao.coluna + l] == '#' || jogo->mapa[jogo->jogador.posicao.linha - 1][jogo->jogador.posicao.coluna + l] == '-' || jogo->mapa[jogo->jogador.posicao.linha - 1][jogo->jogador.posicao.coluna + l] == '+')) ||
						(l > 0 && (jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna + 1] == '#' || jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna + 1] == '-' || jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna + 1] == '+')) ||
						(l < 0 && (jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna - 1] == '#' || jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna - 1] == '-' || jogo->mapa[jogo->jogador.posicao.linha + k][jogo->jogador.posicao.coluna - 1] == '+')))
					{
						continue;
					}
					printf("FIGHT!!!\n");
					resultado = fight(&jogo->jogador, jogo->vampiros[i].level);

					switch (resultado)
					{
						case JOGADOR_GANHOU:
							jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] = 'M';
							jogo->vampiros[i].esta_vivo = FALSE;
							jogo->vampiros[i].turnos_para_reviver = 5;
							abre_a_porta(jogo, i);
							jogo->jogador.status.hp += 25;
							if (jogo->jogador.status.hp > jogo->jogador.status.hp_max)
								jogo->jogador.status.hp = jogo->jogador.status.hp_max;
							aumenta_level_usuario(&jogo->jogador);
							break;

						case JOGADOR_PERDEU:											
							jogo->jogador.esta_vivo = FALSE;
							break;

						case JOGADOR_FUGIU:
							jogo->vampiros[i].level++;
							break;
					}
				}
			}
		}
	}
}


int dracula_morto(Jogo_info *jogo)
{
	return jogo->vampiros[jogo->n_vampiros - 1].esta_vivo == FALSE;
}


void revive_vampiros(Jogo_info *jogo)
{
	int i;

	for (i = 0; i < jogo->n_vampiros; i++)
	{
		if(jogo->vampiros[i].turnos_para_reviver == 0)
		{
			if (jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] == 'M')
			{
				jogo->vampiros[i].esta_vivo = TRUE;
				jogo->mapa[jogo->vampiros[i].posicao.linha][jogo->vampiros[i].posicao.coluna] = 'V';
			}
		}
		else
		{
			jogo->vampiros[i].turnos_para_reviver--;
		}
	}
}


int main()
{
	Jogo_info jogo;

	system("clear");
	srand((unsigned)time(NULL));

	inicia_jogo(&jogo);

	while (jogo.jogador.vidas != 0)
	{
		imprimir_mapa(&jogo);

		while (jogo.jogador.esta_vivo)
		{
			movimentacao(&jogo);
			verifica_combate(&jogo);
			revive_vampiros(&jogo);
			imprimir_mapa(&jogo);

			if (dracula_morto(&jogo))
			{
				printf("Ganhou\n");
				return 0;
			}
		}

		if (jogo.jogador.vidas != 0)
		{
			jogo.jogador.vidas--;
			inicia_jogador(&jogo.jogador);
			carrega_mapa(&jogo);
			carrega_itens(&jogo);
			system("clear");
		}
	}

	return 0;
}
