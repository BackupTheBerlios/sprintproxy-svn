/***************************************************************************
 *                   ANSI-C <<SPRINT-PROXY>>                               *
 *                    Copyright (C) 2004 by                                *
 *                                                                         *
 *        Thomas Kuse     thomas.kuse@fh-stralsund.de                      *
 *        Stefan Hermann  stefan.hermann@fh-stralsund.de                   *
 *        Christian Hass  christian.hass@fh-stralsund.de                   *
 *                                                                         *
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

#ifndef SPRINTPROXY_H
  #define SPRINTPROXY_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <pthread.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <netinet/in.h>
  /*++++++++++++++++++++*/

  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1

  #define HTTP_PORT 80
  #define HTTP_PROXY_PORT 12346
  #define CRLF "\x0D\x0A\x0D\x0A"
  #define MAX_LISTEN_SOCKETS 5
  #define RECEIVE_BUFFER_LENGTH 2048
  #define BUFFER_LENGTH 1024
  /*--------------------*/
  #define TRUE  0x01
  #define FALSE 0x00
  /*--------------------*/

  struct urlPar
  {
    char url[BUFFER_LENGTH];
    unsigned short len_url;
    char host[BUFFER_LENGTH];
    unsigned short len_host;
    char site[BUFFER_LENGTH];
    unsigned short len_site;
  };

  struct webBuf
  {
    char *pBuf;
    int  len;
  };

  struct stats
  {
    unsigned long bytesIN;
    unsigned int  filesIN;
    unsigned long bytesOUT;
    unsigned int  filesOUT;
  };


  /*********************************************************
   *Netzwerkverkehr z�hlen
   */
  struct stats* countTraffic(int vBytes,unsigned short inout);

  /*********************************************************
  *Adress-Struktur RICHTUNG CLIENT f�llen
  */
  void generateProxyAddress(struct sockaddr_in* pAddress);

  /*************************************************************************************
  *Adress-Struktur RICHTUNG WEB f�llen
  */
  int generateWebAddress(struct sockaddr_in *pAddress,struct urlPar *pUrlPar);

  /*************************************************************************************
  *Neuen TCP-Socket erstellen (TCP/IP)*/
  int createSocket(int* pSocket);

  /*************************************************************************************
  * warte auf verbindung...
  */
  int listenSocket(int* pSocket);

  /*************************************************************************************
  * Akzeptiere Socket
  */
  int acceptSocket(int *pAccepted,int *pConnected);

  /*************************************************************************************
  * Erstellter Socket mit Adresse connecten
  */
  int connectSocket(int sSocket,struct sockaddr_in *pAddress);

  /*************************************************************************************
  * binde socket an Adresse
  */
  int bindSocket(int *pSocket,struct sockaddr_in *pAddress);

  /*************************************************************************************
  * char-array an socket schicken
  */
  int sendBuffer(int sSocket, struct webBuf * pWebBuf);

  /*************************************************************************************
  * char-array vom socket lesen
  */

  void receiveBuffer(struct webBuf* pWebBuf,int sProxyClient);
  void receiveHeader(struct webBuf* pWebBuf,int sProxyClient);
  /**************************************************************************************
  *buffer auswerten
  */
  void fillParFromBuf(struct urlPar* pUrlPar, char *pBuf);

  /**************************************************************************************
  * Abwicklung fuer den Modus [Client-Start]
  * @return int EXIT_SUCCESS
  */
  void* handleClient(int* sProxyClient);

  /**************************************************************************************
  * Abwicklung fuer den Modus [SERVER-Start]
  * @return int EXIT_SUCCESS
  */
  void* handleServer();

  /**************************************************************************************
  * Neuer Thread f�r die Verarbeitung eines neuen ClientSockets wird erstellt
  */
  int threadNewClient(int* clientSocket);
#endif
