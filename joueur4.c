
/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

/*-------------------------------------------------------*/
void pic(int num)
{
	char fl;
	int po_num;
	
	fl = num / 100;
	po_num = num % 100;
	switch (po_num)
	{
		case 1: 
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   A *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 2 ... 10:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*  %2d *\n", po_num);
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 11:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   J *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 12:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   Q *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 13:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   K *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}

	}
}

/*------------------------------------------------------*/
void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}

	return;
}

/*------------------------------------------------------*/
void envoi (int sock,char buffer[], char * message) {

    /* modifie le buffer pour le renvoyer */
    intialisationBuffer(buffer);    /*buffer vide*/

    /* copier le message dans le buffer */
    strncpy(buffer,message,256);

	/* mise en attente du prgramme pour simuler un delai de transmission */
    sleep(3);
    
	/*ssize_t write(int fs, const void *buf, size_t N);*/
    write(sock,buffer,strlen(buffer)+1);
    
    printf("\nmessage envoye.\n");
	return;
}

/*------------------------------------------------------*/
void affichage_regle(){
	printf("\nLe but est de faire 21 points.\n");
	printf("L'As vaut 1 ou 11, et les têtes valent 10\n");
	printf("Si vous faites 21 points sue les 2 première cartes : Black Jack. Vous gagnez la manche\n");
	printf("Sinon, vous pouvez piocher une carte. Mais attention, il ne faut pas dépasser 21\n");
	printf("Si vous dépassez 21, vous perdez la manche, si vous faites moins, c'est au tour du croupier de tirer une carte\n");
	printf("Le croupier va piocher jusqu'à avoir 17 minimum.\n");
	printf("--------------------------------------------------\n");
	printf("La personne qui a remporté la partie a gagné\n");
	return;
}

/*------------------------------------------------------*/

int main(int argc, char **argv) {
  
    int 	socket_descriptor, 	/* descripteur de socket */
		longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	buffer[256];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
	int  port;			/*num port*/
    char *	mesg; 			/* message envoyé */
    char    rep;		/*reponse du joueur*/
     
    if (argc != 3) {
	perror("usage : client <adresse-serveur> <numero port>");
	exit(1);
    }
   	prog = argv[0];
    host = argv[1];
	/*choix du num de port par le joueur*/
	port = atoi(argv[2]);
	adresse_locale.sin_port = htons(port);

    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

    
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("erreur : impossible de creer la socket de connexion avec le serveur.");
	exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
	perror("erreur : impossible de se connecter au serveur.");
	exit(1);
    }
    
/*--------------------------------------------------------------------------------------------*/
    /*    CONNEXION ETABLIE     */

    printf("Bienvenue dans le jeu du BlackJack !\n");

    /*afficher les règles du Black Jack */
	affichage_regle();

/*--------------------------------------------------------------------------------------------*/
	
	/* ecoute en continue du serveur */
	char d;
	for(;;){
		/* gestion des messages reçu du serveur */
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
			//si le croupier envoi des cartes
            //afficher carte du croupier
            if (strcmp(buffer,"croupier")==0){
				printf("\nLa carte du croupier: \n");
                sleep(2);
                if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
                    int carte = 0;
					if (memcmp(&buffer[0],"0",1)==0){
						carte = buffer[1] - '0';
					}else if(memcmp(&buffer[0],"1",1)==0){
						carte = buffer[1] - '0';
						carte = carte + 10;
					}
                    pic(carte);
				}
            }
            
            //afficher ses cartes
			if (strcmp(buffer,"joueur")==0){
				printf("Vos cartes : \n");
                sleep(2);
				intialisationBuffer(buffer);
                if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
                    int carte = 0;
					//carte 1
					if (memcmp(&buffer[0],"0",1)==0){
						carte = buffer[1] - '0';
					}else if(memcmp(&buffer[0],"1",1)==0){
						carte = buffer[1] - '0';
						carte = carte + 10;
					}
                    pic(carte);

					//carte 2
					carte = 0;
					if (memcmp(&buffer[2],"0",1)==0){
						carte = buffer[3] - '0';
					}else if(memcmp(&buffer[2],"1",1)==0){
						carte = buffer[3] - '0';
						carte = carte + 10;
					}
                    pic(carte);
				}
            }
            
			//choix de la valeur de l'As
			if (strcmp(buffer,"valeur A")==0){
				printf("Choisissez la valeur de l'As: 'o' pour 11 ou 'n' pour 1 :\n");
				do{
					d = getchar();
				} while (d!='o' && d!='n');
				envoi(socket_descriptor, buffer, &d);
			}

			if (strcmp(buffer,"points")==0){
				printf("Voici vos points :");
				if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
					printf(" %c\n",buffer[0]);
					//printf("Voici les points du croupier :");
					//printf(" %c\n",buffer[1]);
				}
			}

			if (strcmp(buffer,"loose")==0){
				printf("Vous avez perdu ! \n");
				break;
			}
			if (strcmp(buffer,"win")==0){
				printf("Vous avez gagné ! \n");
				break;
			}

			// choisir une nouvelle carte
			if (strcmp(buffer,"cartes?")==0){
				printf("Voulez-vous piocher ? 'o' pour oui ou 'n' pour non :\n");
				do{
					d = getchar();
				} while (d!='o' && d!='n');
				envoi(socket_descriptor, buffer, &d);
			}
		}
	}

	

	/*Fermeture de la connexion*/
    
    
    //printf("connexion avec le serveur fermee, fin du programme.\n");

	
    close(socket_descriptor);
    //exit(0);
}
