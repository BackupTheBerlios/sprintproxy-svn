/***************************************************************************
 *                   ANSI-C <<SPRINT-PROXY>>                               *
 *                    Copyright (C) 2004 by                                *
 *                                                                         *
 *        Umsetzung eines kleinen, aber nuetzlichen Http-Proxys            *
 *                                                                         *
 *        Thomas Kuse     thomas.kuse@fh-stralsund.de                      *
 *        Stefan Hermann  stefan.hermann@fh-stralsund.de                   *
 *        Christian Hass  christian.hass@fh-stralsund.de                   *
 *                                                                         *
 ***************************************************************************
 *       Programmiert als Projekt an der Fachhoschule Stralsund 2004       *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "sprintproxy.h"

#define _OUTPUT
#define _DEBUG
#define _STATS

/*********************************************************
 * Netzwerkverkehr zählen
 * @param int Bytes der übertragenen Datei
 * @param short IN=0 OUT=1
 * @return Pointer auf statische statistik-Struktur
 */
#ifdef _STATS
struct stats* countTraffic(int vBytes, unsigned short inout){
  #ifdef _DEBUG
  fprintf(stderr,"++ countTraffic\n");
  #endif
  static struct stats completeStats;

  if(inout){
    completeStats.bytesOUT+=vBytes;
    completeStats.filesOUT++;
  }else{
    completeStats.bytesIN+=vBytes;
    completeStats.filesIN++;
  }
  #ifdef _DEBUG
  fprintf(stderr,"-- countTraffic\n");
  #endif
  return &completeStats;
}
#endif

/*********************************************************
 * Adress-Struktur vom Server Richtung Client füllen
 * @param pAddress Pointer auf zu füllende Socket-Struktur
 */
int generateProxyAddress(struct sockaddr_in* pAddress){
  #ifdef _DEBUG
  fprintf(stderr,"++ generateProxyAddress\n");
  #endif

  memset(pAddress,0,sizeof(struct sockaddr_in));
  pAddress->sin_family=AF_INET;
  pAddress->sin_port=htons(HTTP_PROXY_PORT);
  pAddress->sin_addr.s_addr=INADDR_ANY;

  if(pAddress->sin_port<=0 || pAddress==NULL){
    #ifdef _DEBUG
    fprintf(stderr,"-- generateProxyAddress | ERR port oder Adresse NULL\n");
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- generateProxyAddress\n");
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 *  Adress-Struktur vom Server Richtung WEB füllen
 *  @param pAddress Zeiger auf Socket-Struktur
 *  @param pUrlPar  Zeiger auf URL-Parameter
 */
int generateWebAddress(struct sockaddr_in *pAddress,struct urlPar *pUrlPar){
  #ifdef _DEBUG
  fprintf(stderr,"++ generateWebAddress\n");
  #endif

  struct hostent* Host;

  memset(pAddress,0,sizeof(struct sockaddr_in)); // zuerst alles auf 0 setzten

  fprintf(stderr,"1\n");

  pAddress->sin_family  = AF_INET;
  pAddress->sin_port    = htons(HTTP_PORT);

  fprintf(stderr,"2\n");

  //Host in verwendbare Werte umrechnen
  pAddress->sin_addr.s_addr=inet_addr(pUrlPar->host);
  if(pAddress->sin_addr.s_addr==INADDR_NONE || (strcmp(pUrlPar->host, "255.255.255.255"))){
    fprintf(stderr,"3 %s:\n",pUrlPar->host);

    Host = gethostbyname(pUrlPar->host);
    if (Host == NULL){
      fprintf(stderr,"-- generateWebAddress | ERR: Host nicht erreichbar\n");
      return FALSE;
    }
    fprintf(stderr,"4\n");

    //Zeiger auf Socketstruktur für Rückgabe füllen
    memcpy((char*)&pAddress->sin_addr.s_addr,Host->h_addr,4);
    fprintf(stderr,"5\n");

    if(pAddress->sin_addr.s_addr==INADDR_NONE){
      fprintf(stderr,"-- generateWebAddress | ERR: Adresse nicht erstellbar\n");
      return FALSE;
    }
    #ifdef _DEBUG
    fprintf(stderr,"-- generateWebAddress\n");
    #endif
    return TRUE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"ERR Adresse nicht generierbar\n");
    fprintf(stderr,"-- generateWebAddress\n");
    #endif
    return FALSE;
  }
}

/*************************************************************************************
 *Neuen TCP-Socket erstellen (TCP/IP)
 * @param int* Pointer auf SocketStructur richtung WEB
 */
int createSocket(int* sProxyWeb){
  #ifdef _DEBUG
  fprintf(stderr,"++ createSocket\n");
  #endif

  *sProxyWeb=socket(AF_INET,SOCK_STREAM,0);

  if(*sProxyWeb==INVALID_SOCKET){
    #ifdef _DEBUG
    fprintf(stderr,"-- createSocket | ERROR kein Socket erstellt\n");
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- createSocket | pSocket:%d\n",*sProxyWeb);
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 * warte auf verbindung...
 * @param int* Pointer auf Socket, der in Listen-Zustand wechseln soll
 */
int listenSocket(int* pSocket){
  #ifdef _DEBUG
  fprintf(stderr,"++ listenSocket\n");
  #endif
  long vWS  = ~0;

  vWS=listen(*pSocket,MAX_LISTEN_SOCKETS);

  if(vWS==SOCKET_ERROR){
    #ifdef _DEBUG
    fprintf(stderr,"-- listenSocket | ERROR beim Hören\n");
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- listenSocket || WAITING.....\n");
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 * Akzeptiere verbundenen Socket
 * @param int* Pointer auf akzeptierten Socket
 * @param int* Rückggabe pointer auf Verbundenen Socket
 */
int acceptSocket(int accepted,int *pConnected){
  #ifdef _DEBUG
  fprintf(stderr,"++ acceptSocket %d\n",accepted);
  #endif

  *pConnected=accept(accepted,NULL,NULL);

  if(*pConnected==INVALID_SOCKET){
    #ifdef _DEBUG
    fprintf(stderr,"-- acceptSocket | Error beim ACCEPT\n");
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- acceptSocket | ACCEPTED %d %d\n",*pConnected, accepted);
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 * Erstellten Socket mit Adresse connecten
 * @param int SocketID
 * @param sockaddr_in* Rückgabepointer auf auf neue Socket-Adresse
 */
int connectSocket(int sSocket,struct sockaddr_in *pAddress){
  #ifdef _DEBUG
  fprintf(stderr,"++ connectSocket\n");
  #endif

  long  receive  = ~0;

  receive=connect(sSocket,(struct sockaddr*)pAddress,sizeof(struct sockaddr_in));

  if(receive==SOCKET_ERROR){
    #ifdef _DEBUG
    fprintf(stderr,"-- connectSocket | SOCKETERROR\n");
    #endif
    return FALSE;
  }

  #ifdef _DEBUG
  fprintf(stderr,"-- connectSocket | Adresse verbunden %d\n",sSocket);
  #endif
  return TRUE;
}

/*************************************************************************************
 * binde socket an Adresse
 * @param int* Pointer auf SocketID
 * @param sockaddr_in* Rückgabepointer auf auf neue Socket-Adresse
 */
int bindSocket(int *pSocket,struct sockaddr_in *pAddress){
  #ifdef _DEBUG
  fprintf(stderr,"++ bindSocket\n");
  #endif
  long  vWS = ~0;

  vWS=bind(*pSocket,(struct sockaddr*)pAddress,sizeof(struct sockaddr_in));

  #ifdef _OUTPUT
  fprintf(stderr,"vWS:%d\n",(int)vWS);
  #endif

  if(vWS==SOCKET_ERROR){
    #ifdef _DEBUG
    fprintf(stderr,"-- bindSocket | SOCKETERROR\n");
    #endif
    return FALSE;
  }else{
    fprintf(stderr,"Socket an port %d gebunden\n",HTTP_PROXY_PORT);
    #ifdef _DEBUG
    fprintf(stderr,"-- bindSocket\n");
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 * char-array an socket schicken
 * @param int SocketID
 * @param netStream* Pointer auf zu versendenden Buffer
 */
int sendBuffer(int sSocket, struct netStream* pWebBuf){
  #ifdef _DEBUG
  fprintf(stderr,"++ sendBuffer\n");
  #endif

  int vSent = ~0;

  vSent=send(sSocket,pWebBuf->pBuf,pWebBuf->len,0);

  #ifdef _STATS
  struct stats* tmpStats=countTraffic(vSent,1);
  fprintf(stderr,"%d Bytes gesendet | GESAMT: FilesOUT %d , BytesOUT %d\n",vSent,tmpStats->filesOUT,tmpStats->bytesOUT);
  #endif

  if(vSent<=0){
    #ifdef _DEBUG
    fprintf(stderr,"-- sendbuffer | Fehler beim Senden %d\n",vSent);
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- sendbuffer");
    #endif
    return TRUE;
  }
}

/*************************************************************************************
 * ERROR-Message an Socket schicken
 */
int sendError(const char* pErrTxt, int sSocket, struct netStream * pStream){
  #ifdef _DEBUG
  fprintf(stderr,"++ sendERROR\n");
  #endif

  int vSent = 1;
  /*

  if (sSocket!=0){
    struct netStream webBuf;
    char bufHead[]="application X -sadasdhwadh";
    char bufFoot[]="application X -sadasdhwadh";

    webBuf.pBuf=(char*)malloc(sizeof(char)*strlen(bufHead)+strlen(pErrTxt)+1);

    strcpy(webBuf.pBuf,bufHead);
    strcat(webBuf.pBuf,pErrTxt);
    strcat(webBuf.pBuf,bufFoot);
    webBuf.len =strlen(webBuf.pBuf)+1;

    vSent=send(sSocket,webBuf.pBuf,webBuf.len,0);

    free(webBuf.pBuf);
  }

  #ifdef _OUTPUT
  fprintf(stderr,"ERROR-OUT-SEND %d Bytes sent: %s\n",vSent,pErrTxt);
  #endif
  */

  if(vSent<=0){
    #ifdef _DEBUG
    fprintf(stderr,"-- sendERROR | Error %d\n",vSent);
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- sendERROR\n");
    #endif
    return TRUE;
  }
}

/****************************************************************************************
 * HTTP-Header vom Socket lesen und in Struktur speichern
 * @param netStream* Struktur zum füllen des Inhalts (pBuf) und Länge (len) des Datenstreams
 * @param int     Socket-ID
 */
int receiveHeader(struct netStream* pWebBuf, int sProxyClient){
  #ifdef _DEBUG
  fprintf(stderr,"++ receiveHEADER\n");
  #endif

  pWebBuf->len=recv(sProxyClient, pWebBuf->pBuf, RECEIVE_BUFFER_LENGTH , 0);
  pWebBuf->pBuf[pWebBuf->len-1]='\0';

  #ifdef _STATS
  fprintf(stderr,"<<<<<<<<%d Bytes read>>>>>>>>",pWebBuf->len);
  fprintf(stderr,"<<<<<<<<\n%s\n>>>>>>>>",pWebBuf->pBuf);
#endif

  if(pWebBuf->len<=0){
    #ifdef _DEBUG
    fprintf(stderr,"-- receiveHEADER | ERR %d | %d\n",pWebBuf->len,sProxyClient);
    #endif
    return FALSE;
  }else{
    #ifdef _DEBUG
    fprintf(stderr,"-- receiveHEADER\n");
    #endif
    return TRUE;
  }
}

/****************************************************************************************
 * Datenstream vom Socket lesen und in Struktur speichern
 * @param netStream* Struktur zum füllen des Inhalts (pBuf) und Länge (len) des Datenstreams
 * @param int     Socket-ID
 */
 int receiveBuffer(struct netStream* pWebBuf,int sProxyClient){
  #ifdef _DEBUG
  fprintf(stderr,"++ receivebuffer\n");
  #endif

  long   vTextLength = 0,
          vCount      = 0;

  char    *pRecord    = NULL,
          *pTmp       = NULL,
          cRec        = 0,
          cBuf[RECEIVE_BUFFER_LENGTH];

  int     vBufLen     = 0;

  /*initialisere speicher-buffer als leeren string*/
  pRecord=(char*)malloc(1);
  pRecord[0]='\0';

  /*datenpakete byte für byte einlesen*/
  while((vCount = recv(sProxyClient, &cRec, 1 , 0))==1){
    cBuf[vBufLen]=cRec;                                 /*empfangenes zeichen speichern*/

    ++vBufLen;

    /*nach einer bestimmten Anzahl Bytes den Block aus cBuf speichern*/
    if(vBufLen==(RECEIVE_BUFFER_LENGTH-1)){
      #ifdef _DEBUG
      fprintf(stderr,"RBL-start %d,%d",(int)vBufLen,(int)vTextLength);
      #endif

      vTextLength+= vBufLen;                            /*neue gesamt-länge berechnen*/
      pTmp=(char*)malloc(sizeof(char)*vTextLength);     /*neuen speicherbereich für gesamten buffer anlegen*/

      memcpy(pTmp,pRecord,vTextLength-vBufLen);         /*alten inhalt übernehmen*/
      free(pRecord);
      pRecord=pTmp;
      memcpy(pRecord+vTextLength-vBufLen,cBuf,vBufLen); /*neuen inhalt speichern*/

      vBufLen=0;
      #ifdef _DEBUG
      fprintf(stderr,"RBL-ende#");
      #endif
    }
  }

  vTextLength+=vBufLen;                                 /*neue gesamt-länge berechnen*/
  if(vTextLength<=0){
    #ifdef _DEBUG
    fprintf(stderr,"\n-- receivebuffer | Error zu kurz %d\n",vTextLength);
    #endif
    return FALSE;
  }

  pTmp=(char*)malloc(sizeof(char)*vTextLength);         /*neuen speicherbereich für gesamten buffer anlegen*/

  memcpy(pTmp,pRecord,vTextLength-vBufLen);             /*alten inhalt übernehmen*/
  free(pRecord);
  pRecord=pTmp;
  memcpy(pRecord+vTextLength-vBufLen,cBuf,vBufLen);     /*neuen inhalt speichern*/

  pWebBuf->len=vTextLength;
  pWebBuf->pBuf=pRecord;

  #ifdef _STATS
  struct stats* tmpStats=countTraffic(vTextLength,0);
  fprintf(stderr,"STATS: FilesIN %d , BytesIN %d\n",tmpStats->filesIN,tmpStats->bytesIN);
  #endif

  #ifdef _DEBUG
  fprintf(stderr,"\n-- receivebuffer\n");
  #endif
  return TRUE;
}

/**************************************************************************************
 * Parameter für den Verbindungsaufbau aus der Anfrage des Clients lesen und speichern
 * @param pUrlPar Pointer auf eine zu füllende Struktur
 * @param pBuf    Pointer auf den vom Client erhaltenen Char-Buffer
 */
 int fillParFromBuf(struct urlPar* pUrlPar, char *pBuf){
  #ifdef _DEBUG
  fprintf(stderr,"++ fillparfrombuf\n");
  #endif
   char   *pFind      = NULL, //Pointer auf den aktuellen Suchzeiger
          *pFindPost  = NULL, //Pointer für den Anfang der POST-Anfrage
          *pFindGet   = NULL, //Pointer für den Anfang der GET-Anfrage
          *pTmp       = NULL,
          *pTmp2      = NULL,
          *pHost      = NULL;
   int    i;

  //parameter anlegen und initialisieren
  pUrlPar->len_host = 0;
  pUrlPar->len_url  = 0;

  //Nach GET oder POST in der Anfrage suchen
  if((pFindGet=strstr(pBuf,"GET"))!=NULL || (pFindPost=strstr(pBuf,"POST"))!=NULL){

    pFind=pFindGet?pFindGet:pFindPost;        //nur den jeweilligen gefundenen Anfang nehmen

    //komplette URL herauslesen
    pFind += pFindGet?4:5;      //GET oder POST überspringen (3 oder 4 Zeichen + Leerzeichen)
    pTmp  =  strchr(pFind,' '); //bis zum ende der Adresse laufen (leerzeichen beendet die url)

    //nach host-adresse suchen und einlesen
    if( (pHost=strstr(pFind,"http://") )!=NULL )
      pHost+=7; //http überspringen
    else
      pHost=pFind;

    pUrlPar->len_url=pTmp-pHost+7;//BUFFER-Länge speichern (URL)
    pTmp2=strchr(pHost,'/');

    //host-laenge relativ zur url ausrechnen
    pUrlPar->len_host = pTmp2-pHost; //BUFFER-Länge speichern (HOST)

    //Parameter der übergebenen Struktur füllen
    strncpy(pUrlPar->url ,pHost-7,pUrlPar->len_url);  //URL in Struktur übernehmen
    strncpy(pUrlPar->host,pHost,  pUrlPar->len_host); //HOST in Struktur übernehmen
    //Strings abschliessen
    pUrlPar->url [pUrlPar->len_url]  = '\0';
    pUrlPar->host[pUrlPar->len_host] = '\0';

    #ifdef _DEBUG
    fprintf(stderr,"-- fillparfrombuf\n");
    #endif
    return TRUE;
  }else{
    //ERROR
    #ifdef _DEBUG
    fprintf(stderr,"-- fillparfrombuf | Error kein GET/Post gefunden\n");
    #endif
    return FALSE;
  }
}

/**************************************************************************************
 * Abwicklung eines neuen Clients
 *   - dynamische Erstellung der benutzten Datenstrukturen
 *   1 Anfrage vom Client erhalten (HEADER)
 *   2 Parameter extrahieren
 *   3 Neuen Socket erstellen
 *   4 Neue Adresse Richtung ausgesuchter IP erstellen
 *   5 Verbindung zu IP herstellen
 *   7 Empfangenen HEADER an IP schicken
 *   8 Antwort von IP empfangen
 *   9 Antwort an Client schicken
 */
void* handleClient(void* pTP){
  #ifdef _DEBUG
  fprintf(stderr,"++ handleclient\n");
  #endif
/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^
  int     sProxyWeb   = ~0;
  struct  sockaddr_in saProxyAddress;
  struct  threadParam *pThreadParam=(struct threadParam*)pTP;

  /* Dynamische Bereitstellung des benötigten Speichers */
  struct  urlPar    *pUrlPar     = (struct urlPar*)   malloc(sizeof(struct urlPar));
  struct  netStream *pWebBuf     = (struct netStream*)malloc(sizeof(struct netStream));
  struct  netStream *pClientBuf  = (struct netStream*)malloc(sizeof(struct netStream));
  struct  netStream *pErrBuf     = (struct netStream*)malloc(sizeof(struct netStream));
/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^/////////^^^^^^^^

  if (receiveHeader      (pClientBuf             ,pThreadParam->socketID))
  if (fillParFromBuf     (pUrlPar                ,pClientBuf->pBuf))
  if (createSocket       (&sProxyWeb))
  if (generateWebAddress (&saProxyAddress        ,pUrlPar))
  if (connectSocket      (sProxyWeb              ,&saProxyAddress))
  if (sendBuffer         (sProxyWeb              ,pClientBuf))
  if (receiveBuffer      (pWebBuf                ,sProxyWeb))
  if (sendBuffer         (pThreadParam->socketID ,pWebBuf));

#ifdef _DEBUG
  fprintf(stderr,"HC - END\n");
#endif

  close(pThreadParam->socketID);
  close(sProxyWeb);
  free(pClientBuf->pBuf);
  free(pClientBuf);
  free(pThreadParam);
  free(pWebBuf->pBuf);
  free(pWebBuf);
  free(pUrlPar);

  #ifdef _DEBUG
  fprintf(stderr,"-- handleclient\n");
  #endif
  //return NULL;
}

/******************************************
 * Initialisieren und starten des Servers
 */
void* handleServer(){
  #ifdef _DEBUG
  fprintf(stderr,"++ handleserver\n");
  #endif
  char    bOK         = TRUE;
  int     sProxyClient= ~0,
          sProxyWait  = ~0;
  struct  sockaddr_in saAddress;
  struct  threadParam *pThreadParam;

  if(!generateProxyAddress(&saAddress))             return NULL;
  if(!createSocket        (&sProxyWait))            return NULL;
  if(!bindSocket          (&sProxyWait,&saAddress)) return NULL;
  if(!listenSocket        (&sProxyWait))            return NULL;

  //Endlos-schleife fuer accepts
  while(1){
    bOK=acceptSocket(sProxyWait,&sProxyClient);
    if(bOK==TRUE){
      #ifdef _DEBUG
      fprintf(stderr,"***threadNewClient %d\n",sProxyClient);
      #endif
      pThreadParam=(struct threadParam*)malloc(sizeof(struct threadParam));
      pThreadParam->socketID=sProxyClient;
      threadNewClient((void*)pThreadParam);
    }
  }
  closesocket(sProxyWait);

  #ifdef _DEBUG
  fprintf(stderr,"-- handleserver\n");
  #endif
}

/**************************************************************************************
 * Neuer Thread für die Verarbeitung eines neuen ClientSockets wird erstellt
 * @param clientSocket Pointer auf Socket-ID
 * @return EXIT_SUCCESS
 */
int threadNewClient(void *pThreadParam){
   pthread_t linux_thread;
   pthread_create(&linux_thread,NULL,(void*)&handleClient,pThreadParam);
   return EXIT_SUCCESS;
}

/**************************************************************************************/
/**************************************************************************************/
int main(int argc, char *argv[]){
  while(1){
    handleServer();
    sleep(1);
  }
  return EXIT_SUCCESS;
}
