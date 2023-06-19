#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>


#define MAX_INTERACAO 100
#define MAX_VERTICES 150
#define MAX_FEROMONIO 100
#define N_FORMIGA 50
#define ALPHA 1.0 // intensidade da trilha de ferom�nio
#define BETA 2.0  // peso da dist�ncia
#define RHO 0.5   // taxa de evapora��o do ferom�nio
#define Q 100     // quantidade de ferom�nio depositada por cada formiga
#define NUM_VERTICE 50
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



// int selecionar_proximo_vertice(grafo *g, int atual, int *visitado) {
//  //printf("Vertice %d - aresta %d -  atual %d - \n",g->num_vertices,g->num_arestas,atual);
//   double soma_probabilidades = 0.0;
//   double distancia = 0.0;
//   double probabilidades[MAX_VERTICES] = {0}; // inicializar com zeros
//   int i;
//   for (i = 0; i < g->num_vertices; i++) {
//     if (i == atual || visitado[i]) {
//       probabilidades[i] = 0.0;
//     } else {
//       double feromonio = 0.0;
//       for (int j = 0; j < g->num_arestas; j++) {
//         if ((g->arestas[j].u == atual && g->arestas[j].v == i) ||
//             (g->arestas[j].u == i && g->arestas[j].v == atual)) {
//           feromonio = g->arestas[j].feromonio;
//           distancia = g->arestas[j].distancia;
//           break;
//         }
//       }

//       probabilidades[i] = pow(feromonio, ALPHA) * pow(1.0 / distancia, BETA);
//       soma_probabilidades += probabilidades[i];
//      // printf("somapro %f\n ",soma_probabilidades);
//     }

//   }
//   //printf("Soma prob %f\n",soma_probabilidades);
//  // for(int j=0 ; j<g->num_vertices; j++){
// //printf("visitado[%d] - %d\n ",j,visitado[j]);
// //}


//   double limite = 0.0;
//   double limite_max=0.5;

//   for (int k = 0; k < g->num_vertices; k++) {
//     limite += probabilidades[k] / soma_probabilidades;
//     if( limite> limite_max){
//           //("Limite %f",limite);
//       return k;
//     }
//   }

//   return -1;
// }

#include <mpi.h>

int selecionar_proximo_vertice(grafo *g, int atual, int *visitado) {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // printf("Vertice %d - aresta %d -  atual %d - \n",g->num_vertices,g->num_arestas,atual);
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
      // printf("somapro %f\n ",soma_probabilidades);
    }
  }

  // printf("Soma prob %f\n",soma_probabilidades);
  // for(int j=0 ; j<g->num_vertices; j++){
  //   printf("visitado[%d] - %d\n ",j,visitado[j]);
  // }

  double limite = 0.0;
  double limite_max = 0.5;
  double local_limite = 0.0;
  int local_resultado = -1;

  for (int k = 0; k < g->num_vertices; k++) {
    local_limite += probabilidades[k] / soma_probabilidades;
    if (local_limite > limite_max) {
      // printf("Limite %f",limite);
      local_resultado = k;
      break;
    }
  }

  int global_resultado = -1;
  MPI_Allreduce(&local_resultado, &global_resultado, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  return global_resultado;
}


// esta fun��o o calculo do custo de uma viagem entre dois pontos
void calcular_distancias(grafo *g, int *x, int *y) {
   // printf("numero vertices função: %d",g->num_vertices);

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
 // printf("NUMERO DE ARESTA %d",g->num_arestas);
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


int main(int argc, char **argv) {


  int rank, size;
  int num_vertices;
  FILE *arqX, *arqY;
  int resultx, resulty, convertX, convertY, l;
  char Linhax[300], Linhay[300];
  double distancia_caminho = 0.0;
  double distancia_melhor_local = INFINITY;
   int x[MAX_VERTICES], y[MAX_VERTICES];
   grafo g;
  int* caminho_melhor_local = (int*)malloc(NUM_VERTICE * sizeof(int));
  int caminho[NUM_VERTICE];
  int visitado[NUM_VERTICE];


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  double start_time = MPI_Wtime();
   // Cada processo calcula o índice inicial e final do seu subvetor
    int inicio = rank * (NUM_VERTICE / 2);
    int fim = inicio + (NUM_VERTICE / 2);
  if(rank ==0){

  g.num_vertices=NUM_VERTICE;
    arqX = fopen("C://Users//pedro//Downloads//grafoX.txt", "rt");
    arqY = fopen("C://Users//pedro//Downloads//grafoY.txt", "rt");

    if ((arqX == NULL) || (arqY == NULL)) {
      printf("Problemas na CRIACAO do arquivo\n");
    }

    l = 0;
    while (!feof(arqX)) {
      resultx = fgets(Linhax, 300, arqX);
      resulty = fgets(Linhay, 300, arqY);
      if (resultx) {
        sscanf(Linhax, "%d", &convertX);
        sscanf(Linhay, "%d", &convertY);
        x[l] = convertX;
        y[l] = convertY;
        l++;
      }
    }
   

    fclose(arqX);
    fclose(arqY);

    calcular_distancias(&g, x, y);

    inicializar_feromonio(&g, MAX_FEROMONIO);
  }

  MPI_Datatype mpi_aresta_type;
  MPI_Type_contiguous(sizeof(aresta), MPI_BYTE, &mpi_aresta_type);
  MPI_Type_commit(&mpi_aresta_type);

  MPI_Datatype mpi_grafo_type;
  MPI_Type_contiguous(sizeof(grafo), MPI_BYTE, &mpi_grafo_type);
  MPI_Type_commit(&mpi_grafo_type);

 

  MPI_Bcast(&g, 1, mpi_grafo_type, 0, MPI_COMM_WORLD);

 srand(time(NULL)+rank);
  for (int iteracao = 0; iteracao < MAX_INTERACAO/size; iteracao++) {
    for (int i = 0; i < N_FORMIGA; i++) {
      for (int j = 0; j < NUM_VERTICE; j++) {
        visitado[j] = 0;
      }
      caminho[0] = rand() % NUM_VERTICE;
      visitado[caminho[0]] = 1;

      for (int j = 1; j < NUM_VERTICE; j++) {
        int proximo_vertice = selecionar_proximo_vertice(&g, caminho[j - 1], &visitado);   
        if (proximo_vertice != -1) {
          caminho[j] = proximo_vertice;
          visitado[proximo_vertice] = 1;
        } else {
          caminho[j] = caminho[0];
          break;
        }
      }
      distancia_caminho = calcular_caminho(&g, caminho);
      if (distancia_caminho < distancia_melhor_local) {
        distancia_melhor_local = distancia_caminho;
        for (int j = 0; j < NUM_VERTICE; j++) {
          caminho_melhor_local[j] = caminho[j];

        }
      }

      double delta_feromonio = Q / distancia_caminho;
      atualizar_feromonio(&g, caminho, delta_feromonio);
    }

  }
    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;

  if (rank == 0) {
    printf("Melhor caminho: ");
    for (int i = 0; i < NUM_VERTICE; i++) {
      printf("%d ", caminho_melhor_local[i]);
    }
    printf("\n");
    printf("Distancia do melhor caminho: %f\n", distancia_melhor_local);
    printf("Tempo de execucao: %lf segundos\n", execution_time);
  }

  free(caminho_melhor_local);
  // Liberar os tipos de dados MPI
  MPI_Type_free(&mpi_aresta_type);
  MPI_Type_free(&mpi_grafo_type);


  MPI_Finalize();


  return 0;
}
