/* Gabriel Fernandes Carvalho */
/* 170142698 */
/* APC(Algoritmos e Programacao de Computadores)*/
/* Trabalho 3 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINHAS_MAPA 30
#define MAX_COLUNAS_MAPA 80
#define MAX_VAMPIROS 50

#define TRUE 1
#define FALSE 0

typedef enum Tipo_vampiro {DRACULA, VAMPIRO} Tipo_vampiro;

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
	int sala_atual;
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

	char mapa[MAX_LINHAS_MAPA][MAX_COLUNAS_MAPA];
	Vampiro_info vampiros[MAX_VAMPIROS];
	Usuario jogador;
} Jogo_info;


void carrega_mapa(Jogo_info *jogo)
{
	int i, j, c;
	FILE *mapa_file = fopen("mapa.txt", "r");
	if (mapa_file == NULL)
	{
		printf("fudeu\n");
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
		for (j = 0; j < jogo->n_colunas; j++)
		{
			c = fgetc(mapa_file);
			if (c == EOF)
			{
				printf("fudeu\n");
				exit(EXIT_FAILURE);
			}

			if (c != '\n' && c != '\r')
			{
				if (c != '#' && c != ' ' && c != '+' && c != '-')
				{
					printf("Caracter invalido no mapa\n");
					exit(EXIT_FAILURE);
				}

				jogo->mapa[i][j] = c;
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


void carrega_itens(Jogo_info *jogo)
{
	FILE *itens_file = fopen("mapa.txt", "r");
	if (itens_file == NULL)
	{
		printf("fudeu\n");
		exit(EXIT_FAILURE);
	}
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

	if (opcao == 1)
	{
		carrega_mapa(jogo);
		carrega_itens(jogo);
	}
	else if (opcao == 2)
	{
		/* Carrega jogo */
		fread(jogo, sizeof(Jogo_info), 1, save_game_file);
	}
	else
	{
		printf("fudeu\n");
		exit(EXIT_FAILURE);
	}


	if (save_game_file != NULL)
		fclose(save_game_file);
}


int dracula_morto(Jogo_info *jogo)
{
	return jogo->vampiros[jogo->n_vampiros - 1].esta_vivo;
}


/* Fornece os atributos iniciais do jogador */
void inicia_jogador(Usuario *jogador)
{
	jogador->level = 1;
	jogador->esta_vivo = TRUE;
	jogador->sala_atual = 0;
	jogador->em_cima_de_objeto = FALSE;

	jogador->status.hp_max = 100;
	jogador->status.hp = 100;
	jogador->status.pocoes = 0;
	jogador->status.ataque = 5;
	jogador->status.precisao = 50;
	jogador->status.atordoamento = 20;
}


int main()
{
	Jogo_info jogo;

	system("clear");
	srand((unsigned)time(NULL));

	inicia_jogo(&jogo);

	while (jogo.jogador.vidas != 0)
	{
		inicia_jogador(&jogo.jogador);

		while (jogo.jogador.esta_vivo)
		{
			movimentacao(mapa, &jogador);
			// imprimir_mapa(mapa, &jogador);
			// verifica_combate(mapa, &jogador, &salas[jogador.sala_atual]);

			if (dracula_morto(&jogo))
			{
				printf("Ganhou\n");
				return 0;
			}
		}

		if (jogo.jogador.vidas != 0)
		{
			jogo.jogador.vidas--;
			carrega_mapa(&jogo);
			carrega_itens(&jogo);
		}
	}

	return 0;
}