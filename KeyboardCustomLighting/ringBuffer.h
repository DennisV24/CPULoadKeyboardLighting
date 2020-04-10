#pragma once

#include <array>
#include <exception>
#include <assert.h>

template <class T, unsigned int SIZE>
class RingBuffer
{
public:
    RingBuffer() :
        curIdx(0),
        curSize(0),
        hasBeenFilled(false),
        data() {}

    void addVal(T& newVal) 
    {
        data.at(curIdx) = newVal;
        curIdx++;
        if (!hasBeenFilled)
        {
            curSize++;
        }

        if (curIdx == maxSize)
        {
            hasBeenFilled = true;
            assert(curSize == maxSize);
            curIdx = 0;
        }
    }

    T sumData() const 
    {
        T retval(0);
        for (unsigned int i = 0; i < curSize; i++)
        {
            retval += data[i];
        }
        return retval;
    }

    T averageData() const 
    {
        T retval = sumData();
        retval = retval / T(curSize);
        return retval;
    }

    auto getDataTimeSorted(unsigned int numSamples) const
    {
        assert(numSamples <= SIZE);
        std::array<T, SIZE> retval;
        retval.fill(T(0));
        
        unsigned bufferIdx = curIdx;
        unsigned idx = 0;
        for (unsigned int idx = 0; idx < numSamples; idx++) {
            retval.at(idx) = data.at(bufferIdx);
            bufferIdx++;

            if (bufferIdx >= SIZE) {
                bufferIdx = 0;
            }
        }
        return retval;
    }

    unsigned int size() const {
        return SIZE;
    }

    T operator[](unsigned int idx) const {
        if (idx >= maxSize) {
            throw "Out of bounds access";
        }
        return data[idx];
    }

private:
    static constexpr unsigned int maxSize = SIZE;
    unsigned int curIdx;
    unsigned int curSize;
    bool hasBeenFilled;
    std::array<T, SIZE> data;
};

