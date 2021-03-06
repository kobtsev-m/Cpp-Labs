#include <iostream>
#include <cstring>
#include <cmath>

#include "rna.h"


// Конструктор по кол-ву нуклеотидов

RNA::RNA(Nucl nucl, int_t quantity) {
    if (quantity > 0) {
        chain = new byte[arr_len(quantity)];
        fill(nucl, quantity);
    }
    else {
        RNA();
    }
}

// Конструктор из строки

RNA::RNA(std::string nucls_str) : capacity(0), size(0) {
    if (nucls_str.length()) {
        for (int_t i=0; i < nucls_str.length(); ++i) {
            add(charToNucl(nucls_str[i]));
        }
    }
    else {
        RNA();
    }
}

// Конструктор копирования

RNA::RNA(const RNA &other) {
    if (this == &other) {
        return;
    }
    this->capacity = other.capacity;
    this->size = other.size;
    this->chain = new byte[arr_len(other.capacity)];
    memcpy(this->chain, other.chain, sizeof(byte)*arr_len(other.capacity));
}

// Деконструктор

RNA::~RNA() {
    if (chain != nullptr) {
        delete[] chain;
        chain = nullptr;
    }
}

// Приватные методы

short_t RNA::get(int_t index) const {
    if (0 <= index && index < size) {
        byte block = chain[index/4];
        block >>= 2*(3 - index%4);
        return block & 0x03;
    }
    return -1;
}

void RNA::fill(Nucl nucl, int_t quantity) {

    byte block = 0;

    for (int_t i=0, j=0; i < quantity; ++i) {

        block |= nucl << 2*(3 - i%4);

        if (i % 4 == 3 || i == quantity-1) {
            chain[j++] = block;
            block = 0;
        }
    }

    size = quantity;
    capacity = quantity;
}

int_t RNA::arr_len(double quantity) const {
    return ceil(quantity/4);
}

// Публичные методы

void RNA::add(Nucl nucl) {

    // Выделение или довыделение памяти
    if (size == capacity) {
        if (capacity) {
            capacity *= 2;
            byte *tmp_chain = new byte[arr_len(capacity)];
            memcpy(tmp_chain, chain, sizeof(byte)*arr_len(capacity/2));
            memset(&tmp_chain[arr_len(capacity/2)], 0, sizeof(byte)*arr_len(capacity/2));
            delete[] chain;
            chain = tmp_chain;
        }
        else {
            capacity = 32;
            chain = new byte[arr_len(capacity)];
            memset(chain, 0, sizeof(byte)*arr_len(capacity));
        }
    }

    chain[size/4] |= nucl << 2*(3 - size%4);
    size++;
}

void RNA::addPlus(Nucl nucl) {

    // Выделение или довыделение памяти
    if (size == capacity) {
        if (capacity) {
            capacity += 32;
            byte *tmp_chain = new byte[arr_len(capacity)];
            memcpy(tmp_chain, chain, sizeof(byte)*arr_len(capacity-32));
            memset(&tmp_chain[arr_len(capacity-32)], 0, sizeof(byte)*8);
            delete[] chain;
            chain = tmp_chain;
        }
        else {
            capacity = 32;
            chain = new byte[arr_len(capacity)];
            memset(chain, 0, sizeof(byte)*arr_len(capacity));
        }
    }

    chain[size/4] |= nucl << 2*(3 - size%4);
    size++;
}

int_t RNA::getCapacity() {
    return arr_len(capacity);
}

char RNA::nuclToChar(short_t nucl) {
    switch (nucl) {
        case 0: return 'A';
        case 1: return 'G';
        case 2: return 'C';
        default: return 'T';
    }
}

Nucl RNA::charToNucl(char symb) {
    switch (symb) {
        case 'A': return A;
        case 'G': return G;
        case 'C': return C;
        default: return T;
    }
}

char* RNA::getString() {

    char *rna_string = new char[size];
    memset(rna_string, 0, sizeof(char)*(size+1));

    for (int_t i=0; i < size; ++i) {
        rna_string[i] = nuclToChar(get(i));
    }
    return rna_string;
}

// Перегрузка операторов

RNA& RNA::operator = (const RNA &other) {

    if (this == &other) {
        return *this;
    }

    this->capacity = other.capacity;
    this->size = other.size;

    delete[] this->chain;
    this->chain = new byte[arr_len(other.capacity)];
    memcpy(this->chain, other.chain, sizeof(byte)*arr_len(other.capacity));

    return *this;
}

bool RNA::operator == (const RNA &other) const {
    if (this->size != other.size) {
        return false;
    }
    for (int_t i=0; i < arr_len(this->size); ++i) {
        if (this->chain[i] != other.chain[i]) {
            return false;
        }
    }
    return true;
}

bool RNA::operator != (const RNA &other) const {
    return !(*this == other);
}

RNA RNA::operator + (const RNA &other) const {

    if (this->size + other.size == 0) {
        return RNA();
    }

    RNA tmp;
    tmp.capacity = this->capacity + other.capacity;
    tmp.size = this->size + other.size;
    tmp.chain = new byte[arr_len(tmp.capacity)];

    memcpy(tmp.chain, this->chain, sizeof(byte)*arr_len(this->size));
    memcpy(&tmp.chain[arr_len(this->size)], other.chain, sizeof(byte)*arr_len(other.size));

    short_t shift = this->size % 4;
    int_t this_len = this->size / 4;

    if (shift) {

        tmp.chain[this_len] |= tmp.chain[this_len+1] >> 2*shift;

        for (int_t i=this_len+1; i < arr_len(tmp.size); ++i) {
            tmp.chain[i] <<= 2*(4 - shift);
            tmp.chain[i] |= tmp.chain[i+1] >> 2*shift;
        }
    }

    return tmp;
}

RNA RNA::operator ! () const {

    RNA tmp = *this;

    for (int_t i=0; i < arr_len(tmp.size); ++i) {
        tmp.chain[i] = ~tmp.chain[i];
    }

    // Обнуление хвоста последнего байта
    tmp.chain[tmp.size/4] >>= 2*(4 - tmp.size%4);
    tmp.chain[tmp.size/4] <<= 2*(4 - tmp.size%4);

    return tmp;
}

// Проверка на комплементарность

bool RNA::isComplementary(const RNA &other) {
    return *this == !other;
}

// Разделение цепочки пополам

RNA RNA::split(int_t index) {

    if (this->size == 0) {
        return RNA();
    }
    if (index < 0 || index >= this->size) {
        return *this;
    }

    RNA tmp;
    tmp.capacity = this->capacity - index;
    tmp.size = this->size - index;
    tmp.chain = new byte[arr_len(tmp.capacity)];

    memcpy(tmp.chain, &this->chain[index/4], sizeof(byte)*(tmp.size/4 + 1));

    short_t shift = index % 4;
    if (shift) {
        for (int_t i=0; i < tmp.size/4 + 1; ++i) {
            tmp.chain[i] <<= 2*shift;
            tmp.chain[i] |= tmp.chain[i+1] >> 2*(4 - shift);
        }
    }

    return tmp;
}

// Перегрузка оператора индексирования

RNA::NuclReference RNA::operator [] (int_t index) {
    return NuclReference(index, *this);
}
RNA::NuclReference& RNA::NuclReference::operator = (Nucl nucl) {

    rna->chain[index/4] &= (0xFF - (0x03 << 2*(3 - index%4)));
    rna->chain[index/4] |= nucl << 2*(3 - index%4);

    return *this;
}
RNA::NuclReference& RNA::NuclReference::operator = (RNA::NuclReference& other) {
    return this->operator=((Nucl)other);
}
RNA::NuclReference::operator Nucl() {
    return (Nucl)this->rna->get(index);
}

// Перегрузка константного оператора индексирования

RNA::NuclReferenceConst RNA::operator [] (int_t index) const {
    return NuclReferenceConst(index, *this);
}
RNA::NuclReferenceConst::operator Nucl() const {
    return (Nucl)this->rna->get(index);
}