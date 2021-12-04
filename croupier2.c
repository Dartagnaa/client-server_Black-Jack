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

/*-----------------------------------------*/
void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}

	return;
}


/* Envoyer un message au client */
void envoi(int sock, char buffer[], char* message) {

    /* modifie le buffer pour le renvoyer */
    intialisationBuffer(buffer);    /*buffer vide*/

    /* copier le message dans le buffer */
    strncpy(buffer,message,256);
    
    /* mise en attente du programme pour simuler un delai de transmission */
    sleep(3);
    
	/*ssize_t write(int fs, const void *buf, size_t N);*/
    /* envoi du message au client*/
    write(sock,buffer,strlen(buffer)+1);

    return;
}


/* création socket + connexion au client */
void connexion(joueur_ * j, int socket_descriptor, char buffer[], char * reponse, int numPort){
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

    /* utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(numPort);

    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port));

/*--------------------------creation de la socket--------------------------------*/  
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
/*-----------------------------------------------------------------------------------*/
    //affiliation du socket au joueur entré
    j->socket = nouv_socket_descriptor;
    //initialise le nombre de point du joueur
    j->point = 0;
    return;
  }
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
/*-----------------------------------------*/

/* jouer une manche */
int jouer(joueur_ joueurs, int * pt_croupier)
{
  int i=0;
	int bsum=0;
	int p1cards[5]={0};
	int p2cards[5]={0};
	int bcards[5]={0};
	int cards[52];
  char go_on;
 	char buffer[256];
  joueur_ j1 = joueurs;
  joueur_ j2 = joueurs->suivant;

//shuff the cards
	choix_carte(cards);

	//give the cards
	p1cards[0]=cards[0];
	p1cards[1]=cards[1];
  p2cards[0]=cards[2];
  p2cards[0]=cards[3];
	bcards[0]=cards[4];
	bcards[1]=cards[5];
	
	for (i=0; i<2; i++)
	{
		if (p1cards[i]%100 == 1)
		{
      envoi(j1->sock, buffer, "valeur A");
			if (strcmp(buffer,"y")==0)
			{
				j1->point = j1->point + 11;
			}
			else if(strcmp(buffer,"n")==0)
			{
				j1->point = j1->point + 1;
			}
		}
  }
}
/*		else if (convert_jkq(pcards[i]) %100 ==10) psum = psum + 10;
		else psum = psum + pcards[i]%100;
		
		if (psum > 21)
		{
			printf("Sum of player's cards now:%d\n\n",psum);
			printf("Computer win!\n");
			return 1;
		}
		else if (psum == 21)
		{
			printf("Sum of player's cards now:%d\n\n",psum);
			printf("Player win!\n");
			return 0;
		}
	}
	printf("Sum of player's cards now:%d\n\n",psum);
  
}
*/

/* recuperer la reponse du joueur -- fonctionne */
char reponse_joueur(int sock, char buffer[]) {
    int longueur;
    char reponse;
    intialisationBuffer(buffer);
    /* on lit ce qu'on a reçu du client et on le stock dans réponse que l'on va renvoyer*/
    if ((longueur = read(sock, buffer, sizeof(buffer))) > 0){
      strncpy(&reponse,buffer,1);
    }

    return reponse;
}

/*----------------------------------------*/

/*                 MAIN                  */

/*---------------------------------------*/

main(int argc, char **argv){
  int 		socket_descriptor; 		/* descripteur de socket */
  char        buffer[256];
  char*       msg;                   /* message à renvoyer au client */
  int longueur;
  char reponse;            /*reponse du char*/
  int manche = 1;
  /* initialiser point du croupier à 0 */
  int point_croupier = 0;
  
  joueur_ * ListeJ;
  joueur_ * j1;
  joueur_ * j2;

  
  /* Conexion des 2 joueurs */
  ListeJ = (joueur_ *) malloc(1 * sizeof(joueur_));
  ListeJ->suiv = (joueur_ *) malloc(1 * sizeof(joueur_));
  j1 = ListeJ;
  j2 = ListeJ->suiv;
  int port1 = 5000;
  int port2 = 6000;
  

  //Etablissement de la connexion avec le joueur 1
  connexion(j1, socket_descriptor, buffer, &reponse,port1);

  while(manche <= 9){
    jouer(ListeJ, &point_croupier);
    manche = manche - 1;
  }
  
  printf("Sortie de connexion, recup reponse :%c\n",reponse);

  /* -------------- DEBUT DE MANCHE ------------------ */

  /* boucle pour faire autant de malloc que de joueur */

  /* revenir au début du tab et commencer à jouer: */
    /* appeler la fonction jouer */
    

		
  /* traitement du message */
  //printf("reception d'un message.\n");
  
  //envoi(nouv_socket_descriptor,buffer,msg);
          
  
	/*Fermeture de la connexion*/
  //close(j1->socket);
}
