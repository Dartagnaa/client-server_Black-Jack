
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
int convert_vdr(int a)
{
	if ((a%100==11) ||(a%100==12) ||(a%100==13)) return (a/100)*100+10;
	else return a;
}


void affichage_carte(int num)
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
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
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
			printf("*   v *\n");
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
/*------------------------------------------------------*/
void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}

	return;
}

/*------------------------------------------------------*/
void renvoi (int sock,char buffer[]) {

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
	printf("Au bout de 9 manches, la personne qui a remporté le plus de manche a gagné\n");
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
	int manche = 1;
	char ecoute = 'o';
	char revanche = 'o';
	int repInt;
	int test = 1;
	while(manche <= 9 && ecoute == 'o' && revanche == 'o'){
		/* gestion des messages reçu du serveur */
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
			/*renseigner le nombre de joueurs*/
			if (strcmp(buffer,"pret") == 0){
				do{
					printf("Etes-vous prêt à jouer ? (1 pour oui, 0 pour non)\n");
					scanf("%d",&repInt);
					switch (repInt){
						case 0 : 
							strncpy(&rep,"n",1);
							test = 1;
							break;
						case 1 : 
							strncpy(&rep,"o",1);
							test = 1;
							break;
						default : test = 0;
					}
				}while (test == 0);
				
				/*le message va se trouver dans le buffer[0]*/	
				intialisationBuffer(buffer);
				strncpy(buffer,&rep,1);
			//si le croupier envoi des cartes
			if (strcmp(buffer,"carte")==0){
				//écouter à nouveau

					//décomposer le message reçu :


					//afficher
					printf("Carte du croupier\n");
					affichage_carte(buffer[0]);
					printf("\n");
					printf("Cartes du joueur:\n");
					affichage_carte(buffer[1]);
					//printf("\n");
					affichage_carte(buffer[2]);
					//printf("\n");
			}
    		}
			/*envoie de la reponse au serveur*/
			renvoi(socket_descriptor,buffer);
			
			manche = 10;
		}
		
	}

	

	/*Fermeture de la connexion*/
    //close(socket_descriptor);
    
    //printf("connexion avec le serveur fermee, fin du programme.\n");

	
    
    exit(0);
}
