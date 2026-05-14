# 一. 信号量与P、V源语

## 1.1 信号量

* **核心概念**：信号量本质上可以理解为 Linux 维护的一个整数计数器，用来进行执行流之间的互斥、同步
* **生命周期**：信号量的生命周期随内核，因此在代码中使用后一定要释放。使用 `ipcs -s` 查看当前系统中所有信号量集，使用 `ipcrm -s` 通过信号量集 id 删除信号量集
* **信号量集**：SystemV 对于信号量的管理以集合信号量集为单位，一个信号量集中包含多个信号量
* **权限与键值**：通过 `key_t` 生成一个 `semid`，表示一个信号量集；在创建时制定权限，内核后续进行校验

## 1.2 P、V操作

* **P操作**，表示申请，可以理解为让一个信号量值 `--`，表示资源被申请走了一份
* **V操作**，表示释放，可以理解为让一个信号量值 `++`，表示资源被释放了一份
* P、V操作都是原子的，

## 1.3 信号量值的含义

* `S>0:`，表示就绪的资源的个数
* `S=0:`，表示没有可以使用的资源

# 二、信号量系统调用接口

## 2.1 semget

```cpp
#include <sys/sem.h>
 int semget(key_t key, int nsems, int semflg)
```

* 这里的 `key`，是信号量集的键值，同消息队列和共享内存。`key` 可以通过 `ftok` 获得，函数原型如下

	```cpp
	 #include <sys/ipc.h>
	 key_t ftok(const char *path, int proj_id);
	```

* `nsems`，表示创建的信号量集中信号量的个数
* `semflg`，表示信号量集权限，比如 `IPC_CREAT`，表示如果不存在该信号集则创建；`IPC_EXCL`，表示创建信号量时检查是否存在，如果存在则报错返回（单独传这个操作是无意义的）；最后是可以传入类似于文件系统中权限掩码的八进制权限

## 2.2 semctl

对信号量集的管理操作，比如初始化、删除信号量（同 `ipcrm -s`）

```cpp
#include <sys/sem.h>
 int semctl(int semid, int semnum, int cmd, ...);
```

* `semid`，是信号量集的标识符，即前面 `semget` 返回的内容
* `semnum`，类似于数组下标的作用，表示是对信号量集中的哪个信号量进行的操作
* `cmd`，表示具体对信号量做哪种操作，常用的操作有 `IPC_RMID`，表示当前希望删除信号量（与之配对的 semnum 传 0 即可）；`SETVAL`，对信号量集中 `semnum` 制定的信号量进行初始化，怎样初始化，由第四个参数决定。
* 第四个参数要求必须是 `union semun` 类型，定义如下
	```cpp
	union semun {
		 int val;
		 struct semid_ds *buf;
		 unsigned short  *array;
	 };
	```
	如果需要把信号集中的某一个信号量设置为具体的值时，就可以使用 `union semun` 的 `val` 来指定信号量初始值

## 2.3 semop

对指定的信号量进行P、V操作，函数声明如下

```cpp
int semop(int semid, struct sembuf *sops, size_t nsops);
```

* `sops` 是 `struct sembuf` 数组，表示要对那些信号量进行怎样的操作
* `nsops`，表示 `sops` 的长度
	* 关于 `struct sembuf`:
		```cpp
		struct sembuf sb;
		sb.sem_num = index;
		sb.sem_op = 1;
		sb.sem_flg = SEM_UNDO;
		```
		其中，`sem_num` 字段，用来指明对哪个信号量集进行操作，就像 `semctl` 中的 `sem_num`；`sem_op` 字段，表示P、V操作，正数表示信号量值 `+` 的V操作，负数表示信号量值 `-` 的P操作

# 三、基于建造者模式的 SystemV 信号量

## 3.1 建造者模式

建造者模式由三方（把抽象类算上就是四方）组成，包括

1. 产品类，最后被建造出来的目标
2. 抽象的建造接口类，实现这些接口的具体建造者类
3. 指挥者，负责调度建造者类中实现的若干接口

建造者模式在以下场景最为适合

1. 构造流程复杂，属性多，容易错乱
2. 想要一套构造流程构造出多种对象
3. 希望构造过程和产品本身解耦

## 3.2 信号量封装

### 3.2.1 产品类

信号量最核心的操作就是P、V操作，这里把产品接口和构造区分开来

```cpp
class Semaphore{
public:
	Semaphore(int semid, int flag)
	{
		_semid = semid;
		_flag = flag;
	}

	void P(int who) {
		struct sembuf sb;
		sb.sem_num = who;
		sb.sem_op = -1;
		sb.sem_flg = SEM_UNDO;
		int ret = semop(_semid, &sb, 1);
		if(ret == -1){
			perror("semop_P fail");
			return;
		}
	}
	void V(int who) {
		struct sembuf sb;
		sb.sem_num = who;
		sb.sem_op = 1;
		sb.sem_flg = SEM_UNDO;
		int ret = semop(_semid, &sb, 1);
		if(ret == -1){
			perror("semop_V fail");
			return;
		}
	}
	~Semaphore()
	{
		if(_flag == GET_FLG){
			return;
		}
		int ret = semctl(_semid, 0, IPC_RMID);
		if(ret == -1){
			perror("semctl_rm fail");
		}
	}
private:
	int _semid;
	int _flag;
};
```

### 3.2.2 抽象建造者和具体建造者

负责完成上面提到的 `semget`，`semctl` 进行的信号量初始化等

```cpp
class SemaphoreBuilderInterface{
public:
	virtual void GetKey() = 0;
	virtual void GetFlag(int flag) = 0;
	virtual void GetVals(const std::vector<int>& vals) = 0;
	virtual std::shared_ptr<Semaphore> GetSem() = 0;
	private:
	virtual std::shared_ptr<Semaphore> Init() = 0;
};

class SemaphoreBuilder: public SemaphoreBuilderInterface{
	const char *path = "/tmp";
	const int proj_id = 0x77;
	public:
	void GetKey() override{
		key_t k = ::ftok(path, proj_id); // 创建键值对
		if (k == -1){
			perror("ftok fail");
		}
		_k = k;
	}
	
	void GetFlag(int flag) override{
		_flag = flag;
	}
  
	void GetVals(const std::vector<int>& vals) override{
		if(vals.size() <= 0 && _flag == BUILD_FLG){
			return;
		}
			_vals = vals;
		}
		std::shared_ptr<Semaphore> GetSem() override{
		int semid = semget(_k, _vals.size(), _flag); // 获取信号量
		if (semid == -1)
		{
			perror("semget fail");
		}
		_semid = semid;
		return Init();
	}
private:
	int _k;
	int _flag;
	std::vector<int> _vals;
	int _semid;
	
	std::shared_ptr<Semaphore> Init() override{
		if (_flag == BUILD_FLG){
			for(int i = 0; i < _vals.size(); i++){
				union semun{
					int val;
					struct semid_ds *buf;
					unsigned short *array;
				} un;
				un.val = _vals[i];
				int ret = semctl(_semid, i, SETVAL, un);
				if(ret < 0){
					perror("semctl init fail");
				}
			}
		}
		return std::make_shared<Semaphore>(_semid, _flag);
	}
};
```

### 3.2.3 指挥者类

```cpp
class Director{
public:
	void Direct(SemaphoreBuilder& sb, int flag = GET_FLG, const std::vector<int>& vals = {}){
		std::cout << "Director working..." << std::endl;
		sb.GetKey();
		sb.GetFlag(flag);
		sb.GetVals(vals);
		sb.GetSem();
	}
private:
};
```