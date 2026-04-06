#include<iostream>
#include<string.h>
#include<unistd.h>
#include<event2/bufferevent.h>
#include<event2/listener.h>
#include<fcntl.h>
#define SERV_PORT 8000
using namespace std;

//读回调
void read_cb(struct bufferevent *bev,void *cbarg)
{
    char buf[1024];

    bufferevent_read(bev,buf,sizeof(buf));

    cout<<"client send:"<<buf<<endl;

    const char *p="I am server,I have already received your message.";

    //写数据给客户端
    bufferevent_write(bev,p,strlen(p)+1);

    sleep(1);

}

//写回调
void write_cb(struct bufferevent *bev,void *cbarg)
{
    cout<<"成功写数据给客户端！"<<endl;

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
    bufferevent_free(bev);
    cout<<"bufferevent 资源已被释放"<<endl;

}

//监听回调
void listener_cb(struct evconnlistener *listener,evutil_socket_t sockfd,struct sockaddr *addr,int len,void *ptr)
{
    cout<<"connect new client."<<endl;
    
    struct event_base *base=(struct event_base *)ptr;

    //创建bufferevent对象
    struct bufferevent *bev;
    bev=bufferevent_socket_new(base,sockfd,BEV_OPT_CLOSE_ON_FREE);

    //给bufferevent缓冲区设置回调
    bufferevent_setcb(bev,read_cb,write_cb,event_cb,NULL);

    //启用bufferevent读缓冲
    bufferevent_enable(bev,EV_READ);

    return ;

}

int main()
{
    struct sockaddr_in serv_addr={};
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    //创建event_base
    struct event_base *base=event_base_new();

    //创建监听器
    struct evconnlistener *listener;
    listener=evconnlistener_new_bind(base,listener_cb,base,LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,-1,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

    //启动循环监听
    event_base_dispatch(base);

    //释放资源
    evconnlistener_free(listener);

    event_base_free(base);

    return 0;
}
