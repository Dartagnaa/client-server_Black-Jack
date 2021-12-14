/*----------------------------------------------
Serveur de notre application
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>			// For sleep and other functions 
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
	int id;
	int socket;
	int point;
	int jcartes[5];
	struct joueur * suiv;

}joueur_;

/*Structure de données pour la connexion avec les threads*/
typedef struct threadParam
{
	joueur_ * joueur;
	int cards[52];
	//int bcards[5];

}threadParam_;

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


void *play(void *param)
{
	threadParam_ * p = (threadParam_ *) malloc(1 * sizeof(threadParam_));
	p=(threadParam_ *) param;
	joueur_ * j = (joueur_ *) malloc(1 * sizeof(joueur_));
	j = p->joueur;

	int cards[52];
	memcpy(cards,p->cards,52);

	int pcards[5];
	for (int a = 0; a < 5; a++)
	{
		pcards[a]=j->jcartes[a];
	}

	int id = j->id;
	

	int i;
	int psum=0;
	int bsum=0;
    char pcardsChar[10]; // car 2 places pour un nombre
	int bcards[5]={0};
    char bcardsChar[10]; // car 2 places pour un nombre
	//int cards[52];
    char buffer[255];
	char d;
	int longueur;
	char *bsumC;	//bsum en format char *
	char points[2];	// stocker les points du joueur et du croupier pour l'envoyer au joueur

	//give the cards to computer
	bcards[0]=cards[0];
	
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
		//printf("somme : %d\n",psum);
		if (psum > 21)
		{
			//printf("Sum of player's cards now:%d\n\n",psum);
			printf("Computer win!\n");
			envoi(j->socket,buffer,"loose");
			return 0;
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
	
	
	//whether player get another cards
	//i=0;
	for (i=0; i<3; i++)
	{
		envoi(j->socket, buffer, "piocher?");
		if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) {
			if (memcmp(&buffer[0],"o",1)==0)
			{
				pcards[i+2]=cards[id+i+10];
				if(pcards[i+2]%100 < 10){
					pcardsChar[0] = '0';
					sprintf(&pcardsChar[1],"%d",pcards[i+2]%100);
				}else if(pcards[i+2]%100 >=10){
					pcardsChar[0] = '1';
					sprintf(&pcardsChar[1],"%d",(pcards[i+2]%100)%10); // modulo pour récupérer la deuxième partie
				}
				//envoi carte au joueur
				envoi(j->socket,buffer,pcardsChar);
				sleep(5);
			
				if (pcards[i+2]%100 == 1)
				{
					printf("valeur A\n");
					envoi(j->socket,buffer,"valeur A");
					intialisationBuffer(buffer);
					sleep(2);
					if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) {
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
				else if (convert_jkq(pcards[i+2]) %100 ==10) psum = psum + 10;
				else psum = psum + pcards[i+2]%100;

				//envoi des points au joueur
				
			
				if (psum > 21)
				{
					//printf("Sum of player's cards now:%d\n\n",psum);
					printf("Computer win!\n");
					envoi(j->socket,buffer,"loose");
					return 0;
				}
				else if (psum == 21)
				{
					//printf("Sum of player's cards now:%d\n\n",psum);
					printf("Player win!\n");
					envoi(j->socket,buffer,"win");
					return 0;
				}else {
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
					printf("Sum of player's cards now:%d\n\n",psum);
				}
			}else{
				break;
			}
		}
	}

	if (i == 3)
	{
		printf("Player win! Because the sum of your 5 cards is no larger than 21! So lucky!\n");
		return 0;
	}

	
	//rafficher les cartes du croupier
	envoi(j->socket,buffer,"croupier");
	if(bcards[0]%100 < 10){
		bcardsChar[0] = '0';
		sprintf(&bcardsChar[1],"%d",bcards[0]%100);
	}else if(bcards[0]%100 >=10){
		bcardsChar[0] = '1';
		sprintf(&bcardsChar[1],"%d",(bcards[0]%100)%10); // modulo pour récupérer la deuxième partie
	}
    envoi(j->socket,buffer,bcardsChar);
	
	//whether computer get another cards until bsum>16
	//i=0;
	for (i=0; i<4 && bsum<17; i++)
	{
		bcards[i+1]=cards[i+20];
		//envoi nouvelle carte du croupier au joueur
		envoi(j->socket,buffer,"croupier");
		if(bcards[i+1]%100 < 10){
			bcardsChar[0] = '0';
			sprintf(&bcardsChar[1],"%d",bcards[i+1]%100);
		}else if(bcards[i+1]%100 >=10){
			bcardsChar[0] = '1';
			sprintf(&bcardsChar[1],"%d",(bcards[i+1]%100)%10); // modulo pour récupérer la deuxième partie
		}
		envoi(j->socket,buffer,bcardsChar);
		
		if (bcards[i]%100 == 1)
		{
			if (bsum+11 <= 21)
			{
				bsum = bsum+11;
			}
			else
			{
				bsum = bsum+1;
			}
		}
		else
		{
			bsum = bsum + convert_jkq(bcards[i+1])%100;
		}
	}
	//envoyer les points du croupier au joueur
	envoi(j->socket, buffer, "pointscroupier");	
	if(bsum < 10){
		points[0] = '0';
		psumReduit[0] = bsum%10;
		sprintf(&points[1],"%d",psumReduit[0]);

	}else if(bsum >=10 && bsum <20){
		points[0] = '1';
		psumReduit[0] = bsum%10;
		sprintf(&points[1],"%d",psumReduit[0]);

	}else if(bsum >=20){
		points[0] = '2';
		psumReduit[0] = bsum%10;
		sprintf(&points[1],"%d",psumReduit[0]);
	}
	envoi(j->socket,buffer,points);

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
	pthread_t thread_clients[3]; /*car 3 clients max*/
	int cards[52];		// Tas de carte du jeu	

    joueur_ * ListeJ;
    joueur_ * aux;
    joueur_ * prec;

    /* Conexion des 2 joueurs */
    ListeJ = (joueur_ *) malloc(1 * sizeof(joueur_));
    ListeJ->suiv = (joueur_ *) malloc(1 * sizeof(joueur_));
    aux = ListeJ;
    prec = ListeJ;
    int port = 5000;

	/* Mélange des cartes avant de lancer la partie */
	shuff(cards);

	/* init + ajouter les cartes dans les paramètres du thread */
	threadParam_ * param;
	param = (threadParam_ *) malloc(1 * sizeof(threadParam_));
	memcpy(param->cards,cards,52);

	/********************************CONNEXION*******************************/

	char buffer[256];
    int  longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in adresse_locale, 		/* structure d'adresse locale*/
				adresse_client_courant; 	/* adresse client courant */
    hostent* ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
	int socket_descriptor;
	int i=0;
    char machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */

    gethostname(machine,TAILLE_MAX_NOM);	/* recuperation du nom de la machine */
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
    adresse_locale.sin_port = htons(port);

    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port));

/*--------------------------creation de la socket--------------------------------*/  
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale*/
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }

    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);

	/********************************THREAD*******************************/

    /* attente des connexions et traitement des donnees recues */
    for(;;) {
        longueur_adresse_courante = sizeof(adresse_client_courant);
        /* adresse_client_courant sera renseigné par accept via les infos du connect */
        if ((aux->socket = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante))< 0) 
        {
            perror("erreur : impossible d'accepter la connexion avec le client.\n");
            exit(1);
        }

        //initialise le nombre de point du joueur
        aux->point = 0;
		aux->id = i;
		aux->jcartes[0] = param->cards[i+1];
		aux->jcartes[1] = param->cards[i+6];

		param->joueur = aux;

		pthread_create(&thread_clients[i],NULL,play,(void *)param);

		prec = aux;
		aux = prec->suiv;
		i++;
    }

}
