#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

const int SIZE = 1<<30; //1GB

class CowBoy {
public:
    CowBoy(size_t size_, std::string name_ = "") {
        if (name_.empty()) {
            name_ = "random"; // TODO
        }
        this->size = size_;
        this->name = name_;
        fd = shm_open(name.c_str(), O_RDWR | O_CREAT, 0666);
        ftruncate(fd, SIZE);
        buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        CBUF = (char*)buf;
    }
    virtual ~CowBoy() {
        munmap(buf, size);
        shm_unlink(name.c_str());
    }

    void* getMemory() {
        return buf;
    }

    operator void*() {
        return buf;
    }

private:
    int fd;
    size_t size;
    std::string name;
    void *buf;
    char *CBUF;

    friend class Cow;
};

class Cow {
public :
    Cow(CowBoy& master_) {
        this->master = &master_;
        this->buf = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, master->fd, 0);
        CBUF = (char*)buf;
    }
    Cow(Cow&& other) :buf(other.buf), CBUF(other.CBUF), master(other.master)
    {
        other.buf = other.CBUF = NULL;
    }

    virtual ~Cow() {
        if (buf)
            munmap(buf, master->size);
    }

    void* getMemory() {
        return buf;
    }

    operator void*() {
        return buf;
    }

private:
    void *buf;
    char *CBUF;
    const CowBoy* master;
};


int main(void) {
    /*
    int fd = shm_open("/tmpmem", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, SIZE);
    char *buf = (char*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(buf, '1', SIZE);

    char *buf2 = (char*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    char *buf3 = (char*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    char *buf4 = (char*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    char *bufarr[500];
    for (int i=0; i<500; ++i) {
        bufarr[i] = (char*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    }
    for (uint i=0; i<500; i++) {
        bufarr[i][0] = 'A' + (i%20);
    }
    buf2[0] = '2';
    buf3[0] = '3';
    buf4[0] = '4';
    memset (buf2, '2', 1<<20);
    memset(buf4+5, '4', SIZE/2);

    munmap(buf4,SIZE);


    shm_unlink("/tmpmem");
    */

    const int COW_WEIGHT = 1<<30;

    std::cout<<"creating the Master Bull" <<std::endl;
    CowBoy master(COW_WEIGHT, "test");
    memset(master, '1', 1<<30);

    std::cout<<"creating pasture for army of cloned cows" <<std::endl;
    const uint HERD_SIZE = 500;
    std::vector<Cow> pasture;
    pasture.reserve(HERD_SIZE);

    std::cout<<"cloning the Master Bull" <<std::endl;
    for (uint i=0; i<HERD_SIZE; i++) {
        pasture.push_back(Cow(master));
    }

    std::cout<<"branding clones" <<std::endl;
    for (uint i=0; i<HERD_SIZE; i++) {
        memset(pasture[i], 'A' + (i%20), 1024);
    }

    std::cout << "replacing 3 clones by new, unique individuals" <<std::endl;
    for (int i=0; i<3; i++) {
        memset(pasture[i], 'x', COW_WEIGHT);
    }


    return EXIT_SUCCESS;
}
