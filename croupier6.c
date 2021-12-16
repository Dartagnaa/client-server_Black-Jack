/*----------------------------------------------
Serveur de notre application - CROUPIER
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>			// pour sleep et d'autres fonctions 
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include <pthread.h>	/*utiliser thread*/
#include <stdbool.h>
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
	int jcartes[5];
	struct joueur * suiv;
}joueur_;

/*Structure de données pour la connexion avec les threads*/
typedef struct threadParam
{
	joueur_ * joueur;
	int cards[52];
}threadParam_;

/*------------------------------------------------------*/
/*                 FONCTIONS SOCKET                     */
/*------------------------------------------------------*/

void intialisationBuffer(char buffer[]){
	for (int i = 0; i < 256; i++)
	{
		buffer[i]=0;
	}
	return;
}

/* ------- Envoyer un message au client -------- */
void envoi(int sock, char buffer[], char* message) {

    intialisationBuffer(buffer);    /*buffer vide*/

    strncpy(buffer,message,256); /* copier le message dans le buffer */
    
    sleep(3); /* mise en attente du programme pour simuler un delai de transmission */

    write(sock,buffer,strlen(buffer)+1); /* envoi du message au client*/

    return;
}


/*------------------------------------------------------*/
/*             FONCTIONS BLACKJACK                      */
/*------------------------------------------------------*/

/* ------- Mélanger les cartes -------- */
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
/* ------- Convertir les cartes mélangées par la fonction shuff en valeur (exemple : V = 11, D = 12, ...) -------- */
int convert_jkq(int a)
{
	if ((a%100==11) ||(a%100==12) ||(a%100==13)) return (a/100)*100+10;
	else return a;
}

/* Décomposition de la valeur de la carte pour le transformer en char et l'envoyer dans le buffer */
int affectationCartes(int bcards[],char bcardsChar[],int num1, int num2, int num3){
	if(bcards[num1]%100 < 10){
		bcardsChar[num2] = '0';
		sprintf(&bcardsChar[num3],"%d",bcards[num1]%100);
	}else if(bcards[num1]%100 >=10){
		bcardsChar[num2] = '1';
		sprintf(&bcardsChar[num3],"%d",(bcards[num1]%100)%10); // modulo pour récupérer la deuxième partie
	}
	return 0;
}

/* Décomposition de la valeur des points pour le transformer en char et l'envoyer dans le buffer */
int affecterPoints(char points[],int sum){
	if(sum < 10){
		points[0] = '0';
		sprintf(&points[1],"%d",sum%10);
	}else if(sum >=10 && sum <20){
		points[0] = '1';
		sprintf(&points[1],"%d",sum%10);
	}else if(sum >=20 && sum<30){
		points[0] = '2';
		sprintf(&points[1],"%d",sum%10);
	}else if(sum>=30){
		points[0] = '3';
		sprintf(&points[1],"%d",sum%10);
	}
	return 0;
}

/*Calcul des points en fonction de la carte piochée*/
int calculPointsCarte (int pcards[],int i,int k,joueur_ * j, char * buffer, int longueur, int * sum, char points[]){
	if (pcards[i+k]%100 == 1)	//s'il a un as
	{
		envoi(j->socket,buffer,"valeur A");
		intialisationBuffer(buffer);
		sleep(2);

		if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) { //écoute le message du client
			if (memcmp(&buffer[0],"o",1)==0)
			{
				*sum = *sum + 11;
			}
			else if(memcmp(&buffer[0],"n",1)==0)
			{
				*sum = *sum +1;
			}
		}
	}
	else if (convert_jkq(pcards[i+k]) %100 ==10) *sum = *sum + 10; //si c'est une tête
	else *sum = *sum + pcards[i+k]%100;

}

/* ------- Fonction d'une manche de blackjack -------- */
void *play(void *param)
{
	/* ------- INITIALISATION ------- */
	threadParam_ * p = (threadParam_ *) malloc(1 * sizeof(threadParam_)); //structure des paramètres du thread
	p=(threadParam_ *) param; //réxupération des paramètres

	joueur_ * j = (joueur_ *) malloc(1 * sizeof(joueur_)); //structure du joueur
	j = p->joueur; //récupération du joueur

	int cards[52]; //tableau des cartes
	memcpy(cards,p->cards,52); //récupération des cartes du jeu

	int pcards[5]; //cartes du joueur
	for (int a = 0; a < 5; a++)
	{
		pcards[a]=j->jcartes[a]; // récupération des cartes du joueur
	}

	int i=0;
	int id = j->id; //id du joueur
	int psum=0; //somme du joueur

	int bsum=0; //somme du croupier
    char pcardsChar[10]; // somme des cartes du joueur en char; [10] car 2 places pour un nombre
	int bcards[5]={0}; //tableau des cartes du croupier
    char bcardsChar[10]; // somme des cartes du croupier en char; [10] car 2 places pour un nombre
    
	char points[2];	// stocker les points du joueur et du croupier pour l'envoyer au joueur
	
	char buffer[255]; //pour envoyer des messages au client
	int longueur;	//pour récupérer la longueur du message dans le buffer (envoyé par le client)
	bool depart=false;

	bcards[0]=cards[0]; //donne la première carte au croupier
	
	//attendre que le joueur soit prêt
	do{
		if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) { //écoute d'un message
			if (memcmp(&buffer[0],"o",1)==0)
			{
				depart=true;
			}
		}
	}while(depart == false);


    //envoyer la carte du croupier au joueur
	envoi(j->socket,buffer,"croupier");
	affectationCartes(bcards,bcardsChar,0,0,1);
    envoi(j->socket,buffer,bcardsChar);

	//envoi les cartes du joueur au joueur
    envoi(j->socket,buffer,"joueur");
	affectationCartes(pcards,pcardsChar,0,0,1);
	affectationCartes(pcards,pcardsChar,1,2,3);
    envoi(j->socket,buffer,pcardsChar);
    sleep(5); //simuler une distance entre serveur et client

	//boucle pour lire la valeur des cartes du joueur
	for (i=0; i<2; i++)
	{
		calculPointsCarte(pcards,i,0,j,buffer,longueur,&psum,points);
		if (psum > 21)
		{
			//envoi des points pour que le client puisse les afficher
			envoi(j->socket, buffer, "points");
			affecterPoints(points,psum);
			envoi(j->socket,buffer,points);

			printf("Croupier gagne !\n");
			envoi(j->socket,buffer,"loose"); //envoi d'un message de défaite au joueur
			return 0;
		}
		else if (psum == 21)
		{
			//envoi des points pour que le client puisse les afficher
			envoi(j->socket, buffer, "points");
			affecterPoints(points,psum);
			envoi(j->socket,buffer,points);

			printf("Joueur gagne!\n");
			envoi(j->socket,buffer,"win"); //envoi d'un message de victoire au joueur
			return 0;
		}
	}
	printf("Somme du joueur %i : %d\n\n",j->id+1,psum);
	envoi(j->socket, buffer, "points");
	affecterPoints(points,psum);
	envoi(j->socket,buffer,points);
	
	//piocher d'autres cartes ?
	for (i=0; i<3; i++)	//ne peut pa piocher plus de 3 cartes
	{
		envoi(j->socket, buffer, "piocher?");
		if((longueur = read(j->socket, buffer, sizeof(buffer))) > 0) {
			if (memcmp(&buffer[0],"o",1)==0)	//si le joueur veut piocher
			{
				pcards[i+2]=cards[id+i+10];	//pioche une carte
				affectationCartes(pcards,pcardsChar,i+2,0,1);
				envoi(j->socket,buffer,pcardsChar); //envoi carte au joueur
				sleep(5);
			
				calculPointsCarte(pcards,i,2,j,buffer,longueur,&psum,points);
				if (psum > 21)
				{
					//envoi des points pour que le client puisse les afficher
					envoi(j->socket, buffer, "points");
					affecterPoints(points,psum);
					envoi(j->socket,buffer,points);

					printf("Croupier gagne !\n");
					envoi(j->socket,buffer,"loose"); //envoi d'un message de défaite au joueur
					return 0;
				}
				else if (psum == 21)
				{
					//envoi des points pour que le client puisse les afficher
					envoi(j->socket, buffer, "points");
					affecterPoints(points,psum);
					envoi(j->socket,buffer,points);

					printf("Joueur gagne!\n");
					envoi(j->socket,buffer,"win"); //envoi d'un message de victoire au joueur
					return 0;
				}
			
				//afficher les points
				envoi(j->socket, buffer, "points");
				affecterPoints(points,psum);					
				envoi(j->socket,buffer,points);
				printf("Somme du joueur %i : %d\n\n",j->id+1,psum);
			}else{
				break; //arreter la boucle
			}
		}
	}

	
	//rafficher les cartes du croupier
	envoi(j->socket,buffer,"croupier");
	affectationCartes(bcards,bcardsChar,0,0,1);
    envoi(j->socket,buffer,bcardsChar);
	
	//calcul des points du croupier pour sa première carte
	if (bcards[0]%100 == 1){
		if (bsum + 11 <= 21){
			bsum = bsum + 11;
		}
		else{
			bsum = bsum + 1;
		}
	}
	else if (convert_jkq(bcards[0])%100 == 10){
		bsum = bsum + 10;
	}else{
		bsum = bsum + bcards[0]%100;
	}
	printf("Somme du croupier : %d\n\n",bsum);

	//le croupier va piocher d'autres cartes
	for (i=1; i<5 && bsum<17; i++)	//jusqu'à 5 cartes ou si la somme de ses cartes est >= à 17
	{
		bcards[i]=cards[i+10]; //affecter la carte piochée au croupier

		//envoi nouvelle carte du croupier au joueur
		envoi(j->socket,buffer,"croupier");
		affectationCartes(bcards,bcardsChar,i,0,1);
		envoi(j->socket,buffer,bcardsChar);
		sleep(5);

		//calcul des points
		if (bcards[i]%100 == 1)
		{
			if (bsum + 11 <= 21){
				bsum = bsum + 11;
			}
			else{
				bsum = bsum + 1;
			}
		}
		else if (convert_jkq(bcards[i])%100 == 10) {
			bsum = bsum + 10;
		}else {
			bsum = bsum + bcards[i]%100;
		}
		printf("Somme du croupier : %d\n",bsum);
	}

	sleep(4);

	//envoyer les points du croupier au joueur
	envoi(j->socket, buffer, "pointscroupier");	
	affecterPoints(points,bsum);
	envoi(j->socket,buffer,points);

	sleep(5);

	//reafficher les points du joueur
	envoi(j->socket, buffer, "points");
	affecterPoints(points,psum);
	envoi(j->socket,buffer,points);

	//dernière étape : comparaison des points du joueur avec ceux du croupier
	if (bsum>21 || psum>bsum)
	{
		printf("Joueur gagne !\n");
		envoi(j->socket,buffer,"win");
		return 0;
	}
	else if (psum == bsum)
	{
		printf("Oh, le joueur et le croupier ont fait le même score!\n");
		envoi(j->socket,buffer,"egalite");
		return 0;
	}
	else if (psum < bsum)
	{
		printf("Croupier gagne !\n");
		envoi(j->socket,buffer,"loose");
		return 0;
	}
	sleep(6);
	//return 0;
}

/*----------------------------------------*/
/*                 MAIN                  */
/*---------------------------------------*/

int main(int argc, char **argv){
  	/* description de socket */
    char * msg;  //message à renvoyer au client
    int longueur; //longueur du message dans le buffer reçu
	int port = 5000; //port du serveur
	char buffer[256];
    int  longueur_adresse_courante; 	// longueur d'adresse courante d'un client
    sockaddr_in adresse_locale, 		// structure d'adresse locale
				adresse_client_courant; 	// adresse client courant
    hostent* ptr_hote; 			// les infos recuperees sur la machine hote
    servent*		ptr_service; 			//les infos recuperees sur le service de la machine
	int socket_descriptor;
    char machine[TAILLE_MAX_NOM+1]; 	// nom de la machine locale

	/* description des joueurs */
    joueur_ * ListeJ; //liste des joueurs
    joueur_ * aux;
    
	/* déclaration du thread et de ses paramètres */
	pthread_t thread_clients[5]; /*car 5 clients max au BJ*/
	threadParam_ * param; //paramètres du thread

	/* partie blackjack */
	int cards[52];	// Tas de carte du jeu	

    /* déclaration de la liste des joueurs */
    ListeJ = (joueur_ *) malloc(1 * sizeof(joueur_));
	aux = ListeJ;

	int i=0; //compter le nombre de client connectés
	
	/********************************CONNEXION*******************************/

    gethostname(machine,TAILLE_MAX_NOM);	// recuperation du nom de la machine
    // recuperation de la structure d'adresse en utilisant le nom
	 if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
	
	/* initialisation de la structure adresse_locale avec les infos recuperees */			
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype;
    adresse_locale.sin_addr.s_addr	= INADDR_ANY;    
    adresse_locale.sin_port = htons(port); // utiliser un nouveau numero de port

    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

	/*--------------------------creation du socket--------------------------------*/  
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale*/
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }

    listen(socket_descriptor,5); // initialisation de la file d'ecoute



	/********************************DEBUT DE LA PARTIE**************************/


	shuff(cards); // Mélange des cartes avant de lancer la partie

	/* déclaration + ajouter les cartes dans les paramètres du thread */
	param = (threadParam_ *) malloc(1 * sizeof(threadParam_));
	memcpy(param->cards,cards,52); //copier les cartes mélangées dans les paramètres du thread



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
		aux->id = i; //attribution de l'identifiant du joueur
		aux->jcartes[0] = param->cards[i+1]; //attribition de la 1ère carte
		aux->jcartes[1] = param->cards[i+6]; //attribition de la 2ème carte (décallée de 6 pour éviter que les prochains joueurs aient la même carte que les joueurs précédents)

		param->joueur = aux; //mettre le joueur dans les paramètres du thread

		pthread_create(&thread_clients[i],NULL,play,(void *)param); //création du thread avec la fonction play

		aux->suiv = (joueur_ *) malloc(1 * sizeof(joueur_)); //création du joueur suivant en cas de futur connexion
		aux = aux->suiv; //décaler le pointeur de joueur
		i++;
    }
	printf("fin");
	
	/*Fermeture de la connexion*/
	//close(socket_descriptor);
}
