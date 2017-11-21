/* Gabriel Fernandes Carvalho */
/* 170142698 */
/* APC(Algoritmos e Programacao de Computadores)*/
/* Trabalho 3 */

#define MAX_LINHAS_MAPA 30
#define MAX_COLUNAS_MAPA 80

/* Armazena informacoes dos vampiros*/
typedef struct Vampiro_info
{
	int level;

	int hp_max;
	int hp;
	int pocoes;
	int ataque;
	int precisao;
	int atordoamento;
} Vampiro_info;

typedef struct jogo
{
	char mapa[MAX_LINHAS_MAPA][MAX_COLUNAS_MAPA];
} jogo;