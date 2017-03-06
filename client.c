#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
	

int sid;
struct sockaddr_in sa;
char nomeavversario[200];

void help(){
	printf("sono disponibili i seguenti comandi: \n !help-->mostra l'elenco dei comandi disponibili \n !who-->mostra l'elenco dei client connessi al server \n !create-->crea una nuova partita e attendi un avversario \n !join-->unisciti ad una partita e inizia a giocare \n !disconnect-->disconnetti il client dal server \n !quit-->disconnetti il client dal server \n !show_map-->mostra la mappa di gioco \n !hit num_cell -->marca la casella num_cell\n");
}

void show_map(int mappa[9]){
	char simboli[] = { ' ', 'X', 'O' }; //array di simboli 0 è spazio, 1 è X, 2 è O
	printf(" %c | %c | %c  \n", simboli[mappa[6]], simboli[mappa[7]], simboli[mappa[8]]);
	printf("-----------\n");
	printf(" %c | %c | %c  \n", simboli[mappa[3]], simboli[mappa[4]], simboli[mappa[5]]);
	printf("-----------\n");
	printf(" %c | %c | %c  \n", simboli[mappa[0]], simboli[mappa[1]], simboli[mappa[2]]);
}

int prepartita(){
	char cmd[20];
	char info[200];
	int statopartita=0; //0 prepartita 1 partita iniziata 2 vinta 3 persa 
	int msg,casella;
//do while per il prepartita
	do{	printf(">");
		scanf("%s",cmd); //comando inserito da tastiera
		//siamo nel prepartita stampo >
		if(!strcmp(cmd,"!help")){
			help();
			
		}
		else 
		if(!strcmp(cmd,"!who")){
			msg=1;
			send(sid,&msg,sizeof(int),0);//invia il comando 1 al server
			recv(sid,&msg,sizeof(int),0);//riceve la dimensione della stringa dal server
			recv(sid,info,msg+1,0);//riceve la stringa dal server
			printf("client connessi al server:%s\n",info);		
			
		}
		else
		if(!strcmp(cmd,"!create")){
			msg=2;
			send(sid,&msg,sizeof(int),0);//invia il comando 2 al server
			printf("nuova partita creata.\n in attesa di un avversario...\n");
			recv(sid,&msg,sizeof(int),0);//riceve la dimensione della stringa dal server
			recv(sid,info,msg+1,0);//riceve il nome dell'avversario
			strcpy(nomeavversario,info);//lo svalva in nome avversario
			statopartita=1;
			printf("%s si è unito alla partita.\nla partita è iniziata.\nIl tuo simbolo è: X\nSta a te\n",info);		
		}
		else
		if(!strcmp(cmd,"!join")){
			msg=3;
			send(sid,&msg,sizeof(int),0);//invia il comando 3 al server
			printf("inserire l'username dell'utente da sfidare: ");
			scanf("%s",info);
			msg=strlen(info);//msg è la dimensione della stringa contenente lo sfidante
			send(sid,&msg,sizeof(int),0);//gli invio la dimensione al server
			send(sid,info,msg+1,0);//gli invio la stringa con il nome
			recv(sid,&msg,sizeof(int),0);//ricevo dal server un numero che mi dice se il nome dello sifdante è esistente oppure non 							hafatto il create o è già occupato in una partita
			if(msg==0)
			printf("impossibile connettersi a %s:utente inesistente\n",info);
			else
			if(msg==1)
			printf("impossibile connettersi a %s:lo sfidante è impegnato in un'altra partita\n",info);
			else 
			if(msg==2)
			printf("Non puoi sfidare te stesso\n");
			else 
			if(msg==3)
			printf("l'avversario non è in ascolto\n");	
			else {
				strcpy(nomeavversario,info);//lo svalva in nome avversario
				printf("la partita è iniziata. \nIl tuo simbolo è: O\n\nIn attesa che %s faccia la sua mossa...\n",info);
				statopartita=1;
				recv(sid,&casella,sizeof(int),0);//in casella c'è la prima casella marcata dall'avversario
				if(casella==5){
					statopartita=0;
					printf("il tuo avversario si è disconnesso: HAI VINTO\n");
					
					}
				else{
					casella-=6;// sottraggo 7(codifica di hit) al messaggio ricevuto
					printf("%s ha marcato la casella %d\n \nSta a te\n",info,casella);
				}
			}
		}	
		else
		if(!strcmp(cmd,"!quit")){
			msg=5;
			send(sid,&msg,sizeof(int),0);
			printf("Client disconnesso correttamente\n");	
			statopartita=2;

		}
	}while(statopartita==0);
		return statopartita;
}

void partita(){
	char cmd[20];
	int mappa[9];
	int msg;
	char info[100];
	int statopartita=1;
	do{	printf("#");
		scanf("%s",cmd); //comando inserito da tastiera
		//siamo nel partita stampo #
		if(!strcmp(cmd,"!help")){
			help();
			
		}
		else 
		if(!strcmp(cmd,"!show_map")){
			msg=6;
			send(sid,&msg,sizeof(int),0);
			recv(sid,mappa,sizeof(int)*9,0);
			show_map(mappa);
			

		}
		else
		if(!strcmp(cmd,"!disconnect")){
			msg=4;
			send(sid,&msg,sizeof(int),0);
			recv(sid,&msg,sizeof(int),0);
			printf("disconnessione avvenuta con successo: TI SEI ARRESO\n");
			statopartita=0;
			

		}
		else
		if (!strcmp(cmd,"!hit")){
			int mossa;
			scanf("%d",&mossa);
			if(mossa>=1 && mossa<=9){
				mossa+=6;
				send(sid,&mossa,sizeof(int),0);
				printf("sta a %s\n",nomeavversario);
				recv(sid,&msg,sizeof(int),0);
				if(msg==0){
					printf("Casella occupata\n");
					}
				else 
				if(msg==5){
					statopartita=0;
					printf("%s si è disconnesso: HAI VINTO\n",nomeavversario);
				}
				else
				if (msg==1){
					statopartita=0;
					printf("HAI VINTO!\n");
				}
				else
				if (msg==2){
					statopartita=0;
					printf("HAI PERSO\n");
				}
				else
				if (msg==3){
					statopartita=0;
					printf("PAREGGIO\n");
				}
				else{
					printf("%s ha marcato la casella %d \nSta a te\n",nomeavversario,msg-6);
				}
						

			}
			else
				printf("mossa non valida\n");
			
		}
		else
		printf("Comando non riconosciuto\n");

	}while(statopartita==1);
}

int main (int argc, char *argv[]) {
	int msg =1625;
	char nome[100];
	char server[100] = "127.0.0.1";
	int porta=1234;
	
	{
	if (argc != 3){
		perror("Mancano argomenti\n");
		exit (1);
		}
	else
	strcpy(server,argv[1]);
	porta = atoi(argv[2]);

	}
	sid=socket(AF_INET, SOCK_STREAM, 0) ;
		
	sa.sin_family=AF_INET;
	sa.sin_port = porta;
	sa.sin_addr.s_addr = inet_addr(server);
	
	connect(sid, (struct sockaddr*)&sa,sizeof(sa));
	printf("Connessione al server %s (porta %d) effettuata con successo\n",server,porta);
	help();

	//do while per il nome
	do{
	printf("inserisci nome utente\n>");
	scanf("%s",nome);
	msg=strlen(nome);
	send(sid,&msg ,sizeof(int),0); //manda lunghezza nome
	
	send(sid,nome ,strlen(nome)+1,0); //manda nome
	recv (sid,&msg,sizeof(int),0);
	if(msg==0)
		printf("nome utente già in uso\n> ");
	}while (msg==0);
	
	while(prepartita()!=2){
		partita();
	}
	sleep(1);
	close(sid);
}
