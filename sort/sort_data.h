#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <random>

/* sort data io, alse generate random data */
template <typename T> class SortData {
  public:
    const static size_t MAX_LEN = 0x1u << 18;

  private:
    const static size_t SEED = 1;
    std::string file_name;
    std::unique_ptr<T> uptr;
    size_t data_len;

  public:
    SortData(std::string file_name_, size_t data_len_)
        : file_name(file_name_), data_len{data_len_} {
        uptr.reset(new T[data_len_]);
    }

    std::pair<T *, size_t> get() { return {uptr.get(), data_len}; }

    void gen() {
        std::default_random_engine eng{SEED};
        std::uniform_int_distribution<unsigned> dist{0, 0xffffu};
        for (size_t i = 0; i < data_len; i++) {
            uptr.get()[i] = dist(eng);
        }
    }

    void read() {
        std::ifstream f{file_name, std::ifstream::in};
        for(size_t i = 0; i < data_len; i++) {
            f >> uptr.get()[i];
        }
        f.close();
    }

    void write() {
        std::ofstream f{file_name, std::ofstream::out};
        for (size_t i = 0; i < data_len; i++) {
            f << uptr.get()[i] << '\n';
        }
        f.close();
    }
};

template <typename T>
std::ostream &operator<<(std::ostream &os, SortData<T> &data) {
    auto [ptr, len] = data.get();
    for (size_t i = 0; i < len; i++) {
        os << ptr[i] << '\n';
    }
    return os;
}