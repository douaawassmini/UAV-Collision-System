#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ── STRUCTURE DRONE ────────────────────────────────
struct Drone {
    int   id;
    float x;
    float y;
    float z;
};

// ── STRUCTURE RÉSULTAT ─────────────────────────────
struct Paire {
    struct Drone *a;
    struct Drone *b;
    float dist_carre;
};

// ── GÉNÉRATION ALÉATOIRE ───────────────────────────
void generer_essaim(struct Drone *essaim, int n) {
    int i;
    for (i = 0; i < n; i++) {
        (essaim + i)->id = i;
        (essaim + i)->x  = (float)(rand() % 10000) / 10.0f;
        (essaim + i)->y  = (float)(rand() % 10000) / 10.0f;
        (essaim + i)->z  = (float)(rand() % 10000) / 10.0f;
    }
}

// ── DISTANCE AU CARRÉ ──────────────────────────────
float distance_carre(struct Drone *a, struct Drone *b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return dx*dx + dy*dy + dz*dz;
}

// ── COMPARATEUR POUR TRI PAR X ─────────────────────
int comparateur_x(const void *a, const void *b) {
    struct Drone *da = (struct Drone *)a;
    struct Drone *db = (struct Drone *)b;
    if (da->x < db->x) return -1;
    if (da->x > db->x) return  1;
    return 0;
}

// ── BRUTE FORCE (≤ 3 drones) ──────────────────────
struct Paire brute_force(struct Drone *debut, int n) {
    struct Paire meilleure;
    meilleure.a          = debut;
    meilleure.b          = debut + 1;
    meilleure.dist_carre = distance_carre(debut, debut + 1);

    int i, j;
    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            float d = distance_carre(debut + i, debut + j);
            if (d < meilleure.dist_carre) {
                meilleure.dist_carre = d;
                meilleure.a          = debut + i;
                meilleure.b          = debut + j;
            }
        }
    }
    return meilleure;
}

// ── BANDE CENTRALE ─────────────────────────────────
struct Paire bande_centrale(struct Drone *debut, int n,
                             float x_milieu, struct Paire meilleure) {
    struct Drone *bande = malloc(n * sizeof(struct Drone));
    if (bande == NULL) return meilleure;

    float d = meilleure.dist_carre;
    int taille = 0;
    int i;

    for (i = 0; i < n; i++) {
        float dx = (debut + i)->x - x_milieu;
        if (dx * dx < d) {
            *(bande + taille) = *(debut + i);
            taille++;
        }
    }

    int j;
    for (i = 0; i < taille; i++) {
        for (j = i + 1; j < taille && j < i + 8; j++) {
            float dist = distance_carre(bande + i, bande + j);
            if (dist < meilleure.dist_carre) {
                meilleure.dist_carre = dist;
                meilleure.a          = debut + i;
                meilleure.b          = debut + j;
            }
        }
    }

    free(bande);
    return meilleure;
}

// ── DIVISER POUR RÉGNER (COMPLET) ─────────────────
struct Paire closest_pair_rec(struct Drone *debut, int n) {

    if (n <= 3)
        return brute_force(debut, n);

    int milieu = n / 2;
    float x_milieu = (debut + milieu)->x;

    struct Paire gauche = closest_pair_rec(debut, milieu);
    struct Paire droite = closest_pair_rec(debut + milieu, n - milieu);

    struct Paire meilleure;
    if (gauche.dist_carre < droite.dist_carre)
        meilleure = gauche;
    else
        meilleure = droite;

    meilleure = bande_centrale(debut, n, x_milieu, meilleure);

    return meilleure;
}

// ── MAIN ─────────────────────────────────────
int main() {
    int n = 10000;
    struct Drone *essaim = malloc(n * sizeof(struct Drone));

    if (essaim == NULL) {
        printf("Erreur : allocation memoire echouee\n");
        return 1;
    }

    printf("==============================================\n");
    printf("   SYSTEME DE DETECTION DE COLLISION UAV\n");
    printf("==============================================\n");
    printf("Initialisation de l'essaim : %d drones\n", n);

    generer_essaim(essaim, n);
    printf("Generation aleatoire    : OK\n");

    qsort(essaim, n, sizeof(struct Drone), comparateur_x);
    printf("Tri spatial par axe X   : OK\n");

    printf("Algorithme Diviser/Regner en cours...\n\n");
    struct Paire res = closest_pair_rec(essaim, n);

    float dist_reelle = sqrt(res.dist_carre);

    printf("==============================================\n");
    printf("   ALERTE COLLISION DETECTEE !\n");
    printf("==============================================\n");
    printf("Drone A : ID=%-6d | x=%8.2f | y=%8.2f | z=%8.2f\n",
        res.a->id, res.a->x, res.a->y, res.a->z);
    printf("Drone B : ID=%-6d | x=%8.2f | y=%8.2f | z=%8.2f\n",
        res.b->id, res.b->x, res.b->y, res.b->z);
    printf("----------------------------------------------\n");
    printf("Distance reelle      : %.4f metres\n", dist_reelle);
    printf("Distance au carre    : %.4f\n", res.dist_carre);
    printf("----------------------------------------------\n");
    printf("ACTION : Declenchement maneuvre d'evitement !\n");
    printf("==============================================\n");

    free(essaim);
    return 0;
}