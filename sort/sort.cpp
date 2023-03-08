#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <cstring>

#include "sort_data.h"

/* common interface for time measurement*/
template <typename T> class Sort {
  public:
    virtual ~Sort() {};
    virtual void sort(T *arr, size_t len) = 0;
    size_t measure_time_sort(T *arr, size_t len) {
        typedef std::chrono::high_resolution_clock clk;
        auto start = clk::now();
        sort(arr, len);
        auto end = clk::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
};

template <typename T> void swap(T &a, T &b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T> class QuickSort : public Sort<T> {
  public:
    void sort(T *arr, size_t len) override {
        if (len <= 1) {
            return;
        }
        size_t l = 0, r = 0;
        T pivot = arr[len - 1];
        while (r < len - 1) {
            if (arr[r] < pivot) {
                swap(arr[r], arr[l]);
                l++;
            }
            r++;
        }
        swap(arr[r], arr[l]);
        sort(&arr[l + 1], r - l);
        sort(arr, l);
    }
};

template <typename T> class HeapSort : public Sort<T> {
  public:
    void sort(T *arr, size_t len) override {
        build_heap(arr, len, 0);
        heap_sort(arr, len);
    }

    /* max-heap */
    void heapify(T* arr, size_t len, size_t root) {
        size_t max, l = root * 2 + 1, r = l + 1;
        max = root;
        if(l < len && arr[l] > arr[max]) {
            max = l;
        }
        if(r < len && arr[r] > arr[max]) {
            max = r;
        }
        if(max != root) {
            swap(arr[max], arr[root]);
            heapify(arr, len, max);
        }
    }

    void build_heap(T* arr, size_t len, size_t root) {
        for(int i = len / 2 - 1; i >= 0; i--) {
            heapify(arr, len, i);
        }
    }

    void heap_sort(T *arr, size_t len) {
        while(len > 1) {
            swap(arr[0], arr[len - 1]);
            len--;
            heapify(arr, len, 0);
        }
    }
};

template <typename T> class CountingSort : public Sort<T> {
  private:
    /* range of data */
    const static size_t MAX = 0xffffu;
  public:
    void sort(T *arr, size_t len) override {
        T *cnt = new T[MAX + 1]{0};
        for(size_t i = 0; i < len; i++) {
            cnt[arr[i]]++;
        }
        for(size_t i = 1; i < MAX + 1; i++) {
            cnt[i] += cnt[i - 1];
        }
        T *tmp = new T[len];
        /* stable */
        for(int i = len - 1; i >= 0; i--) {
            tmp[cnt[arr[i]] - 1] = arr[i];
            cnt[arr[i]]--;
        }
        std::memcpy(arr, tmp, len * sizeof(T));
        delete[] cnt;
        delete[] tmp;
    }
};

template <typename T> class MergeSort : public Sort<T> {
  public:
    void sort(T *arr, size_t len) override {
        if(len <= 1) {
            return;
        }
        size_t mid = len / 2;
        sort(arr, mid);
        sort(&arr[mid], len - mid);
        /* merge */
        size_t l = 0, r = mid;
        T *tmp = new T[len];
        size_t idx = 0;
        while(l < mid and r < len) {
            if(arr[l] < arr[r]) {
                tmp[idx] = arr[l];
                l++;
            } else {
                tmp[idx] = arr[r];
                r++;
            }
            idx++;
        }
        while(l < mid) {
            tmp[idx] = arr[l];
            l++;
            idx++;
        }
        while(r < len) {
            tmp[idx] = arr[r];
            r++;
            idx++;
        }
        std::memcpy(arr, tmp, len * sizeof(T));
        delete[] tmp;
    }
};

int main() {
    using namespace std;

    string input = "../input/input.txt";
    vector<size_t> sort_lens{1 << 3, 1 << 6, 1 << 9, 1 << 12, 1 << 15, 1 << 18};
    vector<pair<string, Sort<unsigned> *>> sort_algos{
        {"../output/quick_sort", new QuickSort<unsigned>},
        {"../output/heap_sort", new HeapSort<unsigned>},
        {"../output/merge_sort", new MergeSort<unsigned>},
        {"../output/counting_sort", new CountingSort<unsigned>}
    };
    auto in = SortData<unsigned>{input, SortData<unsigned>::MAX_LEN};
    in.read();
    for (auto &&algo : sort_algos) {
        ofstream time_file{algo.first + "/time.txt", ofstream::out};
        for (auto len : sort_lens) {
            auto out = SortData<unsigned>{
                algo.first + "/result_" + to_string(static_cast<unsigned>(log2(len))) + ".txt",
                len};
            memcpy(out.get().first, in.get().first, len * sizeof(unsigned));
            auto t = algo.second->measure_time_sort(out.get().first, len);
            time_file << t << '\n';
            out.write();
        }
        time_file << "(ns)\n";
        time_file.close();
        delete algo.second;
    }
}