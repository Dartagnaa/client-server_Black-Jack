
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
void convert_vdr(char * a)
{
	if (strcmp(a,"11") || (strcmp(a,"24")) || (strcmp(a,"37")) || (strcmp(a,"50"))){
		strcpy(a,"V");
	}else if (strcmp(a,"12") || (strcmp(a,"25")) || (strcmp(a,"38")) || (strcmp(a,"51"))){
		strcpy(a,"D");
	}else if (strcmp(a,"13") || (strcmp(a,"26")) || (strcmp(a,"37")) || (strcmp(a,"52"))){
		strcpy(a,"R");
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
	int ecoute = 1;
	int repInt;
	char d;
	while(ecoute == 1){
		/* gestion des messages reçu du serveur */
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {

			//si le croupier envoi des cartes
			char lettre;
			strcpy(&lettre, &buffer[0]);
			printf("%c\n", lettre);
			printf("%i\n",strcmp(&lettre,"C"));
			if (strcmp(&lettre,"C")==0){
				printf("Les cartes ont été tirées :\n");
				convert_vdr(&buffer[1]);
				convert_vdr(&buffer[2]);
				convert_vdr(&buffer[3]);

				printf("Vos cartes : \n %c\n %c\n",buffer[1], buffer[2]);
				printf("Carte du croupier : %c", buffer[3]);
			}
			//choix de la valeur de l'As
			if (strcmp(buffer,"valeur A")==0){
				printf("Choisissez la valeur de l'As: 'o' pour 11 ou 'n' pour 1 :\n");
				do{
					d = getchar();
				} while (d!='o' && d!='n');
				envoi(socket_descriptor, buffer, &d);
			}

			
		}
		ecoute = 0;
	}

	

	/*Fermeture de la connexion*/
    
    
    //printf("connexion avec le serveur fermee, fin du programme.\n");

	
    close(socket_descriptor);
    //exit(0);
}
