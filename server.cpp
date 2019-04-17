#include <iostream>
#include<unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "Spreadsheet.h"
#include "CircularException.h"


using namespace std;

vector<Spreadsheet> SpreadsheetList;
vector<string> userList;
pthread_mutex_t serverLock = PTHREAD_MUTEX_INITIALIZER;

/*
Returns Spreadsheet that a given socket/client belongs to.
*/
Spreadsheet& findSS(int client)
{
	for(int i=0; i < SpreadsheetList.size(); i++) // loop over all spreadsheets in vector
	{
		if(SpreadsheetList[i].containsUser(client))//check if the user is in the spreadsheet 
		{
			return SpreadsheetList[i];//if client socket id is found in the spreadsheet then the current spreadsheet is returned 
		}
	}
}

//normalizes a string so all a-z are uppercase.
string normalize(string content)
{
  string change;
  locale loc;
  for(int i = 0; i < content.length(); i++)
  {
	  change += toupper(content[i],loc);
  }
  return change;
}

//returns true if the given client is associated with a spreadsheet.
bool hasSS(int client)
{
	for(int i=0; i < SpreadsheetList.size(); i++) // loop over all spreadsheets in vector
	{
		if(SpreadsheetList[i].containsUser(client))//check if the user is in the spreadsheet 
		{
			return true;//if client socket id is found in the spreadsheet then the current spreadsheet is returned 
		}
	}
	return false;
}

//returns true if the given filename exists.
bool fileExists(string filename)
{
	string fname = filename+".txt";
	ifstream ifile(fname.c_str());
	return ifile;
}

//Sends messages to the client using their int socket identifier and the supplied message.
int send(int sockt, string message)
{
    int ret;
    ret = send(sockt, message.c_str(), strlen(message.c_str()),0);
    return 0;
}

//sends the message given to all sockets that share a spreadsheet with the given one
void sendAll(int client, string message)
{
	vector<int> socketList = findSS(client).getSocketList();
	for(int i = 0; i < socketList.size(); i++)
	{
		send(socketList[i], message);
	}
}
//receives messages from a client socket, uses sockt pointer to identify which client it came from and then
//parses the message to determine which action to take
 void * receive(void * sockt) 
 {
 
	// set socket
	int client;
	client = *((int*)&sockt);
	
	char buffer[256];
	
	while(1) {
	
		// clear buffer
		bzero(buffer,256);
		
		int numBytes;
		numBytes = read(client,buffer,255);
		
		if (numBytes <= 0)
        {
			if(hasSS(client))
			{				
				//Find the spreadsheet and save it
				Spreadsheet * temp = &findSS(client);
				temp->Save();
				temp->removeUser(client);
				if(temp->getSocketList().size() == 0)
				{
					for(int i = 0; i < SpreadsheetList.size(); i++)
					{
						if(SpreadsheetList[i].getName() == temp->getName())
						{
							SpreadsheetList.erase(SpreadsheetList.begin() + i);
						}
					}
				}
			}
			//close socket and end thread for client
            close(client);
            pthread_exit(0);
        }
		
		// Display message
		printf("Client: %d\nMessage: %s",client, buffer);
		
		// Lock
		pthread_mutex_lock(&serverLock);
		int num;
		//do stuff with message
		string msg = buffer;
		string temp = "";
		string message = "";
		while((num = msg.find_first_of("\n")) == -1)
		{
			numBytes = read(client, buffer, 256);
			temp = buffer;
			msg += temp;
		}	

		msg = msg.substr(0, msg.find_first_of("\n")+1);
		int index = msg.find_first_of(" ");
		string command = "";
		if(index > 0){
		 command = msg.substr(0, msg.find_first_of(" "));
		}
		else
		{
			command = msg.substr(0);
		}
		if(command == "connect")
		{
			//username and spreadsheet name.
			index = msg.find_first_of(" ", 8);
			string username = "";
			if(index > 0){
				username = msg.substr(8, index);
				index = username.find_first_of(" ");
				if(index >0)
				{
					username = username.substr(0, index);
				}
				else
				{
					message = "error 0 connect syntax "+msg.substr(8)+"\n";
					send(client, message);
					pthread_mutex_unlock(&serverLock);
					continue;
				}
			}
			else
			{
				message = "error 0 connect syntax "+msg.substr(8)+"\n";
				send(client, message);
				pthread_mutex_unlock(&serverLock);
				continue;
			}
			bool exists = false;
			if(username != "sysadmin")
			{
				for(int k = 0; k<userList.size(); k++)
				{
					if(userList[k] == username)
					{
						exists = true;
						break;
					}
				}
			}
			else
			{
				exists = true;
			}
			if(exists)
			{
				// open spreadsheet
				//create user and add to spreadsheet
				string ssname = msg.substr(9+username.length(), msg.find("\n"));
				if(ssname == "\n")
				{
					message = "error 2 "+msg.substr(8)+"\n";
					send(client, message);
					pthread_mutex_unlock(&serverLock);
					continue;
				}
				ssname = ssname.substr(0, ssname.length()-1);
				bool found = false;
				for(int i = 0; i<SpreadsheetList.size(); i++)
				{
					if(SpreadsheetList[i].getName() == ssname)
					{
						found = true;
					}
				}
				if(found){
					user usr(username, client);
					for(int i =0; i<SpreadsheetList.size(); i++)
					{
						if(SpreadsheetList[i].getName() == ssname)
						{
							SpreadsheetList[i].addUser(usr);
							SpreadsheetList[i].Save();
							map<string, string> sheet = SpreadsheetList[i].getSheet();

							//convert int to string
							int numberCells = sheet.size();
							stringstream ss;
							ss << numberCells;
							string cells = ss.str();
							
							message = "connected " + cells + " \n";
							send(client, message);
							
							//send cells from spreadsheet to client
							for(map<string, string>::iterator it = sheet.begin(); it != sheet.end(); it++)
							{
								message = "cell " + it->first + " " + it->second + "\n"; 
								send(client, message);
							}
		
						}
					}
				
				}
				else{	
					
					if(fileExists(ssname))
					{
						Spreadsheet SS(ssname);
						user usr(username, client);
						SS.addUser(usr);
						map<string, string> sheet = SS.Open(ssname);
						SpreadsheetList.push_back(SS);
						//convert int to string
						int numberCells = sheet.size();
						stringstream ss;
						ss << numberCells;
						string cells = ss.str();
						
						message = "connected " + cells + " \n";
						send(client, message);
						
						//send cells from spreadsheet to client
						for(map<string, string>::iterator it = sheet.begin(); it != sheet.end(); it++)
						{
							message = "cell " + it->first + " " + it->second + "\n"; 
							send(client, message);
						}
					}
					else{
						Spreadsheet SS(ssname);
						user usr(username, client);
						SS.addUser(usr);
						SpreadsheetList.push_back(SS);
						message = "connected 0\n";
						send(client, message);
					}
				}
			}
			else
			{
				message = "error 4 " + username + "\n";
				send(client, message);
			}
		}
		else if(!hasSS(client))
		{
			message = "error 3 Client is not connected to spreadsheet.\n";
			send(client, message);
			pthread_mutex_unlock(&serverLock);
			continue;
		}
		else if(command == "register")
		{
			//check if username exists 
			string username = "";
			username = msg.substr(9);
			index = username.find_first_of("\n");
			if(index == 0)
			{
				message = "error 0 Username not given.\n";
				send(client, message);
				pthread_mutex_unlock(&serverLock);
				continue;
			}
			else
			{
				username = username.substr(0,index);
			}
			bool used = false;
			for(int j = 0; j<userList.size(); j++)
			{
				if(userList[j] == username)
				{
					used = true;
					break;
				}
			}
			if(used)
			{
				message = "error 4 " + username + " \n";
				send(client, message);
			}
			else
			{
				//add new user registration
				userList.push_back(username);
				
				ofstream stream;
				string filename = "userList.txt";
				stream.open(filename.c_str());
				for(int i = 0; i<userList.size(); i++)
				{
					stream<<userList[i]<<"\n";
				}
				stream.close();
			}
		}
			
		else if(command == string("cell"))
		{
			string cellTemp = msg.substr(5); //cut off command
			string cellName = cellTemp.substr(0, cellTemp.find_first_of(" "));//get cell name
			string cellContents = cellTemp.substr(cellTemp.find_first_of(" ")+1, (cellTemp.find_first_of("\n")-(cellTemp.find_first_of(" ")+1)));//get cell contents
			cellName = normalize(cellName);
			cellContents = normalize(cellContents);
			bool circle = findSS(client).SetContentsOfCell(cellName, cellContents, false);//find spreadsheet and call set cell contents
			if(circle)//bad cell change
			{
				message = "error 1 Introduced a circular exception\n";//prepare error message
				send(client,message);//send message to cell change requester 
			}
			else 
			{
				message = "cell " + cellName + " " + cellContents + "\n";
				sendAll(client, message);//send change to all clients once change is verified
			}
		}
		else if(command == "undo\n")
		{
			if(findSS(client).canUndo()){
				string test = findSS(client).undo();
				message = "cell " + test + "\n";
				sendAll(client, message);//how to send out change to all clients
			}
		}	
		else
		{
			message = "error 2 " + msg + "\n";
			send(client, message);
		}
		// Unlock
		pthread_mutex_unlock(&serverLock);
	}
 }
 
/*
 * Server Start and Initialization - followed the guide/source (1).
 */
int main(int argc, char *argv[])
{

	// Declarations
	pthread_t userThread;
	int ssocket;
	int port;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int client;
	
	// get port, defaults to 2000 as per protocol
	if (argc < 2)
		port = 18000;
	else
		port = atoi(argv[1]);
	
	// get socket for server (ssocket)
	ssocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ssocket < 0) 
		{
		cout << "Error opening socket for server" << endl;
		exit(1);
		}

bzero((char*)&serv_addr, sizeof(serv_addr));
    port=5001;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(port);
	// Bind socket to given port/address
    if (bind(ssocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << bind(ssocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) << endl;
		exit(1);
	}
	
	ifstream stream;
	string name;
	string filename = "userList.txt";
	stream.open(filename.c_str());
	while(stream>>name)
	{
		userList.push_back(name);
	}
	stream.close();
	
	
	//Now start listening for the clients, here process will go in sleep mode
	//and will wait for the incoming connection
	cout<<"server started"<<endl;
    listen(ssocket,5);
	clilen = sizeof(cli_addr);
		
	while(1) {

		int client;
		/* Accept actual connection from the client */
		client = accept(ssocket, (struct sockaddr *) &cli_addr, &clilen);
		if (client < 0) {
			cout << "ERROR on accept" << endl;
		}		
		// send off thread for this connection
		pthread_create(&userThread, 0, receive, (void *)client);
		pthread_detach(userThread);
	}

	// Destroy resources
    close(client);
    close(ssocket);
	
    return 0; 
}
