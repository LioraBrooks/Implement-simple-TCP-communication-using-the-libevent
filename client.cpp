#include<iostream>
#include<string.h>
#include<unistd.h>
#include<event2/bufferevent.h>
#include<fcntl.h>
#include<event2/event.h>
#include<arpa/inet.h>
#define SERV_PORT 8000
using namespace std;

//读回调
void read_cb(struct bufferevent *bev,void *cbarg)
{
    char buf[1024]={0};

    int n=bufferevent_read(bev,buf,sizeof(buf)-1);

    if(n>0)
    {
        buf[n]='\0';    //确保字符串结束
        cout<<"server send:"<<buf<<endl;
    }

    //不自动回复
}

//写回调
void write_cb(struct bufferevent *bev,void *cbarg)
{
    cout<<"成功写数据给server"<<endl;

}

//事件回调
void event_cb(struct bufferevent *bev,short events,void *cbarg)
{
    if(events & BEV_EVENT_EOF)
    {
        cout<<"connection closed"<<endl;
    }
    else if(events & BEV_EVENT_ERROR)
    {
        cout<<"other error"<<endl;
    }
    else if(events & BEV_EVENT_CONNECTED)
    {
        cout<<"connected successfully!"<<endl;
        return;
    }

    bufferevent_free(bev);
    cout<<"bufferevent 资源已被释放"<<endl;

}

//客户端和用户交互，从终端读取数据写给客户端
void read_terminal(evutil_socket_t fd,short what,void *arg)
{
    char buf[1024]={0};

    int len=read(STDIN_FILENO,buf,sizeof(buf));

    struct bufferevent *bev=(struct bufferevent *)arg;

    //向server发送数据
    bufferevent_write(bev,buf,len+1);
}

int main()
{
    struct sockaddr_in serv_addr={};
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr.s_addr);

    int fd=socket(AF_INET,SOCK_STREAM,0);

    //创建event_base
    struct event_base *base=event_base_new();

    //创建bufferevent对象,将fd放到bufferevent上
    struct bufferevent *bev;
    bev=bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);

    //连接到服务器
    bufferevent_socket_connect(bev,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    
    //设置回调
    bufferevent_setcb(bev,read_cb,write_cb,event_cb,NULL);

    bufferevent_enable(bev,EV_READ);
    
    //创建事件
    struct event* ev=event_new(base,STDIN_FILENO,EV_READ|EV_PERSIST,read_terminal,bev);

    //添加事件
    event_add(ev,NULL);

    //启动循环监听
    event_base_dispatch(base);

    //释放资源
    event_free(ev);

    event_base_free(base);

    return 0;
}
