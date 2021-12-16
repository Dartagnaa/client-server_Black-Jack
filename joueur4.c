
/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <port>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>			//pour sleep et d'autres fonctions 


typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

//affichage des cartes dans le terminal
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
			printf("*   V *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 12:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   D *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}
		case 13:
		{
			printf("*******\n");
			printf("*     *\n");
			printf("* %c   *\n", fl);
			printf("*   R *\n");
			printf("*     *\n");
			printf("*******\n");
			break;
		}

	}
}


void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}

	return;
}

//envoi d'un message au serveur
void envoi (int sock,char buffer[], char * message) {

    intialisationBuffer(buffer);

    strncpy(buffer,message,256); //copier le message dans le buffer

    sleep(3); //simulation délai

    write(sock,buffer,strlen(buffer)+1); //écrire dans le buffer
	return;
}


void affichage_regle(int socket_descriptor, char *buffer){
	char rep = ' '; //réponse du joueur

	printf("\nLe but du jeu est de faire 21 points.\n");
	printf("L'As vaut 1 ou 11, et les têtes valent 10\n");
	printf("Si vous faites 21 points sur les 2 première cartes : Black Jack. Vous gagnez !\n");
	printf("Sinon, vous pouvez piocher. Mais attention, il ne faut pas dépasser 21 points.\n");
	printf("Si vous dépassez 21, vous perdez.\n");
	printf("Si vous faites moins, c'est au tour du croupier de piocher.\n");
	printf("Le croupier pioche jusqu'à avoir 17 minimum.\n");
	printf("Si les deux parties obtiennent moins de 21, celui qui a le plus de point a gagné\n");
	printf("--------------------------------------------------\n");
	printf("A vous de jouer ! Êtes-vous prêt à jouer? (o : oui)\n");
	do{
		rep = getchar();
	}while (rep!='o');
	envoi(socket_descriptor, buffer, &rep);
	printf("--------------------------------------------------\n");
	return;
}

/*----------------------------------------*/
/*                 MAIN                  */
/*---------------------------------------*/

int main(int argc, char **argv) {
  
    int socket_descriptor, 	// descripteur de socket
		longueur; 		   // longueur d'un buffer utilisé
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    servent *	ptr_service; 		/* info sur service */
    char 	buffer[256];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
	int  port;			/*num port serveur*/

	char d; //choix du joueur
	int carte = 0;
    
	//vérifier que la syntaxe est respectée
    if (argc != 3) {
		perror("usage : client <adresse-serveur> <numero port>");
		exit(1);
    }
   	prog = argv[0];
    host = argv[1];

	
	port = atoi(argv[2]); /*choix du num de port par le joueur*/
	adresse_locale.sin_port = htons(port);

    if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse.");
		exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET;

    /* creation du socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }
    
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }
    

   /*    CONNEXION ETABLIE     */

    printf("Bienvenue dans le jeu du BlackJack !\n");

	affichage_regle(socket_descriptor,buffer); /*afficher les règles du Black Jack */

	/* ecoute en continue du serveur */
	for(;;){ /* gestion des messages reçu du serveur */
		
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) { //écoute d'un message
			
            //afficher carte du croupier si message = "croupier"
            if (strcmp(buffer,"croupier")==0){
				printf("\nLa carte du croupier: \n");
                sleep(2);
                if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
					//vérif de la dizaine
					if (memcmp(&buffer[0],"0",1)==0){ 
						carte = buffer[1] - '0';
					}else if(memcmp(&buffer[0],"1",1)==0){
						carte = buffer[1] - '0';
						carte = carte + 10;
					}
                    pic(carte); //afficher la carte dans terminal
				}
            }
            
            //afficher ses cartes si message = "joueur"
			if (strcmp(buffer,"joueur")==0){
				printf("Vos cartes : \n");
				intialisationBuffer(buffer);
				sleep(2);

                if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
					//carte 1
					if (memcmp(&buffer[0],"0",1)==0){
						carte = buffer[1] - '0';
					}else if(memcmp(&buffer[0],"1",1)==0){
						carte = buffer[1] - '0';
						carte = carte + 10;
					}
                    pic(carte);

					//carte 2
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
				printf("Choisissez la valeur de l'As: ('o' : 11 || 'n' : 1) :\n");
				do{
					d = getchar();
				} while (d!='o' && d!='n');
				envoi(socket_descriptor, buffer, &d);
			}

			//afficher les points du joueur
			if (strcmp(buffer,"points")==0){
				printf("Voici vos points :");
				if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
					printf(" %c%c\n",buffer[0],buffer[1]);
				}
			}
			//afficher les points du croupier
			if (strcmp(buffer,"pointscroupier")==0){
				printf("Voici les points du croupier :");
				if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
					printf(" %c%c\n",buffer[0],buffer[1]);
				}
			}
			

			// choisir de piocher une carte
			if (strcmp(buffer,"piocher?")==0){
				printf("Voulez-vous piocher ? ('o' : oui || 'n' : non) :\n");
				do{
					d = getchar();
				} while (d!='o' && d!='n');
				envoi(socket_descriptor, buffer, &d);

				if(d=='o'){
					printf("Votre nouvelle carte: \n");
					intialisationBuffer(buffer);
					sleep(2);

					if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
						//nouvelle carte
						if (memcmp(&buffer[0],"0",1)==0){
							carte = buffer[1] - '0';
						}else if(memcmp(&buffer[0],"1",1)==0){
							carte = buffer[1] - '0';
							carte = carte + 10;
						}
						pic(carte);
					}
				}
				else if(d=='n'){
					printf("\nLe croupier va piocher. Affichage de sa première carte.\n");
				}
			}

			//afficher les résultats
			if (strcmp(buffer,"loose")==0){
				printf("Vous avez perdu ! \n");
				break;
			}
			if (strcmp(buffer,"egalite")==0){
				printf("Vous êtes à égalité avec le croupier !\n");
				break;
			}
			if (strcmp(buffer,"win")==0){
				printf("Vous avez gagné ! \n");
				break;
			}
		}
	}

	

	/*Fermeture de la connexion*/
    close(socket_descriptor);
}
