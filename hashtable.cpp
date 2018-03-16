#include <vector>
#include <list>
#include <stdexcept>
#include <iostream>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {

public:
    typedef typename std::pair<const KeyType, ValueType> Record;
    typedef typename std::vector<std::list<Record> > Table;

    class ListIter {
        typedef typename std::list<Record>::ListIter List_Iter;
    private:
        size_t row;
        List_Iter li;
        HashMap *ref;
    public:
        ListIter(size_t r = 0, List_Iter it = List_Iter(), HashMap *re = nullptr)
                : row(r), li(it), ref(re) {
        }

        bool operator==(ListIter other) {
            return (row == other.row) && (li == other.li);
        }

        bool operator!=(ListIter other) { return !(*this == other); }

        Record &operator*() { return *li; }

        List_Iter operator->() { return li; }

        ListIter operator++() {
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

        ListIter operator++(int) {
            ListIter tmp = *this;
            ++(*this);
            return tmp;
        }
    };

    class ConstListIter {
        typedef typename std::list<Record>::ConstListIter Const_List_Iter;
    private:
        size_t row;
        Const_List_Iter li;
        const HashMap *ref;
    public:
        ConstListIter(size_t r = 0, Const_List_Iter it = Const_List_Iter(), const HashMap *re = nullptr)
                : row(r), li(it), ref(re) {
        }

        bool operator==(ConstListIter other) {
            return (row == other.row) && (li == other.li);
        }

        bool operator!=(ConstListIter other) { return !(*this == other); }

        const Record &operator*() { return *li; }

        Const_List_Iter operator->() { return li; }

        ConstListIter operator++() {
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

        ConstListIter operator++(int) {
            ConstListIter tmp = *this;
            ++(*this);
            return tmp;
        }
    };

private:
	const int initial_size = 128;
	const int multiply_factor = 4;
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

    void _insert(const Record& kv, Table &t) {
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

    void _erase(const KeyType& k, Table &t) {
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

    ListIter _find(const KeyType& k, Table &t) {
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
            return ListIter(pos, last, this);
        }
        return end();
    }

    ConstListIter _find(const KeyType& k, const Table &t) const {
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
            return ConstListIter(pos, last, this);
        }
        return end();
    }

public:

    HashMap &operator=(const HashMap &other) {
        table = Table(other.table);
        size_ = other.size_;
        hasher = other.hasher;
        return *this;
    }

    explicit HashMap(const Hash &h) : table(Table(initial_size)), size_(0), hasher(h) {
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

    ListIter begin() {
        size_t r = maybe_begin;
        while (r < table.size() and table[r].empty()) {
            ++r;
        }
		maybe_begin = r;
        if (r == table.size()) {
            return end();
        }
        return ListIter(r, table[r].begin(), this);
    }

    ConstListIter begin() const {
        int r = maybe_begin;
        while (r < table.size() and table[r].empty()) {
            ++r;
        }
        if (r == table.size()) {
            return end();
        }
        return ConstListIter(r, table[r].begin(), this);
    }

    ListIter end() {
        return ListIter(table.size() - 1, table.back().end(), this);
    }

    ConstListIter end() const {
        return ConstListIter(table.size() - 1, table.back().end(), this);
    }

    ListIter find(const KeyType& k) {
        return _find(k, table);
    }

    ConstListIter find(const KeyType& k) const {
        return _find(k, table);
    }

    ValueType &operator[](const KeyType& k) {
        if (find(k) == end()) {
            insert({k, ValueType()});
        }
        return (*find(k)).second;
    }

    const ValueType &at(const KeyType& k) const {
        if (find(k) == end()) {
            throw std::out_of_range("No such key!");
        }
        return (*find(k)).second;
    }

    void clear() {
        *this = HashMap();
    }
};
