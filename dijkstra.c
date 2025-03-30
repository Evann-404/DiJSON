#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#define MAX_NEIGHBORS 10
#define MAX_NODES 10

//* Structure pour stocker un nœud du graphe
typedef struct {
    int id;                         //* Identifiant du noeud
    int neighbor_count;             //* Nombre de voisins
    int neighbors[MAX_NEIGHBORS];   //* Liste des voisins
    char name[50];                  //* Nom du noeud si il y en a
} Node;

/**
 * Fonction pour lire le contenu d'un fichier json et le renvoyer sous la forme d'une chiane de caractères
 * @param filename Nom du fichier à lire
 * @return Contenu du fichier JSON sous la forme d'une chaine
 */
char *json_reading(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file); //* Taille du fichier
    fseek(file, 0, SEEK_SET);
    
    char *data = (char *)malloc(length + 1);
    if (!data) {
        perror("malloc");
        fclose(file);
        return NULL;
    }
    
    fread(data, 1, length, file);
    data[length] = '\0'; //* Ajout caractère fin de chaine
    fclose(file);
    
    return data;
}

/**
 * Fonction pour analyser les données JSON et remplir le tableau de noeuds
 * @param json_data Données JSON sous forme de chaîne
 * @param nodes Tableau de nœuds à remplir avec les données
 * @return Nombre de nœuds trouvés
 */
int json_parsing(const char *json_data, Node nodes[]) {
    cJSON *root = cJSON_Parse(json_data); //* Parse le JSON
    if (!root) {
        perror("cJSON_Parse");
        return -1;
    }

    cJSON *anchors = cJSON_GetObjectItem(root, "anchors"); //* Récupère le tableau anchors
    if (!cJSON_IsArray(anchors)) {
        perror("cJSON_GetObjectItem");
        return -1;
    }

    int node_count = 0;
    cJSON *anchor;
    cJSON_ArrayForEach(anchor, anchors) { //* Pour chaque noeud dans anchors
        if (node_count >= MAX_NODES)
            break;

        //* Récupération des champs id, name et neighbors
        cJSON *id = cJSON_GetObjectItem(anchor, "id");
        cJSON *name = cJSON_GetObjectItem(anchor, "name");
        cJSON *neighbors = cJSON_GetObjectItem(anchor, "neighbors");

        if (cJSON_IsNumber(id))
            nodes[node_count].id = id->valueint;
        
        if (cJSON_IsString(name)) {
            strncpy(nodes[node_count].name, name->valuestring, sizeof(nodes[node_count].name));
        } else {
            nodes[node_count].name[0] = '\0'; //* Si auncun nom alors ça reste vide
        }

        nodes[node_count].neighbor_count = 0;
        if (cJSON_IsArray(neighbors)) {
            int neighbor_index = 0;
            cJSON *neighbor;
            cJSON_ArrayForEach(neighbor, neighbors) {
                if (cJSON_IsNumber(neighbor) && neighbor_index < MAX_NEIGHBORS) {
                    nodes[node_count].neighbors[neighbor_index++] = neighbor->valueint;
                }
            }
            nodes[node_count].neighbor_count = neighbor_index;
        }

        node_count++;
    }

    cJSON_Delete(root);
    return node_count;
}

/**
 * Fonction qui implémente l'algorithme de dijsktra pour truover le chemin le plus court sur des arcs avec une pondération de 1
 * @param nodes Tableau des noeuds du graphe
 * @param node_count Nombre total de noeuds
 * @param start Position de départ
 * @param dest Position finale souhaitée
 */
void dijkstra (Node nodes[], int node_count, int start, int dest) {
    int distances[MAX_NODES];       //* Tableau des distances minimales
    int visite[MAX_NODES];          //* Tableau pour marquer les noeuds visités
    int predecesseur[MAX_NODES];    //* Tableau pour avoir le chemin de start à dest

    //* Initialise les distances à l'infinii et les prédécesseurs à -1
    for (int i = 0; i < node_count; i++) {
        distances[i] = INT_MAX;
        visite[i] = 0;
        predecesseur[i] = -1;
    }
    distances[start] = 0; //* La distance de start à lui-même est de 0

    //* Boucle de l'algorithme
    for (int i = 0; i < node_count; i++) {
        int min_dist = INT_MAX;
        int x = -1;

        //* Trouver un noeud non visité avec la distance minimale
        for (int j = 0; j < node_count; j++) {
            if(!visite[j] && distances[j] < min_dist) {
                min_dist = distances[j];
                x = j;
            }
        }

        if (x == -1) //* Si pas de neoud alors cest inaccessible
            break;

        visite[x] = 1; //* Marquage du noeud comme visité

        //* Mettre à jour les distances des voisins du noeud x
        for (int i = 0; i < nodes[x].neighbor_count; i++) {
            int neighbour = nodes[x].neighbors[i];
            int new_dist = distances[x] + 1; //* arc avec une pondération de 1
            
            if (new_dist < distances[neighbour - 1]) {
                distances[neighbour - 1] = new_dist;
                predecesseur[neighbour - 1] = x;
            }
        }
    }

    //* Affichage du chemin si il existe
    if (distances[dest] == INT_MAX) {
        printf("Noeud %d est inaccessibile depuis le noeud %d.\n", dest + 1, start + 1);
    } else {
        printf("Chemin: ");
        int curr = dest;
        while (curr != start) {
            printf("%d <- ", curr + 1);
            curr = predecesseur[curr];
        }
        printf("%d\n", start + 1);
    }
}



int main() {
    const char *file = "etage.json";
    Node nodes[10];

    char *json_data = json_reading(file);
    if (!json_data) 
        return 1;

    int node_count = json_parsing(json_data, nodes);
    free(json_data);

    if (node_count < 0) {
        perror("json_parsing");
        return 1;
    }

    dijkstra(nodes, node_count, 9, 5);

    return 0;
}