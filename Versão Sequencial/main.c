#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define MAX_INTERACAO 100
#define MAX_VERTICES 150
#define MAX_FEROMONIO 100
#define N_FORMIGA 50
#define ALPHA 1.0 // intensidade da trilha de ferom�nio
#define BETA 2.0  // peso da dist�ncia
#define RHO 0.5   // taxa de evapora��o do ferom�nio
#define Q 100     // quantidade de ferom�nio depositada por cada formiga

typedef struct {
  int u;
  int v;
  double distancia;
  double feromonio;
} aresta;

typedef struct {
  int num_vertices;
  int num_arestas;
  aresta arestas[MAX_VERTICES * (MAX_VERTICES - 1) / 2];
} grafo;

double distancia_euclidiana(int x1, int y1, int x2, int y2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void inicializar_feromonio(grafo *g, double valor) {

  for (int i = 0; i < g->num_arestas; i++) {
    g->arestas[i].feromonio = valor;
  }
}

void atualizar_feromonio(grafo *g, int *caminho, double delta) {

  for (int i = 0; i < g->num_vertices - 1; i++) {
    int u = caminho[i];
    int v = caminho[i + 1];
    for (int j = 0; j < g->num_arestas; j++) {
      if ((g->arestas[j].u == u && g->arestas[j].v == v) ||
          (g->arestas[j].u == v && g->arestas[j].v == u)) {
        g->arestas[j].feromonio += delta;
      }
    }
  }
}



int selecionar_proximo_vertice(grafo *g, int atual, int *visitado) {
  double soma_probabilidades = 0.0;
  double distancia = 0.0;
  double probabilidades[MAX_VERTICES] = {0}; // inicializar com zeros
  int i;
  for (i = 0; i < g->num_vertices; i++) {
    if (i == atual || visitado[i]) {
      probabilidades[i] = 0.0;
    } else {
      double feromonio = 0.0;
      for (int j = 0; j < g->num_arestas; j++) {
        if ((g->arestas[j].u == atual && g->arestas[j].v == i) ||
            (g->arestas[j].u == i && g->arestas[j].v == atual)) {
          feromonio = g->arestas[j].feromonio;
          distancia = g->arestas[j].distancia;
          break;
        }
      }

      probabilidades[i] = pow(feromonio, ALPHA) * pow(1.0 / distancia, BETA);
      soma_probabilidades += probabilidades[i];
    }
  }


  double limite = 0.0;
  double limite_atual=0.0;
  for (int k = 0; k < g->num_vertices; k++) {
    limite += probabilidades[k] / soma_probabilidades;
    if( limite> limite_atual){
      return k;
    }
  }

  return -1;
}


// esta fun��o o calculo do custo de uma viagem entre dois pontos
void calcular_distancias(grafo *g, int *x, int *y) {
  for (int i = 0; i < g->num_vertices; i++) {
    for (int j = i + 1; j < g->num_vertices; j++) {
      double dist = distancia_euclidiana(x[i], y[i], x[j], y[j]);
      // printf("Distancia de %d com %d = %.2f \n",i,j,dist);
      g->arestas[g->num_arestas].u = i;
      g->arestas[g->num_arestas].v = j;
      g->arestas[g->num_arestas].distancia = dist;
      g->arestas[g->num_arestas].feromonio = 0.0;
      g->num_arestas++;
    }
  }
}

// Esta fun��o usa o vetor com o melhor caminho para calcular o custo da viagem
double calcular_caminho(grafo *g, int *caminho_melhor) {
  double valor_total = 0.0;
 int k;
  for (int i = 0; i < g->num_vertices-1 ; i++) {

    for ( k = 0; k < g->num_arestas ; k++) {

      if ((caminho_melhor[i] == g->arestas[k].u &&
           caminho_melhor[i + 1] == g->arestas[k].v) ||
          (caminho_melhor[i] == g->arestas[k].v &&
           caminho_melhor[i + 1] == g->arestas[k].u)) {
        valor_total += g->arestas[k].distancia;
      }
    }
  }

  return valor_total;
}

double get_current_time(){
  struct timeval time;
  gettimeofday(&time,NULL);
  return (double)time.tv_sec + (double) time.tv_usec * 1e-6;
}

int main(int argc, char *argv[]) {
  // variaveis para fazer a conta do tempo de processamento
  double start,finish;

  // inicia o n�mero de vertices no grafo
  int num_vertices;
  printf("Digite o n�mero de v�rtices: ");
  scanf("%d", &num_vertices);
  start= get_current_time();
  grafo g;
  g.num_vertices = num_vertices;
  g.num_arestas = 0;

  // adiciona as arestas no grafo
  FILE *arqX,*arqY;
  int resultx,resulty,convertX,convertY,l;
  char X[10],Y[10], *Linhax[300],*Linhay[300];
arqX = fopen("C://Users//pedro//Documents//CaixeiroViajante//grafoX.txt", "rt");  // Cria um arquivo texto para leitura
arqY = fopen("C://Users//pedro//Documents//CaixeiroViajante//grafoY.txt", "rt");  // Cria um arquivo texto para leitura

  if ((arqX == NULL)||(arqY == NULL)) // Se n�o conseguiu criar
  {
   printf("Problemas na CRIACAO do arquivo\n");
  }

  int x[MAX_VERTICES], y[MAX_VERTICES];
  l=0;
  while (!feof(arqX))
  {
	// L� uma linha (inclusive com o '\n')
      resultx = fgets(Linhax, 300, arqX);
    // o 'fgets' l� at� 99 caracteres ou at� o '\n'
      resulty = fgets(Linhay, 300, arqY);
      if (resultx){
         // Se foi poss�vel ler
	      //printf("%s",Linha);
        sscanf(Linhax, "%d", &convertX);
        sscanf(Linhay,"%d",&convertY);
       //printf("%d", convert);
        x[l] = convertX;
        y[l] = convertY;
      l++;
      }
  }



   fclose(arqX);
   fclose(arqY);


  calcular_distancias(&g, x, y);

  inicializar_feromonio(&g, MAX_FEROMONIO);

  srand(time(NULL));


  int caminho_melhor[num_vertices ];
  int caminho[num_vertices ];
  double distancia_melhor = INFINITY;
  double distancia_caminho = 0.0;
  #pragma omp parallel for private(caminho, distancia_caminho) reduction(min:distancia_melhor)
  for (int iteracao = 0; iteracao < MAX_INTERACAO; iteracao++) {

    for (int i = 0; i < N_FORMIGA; i++) {
      // cria um vetor de vertices visitados e inicializa com 0
      int visitado[num_vertices];
      for (int j = 0; j < num_vertices; j++) {
        visitado[j] = 0;
      }

      // cria um vetor caminho e inicializa ele com um valor aleat�rio entre o
      // n�mero de v�rtices. Adiciona esse caminho como visitado.
      caminho[0] = rand()%num_vertices;
      visitado[caminho[0]] = 1;

      // incia intera��o
      for (int j = 1; j < num_vertices; j++) {
        int proximo_vertice =
            selecionar_proximo_vertice(&g, caminho[j-1], visitado);

        if (proximo_vertice != -1) {
          caminho[j] = proximo_vertice;
          visitado[proximo_vertice] = 1;

        } else {
          caminho[j] = caminho[0];

          break;
        }
      }


      // se o caminho encontrado for melhor que o anterior ent�o salva em
      // caminho_melhor
      distancia_caminho= calcular_caminho(&g, caminho);
      if (distancia_caminho < distancia_melhor) {

       //intf("Distancia desse caminho: %f\n",distancia_caminho);
        distancia_melhor = distancia_caminho;
        for (int j = 0; j < num_vertices ; j++) {
          caminho_melhor[j] = caminho[j];
        }

      }

      double delta_feromonio = Q / distancia_caminho;
      atualizar_feromonio(&g, caminho, delta_feromonio);
    }
  }

  printf("Melhor caminho: ");
  for (int i = 0; i < num_vertices; i++) {
    printf("%d ", caminho_melhor[i]);
  }
  printf("\n");
  printf("Distancia do melhor caminho: %f\n",distancia_melhor);
  finish= get_current_time();
  printf("Elapsed time: %f seconds\n",finish-start);

  return 0;
}

