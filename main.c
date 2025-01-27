#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TAM_POPULACAO 5 /* CORRESPONDE A QNT DE SOLUCOES DE UMA POPULACAO */
#define TAM_SOLUCAO 574 /* TAMANHO DA INSTANCIA, CORRESPONDE A UM CROMOSSOMO/INDIVIDUO/SOLUCAO INICIAL TRAÇANDO PARALELO COM ALGORITMOS GENETICOS */
#define SOLUCAO_IDEAL 36905 /* MELHOR CUSTO CONHECIDO PARA A INSTANCIA, CORRESPONDE AO INDIVIDUO MODELO IDEAL */

int **populacaoAtual, classificacao[TAM_POPULACAO]; /* População atual, Temporariamente a proxima populacao */

unsigned long long int distTotal[TAM_POPULACAO], distAux, *fitness, novasDists[2];

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
    void iniciarCruzamentoOX1(int*, int*);    /* OK - Faz cruzamento entre os 2 melhores com método OX1*/
    void iniciarMutacao(int *, int*);   /* OK - Aplica mutação nos filhos gerado com a tecnica pair-swap de um nó fixo com todos os outros possiveis até encontrar melhoria*/
    int atualizarPopulacao(int*, int*); /* OK - Atualiza população caso tenham tido filhos melhores */
/*=============================================================================================*/



void main(int argc, char *argv[]){
    clock_t inicio, fim;
    int i = 1, iteracoes,  /* Criterio de parada */
        populacaoMudou = 1; /* Auxiliar para verificar se algum filho foi inserido na populacao, caso sim, avaliação e seleção deverão ser feitas novamente */
    
    lerArquivo(argv[1]);
    alocarMemoria();
    
    printf("\n-> Inicializando populacao ...");
    inicializarPopulacao();
    printf("<- Populacao inicializada!!\n\n");
    mostrarPopulacao();

    printf("Entre com quantas iteracoes voce quer que o algoritmo genetico rode: ");
    scanf("%d", &iteracoes); i = iteracoes;

    inicio = time(NULL);
    do {
        if (populacaoMudou){
            iniciarAvaliacao(); /* Avaliação: avalia o fitness */
            iniciarSelecao();   /* Seleção: ordena os melhores com base no fitness */
        }
        populacaoMudou = 0;
        mostrarPopulacao(); /* Imprimir resultados */
        
        int filho1[TAM_SOLUCAO] = {0},
            filho2[TAM_SOLUCAO] = {0};
        iniciarCruzamentoOX1(filho1, filho2); /* Reprodução: cross-over OX1*/

        iniciarMutacao(filho1, filho2); /* Mutação: pair-swap*/

        populacaoMudou = atualizarPopulacao(filho1, filho2);
        if (populacaoMudou)
            mostrarPopulacao();

        i--;
    }while (i > 0);
    if (populacaoMudou){
        iniciarAvaliacao(); /* Avaliação: avalia o fitness */
        iniciarSelecao();   /* Seleção: ordena os melhores com base no fitness */
    }
    fim = time(NULL);

    printf("\nALGORITMO GENETICO FINALIZADO:\n");
    mostrarPopulacao();
    printf("MELHOR SOLUCAO GERADA:\n"); 
    printf("\tIndividuo %d | DistTotal: %lli (Fit: %lli)| ", classificacao[0], distTotal[classificacao[0]], fitness[classificacao[0]]);
    printf("Tempo de execucao: %.2f | Iteracoes: %i", (float) fim - inicio, iteracoes);
}

// Manutenção da população
int atualizarPopulacao(int *filho1, int *filho2)
{
    printf("\n-> Atualizando populacao ...\n");

    int pior = classificacao[TAM_POPULACAO - 1],
        segundoPior = classificacao[TAM_POPULACAO - 2],
        terceiroPior = classificacao[TAM_POPULACAO - 3],
        populacaoMudou = 0;

    if (novasDists[0] >= distTotal[pior] ) /* Manutenção da população: para o filho 1 */
        printf("\tFilho 1 nao entra na populacao\n");
    else{
        printf("\tFilho 1 entrou no lugar do %d\n", pior);
        populacaoAtual[pior] = filho1;
        distTotal[pior] = novasDists[0];
        pior = segundoPior; 
        segundoPior = classificacao[TAM_POPULACAO - 3];

        populacaoMudou = 1;
    }   

    if (novasDists[1] >= distTotal[pior] ) /* Manutenção da população: para o filho 2 */
            printf("\tFilho 2 nao entra na populacao\n");
        else{
            printf("\tFilho 2 entrou no lugar do %d\n", pior);
            populacaoAtual[pior] = filho2;
            distTotal[pior] = novasDists[1];

            populacaoMudou = 1;
            }
    printf("<- Populacao atualizada !\n\n");

    return populacaoMudou;
}


void iniciarMutacao(int *filho1, int *filho2)
{
        long int diffMutacao1, diffMutacao2;
        printf("\n-> Iniciando Mutacao para filhos ...");
        diffMutacao1 = iniciarPairswap(filho1);
        diffMutacao2 = iniciarPairswap(filho2);

        novasDists[0] -= diffMutacao1;
        novasDists[1] -= diffMutacao2;

        if (!diffMutacao1){
            printf("\n\tFilho 1 | Nao melhorou: ----- | \n");
        }
        else
            printf("\n\tFilho 1 | NovaDistTotal: %lli (-%li) | \n",  novasDists[0], diffMutacao1); // mostrarSolucao(filho1); 
        
        
        if (!diffMutacao2){
            printf("\tFilho 2 | Nao melhorou: ----- | \n");
        }
        else
            printf("\tFilho 2 | NovaDistTotal: %lli (-%li) | \n ", novasDists[1], diffMutacao2); // mostrarSolucao(filho2); 

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
    
    
    // printf("\n\nMutacao iniciada: pair-swap de %d\n", noFixo);
    
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
void iniciarCruzamentoOX1(int *filho1, int *filho2)
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
        if(i == TAM_SOLUCAO && pontoCorte1 == 0)
            break;
        else if(i == TAM_SOLUCAO)
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
        if(i == TAM_SOLUCAO && pontoCorte1 == 0)
            break;
        else if(i == TAM_SOLUCAO)
            i = 0;
        if(j == TAM_SOLUCAO)
            j = 0;

        if(!verificaInseridoFilho(inseridosFilho2, populacaoAtual[pai1][j], tamSegmento)){
            filho2[i] = populacaoAtual[pai1][j];
            i++;
        }
        j++;
    }

    novasDists[0] = calculaDistTotal(filho1);
    novasDists[1] = calculaDistTotal(filho2);
    printf("\n\tFilho 1 | DistTotal: %lli | \n", novasDists[0]);  // mostrarSolucao(filho1); 
    printf("\tFilho 2 | DistTotal: %lli | \n", novasDists[1]);    // mostrarSolucao(filho2);

    printf("<- Crossover-OX1 finalizado !\n");
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
    int pontoCorte1 = rand() % TAM_SOLUCAO, /* Define os pontos de corte, se forem iguais, calcula de novo o 2°*/
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

    printf("\n-> Mostrando classificacao\n\t");
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
    srand(time(NULL)); // seed a mesma para testes

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
    fitness = (unsigned long long int*) malloc(sizeof(unsigned long long int) * TAM_POPULACAO);
    
    for (solucao = 0; solucao < TAM_POPULACAO; solucao++)
    {
        populacaoAtual[solucao] = (int*) malloc(sizeof(int) * TAM_SOLUCAO);
    }
}

// Impressão de resultados
void mostrarPopulacao()
{
    printf("-> Mostrando populacao\n");

    for (int i = 0; i < TAM_POPULACAO; i++)
    {   
        printf("\tIndividuo %d | DistTotal: %lli | \n", i, distTotal[i]);
        // mostrarSolucao(populacaoAtual[i]);
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