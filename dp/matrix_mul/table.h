#include <ranges>
#include <cassert>
#include <memory>
#include <cstddef>
#include <cstring>

using namespace std;

template <typename T> class Table {
  private:
    unique_ptr<T> table;
    size_t width, height;

  public:
    Table(size_t width_, size_t height_) : table{new T[width_ * height_]}, width{width_}, height(height_) {
        memset(table.get(), 0, width_ * height_ * sizeof(T));
    }
    Table(Table &t) {
        table.swap(t.table);
        width = t.width;
        height = t.height;
    }
    T &at(size_t x, size_t y) {
        assert(x < width and y < height);
        return table.get()[x * width + y];
    }

    size_t get_width() const { return width; }
    size_t get_height() const { return height; }
};