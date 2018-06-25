
#include "SA.h"
#include "memoria.h"
#include "inttypes.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>


uint16_t aloca (void);
void escreva_bloco_dados(uint16_t numero ,uint8_t *valor);
void leia_bloco_dados(uint16_t numero, uint8_t *valor);

void le_entrada_arquivo (uint16_t numero, 	struct inodo *tmp)
{
	uint16_t posicao = 2 + numero*sizeof(struct inodo);
	memoria.read(posicao, sizeof(struct inodo), (uint8_t *) tmp);
}
void escreve_entrada( uint16_t numero , struct inodo tmp)
{
	uint16_t posicao = 2 + numero*sizeof(struct inodo);
	memoria.write(posicao, sizeof(struct inodo), (uint8_t *)&tmp);
}
	


MEU_FILE * meu_fopen (const char *st, const char *modo)
{
	MEU_FILE * tmp = NULL;
	struct inodo meu_inodo;
	if (strcmp(modo, "w")==0)
	{
		int livre = -1;
		for (int x=0;x<12;x++)
		{
			le_entrada_arquivo(x, &meu_inodo);
			if ((meu_inodo.status==0) && (livre==-1)) livre = x;
			if (strcmp(meu_inodo.nome, st)==0)
			{
					// trabalho pois precisa desalocar blocos
					
			}
		}
		meu_inodo.status = 1;
		meu_inodo.tam=0;
		strcpy(meu_inodo.nome,st);
		meu_inodo.indireto=0xffff;
		escreve_entrada( livre , meu_inodo );		
	
		tmp  = (MEU_FILE *) malloc (sizeof(MEU_FILE));
		tmp->id = livre;
		tmp->posicao = 0;
		
		
	}
	return tmp;
}


uint16_t le_cabecalho ()
{
	uint16_t numero;
	memoria.read(0, 2, (uint8_t *) &numero);
	return numero;
}
void escreve_cabecalho (uint16_t numero)
{
	memoria.write(0, 2, (uint8_t *) &numero);
}
void leia_entrada (uint8_t numero, struct inodo *i)
{
	uint16_t posicao = 2 + numero*sizeof(struct inodo);
	memoria.read(posicao, sizeof(struct inodo), (uint8_t *)i);
}

void escreva_entrada (uint8_t numero, struct inodo *i)
{
	uint16_t posicao = 2 + numero*sizeof(struct inodo);
	memoria.write(posicao, sizeof(struct inodo), (uint8_t *)i);
}

void meu_fseek ( MEU_FILE *A, uint16_t offset )
{
	A->posicao = offset;
}

int meu_fgetc( MEU_FILE *A )
{
	struct inodo lida;
	bloco_indice bloco_idx;
	bloco_dados  bloco;
	uint16_t n;
	uint8_t valor;
	
	
	if (A == NULL) return 0;
	uint16_t id = A->id;
	uint16_t posicao = A->posicao;
	
		
	leia_entrada(id, &lida);

	if (posicao < 32)
	{
		valor = lida.dados_diretos[posicao];
		
	}
	else  
	{	
		if (lida.indireto==0xFFFF) 
		{
			return 0;
		}
		uint16_t numero_entrada = posicao / 32;
		uint8_t deslocamento    = posicao % 32;
		numero_entrada--;
		
		
		leia_bloco_dados (lida.indireto, (uint8_t *) &bloco_idx);
		if (bloco_idx[numero_entrada]==0xffff)
		{
			return 0;
		}
		leia_bloco_dados (lida.indireto, (uint8_t *) &bloco_idx);
		n = bloco_idx[numero_entrada];
		
		leia_bloco_dados (n, (uint8_t *) &bloco);
		valor = bloco[deslocamento];

		 
		
	}
	
	A->posicao++;
	return valor;
}

int meu_feof (MEU_FILE *A)
{
	struct inodo lida;
	if (A == NULL) return 1;
	uint16_t id = A->id;

	
		
	leia_entrada(id, &lida);
	
	if (lida.tam > A->posicao) return 0;
	return 1;
	
}

void meu_fputc ( uint8_t valor , MEU_FILE *A )
{
	struct inodo lida;
	bloco_indice bloco_idx;
	bloco_dados  bloco;
	uint16_t n;
	
	
	if (A == NULL) return;
	uint16_t id = A->id;
	uint16_t posicao = A->posicao;
	
		
	leia_entrada(id, &lida);

	if (posicao < 32)
	{
		lida.dados_diretos[posicao] = valor;
		
	}
	else  
	{	
		if (lida.indireto==0xFFFF) 
		{
			lida.indireto = aloca ();
		}
		uint16_t numero_entrada = posicao / 32;
		uint8_t deslocamento    = posicao % 32;
		numero_entrada--;
		
		
		leia_bloco_dados (lida.indireto, (uint8_t *) &bloco_idx);
		if (bloco_idx[numero_entrada]==0xffff)
		{
			bloco_idx[numero_entrada] = aloca();
			escreva_bloco_dados(lida.indireto , (uint8_t *) bloco_idx);
		}
		leia_bloco_dados (lida.indireto, (uint8_t *) &bloco_idx);
		n = bloco_idx[numero_entrada];
		
		leia_bloco_dados (n, (uint8_t *) &bloco);
		bloco[deslocamento] = valor;
		escreva_bloco_dados(n ,  (uint8_t *) &bloco);
		 
		
	}
	if (A->posicao >= lida.tam) lida.tam++;
	A->posicao++;
	escreva_entrada(id, &lida);
	
}

void cria_entrada (uint8_t numero)
{
	struct inodo tmp;
	tmp.status = 0;
	tmp.tam=0;
	strcpy(tmp.nome,"VAZIO");
	tmp.indireto=0xFFFF;
	for (int a=0;a<32;a++) tmp.dados_diretos[a]=0;
	
	uint16_t posicao = 2 + numero*sizeof(struct inodo);
	memoria.write(posicao, sizeof(struct inodo), (uint8_t *)&tmp);
	
}
void escreva_bloco_ponteiro(uint16_t numero, uint16_t valor)
{
	uint16_t endereco = numero * 2 + INICIO_PONTEIROS; 
	memoria.write(endereco, 2, (uint8_t *)&valor);
}
void leia_bloco_ponteiro(uint16_t numero, uint16_t *valor)
{
	uint16_t endereco = numero * 2 + INICIO_PONTEIROS; 
	memoria.read(endereco, 2, (uint8_t *)valor);
}
uint16_t aloca (void)
{
	bloco_indice  bloco_idx;
	 
	uint16_t lido;
	uint16_t  novo_bloco = le_cabecalho();
	
	leia_bloco_ponteiro(novo_bloco, &lido);
	
	escreve_cabecalho (lido);
	
	leia_bloco_dados (novo_bloco, (uint8_t *) &bloco_idx);
	for (int x=0;x<16;x++) bloco_idx[x]=0xffff;
	escreva_bloco_dados(novo_bloco, (uint8_t *) &bloco_idx);
	
	
	return novo_bloco;
}
void escreva_bloco_dados(uint16_t numero ,uint8_t *valor)
{
		uint16_t endereco = numero * sizeof(bloco_dados) + INICIO_DADOS; 
		memoria.write(endereco,  sizeof(bloco_dados),  valor);
}

void leia_bloco_dados(uint16_t numero ,uint8_t *valor)
{
		uint16_t endereco = numero * sizeof(bloco_dados) + INICIO_DADOS; 
		memoria.read(endereco, sizeof(bloco_dados),  valor);
}

void cria_blocos_livres()
{
	for (int x=0;x<224;x++)
		escreva_bloco_ponteiro(x, x+1);
	
	escreva_bloco_ponteiro(224, 0xFFFF);
}

void cria_blocos_dados()
{
	uint8_t buffer[32];
	
	for (int x=0;x<32;x++) buffer[x]=0;
	
	for (int x=0;x<225;x++)
		escreva_bloco_dados(x, buffer);
	

}
void formata (void)
{
	memoria.init();
	escreve_cabecalho(0);
	for (int x=0;x<12;x++) cria_entrada(x);
	cria_blocos_livres();
	
	cria_blocos_dados();
	
}


