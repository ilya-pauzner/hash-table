#include <vector>
#include <list>
#include <stdexcept>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {

public:
    typedef typename std::pair<const KeyType, ValueType> Record;
    typedef typename std::vector<std::list<Record> > Table;

    class iterator {
        typedef typename std::list<Record>::iterator list_iterator;
    private:
        size_t row;
        list_iterator li;
        HashMap* ref;
    public:
        iterator(size_t r = 0, list_iterator it = list_iterator(), HashMap* re = nullptr)
                : row(r), li(it), ref(re) {
        }

        bool operator==(iterator other) {
            return (row == other.row) && (li == other.li);
        }

        bool operator!=(iterator other) { return !(*this == other); }

        Record& operator*() { return *li; }

        list_iterator operator->() { return li; }

        iterator operator++() {
            ++li;
            if (li == ref->table[row].end() and row + 1 != ref->table.size()) {
                ++row;
                while (row < ref->table.size() and ref->table[row].empty()) {
                    ++row;
                }
                if (row != ref->table.size()) {
                    li = ref->table[row].begin();
                } else {
                    row = ref->table.size() - 1;
                    li = ref->table[row].end();
                }
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
    };

    class const_iterator {
        typedef typename std::list<Record>::const_iterator list_const_iterator;
    private:
        size_t row;
        list_const_iterator li;
        const HashMap* ref;
    public:
        const_iterator(size_t r = 0, list_const_iterator it = list_const_iterator(), const HashMap* re = nullptr)
                : row(r), li(it), ref(re) {
        }

        bool operator==(const_iterator other) {
            return (row == other.row) && (li == other.li);
        }

        bool operator!=(const_iterator other) { return !(*this == other); }

        const Record& operator*() { return *li; }

        list_const_iterator operator->() { return li; }

        const_iterator operator++() {
            ++li;
            if (li == ref->table[row].end() and row + 1 != ref->table.size()) {
                ++row;
                while (row < ref->table.size() and ref->table[row].empty()) {
                    ++row;
                }
                if (row != ref->table.size()) {
                    li = ref->table[row].begin();
                } else {
                    row = ref->table.size() - 1;
                    li = ref->table[row].end();
                }
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
    };

private:
    const size_t initial_size = 128;
    const size_t multiply_factor = 4;
    const double max_load_factor = 0.5;

    size_t size_ = 0;
    size_t maybe_begin = initial_size - 1;

    Table table;
    Hash hasher;

    //

    void _rehash() {
        if (size_ < initial_size or table.size() < initial_size) {
            return;
        }
        size_t new_size = 0;
        if (max_load_factor * table.size() <= size_) {
            new_size = multiply_factor * size_;
        } else {
            return;
        }
        std::list<Record> l;
        for (auto elem : *this) {
            l.push_back(elem);
        }
        for (auto elem : l) {
            _erase(elem.first, table);
        }
        table.resize(new_size);
        maybe_begin = table.size() - 1;
        for (auto elem : l) {
            _insert(elem, table);
        }
    }

    void _insert(const Record& kv, Table& t) {
        size_t pos = hasher(kv.first) & (t.size() - 1);
        size_t times = 0;
        for (auto elem : t[pos]) {
            if (elem.first == kv.first) {
                ++times;
                break;
            }
        }
        if (times == 0) {
            t[pos].push_front(kv);
            ++size_;
            maybe_begin = std::min(maybe_begin, pos);
        }
    }

    void _erase(const KeyType& k, Table& t) {
        size_t pos = hasher(k) & (t.size() - 1);
        if (t[pos].empty()) {
            return;
        }
        auto last = t[pos].begin();
        for (auto it = t[pos].begin(); it != t[pos].end(); ++it) {
            if (it->first == k) {
                last = it;
                break;
            }
        }

        if (last->first == k) {
            t[pos].erase(last);
            --size_;
        }
    }

    iterator _find(const KeyType& k, Table& t) {
        size_t pos = hasher(k) & (t.size() - 1);
        if (t[pos].empty()) {
            return end();
        }
        auto last = t[pos].begin();
        for (auto it = t[pos].begin(); it != t[pos].end(); ++it) {
            if (it->first == k) {
                last = it;
                break;
            }
        }
        if (last->first == k) {
            return iterator(pos, last, this);
        }
        return end();
    }

    const_iterator _find(const KeyType& k, const Table& t) const {
        size_t pos = hasher(k) & (t.size() - 1);
        if (t[pos].empty()) {
            return end();
        }
        auto last = t[pos].begin();
        for (auto it = t[pos].begin(); it != t[pos].end(); ++it) {
            if (it->first == k) {
                last = it;
                break;
            }
        }
        if (last->first == k) {
            return const_iterator(pos, last, this);
        }
        return end();
    }

public:

    HashMap& operator=(const HashMap& other) {
        table = Table(other.table);
        size_ = other.size_;
        hasher = other.hasher;
        return *this;
    }

    explicit HashMap(const Hash& h) : table(Table(initial_size)), size_(0), hasher(h) {
    }

    explicit HashMap() {
        table = Table(initial_size);
        size_ = 0;
    }

    template<typename ListIter>
    HashMap(ListIter first, ListIter last) {
        *this = HashMap();
        while (first != last) {
            _insert(*first, table);
            ++first;
        }
    }

    template<typename ListIter>
    HashMap(ListIter first, ListIter last, Hash h) {
        *this = HashMap();
        while (first != last) {
            _insert(*first, table);
            ++first;
        }
        hasher = h;
    }

    HashMap(std::initializer_list<Record> l) {
        *this = HashMap(l.begin(), l.end());
    }

    HashMap(std::initializer_list<Record> l, Hash h) {
        *this = HashMap(l.begin(), l.end(), h);
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    void insert(const Record& kv) {
        _insert(kv, table);
        _rehash();
    }

    void erase(const KeyType& k) {
        _erase(k, table);
        _rehash();
    }

    iterator begin() {
        size_t r = maybe_begin;
        while (r < table.size() and table[r].empty()) {
            ++r;
        }
        maybe_begin = r;
        if (r == table.size()) {
            return end();
        }
        return iterator(r, table[r].begin(), this);
    }

    const_iterator begin() const {
        int r = maybe_begin;
        while (r < table.size() and table[r].empty()) {
            ++r;
        }
        if (r == table.size()) {
            return end();
        }
        return const_iterator(r, table[r].begin(), this);
    }

    iterator end() {
        return iterator(table.size() - 1, table.back().end(), this);
    }

    const_iterator end() const {
        return const_iterator(table.size() - 1, table.back().end(), this);
    }

    iterator find(const KeyType& k) {
        return _find(k, table);
    }

    const_iterator find(const KeyType& k) const {
        return _find(k, table);
    }

    ValueType& operator[](const KeyType& k) {
        if (find(k) == end()) {
            insert({k, ValueType()});
        }
        return (*find(k)).second;
    }

    const ValueType& at(const KeyType& k) const {
        if (find(k) == end()) {
            throw std::out_of_range("No such key!");
        }
        return (*find(k)).second;
    }

    void clear() {
        *this = HashMap();
    }
};
