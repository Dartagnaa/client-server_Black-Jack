/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/* Tableau dynamique de socket des joueurs */
typedef struct joueur
{
  int socket;
  int point;
	struct joueur * suiv;

}joueur_;

/*------------------------------------------------------*/

/*             FONCTIONS PARTIE RESEAUX                 */
  
/*------------------------------------------------------*/
void reinitialise_buffer(char buffer[]){
    for (int i = 0; i < 256; i++)
    {
        buffer[i] = ' ';
    }
}

/* Envoyer un message au client */
void envoi(int sock, char buffer[], char* message) {

    /* modifie le buffer pour le renvoyer */
    reinitialise_buffer(buffer);    /*buffer vide*/

    /* copier le message dans le buffer */
    strncpy(buffer,message,256);
    
    /* mise en attente du programme pour simuler un delai de transmission */
    sleep(3);
    
	/*ssize_t write(int fs, const void *buf, size_t N);*/
    /* envoi du message au client*/
    write(sock,buffer,strlen(buffer)+1);

    return;
}

/*----------------------------------------*/

/*      FONCTIONS POUR LE BLACK JACK     */

/*---------------------------------------*/
/* Choisir une carte */
int choix_carte(int cards[])
{
	int t;
	int i;
	int desk[52];	
	for (i=0;i<52;i++){
		desk[i] = (i/13+3)*100 + i%13 + 1;
    }
	srand(time(NULL));
	for (i = 0; i < 52; i++)
	{
		do
		{
			t = rand() % 52;
		} while (desk[t] == 0);
		cards[i] = desk[t];
		desk[t] = 0;
	}
	
	return 0;
}
/*------------------------------------------------------*/
void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}

	return;
}


/* jouer une manche */
int jouer(int sock)
{
  	int i;
	int psum=0;
	int bsum=0;
	int pcards[5]={0};
	int bcards[5]={0};
	int cards[52];
	int nbJoueurs;
	char d;
 	char buffer[256];

  	envoi(sock,buffer," joueur");
	int longueur;
  	while((longueur = read(sock, buffer, sizeof(buffer))) > 0) {
    //sprintf(nbJoueurs,int atoi(buffer));
    }
	printf("\n");
}




/* recuperer la reponse du joueur */
char * reponse_joueur(int sock, char buffer[]) {
    int longueur;
    intialisationBuffer(buffer);
    printf("\nsock : %i buffer : %c\n",sock, buffer[0]);
   /* on lit ce qu'on a reçu du client */
    if ((longueur = read(sock, buffer, sizeof(buffer))) > 0){
      printf("\nsock : %i buffer : %c\n",sock, buffer[0]);
    	return 0;
    }
    
    printf("\ntest1\n");
    char reponse[256];
    printf("\ntest2\n");
    for (int i = 0; i < longueur; i++)
    {
        reponse[i]=buffer[i];
    }
    printf("\nreponse : %d\n",reponse[0]);

    return reponse;
}


/* création socket + connexion serveur */
void connexion(joueur_ * j, int socket_descriptor, char buffer[], char reponse){
    int    nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
      longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */


    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */
    
    /*-----------------------------------------------------------*/

    /* Port : utiliser un service existant, par ex. "irc" */
    /*if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);*/

    /* utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);

    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }

    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);

    /* attente des connexions et traitement des donnees recues */
    for(;;) {
    
      longueur_adresse_courante = sizeof(adresse_client_courant);
      
      /* adresse_client_courant sera renseigné par accept via les infos du connect */
      if ((nouv_socket_descriptor = 
        accept(socket_descriptor, 
              (sockaddr*)(&adresse_client_courant),
              &longueur_adresse_courante))
        < 0) {
        perror("erreur : impossible d'accepter la connexion avec le client.\n");
        exit(1);
		}

    j->socket = nouv_socket_descriptor;
    /*demander le nombre de joueur*/
    envoi(j->socket,buffer,"joueur");
    printf("\n");
    j->point = 0;
    printf("\n");
    char * reponseJ = reponse_joueur(j->socket,buffer);
    //strcmp(&reponse, reponseJ);
    printf("/nfin reponse/n");
    //envoi(j->socket,buffer,reponse);
    return 0;
  }
}

main(int argc, char **argv){
  int 		socket_descriptor; 		/* descripteur de socket */
  char        buffer[256];
  char*       msg;                   /* message à renvoyer au client */
  int longueur;
  char reponse[256];            /*reponse du char*/
  /* initialiser point du croupier à 0 */
  int point_croupier = 0;
  
  joueur_ * ListeJ;
  joueur_ * next;
  
  /* Conexion du premier joueur + création du tableau dynamique */
  ListeJ = (joueur_ *) malloc(1 * sizeof(joueur_));
  next = ListeJ;

  connexion(next, socket_descriptor, buffer, *reponse);
  printf("sortie");
  if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
    	/*renseigner le nombre de joueurs*/
		int nbjoueur = buffer[0]; 
		printf("\nNombre de joueurs renseignes : %i \n", nbjoueur);
		
    }
		printf("\nTEST SORTIE\n");
  

  /* demander et recuperer le nombre de joueur à partir du joueur 1 */

  /* -------------- DEBUT DE MANCHE ------------------ */

  /* boucle pour faire autant de malloc que de joueur */

  /* revenir au début du tab et commencer à jouer: */
    /* appeler la fonction jouer */
    

		
  /* traitement du message */
  //printf("reception d'un message.\n");
  
  //envoi(nouv_socket_descriptor,buffer,msg);
          
  
  //close(next->socket);
}
