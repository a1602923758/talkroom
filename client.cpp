#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
using namespace std;
int sock;
struct user_msg{
	char from[30];
	char to[30];
	char msg[100];
	int flag;
};
user_msg send_buf,recv_buf;



void create_sock(){
	sockaddr_in addr;
	sock=socket(AF_INET,SOCK_STREAM,0);
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	addr.sin_port=htons(6000);
	connect(sock,(sockaddr*)&addr,sizeof(addr));
}

void registe(){
	char name[30],pwd[30];
	cout<<"********用户注册********"<<endl;
	cout<<"用户名：";
	cin>>name;
	cout<<"密码：";
	cin>>pwd;
	strcpy(send_buf.from,name);
	strcpy(send_buf.to,name);
	send_buf.flag=1;
	strcpy(send_buf.msg,pwd);
	send(sock,(char*)&send_buf,sizeof(send_buf),0);
}
bool is_registe(){
	recv(sock,(char*)&recv_buf,sizeof(recv_buf),0);
	if (recv_buf.flag==1 && strcmp(recv_buf.msg,"ok")==0) {
		cout<<"注册成功..."<<endl;
		return true;
	}
	else cout<<"注册失败..."<<endl;
	return false;
}

void login(){
	char name[30],pwd[30];
	cout<<"********用户登录********"<<endl;
	cout<<"用户名：";
	cin>>name;
	cout<<"密码：";
	cin>>pwd;
	strcpy(send_buf.from,name);
	strcpy(send_buf.to,name);
	send_buf.flag=2;
	strcpy(send_buf.msg,pwd);
	send(sock,(char*)&send_buf,sizeof(send_buf),0);
}

bool is_login(){
	recv(sock,(char*)&recv_buf,sizeof(recv_buf),0);
	if (recv_buf.flag==2 && strcmp(recv_buf.msg,"ok")==0) {
		cout<<"登录成功..."<<endl;
		return true;
	}
	else cout<<"登录失败..."<<endl;
	return false;
}

void show(){
	//recv(sock,(char*)&recv_buf,sizeof(recv_buf),0);
	if (recv_buf.flag==3) cout<<recv_buf.msg<<endl;
}

//void add_multi(){
//	send_buf.flag=4;
//	send(sock,(char*)&send_buf,sizeof(send_buf),0);
//}

//void is_add(){
//	if (strcmp(recv_buf.msg,"ok")==0) {
//		cout<<"加入群聊..."<<endl;
//	}
//}
void show_online(){
	send_buf.flag=3;
	send(sock,(char*)&send_buf,sizeof(send_buf),0);
}
void multi_send(){
	send_buf.flag=6;
	while(true){
		memset(send_buf.msg,0,sizeof(send_buf.msg));
		cin>>send_buf.msg;
		send(sock,(char*)&send_buf,sizeof(send_buf),0);
		if (strcmp(send_buf.msg,"exit")==0) exit(0);
		else if (strcmp(send_buf.msg,"quit")==0) break;
	}
}
void multi_recv(){
	cout<<"【群聊】"<<recv_buf.from<<":"<<recv_buf.msg<<endl;
}
void single_user(){
	cout<<"选择私聊用户："<<endl;
	cin>>send_buf.to;
}
void single_send(){
	send_buf.flag=5;
	while(true){
		cin>>send_buf.msg;
		if (strcmp(send_buf.msg,"quit")==0) break;
		send(sock,(char*)&send_buf,sizeof(send_buf),0);
		if (strcmp(send_buf.msg,"exit")==0) exit(0);
	}
}
void single_recv(){
	cout<<"【私聊】"<<recv_buf.from<<":"<<recv_buf.msg<<endl;
}
void menu(){
	int choice;
	while(true){
		cout<<"选择模式："<<endl;
		cout<<"0) 退出"<<endl;
		cout<<"1) 私聊"<<endl;
		cout<<"2) 群聊"<<endl;
		cin>>choice;
		if (choice==0) {
			strcpy(send_buf.msg,"exit");
			send(sock,(char*)&send_buf,sizeof(send_buf),0);
			exit(0);
		}
		else if (choice==1){
			int opt;
			while(true){
				cout<<"******私聊******"<<endl;
				cout<<"0) 退出"<<endl;
				cout<<"1) 显示在线"<<endl;
				cout<<"2) 选择好友"<<endl;
				cout<<"3) 发送消息"<<endl;
				cout<<"******私聊******"<<endl;
				cin>>opt;
				if (opt==0) break;
				else if(opt==1) show_online();
				else if (opt==2) single_user();
				else if (opt==3) single_send();
				else cout<<"try again..."<<endl;
			}
		}
		else if (choice==2){
			int opt;
			while(true){
				cout<<"******群聊******"<<endl;
				cout<<"0) 退出"<<endl;
				//cout<<"1) 加入群聊"<<endl;
				cout<<"1) 发送消息"<<endl;
				cout<<"******群聊******"<<endl;
				cin>>opt;
				if (opt==0) break;
				//else if (opt==1) add_multi();
				else if (opt==1) multi_send();
				else cout<<"try again"<<endl;
			}
		}
		else cout<<"try again"<<endl;
	}
}


void* print(void* arg){
	int len;
	while(true){
		len=recv(sock,(char*)&recv_buf,sizeof(recv_buf),0);
		if (len<=0) continue;
		switch(recv_buf.flag){
			case 3:
				show();
				break;
			//case 4:
			//	is_add();
			//	break;
			case 5:
				single_recv();
				break;
			case 6:
				multi_recv();
				break;
		}
	}
}
//void* scan(void* arg){
//}
void index(){
	char c;
	cout<<"是否已注册？(Y/N)"<<endl;
	cin>>c;
	if (c=='N'){
		while(true){
			registe();
			if (is_registe()) break;
		}
	}
	else if (c!='Y') return;
	while(true){
		login();
		if (is_login()) break;
	}
	pthread_t pt1,pt2;
	pthread_create(&pt1,NULL,print,NULL);
	menu();

}

int main(){
	create_sock();
	index();
//	pthread_t pt1,pt2;
//	pthread_create(&pt1,NULL,print,NULL);
	//pthread_create(&pt2,NULL,scan,NULL);
//	pthread_join(pt1,NULL);
	//pthread_join(pt2,NULL);
	close(sock);
	return 0;
}
