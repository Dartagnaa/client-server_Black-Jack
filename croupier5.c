/*----------------------------------------------
Serveur de notre application
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include <pthread.h>	/*utiliser thread*/
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

/*Structure de données pour la connexion avec les threads*/
typedef struct connect
{
	joueur_ * joueur;
	int numPort;
}connect_;

/*------------------------------------------------------*/
/*                       THREADS                        */
/*------------------------------------------------------*/


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

/* ------- Envoyer un message au client -------- */
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

/* ------  création socket + connexion au client ---------- */
void * connexion(void* paramConnexion){
	/*Récupération des paramètres de la fonction connexion*/
	connect_ param = *(connect_ * ) paramConnexion;
	joueur_ * j = param.joueur;
	int numPort = param.numPort;
	
	char buffer[256];
    int    nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
      longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
	printf("on est la");
    gethostname(machine,TAILLE_MAX_NOM);	/* recuperation du nom de la machine */
    int socket_descriptor = j->socket;
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
        if ((nouv_socket_descriptor = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante))< 0) 
        {
            perror("erreur : impossible d'accepter la connexion avec le client.\n");
            exit(1);
        }
/*-----------------------------------------------------------------------------------*/

        //affiliation du socket au joueur entré
        j->socket = nouv_socket_descriptor;
        //initialise le nombre de point du joueur
        j->point = 0;
    }
}

/*------------------------------------------------------*/

/*             FONCTIONS BlackJack                     */
  
/*------------------------------------------------------*/


int shuff(int cards[])
{
	int t;
	int i;
	int desk[52];	
	for (i=0;i<52;i++)
		desk[i] = (i/13+3)*100 + i%13 + 1;

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

int convert_jkq(int a)
{
	if ((a%100==11) ||(a%100==12) ||(a%100==13)) return (a/100)*100+10;
	else return a;
}


int play(joueur_ * j)
{
	int i;
	int psum=0;
	int bsum=0;
	int pcards[5]={0};
    char pcardsChar[10]; // car 2 places pour un nombre
	int bcards[5]={0};
    char bcardsChar[10]; // car 2 places pour un nombre
	int cards[52];
    char buffer[255];
	char d;
	int longueur;
	char *bsumC;	//bsum en format char *
	char points[2];	// stocker les points du joueur et du croupier pour l'envoyer au joueur
	
	//shuff the cards
	shuff(cards);

	//give the cards
	pcards[0]=cards[0];
	pcards[1]=cards[1];
	bcards[0]=cards[2];
	bcards[1]=cards[3];
	
	//the 2 cards player get
    //envoyer la carte du croupier au joueur
	envoi(j->socket,buffer,"croupier");
	if(bcards[0]%100 < 10){
		bcardsChar[0] = '0';
		sprintf(&bcardsChar[1],"%d",bcards[0]%100);
	}else if(bcards[0]%100 >=10){
		bcardsChar[0] = '1';
		sprintf(&bcardsChar[1],"%d",(bcards[0]%100)%10); // modulo pour récupérer la deuxième partie
	}
    envoi(j->socket,buffer,bcardsChar);

    envoi(j->socket,buffer,"joueur");
	if(pcards[0]%100 < 10){
		pcardsChar[0] = '0';
		sprintf(&pcardsChar[1],"%d",pcards[0]%100);
	}else if(pcards[0]%100 >=10){
		pcardsChar[0] = '1';
		sprintf(&pcardsChar[1],"%d",(pcards[0]%100)%10); // modulo pour récupérer la deuxième partie
	}
	if(pcards[1]%100 < 10){
		pcardsChar[2] = '0';
		sprintf(&pcardsChar[3],"%d",pcards[1]%100);
	}else if(pcards[1]%100 >=10){
		pcardsChar[2] = '1';
		sprintf(&pcardsChar[3],"%d",(pcards[1]%100)%10); // modulo pour récupérer la deuxième partie
	}
    envoi(j->socket,buffer,pcardsChar);
    sleep(5);

	i=0;
	for (i=0; i<2; i++)
	{
		if (pcards[i]%100 == 1)
		{
			printf("valeur A\n");
			envoi(j->socket,buffer,"valeur A");
			intialisationBuffer(buffer);
			sleep(2);
			if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) {
				printf("buffer : %c", buffer[0]);
				if (memcmp(&buffer[0],"o",1)==0)
				{
					//printf("You've chosen value 11 for card A.\n");
					psum = psum + 11;
				}
				else if(memcmp(&buffer[0],"n",1)==0)
				{
					//printf("You've chosen value 1 for card A.\n");
					psum = psum +1;
				}
			}
		}
		else if (convert_jkq(pcards[i]) %100 ==10) {
			psum = psum + 10;
		}else {
			psum = psum + pcards[i]%100;
		}

		if (psum > 21)
		{
			//printf("Sum of player's cards now:%d\n\n",psum);
			printf("Computer win!\n");
			envoi(j->socket,buffer,"loose");
			return 1;
		}
		else if (psum == 21)
		{
			//printf("Sum of player's cards now:%d\n\n",psum);
			printf("Player win!\n");
			envoi(j->socket,buffer,"win");
			return 0;
		}
	}
	printf("Sum of player's cards now:%d\n\n",psum);
	envoi(j->socket, buffer, "points");
	
	int psumReduit[2]={0};
	if(psum < 10){
		points[0] = '0';
		psumReduit[0] = psum%10;
		sprintf(&points[1],"%d",psumReduit[0]);

	}else if(psum >=10 && psum <20){
		points[0] = '1';
		psumReduit[0] = psum%10;
		sprintf(&points[1],"%d",psumReduit[0]);

	}else if(psum >=20){
		points[0] = '2';
		psumReduit[0] = psum%10;
		sprintf(&points[1],"%d",psumReduit[0]);
	}
	envoi(j->socket,buffer,points);
	
/*	
	//whether player get another cards
	i=0;
	for (i=0; i<3; i++)
	{
		char j = 'n';
		envoi(j->socket, buffer, "cartes?");
		if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) {
			if (strcmp(buffer,"o")==0)
			{
				printf("You've got another card now.\n");
				pcards[i+2]=cards[i+4];
				printf("and your card %d is:\n", i+3);
				pic(pcards[i+2]);
				
				if (pcards[i+2]%100 == 1)
				{
					printf("Choose A value of the card %d, input 'y' for 11 or 'n' for 1:\n", i+3);
					do{
						d = getchar();
					} while (d!='y' && d!='n');	
					if (d == 'y')
					{
						printf("You've chosen value 11 for card A.\n");
						psum = psum + 11;
					}
					else if(d == 'n')
					{
						printf("You've chosen value 1 for card A.\n");
						psum = psum +1;
					}
				}
				else if (convert_jkq(pcards[i+2]) %100 ==10) psum = psum + 10;
				else psum = psum + pcards[i+2]%100;
				
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
				else printf("Sum of player's cards now:%d\n\n",psum);
			}
			else 
			{
				printf("Sum of player's cards now:%d\n\n",psum);
				break;
			}
		}
	}
	if (i == 3)
	{
		printf("Player win! Because the sum of your 5 cards is no larger than 21! So lucky!\n");
		return 0;
	}
	
	//the 2 cards of boss/computer
	//i=0;
	printf("Computer's cards:\n");
	pic(bcards[0]);
	pic(bcards[1]);


	
	//whether computer get another cards until bsum>16
	//i=0;
	for (i=0; i<3 && bsum<17; i++)
	{
		bcards[i+2]=cards[i+7];
		printf("Computer's card %d is:\n", i+3);
		pic(bcards[i+2]);
		
		if (bcards[i+2]%100 == 1)
		{
			if (bsum+11 <= 21)
			{
				printf("Computer has chosen A as 11\n");
				bsum = bsum+11;
				printf("Sum of computer's cards now:%d\n\n", bsum);
			}
			else
			{
				printf("Computer has chosen A as 1\n");
				bsum = bsum+1;
				printf("Sum of computer's cards now:%d\n\n", bsum);
			}
		}
		else
		{
			bsum = bsum + convert_jkq(bcards[i+2])%100;
			printf("Sum of computer's cards now:%d\n\n", bsum);
		}
	}
	if (i == 3)
	{
		printf("Computer win! Because the sum of its 5 cards is no larger than 21! So lucky!\n");
		return 1;
	}
	
	//the last step
	if (bsum>21 || psum>bsum)
	{
		printf("Player win!\n");
		return 0;
	}
	else if (psum == bsum)
	{
		printf("Oh, player and computer get the same score!\n");
		return 3;
	}
	else if (psum < bsum)
	{
		printf("Computer win!\n");
		return 1;
	}
		
	return 3;
    */
}




/*----------------------------------------*/
/*                 MAIN                  */
/*---------------------------------------*/

int main(int argc, char **argv){
  	/* descripteur de socket */
    char*       msg;                   /* message à renvoyer au client */
    int longueur;
    char reponse;            /*reponse du char*/
    int manche = 1;
	int ret=0;
    /* initialiser point du croupier à 0 */
    int point_croupier = 0;
	pthread_t thread_clients [2]; /*Création de 2 thread car 2 clients*/

    joueur_ * ListeJ;
    joueur_ * aux;
    joueur_ * prec;

    /* Conexion des 2 joueurs */
    ListeJ = (joueur_ *) malloc(1 * sizeof(joueur_));
    ListeJ->suiv = (joueur_ *) malloc(1 * sizeof(joueur_));
    aux = ListeJ;
    prec = ListeJ;
    int port = 5000;

    //Etablissement de la connexion avec le joueur 1
    //connexion(j1, buffer, port);
	
	connect_ paramConnect;
	paramConnect.numPort = port;
	
	printf ("Creation des threads \n");
    for (int i = 0; i < 2; i++)
	{
		printf("TEST");
		paramConnect.joueur = aux;
		printf("test");
		ret = pthread_create (
			& thread_clients [i], NULL,
			connexion, (void *) connect);
		printf("coucou");
		sleep(2);
		/*if (ret)
		{
		fprintf (stderr, "%s", strerror (ret));
		}*/
		prec = aux;
		aux = prec->suiv;
	}

	for (i = 0; i < 2; i++)
   {
      pthread_join (thread_clients [i], NULL);
   }
    //play(j1);


}
