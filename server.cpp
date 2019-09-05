#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <map>
using namespace std;
const int max_events=50;
int user_num=0;
map<string,int> user_online,user_multi;
map<string,string> user_info;
int sock;
int epfd;
epoll_event pevents[max_events];
epoll_event event;

// 数据包
struct user_msg{
	char from[30]; //源端
	char to[30]; //目的端
	char msg[100]; //消息
	int flag; // 种类
};
user_msg send_buf;

void create_server(){
	sockaddr_in server;
	sock=socket(AF_INET,SOCK_STREAM,0);
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
	server.sin_port=htons(6000);
	bind(sock,(sockaddr*)&server,sizeof(server));
	listen(sock,50);
}
void create_epoll(){
	epfd=epoll_create(max_events);
	event.events=EPOLLIN;
	event.data.fd=sock;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&event);
}
void create_user(int client,user_msg buf){
	if (user_info.count(string(buf.from))==1) strcpy(buf.msg,"user exist");
	else{
		user_info[string(buf.from)]=string(buf.msg);
		strcpy(buf.msg,"ok");
	}
	send(client,(char*)&buf,sizeof(buf),0);
}
void login(int client,user_msg buf){
	if (user_info.count(string(buf.from))==1 && user_info[string(buf.from)]==string(buf.msg)){
		strcpy(buf.msg,"ok");
		user_online[string(buf.from)]=client;
		user_multi[string(buf.from)]=client;
		user_num++;
	}
	else strcpy(buf.msg,"no user");
	send(client,(char*)&buf,sizeof(buf),0);
}


void multi_send(int client,user_msg buf){
	if (strcmp(buf.msg,"quit")==0){
		cout<<client<<" quit multi..."<<endl;
		for (auto iter=user_multi.begin();iter!=user_multi.end();++iter){
			if (iter->second==client && iter->first==string(buf.from)){
				user_multi.erase(iter);
				break;
			}
		}
		return;
	}
	for (auto iter=user_multi.begin();iter!=user_multi.end();++iter){
		if (iter->first==string(buf.from) && iter->second==client) continue;
		send(iter->second,(char*)&buf,sizeof(buf),0);
	}
}

void single_send(user_msg buf){
	send(user_online[string(buf.to)],(char*)&buf,sizeof(buf),0);
}

//void add_multi(int client,user_msg buf){
//	if (user_multi.count(string(buf.from))==1) strcpy(buf.msg,"user exist");
//	else{
//		user_multi[string(buf.from)]=client;
//		strcpy(buf.msg,"ok");
//	}
//	send(client,(char*)&buf,sizeof(buf),0);
//	cout<<buf.from<<"加入群聊"<<endl;
//}
void show_online(int client,user_msg buf){
	string temp;
	for (auto iter=user_online.begin();iter!=user_online.end();++iter){
		temp+=iter->first+",";
	}
	strcpy(buf.msg,temp.c_str());
	send(client,(char*)&buf,sizeof(buf),0);
}

int main(){
	create_server();
	create_epoll();

	sockaddr_in client_addr;
	socklen_t addr_size;
	while(true){
		int res=epoll_wait(epfd,pevents,max_events,-1);
		if (res<=0) continue;
		for (int i=0;i<res;++i){
			if (pevents[i].data.fd==sock){
				memset(&client_addr,0,sizeof(client_addr));
				addr_size=sizeof(client_addr);
				int client=accept(sock,(sockaddr*)&client_addr,&addr_size);
				event.events=EPOLLIN;
				event.data.fd=client;
				epoll_ctl(epfd,EPOLL_CTL_ADD,client,&event);
				cout<<client<<" connect success..."<<endl;
			}
			else if(pevents[i].events & EPOLLIN){
				user_msg buf;
				int client=pevents[i].data.fd;
				int len=recv(client,(char*)&buf,sizeof(buf),0);
				if (strcmp(buf.msg,"exit")==0){
					epoll_ctl(epfd,EPOLL_CTL_DEL,client,NULL);
					cout<<client<<" exit..."<<endl;
					for (auto iter=user_multi.begin();iter!=user_multi.end();++iter){
						if (iter->first==buf.from && iter->second==client){
							user_multi.erase(iter);
							break;
						}
					}
					for (auto iter=user_online.begin();iter!=user_online.end();++iter){
						if (iter->first==buf.from && iter->second==client){
							user_online.erase(iter);
							break;
						}
					}
					user_num--;
					close(client);
					continue;
				}
				switch(buf.flag){
					case 1:
						create_user(client,buf);
						break;
					case 2:
						login(client,buf);
						break;
					case 3:
						show_online(client,buf);
						break;
					//case 4:
					//	add_multi(client,buf);
					case 5:
						single_send(buf);
						break;
					case 6:
						multi_send(client,buf);
						break;

				}
			}
		
		}

	}
	close(epfd);
	close(sock);
	return 0;
}
