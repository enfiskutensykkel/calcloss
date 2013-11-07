#ifndef __RANGE_H__
#define __RANGE_H__

#include <vector>
#include <sys/time.h>
#include <tr1/cstdint>


/* Forward declaration of flowdata */
class flowdata;



/*
 * A range object represents a byte range (typically payload of a TCP segment).
 */
class range
{
	friend class flowdata;

	private:
		uint64_t seqno_lo;	// the lower sequence number in the range (range start)
		uint64_t seqno_hi;	// the upper sequence number in the range (range end)

	public:
		range(uint64_t seqno_start, uint64_t seqno_end);
		range(const range& other);
		range& operator=(const range& rhs);
		bool operator<(const range& rhs);
		bool operator<(const range& rhs) const;
};



/*
 * A rangedata object represents statistics about a byte range.
 */
class rangedata
{
	friend class flowdata;

	public:
	
		/* The elapsed time between when the range first was sent until it got ACK'ed */
		timeval latency();

		/* Number of times the range was retransmitted */
		int retransmissions();

		/* Number of times the range was lost */
		int lost();

		/* Number of times the range was ACK'ed (more than 1 means that we got dupACKs for this range) */
		int ack_count();

		/* Constructors and overloads for comparison operators */
		rangedata(const timeval& timestamp);

		inline rangedata(const rangedata& other)
		{
			*this = other;
		};

		rangedata& operator=(const rangedata& other);

	private: 
		std::vector<timeval> sent;	// the timestamps this range was registered as sent
		std::vector<timeval> ackd;	// the timestamps this range was acknowledged
};

#endif
