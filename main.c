#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TAM_POPULACAO 5 /* CORRESPONDE A QNT DE SOLUCOES DE UMA POPULACAO */
#define TAM_SOLUCAO 10 /* TAMANHO DA INSTANCIA, CORRESPONDE A UM CROMOSSOMO/INDIVIDUO/SOLUCAO INICIAL TRAÇANDO PARALELO COM ALGORITMOS GENETICOS */
#define SOLUCAO_IDEAL 1000 /* MELHOR CUSTO CONHECIDO PARA A INSTANCIA, CORRESPONDE AO INDIVIDUO MODELO IDEAL */

int **populacaoAtual, **proxPopulacao, classificacao[TAM_POPULACAO]; /* População atual, Temporariamente a proxima populacao */

unsigned long long int distTotal[TAM_POPULACAO], distAux, *fitness;

// Estrutura para guardar as cidades do arquivo
typedef struct{
    float x, y;
    int visitado;
}Cidades;

Cidades cidades[TAM_SOLUCAO]; /* vetor contendo as cidades */

/*=============================================================================================*/

// Funções do código anterior
void limparVisitados();
void mostrarSolucao(int *);
int calculaDistancia(Cidades , Cidades );
void lerArquivo(char *); 
void buscaLocalVizinhoMaisProximo(int *, int);
// Funções novas secundárias
void mostrarPopulacao(void);


//Funções novas  e hierarquia entre elas
void gerarNovaSolucao(int *, int*, int);
void alocarMemoria(void);            /* OK */
void inicializarPopulacao(void);     /* OK */
    void buscaLocalVizinhoMaisProximo(int*, int); /* Gerar 1° solucao variando alatoriamente o nó inicial*/
int rodarAlgoritmoGenetico(void);
    void iniciarAvaliacao(void);         /* OK - Avalia as distancias totais da populacao para calcular fitness, retorna o melhor */
    void iniciarSelecao(void);
    void iniciarCruzamento(void);
    void iniciarMutacao(void);

/*=============================================================================================*/



void main(int argc, char *argv[]){
    clock_t inicio, fim;
    

    lerArquivo(argv[1]);
    alocarMemoria();
    
    printf("\n-> Inicializando populacao ...");
    inicializarPopulacao();
    printf("<- Populacao inicializada!!\n\n");
    

    iniciarAvaliacao(); /* Avalia o fitness */
    iniciarSelecao();   /* Ordena os melhores com base no fitness */
    mostrarPopulacao(); /* Imprimir resultados */



// MODELO BASE: aqui sao geradas novas populacoes e é feito a avaliacao, selecao, cruzamento e mutacao ATÉ encontrar um fitness 0, ou seja, individuo perfeito
    // O QUE ALTERAR: definir 1 ou + criterios de parada mais viaveis para o tsp    
    /* 
        int DoOneRun(void){
            int generations = 1;
            int perfectGeneration = 0;

            InitializeOrganisms();

            while(1){
                perfectGeneration = EvaluateOrganisms();
                if( perfectGeneration==TRUE ) return generations;
                ProduceNextGeneration();
                ++generations;
            }
        }
    */
}




void iniciarSelecao()
{
    int temp;
    int usado[TAM_POPULACAO] = {0}; // Array para marcar índices já usados

    for (int i = 0; i < TAM_POPULACAO; i++)
    {   
        temp = -1;

        // Encontra o índice do menor fitness que ainda não foi usado
        for(int j = 0; j < TAM_POPULACAO; j++){
            if (!usado[j] && (temp == -1 || fitness[j] < fitness[temp])){
                temp = j;
            }
        }

        // Marca o índice como usado e o adiciona à classificação
        usado[temp] = 1;
        classificacao[i] = temp;
    }

    printf("-> Mostrando classificacao\n\t");
    for (int i = 0; i < TAM_POPULACAO; i++)
    {
        printf("%d, ", classificacao[i]);
    }
    printf("\n<- \n\n");
}

void iniciarAvaliacao()
{
    for (int i = 0; i < TAM_POPULACAO; i++)
    {
        fitness[i] = distTotal[i] - SOLUCAO_IDEAL;
    }

}   

// INICIALIZA A POPULACAO
void inicializarPopulacao()
{
    srand(1); // seed a mesma para testes

    printf("\n\t-> Iniciando vizinho mais proximo para cada individuo ...");
    for(int i = 0; i < TAM_POPULACAO; i++){   // Inicializa a população com construtiva vizinho mais proximo, alterando o nó inicial para gerar variação
        buscaLocalVizinhoMaisProximo(populacaoAtual[i], i);
    }
    printf("\n\t<- Vizinho mais proximo concluido!!\n");
}

// aloca memoria para as populacoes de solucoes 
void alocarMemoria(void)
{
    int solucao;

    populacaoAtual = (int**) malloc(sizeof(int*) * TAM_POPULACAO);
    proxPopulacao =  (int**) malloc(sizeof(int*) * TAM_POPULACAO);
    fitness = (unsigned long long int*) malloc(sizeof(unsigned long long int) * TAM_POPULACAO);
    
    for (solucao = 0; solucao < TAM_POPULACAO; solucao++)
    {
        populacaoAtual[solucao] = (int*) malloc(sizeof(int) * TAM_SOLUCAO);
        proxPopulacao[solucao] = (int*) malloc(sizeof(int) * TAM_SOLUCAO);
    }
}

// Impressão de resultados
void mostrarPopulacao()
{
    printf("-> Mostrando populacao\n");

    for (int i = 0; i < TAM_POPULACAO; i++)
    {   
        printf("\tDistTotal: %lli | Fitness: %lli | Sol[%d]: ", distTotal[i], fitness[i], i);
        mostrarSolucao(populacaoAtual[i]);
    }
    printf("<- \n\n");
}

/*=============================================================================================*/ 
// FUNCOES DO CODIGO ANTERIOR 

// CONSTRUTIVA VIZINHO MAIS PROXIMO
void buscaLocalVizinhoMaisProximo(int *solucao, int idSolucao)
{
    distTotal[idSolucao] = 0, distAux = 0;
    int i = 0, distancia, noInicial = rand() % TAM_SOLUCAO;

    /* Primeira cidade escolhida para iniciar o algoritmo, ESCOLHA ALTERANDO 'noAtual' */
    int noAtual = noInicial, vizMaisProx;
    cidades[noAtual].visitado = 1;
    solucao[0] = noAtual+1; /* OBS: cidades vão de 1 até TAMSOLUCAO */

    while (i < TAM_SOLUCAO - 1)
    {
        distAux = __INT_MAX__;
        // printf("|| Viz + prox do noh %d: \n", noAtual+1);

        for (int j = 0; j < TAM_SOLUCAO; j++)
        {
            // PRINTFS AUXILIARES
            /* 
            printf("dist atual: %d | ", distAux);
            printf("dist para no: %d -> %d | ", j+1, distancia);
            printf("vis?: %d \n", cidades[j].visitado);
            */

            distancia = calculaDistancia(cidades[noAtual], cidades[j]);

            if(distAux > distancia && cidades[j].visitado == 0)
            {
                distAux = distancia;
                vizMaisProx = j; 
            }
        }
        cidades[vizMaisProx].visitado = 1;
        distTotal[idSolucao] += distAux;
                
        // printf("##viz escolhido: %d & distancia total: %d \n\n", vizMaisProx+1, distTotal);

        solucao[++i] = vizMaisProx+1;
        noAtual = vizMaisProx; 
    }
    distTotal[idSolucao] += calculaDistancia(cidades[noAtual], cidades[noInicial]); 
    // solucao[++i] = noInicial+1; /* REMOVIDO */ 
    
    // printf("\nDistancia total para individuo/solucao %d: %lld\n\n", idSolucao, distTotal[idSolucao]);


    limparVisitados();
}


// CALCULA DISTANCIA ENTRE CIDADE a(x,y) E b(x',y')
int calculaDistancia(Cidades a, Cidades b) {
    int xa = (int)a.x, ya = (int)a.y;
    int xb = (int)b.x, yb = (int)b.y;
    return sqrt((xb - xa)*(xb - xa) + (yb - ya)*(yb - ya));
}

// MOSTRA A SOLUCAO
void mostrarSolucao(int *solucao)
{
    int i = 0;

    printf("(");
    while(i < TAM_SOLUCAO-1)
        printf("%d, ", solucao[i++]);
    printf("%d)\n", solucao[i]);
}

// RESETA OS VISITADOS
void limparVisitados()
{
    for (int i = 0; i < TAM_SOLUCAO; i++)
    {
        cidades[i].visitado = 0;
    }
}

void lerArquivo(char *nome)
{
    FILE *arq = fopen(nome,"r");
    if(arq == NULL)
    {
        fprintf(stderr, "\nArquivo não foi aberto! Abortando programa! ");
        EXIT_FAILURE;
    }
    else
    {
        printf("\n<- Arquivo aberto com sucesso!\n ");

        int i = 0, idAux;
        float x,y;
        char linhaAux[75];

        // pula as 6 primeiras linhas dos arquivos.tsp
        while (i < 6)
        {
            fgets(linhaAux, sizeof(linhaAux), arq);     
            i++;
        }
        
        // le as cidades (x, y, visitado=0)
        i = 0;
        while(i < TAM_SOLUCAO)
        {
            fscanf(arq, "%d %f %f", &idAux, &cidades[i].x, &cidades[i].y);
            cidades[i].visitado = 0;
            i++;
        }

        /* calcularDistancias(cidades);  PARA SALVAR AS DISTANCIAS NUM VETOR*/
    }

    fclose(arq);
}




/*
// GERA NOVA SOLUCAO A PARTIR DE OUTRA
void gerarNovaSolucao(int *solucaoOrigem, int *solucaoDestino, int id){
    int i = 0;

    for (i = 0; i < TAM_SOLUCAO; i++){
        solucaoDestino[i] = solucaoOrigem[i];
    }

    embaralhaSolucao(solucaoDestino);

    distTotal[id] = 0;
    for(i = 1; i < TAM_SOLUCAO; i++){
        distTotal[id] += calculaDistancia(cidades[solucaoDestino[i-1] - 1], cidades[solucaoDestino[i] - 1]);
    }
    distTotal[id] += calculaDistancia(cidades[solucaoDestino[i-1] - 1], cidades[solucaoDestino[0] - 1]);
}
*/

/*
void embaralhaSolucao(int *solucao){
    int j, temp;

    for (int i = TAM_SOLUCAO-1; i > 0; i--) /* Algoritmo de Fisher-Yates para embaralhar a solucao e a diferenciar da original 
    {
        j = rand() % (i+1); /* gera um indice aleatorio entre 0 e i para ser trocado a partir do ultimo indice 
        temp = solucao[i];
        solucao[i] = solucao[j];
        solucao[j] = temp;
    }
}
*/