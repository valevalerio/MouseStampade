#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#define N 8


#define USGEXT(exp,msg) \
	if ((exp)) { fprintf( (stderr) , (msg) ); exit(1); }

pthread_mutex_t toccMtx = PTHREAD_MUTEX_INITIALIZER; //inizializzo la mutex che sceglie i threads
pthread_cond_t condTocc = PTHREAD_COND_INITIALIZER;  //variabili di condizionamento per i thread occupati

pthread_mutex_t cidMtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t coda_cts = PTHREAD_COND_INITIALIZER;  //coda dei clienti da servire

pthread_mutex_t arrmtx[N];
pthread_cond_t arrcond[N];

pthread_mutex_t usersmtx=PTHREAD_MUTEX_INITIALIZER; //mutex per gli utenti

int toccup=0; //numero di thread occupati



typedef struct users_{
	char name[100];
	int stato;
	int indexpartita;
}users_t;

typedef struct tab{
	int griglia[9];
	int creator;
	int joiner;
	int ums[2];
}tab_t;


tab_t tabelloni[N];
users_t USERS[N];


int check_mossa(int mappa[9],int indice){
	if(mappa[indice]!=0){
		return 0;
	}
	else return 1;

}
int trovasimbolo(int mappa[9]){
	int xcount=0;
	int ocount=0;
	int i;
	for(i=0; i<9;i++){
		if(mappa[i]==1)
			xcount++;
		else 
		if(mappa[i]==2)
			ocount++;
	}
	if(xcount>ocount) return 2;//o
	else return 1;//x
}
int vittoria(int mappa[9]){
	int vit = -1;
	if ((	mappa[0] == mappa[1] && mappa[1] == mappa[2] && mappa[1] != 0) ||
		(mappa[3] == mappa[4] && mappa[4] == mappa[5] && mappa[3] != 0) ||
		(mappa[6] == mappa[7] && mappa[7] == mappa[8] && mappa[8] != 0) ||
		(mappa[0] == mappa[3] && mappa[3] == mappa[6] && mappa[6] != 0) || 
		(mappa[4] == mappa[1] && mappa[1] == mappa[7] && mappa[7] != 0) || 
		(mappa[8] == mappa[5] && mappa[5] == mappa[2] && mappa[2] != 0) || 
		(mappa[0] == mappa[4] && mappa[4] == mappa[8] && mappa[8] != 0) || 
		(mappa[6] == mappa[4] && mappa[4] == mappa[2] && mappa[2] != 0))
		vit = 1;
	return vit;
}
int checkpartita(int mappa[9]){
	int i;
	int casellepiene=0;
	int par = -1;//se = 0 c'è pareggio
	//se = 1 c'è vittoria
	//se = -1 si va avanti
	par=vittoria(mappa);
	if (par==-1)
		for (i = 0; i < 9; i++){
			if (mappa[i] != 0)
				casellepiene++;
		}
	if (casellepiene==9 && par!=1 )
		par = 0;
	return par;
}

void finepartita(int indexpart){//aggiorno i tabelloni e li metto a 0 
	int i;
	int join=tabelloni[indexpart].joiner;
	int create=tabelloni[indexpart].creator;

		
	for(i=0;i<9;i++){
		tabelloni[indexpart].griglia[i]=0;
	}
	tabelloni[indexpart].ums[0]=-1;
	tabelloni[indexpart].ums[1]=-1;
	tabelloni[indexpart].joiner=-1;
	tabelloni[indexpart].creator=-1;


	pthread_mutex_lock(&usersmtx);
	USERS[	join	].indexpartita=-1;
	USERS[	join	].stato=0;
	USERS[	create	].indexpartita=-1;
	USERS[	create	].stato=0;
	pthread_mutex_unlock(&usersmtx);
	


}
// funzione che controlla se il nome utente è già presente 
int check(char * name){  
	int i=0;
	int trovato = -1;
	for(i; i<N; i++){
		printf("%s\t %d\t %d;;\n",USERS[i].name,USERS[i].stato,USERS[i].indexpartita);
		if(!strcmp(name,USERS[i].name))
			trovato = i;
	
	}
	return trovato;
}

void add(char * name,int id) {
	int st=0;
	strcpy(USERS[id].name,name); //questa funzione copia il nome del users che il thread sta servendo, nella riga corrispondente al thread
	USERS[id].stato=st;
}

void removeuser(int id) {
	strcpy(USERS[id].name,""); //qui si rimuove
	USERS[id].stato=-1;
}
int getStato (char*name){
	int i;
	for(i=0;i<N;i++)
		if (!strcmp(name,USERS[i].name))
			return USERS[i].stato;
	return -100;

}
int getTid (char*name){
	int i;
	for(i=0;i<N;i++)
		if (!strcmp(name,USERS[i].name))
			return i;
	return -100;

}
	int sid;
	struct sockaddr_in sa;
	int cid=-100;
	struct sockaddr_in ca;
	
void startsock(int port,char * ospite){
		memset(&sa,0,sizeof(sa)); //mette a zero la socket address del server
		memset(&sa,0,sizeof(ca));// del client
		
		sid=socket(AF_INET, SOCK_STREAM, 0) ;
		
//		USGEXT ((espressione),"messaggio d'errore")
		USGEXT((sid<0),"errore socket")

		sa.sin_family=AF_INET;
		sa.sin_port = port;
		sa.sin_addr.s_addr = inet_addr(ospite);
		
		USGEXT (bind(sid, (struct sockaddr *) &sa, sizeof(sa)) ==-1,"bind\n")
		listen(sid, SOMAXCONN);
		printf("socket init\n");
		
	}
	
char * who(){
	char * utenti = (char *) malloc(sizeof(char)*300);
	int i;
	char tmp[100]="";
	utenti[0]='\0';
	char state[20];
	for(i=0;i<N;i++){
		if (strcmp(USERS[i].name,"")){
			if (USERS[i].stato==0) strcpy(state,"(CONNECTED)");
			else
			if(USERS[i].stato==1) strcpy(state,"(WAIT)");
			else strcpy(state,"(BUSY)");
			sprintf(tmp,"%s %s ",USERS[i].name,state);
			utenti=strcat(utenti,tmp);
		}
	}
	return utenti;

}
int prepartita(int clientsock, int tid){
	int msg;
	char info[300];
	char * utenti;
	int i;
	int statopartita=0;
	do{
		recv(clientsock,&msg, sizeof(int),0);
		if(msg==1){
			utenti = who();
			msg=strlen(utenti);
			send(clientsock,&msg,sizeof(int),0);
			send(clientsock,utenti,msg+1,0);
			free(utenti);
		}
		else				
		if (msg==2){//create 
			statopartita=1;

			pthread_mutex_lock(&usersmtx);
			
			USERS[tid].stato = 1 ;//lo stato del client diventa waiting 
			USERS[tid].indexpartita = tid;
			printf("stato %d\n",USERS[tid].stato);

			pthread_mutex_unlock(&usersmtx);

			pthread_mutex_lock(&arrmtx[tid]);
			tabelloni[tid].ums[0] = -1;//modifico la struttura della tabella ponendo il campo ultimamossa a -1
			tabelloni[tid].ums[1] = -1;
			tabelloni[tid].joiner = -1;
			tabelloni[tid].creator= tid;
			while(tabelloni[tid].joiner==-1){//finchè nessuno fa la join rimane a -1
				pthread_cond_wait(&arrcond[tid],&arrmtx[tid]);//lo metto in attesa sulla coda relativa alla mutex e a questa corrisponde la signal che farò nella join
			}
			msg=strlen(USERS[tabelloni[tid].joiner].name);
			send(clientsock,&msg,sizeof(int),0);//invia al client il nome dell'avversario
			send(clientsock,USERS[tabelloni[tid].joiner].name,msg+1,0);
			pthread_mutex_unlock(&arrmtx[tid]);
			
			

		}
		else
		if(msg==3){//JOIN
			recv(clientsock,&msg,sizeof(int),0);
			recv(clientsock,info,msg+1,0);
			if(check(info)!=-1){
				if(getStato(info)==0){//giocatore info non in ascolto
					msg=3;
					send(clientsock,&msg,sizeof(int),0);
				}
				else
				if(getStato(info)==1){//giocatore info in wait, va bene
					int avversario=getTid(info); //identificatore del threads che serve il client a cui mi voglio connettere
					msg=4;
					send(clientsock,&msg,sizeof(int),0);
					pthread_mutex_lock(&arrmtx[avversario]);//un giocatore che si vuole unire alla partita di avversario prende il lucchetto 
					tabelloni[avversario].joiner=tid;//metto il mio identificatore in pl2 dello sfidante

					pthread_mutex_lock(&usersmtx);
					USERS[tid].stato=2;
					USERS[tid].indexpartita=avversario;
					USERS[avversario].stato=2;
					pthread_mutex_unlock(&usersmtx);

					statopartita=1;
					pthread_cond_signal(&arrcond[avversario]);//risveglio la coda quando io faccio la join
					pthread_mutex_unlock(&arrmtx[avversario]);

					pthread_mutex_lock(&arrmtx[avversario]);
					printf("sto facendo la join, avversario = %d\n", avversario);
					while(tabelloni[avversario].ums[0]==-1){
						pthread_cond_wait(&arrcond[avversario],&arrmtx[avversario]);
					}

					if(tabelloni[avversario].ums[0]==-2){//il mio avversario ha fatto disconnect
						printf("l'avversario ha fatto disconnect,avversario=%d\n", avversario);
						finepartita(avversario);
						printf("partita finita\n");
						statopartita=0;
						msg=5;
						send(clientsock,&msg,sizeof(int),0);

					}
					else{

//						tabelloni[avversario].griglia[tabelloni[tid].ultimamossa]=1;
						msg=tabelloni[avversario].ums[0]+7;
						tabelloni[avversario].ums[0]=-1;
						send(clientsock,&msg,sizeof(int),0);

					}
					pthread_cond_signal(&arrcond[avversario]);
					pthread_mutex_unlock(&arrmtx[avversario]);
				}
				else
				if(getStato(info)==2){//giocatore info già occupato
					msg=1;
					send(clientsock,&msg,sizeof(int),0);
				}
//				
			
			}
			else{
				msg=0;//giocatore non presente
				send(clientsock,&msg,sizeof(int),0);
			}

		}
		else
		if(msg==5){//quit
			statopartita=2;
			pthread_mutex_lock(&usersmtx);
			removeuser(tid);
			pthread_mutex_unlock(&usersmtx);
			
			}
	}while(statopartita==0);
	return statopartita;

}

void partita(int clientsock,int tid){	
	int msg;
	char info[300];
	int statopartita=1;
	int i;
	do{
		recv(clientsock,&msg, sizeof(int),0);
		if(msg==4){//disconnect
			int indexpart;
			int nemico;
			pthread_mutex_lock(&usersmtx);
				indexpart = USERS[tid].indexpartita;
			pthread_mutex_unlock(&usersmtx);

			pthread_mutex_lock(&arrmtx[indexpart]);//prendo la mutex
			if (indexpart==tid) { //sono io che ho fatto la create
				nemico=1; //l'avversario è chi ha fatto la join. l'indice dell'array delle ultime mosse è 1
			}
			else
				nemico=0;
			tabelloni[indexpart].ums[(nemico+1)%2]=-2;

			pthread_cond_signal(&arrcond[indexpart]);
			pthread_mutex_unlock(&arrmtx[indexpart]);
			statopartita=0;
			//finepartita(tid); questa chiamata la fa l'avversario
			msg=1;
			send(clientsock,&msg,sizeof(int),0);
		}			
		else
		if(msg==6){//show_map
			send(clientsock,&tabelloni[USERS[tid].indexpartita].griglia,sizeof(int)*9,0);

		}
		else
		if(msg>=7){//hitcell
			int casella=msg-7;
			int i=0;
			int indexpart;
			int nemico;
			indexpart=USERS[tid].indexpartita;
			if(indexpart==tid)
				nemico=1;
			else
				nemico=0;
			if(check_mossa(tabelloni[indexpart].griglia,casella) == 1){
				int mysymbol = trovasimbolo(tabelloni[indexpart].griglia);
				int itsymbol;
				int esito=-1;
				
				if (mysymbol==1)
					itsymbol=2;
				else
					itsymbol=1;
				pthread_mutex_lock(&arrmtx[indexpart]);
				tabelloni[indexpart].ums[(nemico+1)%2]=casella;
				printf("TID=%d\n%d | %d\n %d | %d \n",tid,tabelloni[indexpart].creator,tabelloni[indexpart].joiner,tabelloni[indexpart].ums[0],tabelloni[indexpart].ums[1]);


				tabelloni[indexpart].griglia[casella]=mysymbol;//SOLO QUI VENGONO AGGIORNATE LE CELLE DELLA GRIGLIA


				esito = checkpartita(tabelloni[indexpart].griglia);
				printf("TID=%d\nho vinto? checkpartita %d\n",tid,esito);
				pthread_cond_signal(&arrcond[indexpart]);
				pthread_mutex_unlock(&arrmtx[indexpart]);
				if (esito==-1){

					pthread_mutex_lock(&arrmtx[indexpart]);
					while(tabelloni[indexpart].ums[nemico]==-1){
						pthread_cond_wait(&arrcond[indexpart],&arrmtx[indexpart]);
					}
					//se sono qui l'avversario ha fatto la mossa
					esito=checkpartita(tabelloni[indexpart].griglia);
					printf("TID =%d\nmi sono svelgiato: checkpartita %d\n",tid,esito);
					if (esito==-1){
						if(tabelloni[indexpart].ums[nemico]==-2){//il mio avversario ha fatto disconnect
							//aggiorno i tabelloni e li metto a 0 
							statopartita=0;
							finepartita(indexpart);
							msg=5;//l'avversario è arreso
							send(clientsock,&msg,sizeof(int),0);
						}

						else{
							msg=tabelloni[indexpart].ums[nemico]+7;		
							send(clientsock,&msg,sizeof(int),0);
						}

						tabelloni[indexpart].ums[nemico]=-1;
					}
					else{
						if(esito==0)
							msg=3;//pareggio
						if (esito==1)//ha vinto l'avversario
							msg=2;
						//la partita è finita
						printf("TID=%d ho perso",tid);
						finepartita(indexpart);
						printf("TID=%d partitafinita\n",tid);
						statopartita=0;
						send(clientsock,&msg,sizeof(int),0);	
						}
					pthread_cond_signal(&arrcond[indexpart]);
					pthread_mutex_unlock(&arrmtx[indexpart]);
				}
				else{
						if(esito==0)
							msg=3;//pareggio
						if (esito==1)
							msg=1;//ho vinto
						//la partita è finita
						statopartita=0;

						send(clientsock,&msg,sizeof(int),0);
				}
				
			}	
			else{//mando zero se la mossa non è valida
				msg=0;
				send(clientsock,&msg,sizeof(int),0);				
			}	
		}
	}while(statopartita==1);
			
}
void * funct(void * arg){
	int id = (int) arg;
	printf("thread %d pronto\n",id);
	int sck_cli;
	int richiesta=0,len=0,finire=0, errore=0;
	char nome[100];
	int comando;

	
	//si mette in attesa, sulla mutex ...
	//..wait
	while(1){//per servire clineti all'infinito		
		pthread_mutex_lock(&cidMtx);
		while(cid==-100){
			printf("thr %d aspetta\n",id);
			pthread_cond_wait(&coda_cts,&cidMtx);
		}
		sck_cli=cid;
		cid =-100;
		//c'è un client da servire
		pthread_mutex_unlock(&cidMtx);
		//trhead è pronto a servire 
		do{
			recv(sck_cli,(void *) &comando,sizeof(int),0);
		
			recv(sck_cli,(void*) nome, comando+1,0);
			pthread_mutex_lock(&usersmtx);
			if (check(nome)==-1){ //controllo che il nome non ci sia già e se non c'è lo ggiungo e mando un mess al client 1
				printf("--%s-- aggiunto\n",nome);
				add(nome,id);
				comando=1;
				send(sck_cli, (void *) &comando, sizeof(int),0);
			}
			else { //se il nome è già present mando al client un messaggio 0 per dirgli che già c'è
				
				comando=0;
				send(sck_cli, (void *) &comando, sizeof(int),0);
			}
			pthread_mutex_unlock(&usersmtx);
		}while (comando == 0);
		//qui andrà chiamato prepartita(sck_cli)
		while(prepartita(sck_cli,id)!=2){
			partita(sck_cli,id);
			}
		
		close(sck_cli);
	}
	
}

int main (int argc, char *argv[]) {
	char host[]=  "127.0.0.1";
	int porta=1234;
	int len =sizeof(sa);
		
	pthread_t TIDS[N];
	int i=0;
	
	{
	if (argc != 3){
		perror("Mancano argomenti\n");
		exit (1);
		}
	else
	strcpy(host,argv[1]);
	porta = atoi(argv[2]);

	}

	for(i=0; i<N; i++) { //for di inizializzazione di tutto
		pthread_create	(TIDS+i,NULL,&funct,i); //creo i thread

		strcpy(USERS[i].name,"");//metto tutti i campi degli users vuoti
		USERS[i].indexpartita=-1;

		tabelloni[i].ums[0] = tabelloni[i].ums[1] = -1;
		tabelloni[i].creator=-1;
		tabelloni[i].joiner=-1;
		

		pthread_mutex_init(arrmtx+i,NULL);
		pthread_cond_init(arrcond+i,NULL);
	}
	startsock(porta,host); //dopo questa chiamata di funzione dentro sid c'è il descrittore del file della socket, pronta ad accettare delle richieste 
	while(1){
		cid=accept(sid,(struct sockaddr *) &sa, &len);
		printf("A client has connected\n");
		//accesso in mutua esclusione, servirà mutex
		pthread_mutex_lock(&toccMtx); //chiudo il lucchetto
		while (toccup==N){
			pthread_cond_wait(&condTocc,&toccMtx);
			//vai in wait, rilasciando la mutex come argomento della wait ho la variabile di condizione(coda) e la mutex(indirizzi) 
			//appena esco da wait riprendo la mutex su cui stavo aspettando
		}
		toccup++;
		pthread_cond_broadcast(&condTocc); //risveglio uno tra i threads in attesa di tocc
		pthread_mutex_unlock(&toccMtx); //sblocco la mutex
		
		//si è liberato almeno un thread
		pthread_cond_signal(&coda_cts);

		
		
	}
	
}
