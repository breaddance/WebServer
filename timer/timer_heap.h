#ifndef MIN_HEAP
#define MIN_HEAP


#include<time.h>
#include "../log/log.h"
using std::exception;

class heap_timer;

struct client_data
{
    struct sockaddr_in address;
    int sockfd;
    heap_timer *timer;
};

//定时器类
class heap_timer
{
public :
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }
public :
    time_t expire;
    void (*cb_func)(client_data*);
    client_data* user_data;
};

//时间堆
class time_heap
{
public :
    time_heap(int cap) throw (std::exception):capacity(cap),cur_size(0)
    {
        array = new heap_timer*[capacity];
        if(!array)
        {
            throw std::exception();
        }
        for(int i = 0;i < capacity; ++i)
        {
            array[i] = NULL;
        }
    }
    //构造函数之二，用已有数组来初始化堆
    time_heap(heap_timer** init_array,int size,int capacity) throw
    (std::exception):cur_size(size),capacity(capacity)
    {
        if(capacity < size)
        {
            throw std::exception();
        }
        
        array = new heap_timer* [capacity];//创建堆数组
        if(!array)
        {
            throw std::exception();
        }
        for(int i = 0; i < capacity; ++i)
        {
            array[i] = NULL;
        }
        if(size != 0)
        {
            //初始化对数组
            for(int i = 0; i < size; ++i)
            {
                array[i] = init_array[i];
            }
           
           for(int i = (cur_size - 1)/2;i >= 0;--i)
           {
               percolate_down(i);
           }
           
        }
    }
    ~time_heap()
    {
        for(int i = 0;i < cur_size;++i)
        {
            delete array[i];
        }

        delete [] array;
    }

public :
    //添加目标定时器
    void add_timer(heap_timer* timer) throw (std::exception)
    {
        if(!timer)
        {
            return;
        }
        if(cur_size >= capacity)
        {
            resize();
        }
        //新插入一个元素当前堆的大小加1,hole是建立空穴的位置
        int hole = cur_size++;
        int parent = 0;
        //对从空穴到根节点的路径上的所有节执行上虑操作
        for(;hole > 0;hole = parent)
        {
            parent = (hole -1) / 2;
            if(array[parent]->expire <= timer->expire)
            {
                break;
            }
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }

    //删除目标定时器
    void del_timer(heap_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        //仅仅将定时器的目标函数设置为空，即所谓的延迟销毁
        //这将节省删除定时器造成的开销，但是这样做容易使对数组膨胀
        timer->cb_func = NULL;
    }
    bool empty() const {return cur_size == 0;}
    heap_timer* top() const
    {
        if(empty())
        {
            return NULL;
        }

        return array[0];
    }
    void pop_timer()
    {
        if(empty())
        {
            return;
        }
        if(array[0])
        {
            delete array[0];
            array[0] = array[--cur_size];
            percolate_down(0);
        }
    }
    void tick()
    {
        LOG_INFO("%s", "timer tick");
        Log::get_instance()->flush();
        heap_timer* tmp = array[0];
        time_t cur = time(NULL);//记录当前的时间
        while(!empty())
        {
            if(!tmp)
            {
                break;
            }
            //如果顶部定时器没有到期则推出循环
            if(tmp->expire > cur)
            {
                break;
            }
            if(array[0]->cb_func)
            {
                array[0]->cb_func(array[0]->user_data);
            }
            pop_timer();
            tmp = array[0];
        }
    }
    void adjust_time(heap_timer * timer)
    {
        if(timer)
        {
            return ;
        }
        for(int i = 0;i<cur_size;i++)
        {
            if(array[i] == timer)
            {
                percolate_down(i);
                break;
            }else
            {
                continue;
            }
        }
    }

private :
    //最小堆的下滤操作
    void percolate_down(int hole)
    {
        heap_timer* temp = array[hole];
        int child = 0;
        for(;((hole*2+1) <= (cur_size-1));hole = child)
        {
            child = hole * 2;
            if((child < (cur_size-1))&&(array[child + 1]->expire < array[child]->expire))
            {
                ++child;
            }
            if(array[child]->expire < temp->expire)
            {
                array[hole]=array[child];
            }
            else
            {
                break;
            }
        }

        array[hole] = temp;
    }

private :
    //将堆数组容量扩大一倍
    void resize() throw(std::exception)
    {
        heap_timer** temp = new heap_timer* [2*capacity];
        for(int i = 0;i < 2*capacity;++ i)
        {
            temp[i] = NULL;
        }

        if(!temp)
        {
            throw std::exception();
        }
        capacity = 2 * capacity;
        for(int i = 0;i<cur_size; ++i)
        {
            temp[i] = array[i];
        }
        
        delete [] array;
        array = temp;
    }


private : 
    heap_timer** array;
    int capacity;
    int cur_size;
};
#endif //min_heap.h