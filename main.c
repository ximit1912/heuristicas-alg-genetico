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
void limparVisitados(void);
void mostrarSolucao(int *);
int calculaDistancia(Cidades , Cidades );
void lerArquivo(char *); 
void buscaLocalVizinhoMaisProximo(int *, int);
// Funções novas secundárias
void mostrarPopulacao(void);
void cortarSegmento(int*, int*);
int verificaInseridoFilho(int*, int, int);
unsigned long long int calculaDistTotal(int*);
long int iniciarPairswap(int*);

//Funções novas  e hierarquia entre elas
void gerarNovaSolucao(int *, int*, int);
void alocarMemoria(void);            /* OK */
void inicializarPopulacao(void);     /* OK */
    void buscaLocalVizinhoMaisProximo(int*, int); /* Gerar 1° solucao variando alatoriamente o nó inicial*/
int rodarAlgoritmoGenetico(void);
    void iniciarAvaliacao(void);         /* OK - Avalia as distancias totais da populacao para calcular fitness, retorna o melhor */
    void iniciarSelecao(void);           /* OK - Classifica para Seleção em ordem da melhor para a pior solução*/
    unsigned long long int* iniciarCruzamentoOX1(int*, int*);    /* OK - Faz cruzamento entre os 2 melhores com método OX1*/
    void iniciarMutacao(int *, int*, unsigned long long int*);               /*  */

/*=============================================================================================*/



void main(int argc, char *argv[]){
    clock_t inicio, fim;
    int iteracoes = 1; /* Criterio de parada */

    lerArquivo(argv[1]);
    alocarMemoria();
    
    printf("\n-> Inicializando populacao ...");
    inicializarPopulacao();
    printf("<- Populacao inicializada!!\n\n");
    
    do {
        iniciarAvaliacao(); /* Avaliação: avalia o fitness */
        iniciarSelecao();   /* Seleção: ordena os melhores com base no fitness */
        mostrarPopulacao(); /* Imprimir resultados */
        
        int filho1[TAM_SOLUCAO] = {0},
            filho2[TAM_SOLUCAO] = {0};
        unsigned long long int *novasDists =  
        iniciarCruzamentoOX1(filho1, filho2); /* Reprodução: cross-over OX1*/

        iniciarMutacao(filho1, filho2, novasDists);

        iteracoes--;
    }while (iteracoes > 0);

}

void iniciarMutacao(int *filho1, int *filho2, unsigned long long int *novasDists)
{
        long int diffMutacao1, diffMutacao2;
        printf("\n-> Iniciando Mutacao para filhos ...");
        diffMutacao1 = iniciarPairswap(filho1);
        diffMutacao2 = iniciarPairswap(filho2);


        if (!diffMutacao1){
            printf("\n\t Nao melhorou: ---------- | Filho 1: ");
        }
        else
            printf("\n\t NovaDistTotal: %lli (-%li) | Filho 1: ",  novasDists[0] - diffMutacao1, diffMutacao1); mostrarSolucao(filho1); 
        
        
        if (!diffMutacao2){
            printf("\t Nao melhorou: ---------- | Filho 2: ");
        }
        else
            printf("\t NovaDistTotal: %lli (-%li) | Filho 2: ", novasDists[1] - diffMutacao2, diffMutacao2); mostrarSolucao(filho2); 

        printf("<- Mutacao finalizada !\n");   
}

// Faz Mutação com pair-swap, retorna a diferença do custo total
long int iniciarPairswap(int *solucao)
{
    int noFixo = (rand() % (TAM_SOLUCAO-1)) + 1, /* produz valores aleatorios de 1 a (TAMSOL-1), para nao ser a origem */
        melhorou = 0,  
        i = TAM_SOLUCAO-1, 
        temp;
    unsigned long long int distNova, distAntiga, distVizinhosNoFixo;
    long int distDiferenca = 0;
    
    
    // printf("\n\nFirst-Improvement iniciada: pair-swap de %d\n", noFixo);

    // Calcula as distancias para os vizinhos do noFixo (1° elemento do par)
    distVizinhosNoFixo = calculaDistancia(cidades[solucao[noFixo - 1]-1], cidades[solucao[noFixo]-1]);
    distVizinhosNoFixo += calculaDistancia(cidades[solucao[noFixo]-1], cidades[solucao[noFixo + 1]-1]);
    
    while(!melhorou && i > 0)
    {
        
        if(i != noFixo && i != noFixo+1 && i != noFixo-1)
        {   
            // Recalcula as distancias para os vizinhos do noFixo (1° elemento do par)
            distAntiga = distVizinhosNoFixo;
            // Calcula as distancias para os vizinhos do i (2° elemento do par)
            distAntiga += calculaDistancia(cidades[solucao[i - 1]-1], cidades[solucao[i]-1]);
            distAntiga += calculaDistancia(cidades[solucao[i]-1], cidades[solucao[i + 1]-1]);

            // Calcula as novas distancias para os novos vizinhos de i "swappado" (antes vizinhos de noFixo)
            distNova = calculaDistancia(cidades[solucao[noFixo - 1]-1], cidades[solucao[i]-1]);
            distNova += calculaDistancia(cidades[solucao[i]-1], cidades[solucao[noFixo + 1]-1]);
            // Calcula as novas distancias para os novos vizinhos de noFixo "swappado" (antes vizinhos de i)
            distNova += calculaDistancia(cidades[solucao[i - 1]-1], cidades[solucao[noFixo]-1]);
            distNova += calculaDistancia(cidades[solucao[noFixo]-1], cidades[solucao[i + 1]-1]);

            // printf("Trocando %d e %d, distAntiga: %lld, distNova: %lld\n", solucao[noFixo], solucao[i], distAntiga, distNova);
            if(distNova < distAntiga)
            {   
                // Atualiza a distancia/custo total 
                distDiferenca -= distAntiga;
                distDiferenca += distNova;
                melhorou = 1;
                
                // Faz a troca do par no conjunto solucao 
                temp = solucao[noFixo];
                solucao[noFixo] = solucao[i];
                solucao[i] = temp;
            }
        }    

        i--;
    }

    if(!melhorou)
        return 0;
    else
    {
        // printf("#Encontrado!\n\n#Pair-swap de conjSol[%d]=%d e conjSol[%d]=%d encontrou uma solucao melhor!", noFixo, solucao[i], i, solucao[noFixo]);
        // printf("\n#Diferenca: %ld\n", distDiferenca);
        return -distDiferenca;
    }
}

// Faz Cross-over OX1
unsigned long long int* iniciarCruzamentoOX1(int *filho1, int *filho2)
{
    printf("-> Inicializando Crossover OX1 ...\n");
    srand(time(NULL));               /* Seed fixa para testes */
        
    int pai1 = classificacao[0], 
        pai2 = classificacao[1],   /* Escolhe as 2 melhores solucoes como pais para o Cross-over */
        pontoCorte1, pontoCorte2, tamSegmento;

    cortarSegmento(&pontoCorte1, &pontoCorte2); /* Corta 2 pontos para escolher 1 segmento */
    tamSegmento = pontoCorte2 - pontoCorte1 + 1;
    printf("\tPontos de corte: %d - %d (%dun)", pontoCorte1, pontoCorte2, tamSegmento);

    int inseridosFilho1[tamSegmento],
        inseridosFilho2[tamSegmento],
        j = 0;
    for(int i = pontoCorte1; i <= pontoCorte2; i++){    /* Copia os segmentos dos pais para os filhos */
        filho1[i] = populacaoAtual[pai1][i],
        filho2[i] = populacaoAtual[pai2][i];
        
        inseridosFilho1[j] = filho1[i],
        inseridosFilho2[j++] = filho2[i];
    }

    j = pontoCorte2 + 1;
    // Preenche os vértices do filho 1 copiando os vértices do pai 2 que ainda nao foram inseridos do segmento, em ordem
    for(int i = pontoCorte2 + 1; i != pontoCorte1;){
        if(i == TAM_SOLUCAO)
            i = 0;
        if(j == TAM_SOLUCAO)
            j = 0;

        if(!verificaInseridoFilho(inseridosFilho1, populacaoAtual[pai2][j], tamSegmento)){
            filho1[i] = populacaoAtual[pai2][j];
            i++;
        }
        j++;
    }

    j = pontoCorte2 + 1;
    // Preenche os vértices do filho 2 copiando os vértices do pai 1 que ainda nao foram inseridos do segmento, em ordem
    for(int i = pontoCorte2 + 1; i != pontoCorte1;){
        if(i == TAM_SOLUCAO)
            i = 0;
        if(j == TAM_SOLUCAO)
            j = 0;

        if(!verificaInseridoFilho(inseridosFilho2, populacaoAtual[pai1][j], tamSegmento)){
            filho2[i] = populacaoAtual[pai1][j];
            i++;
        }
        j++;
    }

    unsigned long long int* novasDists = (unsigned long long int*)malloc(2 * sizeof(unsigned long long int));
    novasDists[0] = calculaDistTotal(filho1);
    novasDists[1] = calculaDistTotal(filho2);
    printf("\n\tDistTotal: %lli | ", novasDists[0]); printf("Filho 1: "); mostrarSolucao(filho1); 
    printf("\tDistTotal: %lli | ", novasDists[1]); printf("Filho 2: "); mostrarSolucao(filho2);

    printf("<- Crossover-OX1 finalizado !\n");

    return novasDists;
}

// Retorna custo de uma solucao
unsigned long long int calculaDistTotal(int *solucao)
{
    distAux = 0;
    int i;
    for (i = 1; i < TAM_SOLUCAO; i++)
    {   
        distAux += calculaDistancia(cidades[solucao[i-1] - 1], cidades[solucao[i] - 1]);
    }
    
    return distAux += calculaDistancia(cidades[solucao[i-1] - 1], cidades[solucao[0] - 1]);
}

// Verifica se um no já foi inserido no filho
int verificaInseridoFilho(int *inseridosFilho, int no, int tamSegmento)
{
    for(int i = 0; i < tamSegmento; i++){
        if (inseridosFilho[i] == no)
            return 1;
    }

    return 0;
}

// Define 2 pontos de corte para um segmento
void cortarSegmento(int *p1, int *p2)
{
    int pontoCorte1 = (rand() % (TAM_SOLUCAO-1) + 1) , /* Define os pontos de corte, se forem iguais, calcula de novo o 2°*/
        pontoCorte2 = rand() % TAM_SOLUCAO;
    while (pontoCorte1 == pontoCorte2)
        pontoCorte2 = rand() % TAM_SOLUCAO;
    
    if (pontoCorte1 > pontoCorte2)
    {
        int temp = pontoCorte2;
        pontoCorte2 = pontoCorte1;
        pontoCorte1 = temp;   
    }

    *p1 = pontoCorte1,
    *p2 = pontoCorte2;
}


// Faz Seleção
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






// BUSCA LOCAL VIZINHO MAIS PROXIMO
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