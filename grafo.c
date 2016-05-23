#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include <string.h>
#include "grafo.h"
#include "lista.h"

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

    lista arestas;
};

typedef struct aresta{
    long int peso;
    vertice *destino;
} *aresta;

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

int cordal(grafo g){
    int eh_cordal = 1;

    grafo novo_grafo = copia_grafo(g); // Copia o grafo

    no n = primeiro_no(novo_grafo->vertices);
    while(tamanho_lista(novo_grafo->vertices) > 0 && n) {
        vertice v = (vertice) conteudo(n);
        if(simplicial(v, novo_grafo)) {
            destroi_referencias(v, novo_grafo);
            remove_no(novo_grafo->vertices, n, *destroi_vertice);
            n = primeiro_no(novo_grafo->vertices);
            eh_cordal = 1;
        } else {
            n = proximo_no(n);
            eh_cordal = 0;
        }
    }
    destroi_grafo(novo_grafo);
    return eh_cordal; // Xablau
}
