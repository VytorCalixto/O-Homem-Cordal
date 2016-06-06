#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include <string.h>
#include "grafo.h"
#include "lista.h"

typedef struct rotulo{
    unsigned int tamanho;
    int* rtl;
} *rotulo;

typedef struct grafo {
    int direcionado;
    int ponderado;
    char* nome;
    lista vertices;
};

typedef struct vertice{
    char* nome;
    unsigned int grau; // Para grafos não-direcionados
    unsigned int grau_ent; // Entrada dos direcionados
    unsigned int grau_sai; // Saída dos direcionados
    rotulo rtl;
    int visitado;

    lista arestas;
};

typedef struct aresta{
    long int peso;
    vertice destino;
} *aresta;

rotulo constroi_rotulo() {
    rotulo r = malloc(sizeof(struct rotulo));
    if(!r) {puts("NULL");return NULL;}
    r->tamanho = 0;
    r->rtl = malloc(sizeof(int)*256);
    return r;
}

void insere_rotulo(rotulo r, int n) {
    // r->rtl = realloc(r->rtl, sizeof(int)*((r->tamanho)+1));
    r->rtl[r->tamanho] = n;
    ++r->tamanho;
}

void destroi_rotulo(rotulo r) {
    free(r->rtl);
    free(r);
}

// Retorna 1 se a > b, 0 caso contrário
int compara_rotulo(rotulo a, rotulo b) {
    if(b->tamanho == 0) return 1;
    if(a->tamanho == 0) return 0;
    int min = (a->tamanho < b->tamanho) ? a->tamanho : b->tamanho;
    int i = 0;
    while(i < min) {
        if(a->rtl[i] > b->rtl[i]) return 1;
        if(a->rtl[i] < b->rtl[i]) return 0;
        if(a->rtl[i] == b->rtl[i]) ++i;
    }
    //Em caso de empate, retorna o menor rótulo
    return (a->tamanho < b->tamanho) ? a->tamanho : b->tamanho;
}

no maior_rotulo(lista l) {
    no n;
    vertice v;
    for(n = primeiro_no(l); n; n = proximo_no(n)) {
        v = (vertice) conteudo(n);
        if(!v->visitado) {
            break;
        }
    }
    if(!n) {
        return NULL;
    }
    printf("primeiro: %s rótulo:", nome_vertice((vertice) conteudo(n)));
    for(int j=0; j < ((vertice) conteudo(n))->rtl->tamanho; ++j) {
        printf("%d\t", ((vertice) conteudo(n))->rtl->rtl[j]);
    }
    puts("");
    no maior = n;
    while(n) {
        vertice w = (vertice) conteudo(n);
        if(!compara_rotulo(v->rtl, w->rtl) && !w->visitado) {
            maior = n;
        }
        n = proximo_no(n);
    }
    printf("Maior: %s Rótulo:", nome_vertice((vertice) conteudo(maior)));
    for(int j=0; j < ((vertice) conteudo(maior))->rtl->tamanho; ++j) {
        printf("%d\t", ((vertice) conteudo(maior))->rtl->rtl[j]);
    }
    puts("");
    return maior;
}

char *nome_vertice(vertice v){
    return v->nome;
}

char *nome_grafo(grafo g){
    return g->nome;
}

int direcionado(grafo g){
    return g->direcionado;
}

int ponderado(grafo g){
    return g->ponderado;
}

unsigned int n_vertices(grafo g) {
    unsigned int soma = 0;
    for(no n=primeiro_no(g->vertices); n; n=proximo_no(n)) {
        ++soma;
    }

    return soma;
}

unsigned int n_arestas(grafo g) {
    unsigned int soma = 0;
    for(no n=primeiro_no(g->vertices); n; n=proximo_no(n)) {
        vertice v = (vertice) conteudo(n);
        for(no p=primeiro_no(v->arestas); p; p=proximo_no(p)) {
            ++soma;
        }
    }

    return soma;
}

grafo le_grafo(FILE *input){
    grafo gf = malloc(sizeof(struct grafo));
    Agraph_t *g = agread(input, NULL);
    if(!g) {
        return NULL;
    }

    gf->nome = malloc(1 + strlen(agnameof(g)));
    strcpy(gf->nome, agnameof(g));
    gf->direcionado = (agisdirected(g) ? 1 : 0);
    gf->ponderado = 0;

    // Insere todos os vértices
    gf->vertices = constroi_lista();
    for(Agnode_t *n=agfstnode(g); n; n=agnxtnode(g, n)) {
        vertice v = malloc(sizeof(struct vertice));
        v->nome = agnameof(n);
        v->grau = (unsigned int) agdegree(g, n, TRUE, TRUE);
        v->grau_ent = (unsigned int) agdegree(g, n, TRUE, FALSE);
        v->grau_sai = (unsigned int) agdegree(g, n, FALSE, TRUE);
        v->arestas = constroi_lista();
        v->rtl = constroi_rotulo();
        v->visitado = 0;
        insere_lista(v, gf->vertices);
    }

    // Para cada vértice insere suas arestas
    for(no n=primeiro_no(gf->vertices); n; n=proximo_no(n)) {
        vertice v = (vertice) conteudo(n);
        Agnode_t *node = agnode(g, v->nome, FALSE);
        for(Agedge_t *e=agfstout(g, node); e; e=agnxtout(g, e)) {
            if(!gf->ponderado && agget(e, (char *)"peso") != NULL) {
                gf->ponderado = 1;
            }
            for(no p=primeiro_no(gf->vertices); p; p=proximo_no(p)) {
                vertice w = (vertice) conteudo(p);
                if(strcmp(nome_vertice(w), agnameof(e->node)) == 0) {
                    aresta a = malloc(sizeof(struct aresta));
                    // a->peso = atol(agget(e, (char *)"peso"));
                    char *peso = agget(e, (char *)"peso");
                    if(peso != NULL) {
                        a->peso = atol(peso);
                    }
                    a->destino = w;
                    insere_lista(a, v->arestas);
                }
            }
        }
    }

    agclose(g);
    return gf;
}

int destroi_aresta(void *x) {
    aresta a = (aresta) x;
    if(a) {
        free(a);
        return 1;
    }
    return 0;
 }

int destroi_vertice(void *x) {
    vertice v = (vertice) x;
    if(v && destroi_lista(v->arestas, *destroi_aresta)) {
        free(v);
        return 1;
    }
    return 0;
}

void destroi_referencias(vertice v, grafo g) {
    for(no n = primeiro_no(g->vertices); n; n = proximo_no(n)) {
        vertice w = (vertice) conteudo(n);
        for(no p = primeiro_no(w->arestas); p; p = proximo_no(p)) {
            aresta a = (aresta) conteudo(p);
            if(strcmp(nome_vertice(a->destino), nome_vertice(v)) == 0) {
                printf("removendo %s da aresta com o vértice %s\n", v->nome, w->nome);
                remove_no(w->arestas, p, *destroi_aresta);
            }
        }
    }
}

int destroi_grafo(void *g){
    grafo gf = (grafo) g;
    if(destroi_lista(gf->vertices, *destroi_vertice)){
        free(gf->nome);
        free(gf);
        return 1;
    }
    return 0;
}

grafo escreve_grafo(FILE *output, grafo g){
    fprintf(output, "strict %s",direcionado(g) ? "digraph " : "graph ");
    fprintf(output, "\"%s\" {\n\n",nome_grafo(g));
    for(no n=primeiro_no(((grafo) g)->vertices); n; n=proximo_no(n)) {
        fprintf(output, "\"%s\"\n",nome_vertice((vertice)conteudo(n)));
    }
    for(no n=primeiro_no(((grafo) g)->vertices); n; n=proximo_no(n)) {
        vertice v = (vertice) conteudo(n);
        for(no p = primeiro_no(v->arestas); p; p=proximo_no(p)) {
            aresta a = (aresta) conteudo(p);
            fprintf(output, "\"%s\"", nome_vertice(v));
            fprintf(output, "%s", direcionado(g) ? " -> " : " -- ");
            fprintf(output, "\"%s\"", nome_vertice(a->destino));
            if(ponderado(g)) fprintf(output, " [peso = %li]", a->peso);
            fprintf(output, "\n");
        }
    }
    fprintf(output, "\n}");
    return g;
}

grafo copia_grafo(grafo g) {
    grafo gf = malloc(sizeof(struct grafo));
    gf->nome = malloc(1 + strlen(g->nome));
    strcpy(gf->nome, g->nome);
    gf->direcionado = g->direcionado;
    gf->ponderado = g->ponderado;
    gf->vertices = constroi_lista();
    //copia os vértices
    for(no n=primeiro_no(g->vertices); n; n=proximo_no(n)) {
        vertice v = (vertice) conteudo(n);
        vertice w = malloc(sizeof(struct vertice));
        w->nome = malloc(1 + strlen(v->nome));
        strcpy(w->nome, v->nome);
        w->grau = v->grau;
        w->grau_ent = v->grau_ent;
        w->grau_sai = v->grau_sai;
        w->arestas = constroi_lista();
        w->rtl = constroi_rotulo();
        w->rtl->tamanho = v->rtl->tamanho;
        for(int i=0; i < v->rtl->tamanho; ++i) {
            w->rtl->rtl[i] = v->rtl->rtl[i];
        }
        insere_lista(w, gf->vertices);
    }

    for(no n=primeiro_no(gf->vertices); n; n=proximo_no(n)) {
        //vértice atual do grafo de cópia
        vertice v = (vertice) conteudo(n);
        for(no p=primeiro_no(g->vertices); p; p=proximo_no(p)) {
            // vértice do grafo original
            vertice w = (vertice) conteudo(p);
            // se os vértices são os mesmos, começamos a cópia das arestas
            if(strcmp(nome_vertice(w), nome_vertice(v)) == 0) {
                for(no t=primeiro_no(w->arestas); t; t=proximo_no(t)) {
                    aresta a = (aresta) conteudo(t);
                    aresta b = malloc(sizeof(struct aresta));
                    b->peso = a->peso;
                    vertice dest = a->destino;
                    // Precisamos encontrar o destino entre os vértices do grafo
                    //      de cópia
                    for(no r=primeiro_no(gf->vertices); r; r=proximo_no(r)) {
                        vertice y = (vertice) conteudo(r);
                        if(strcmp(nome_vertice(dest), nome_vertice(y)) == 0) {
                            b->destino = y;
                            insere_lista(b, v->arestas);
                        }
                    }
                }
            }
        }
    }

    return gf;
}

lista vizinhanca(vertice v, int direcao, grafo g){
    no atual = primeiro_no(g->vertices);

    vertice w = (vertice) conteudo(atual);

    while(strcmp(nome_vertice(v), nome_vertice(w)) != 0){
        atual = proximo_no(atual);
        w = (vertice) conteudo(atual);
    }

    lista l = constroi_lista();

    switch (direcao) {
        case 0:
        for(no n=primeiro_no(((grafo) g)->vertices); n; n=proximo_no(n)) {
            vertice vert = (vertice) conteudo(n);
            for(no p = primeiro_no(vert->arestas); p; p=proximo_no(p)) {
                aresta a = (aresta) conteudo(p);
                if(strcmp(nome_vertice((vertice)conteudo(n)), nome_vertice(v)) == 0)
                  insere_lista(a->destino,l);
                if(strcmp(nome_vertice(a->destino), nome_vertice(v)) == 0)
                    insere_lista(vert,l);
            }
        }
        break;
        case -1:
        for(no p = primeiro_no(w->arestas); p; p=proximo_no(p)) {
            aresta a = (aresta) conteudo(p);
            insere_lista(a->destino,l);
        }
        break;
        case 1:
        for(no n=primeiro_no(((grafo) g)->vertices); n; n=proximo_no(n)) {
            vertice vert = (vertice) conteudo(n);
            for(no p = primeiro_no(vert->arestas); p; p=proximo_no(p)) {
                aresta a = (aresta) conteudo(p);
                if(strcmp(a->destino, v) == 0)
                    insere_lista(a->destino,l);
            }
        }
        break;
        default:
        return NULL;
        break;
    }

    return l;
}

unsigned int grau(vertice v, int direcao, grafo g){
    no atual = primeiro_no(g->vertices);

    while(((vertice)conteudo(atual) != v) || atual == NULL){
        atual = proximo_no(atual);
    }

    switch (direcao) {
        case 0:
        return (((vertice)conteudo(atual))->grau);
        break;
        case -1:
        return (((vertice)conteudo(atual))->grau_ent);
        break;
        case 1:
        return (((vertice)conteudo(atual))->grau_sai);
        break;
        default:
        return -1;
        break;
    }
}

int compara_vertice(void* a, void* b){
    vertice v = (vertice) a;
    vertice w = (vertice) b;
    if (strcmp(nome_vertice(v), nome_vertice(w)) == 0)
        return 1;
    return 0;
}

int clique(lista l, grafo g) {
    lista vizinhos;
    for(no n=primeiro_no(l); n; n=proximo_no(n)) {
        vertice v = (vertice) conteudo(n);

        vizinhos = vizinhanca(v, (direcionado(g) ? 1 : 0), g);

        if(tamanho_lista(vizinhos) == 0) continue;

        // Percorremos a lista l. Se todo elemento de l estiver na vizinhança de v,
        // então v é vizinho de todos os vértices em l
        for(no p=primeiro_no(l); p; p=proximo_no(p)) {
            vertice w = (vertice) conteudo(p);
            if(w != v) {
                if(!pertence(*compara_vertice, w, vizinhos)) return 0;
            }
        }
    }
    return 1;
}

int simplicial(vertice v, grafo g){
    lista l = vizinhanca(v,(direcionado(g) ? 1 : 0),g);
    return (clique(l,g));
}

//------------------------------------------------------------------------------
// devolve uma lista de vertices com a ordem dos vértices dada por uma
// busca em largura lexicográfica

lista busca_largura_lexicografica(grafo g) {
    lista busca = constroi_lista();
    no n = primeiro_no(g->vertices);
    vertice v = (vertice) conteudo(n);
    insere_rotulo(v->rtl, tamanho_lista(g->vertices));
    int i = tamanho_lista(g->vertices);

    while(i > 0) {
        lista vizinhos = vizinhanca(v, 0, g);
        for(no m = primeiro_no(vizinhos); m; m = proximo_no(m)) {
            vertice w = (vertice) conteudo(m);
            insere_rotulo(w->rtl, i-1);
        }
        v->visitado=1;
        insere_lista(v, busca);
        n = maior_rotulo(g->vertices);
        if(!n) {
            break;
        }
        v = (vertice) conteudo(n);
        --i;
    }

    return busca;
}

//------------------------------------------------------------------------------
// devolve 1, se a lista l representa uma
//            ordem perfeita de eliminação para o grafo g ou
//         0, caso contrário
//
// o tempo de execução é O(|V(G)|+|E(G)|)

int ordem_perfeita_eliminacao(lista l, grafo g) {
    no n = primeiro_no(l);
    vertice v = (vertice) conteudo(n);

    do {
        int i = 0;
        lista vizinhos = vizinhanca(v, 0, g);
        vertice vizinhos_lista[tamanho_lista(vizinhos)];
        no m = proximo_no(n);
        do {
            vertice w = (vertice) conteudo(m);
            if(pertence(*compara_vertice, w, vizinhos)) {
                vizinhos_lista[i] = w;
                ++i;
            }
            m = proximo_no(m);
        } while(m);
        lista vizinhos_0 = vizinhanca(vizinhos_lista[0], 0, g);
        for(int j=1; j<i; ++j) {
            if(!pertence(*compara_vertice, vizinhos_lista[j], vizinhos_0)) return 0;
        }
        no next = proximo_no(n);
        remove_no(l, n, NULL);
        n = next;
    } while(tamanho_lista(l) > 0);

    return 1;
}

//------------------------------------------------------------------------------
// devolve 1, se g é um grafo cordal ou
//         0, caso contrário

int cordal(grafo g){
    // int eh_cordal = 1;
    //
    // grafo novo_grafo = copia_grafo(g); // Copia o grafo
    //
    // no n = primeiro_no(novo_grafo->vertices);
    // while(tamanho_lista(novo_grafo->vertices) > 0 && n) {
    //     vertice v = (vertice) conteudo(n);
    //     if(simplicial(v, novo_grafo)) {
    //         destroi_referencias(v, novo_grafo);
    //         remove_no(novo_grafo->vertices, n, *destroi_vertice);
    //         n = primeiro_no(novo_grafo->vertices);
    //         eh_cordal = 1;
    //     } else {
    //         n = proximo_no(n);
    //         eh_cordal = 0;
    //     }
    // }
    // destroi_grafo(novo_grafo);
    // return eh_cordal; // Xablau

    lista busca = busca_largura_lexicografica(g);
    puts("");
    for(no n = primeiro_no(busca); n; n = proximo_no(n)) {
        printf("%s\t", nome_vertice((vertice) conteudo(n)));
    }
    return 1;
    // return ordem_perfeita_eliminacao(busca, g);
}
