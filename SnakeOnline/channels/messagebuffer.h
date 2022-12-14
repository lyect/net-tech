#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include <QtGlobal>
#include <QVector>

namespace Network {

template <typename T>
class MessageBuffer {
private:
	QVector<T> buffer;
	QVector<bool> stored;
	const qsizetype capacity = 100;
	int stored_num;
	long long base;
public:

	MessageBuffer() {
		buffer = QVector<T>();
		stored = QVector<bool>();
		stored_num = 0;
		base = -1;
	}

	void setBase(const long long &_base) {
		base = _base;
	}

	const long long &getBase() const {
		return base;
	}

	const qsizetype &getCapacity() const {
		return capacity;
	}

	bool isFull() {
		return stored_num == capacity;
	}

	void insertByPosition(const T &value, const long long &pos) {
		if (buffer.size() == 0) {
			buffer.resize(capacity);
			stored.resize(capacity, false);
			base = pos - 1;
		}

		int index = pos % capacity;

		if (!stored[index]) {
			buffer[index] = value;
			stored[index] = true;
			stored_num += 1;
		}
	}

	T getByPosition(const long long &pos) {

		int index = pos % capacity;

		stored[index] = false;
		stored_num -= 1;

		if ((base + 1) % capacity == index) {
			base += 1;
		}

		return buffer[index];
	}

	bool hasContinuousBlock() {
		return stored[(base + 1) % capacity];
	}

	QPair<int, int> getContinuousBlock() const {
		int cont_block_start = base + 1;

		int cont_block_end = cont_block_start;
		while (stored[cont_block_end % capacity]) {
			++cont_block_end;
		}

		return {cont_block_start, cont_block_end};
	}
};

}

#endif // MESSAGEBUFFER_H
