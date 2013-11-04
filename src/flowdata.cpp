#include "flow.h"
#include "range.h"
#include <map>
#include <vector>
#include <tr1/cstdint>
#include <sys/time.h>
#include <assert.h>




void flowdata::get_ranges(range_list& list, range& key, bool include)
{
	range_map::iterator it, last, lo, hi, inserted;

	lo = ranges.lower_bound(key);
	hi = ranges.upper_bound(key);

	if (lo == hi)
	{
		return;
	}

	last = hi--;

	// Check if we have new leading data
	if (key.seqno_lo < lo->first.seqno_lo)
	{
		inserted = ranges.insert(lo, range_map::value_type(range(key.seqno_lo, lo->first.seqno_lo), lo->second));
		if (include)
			list.push_back(inserted);
	}

	// Check if we have new trailing data
	if (key.seqno_hi > hi->first.seqno_hi)
	{
		last = ranges.insert(hi, range_map::value_type(range(hi->first.seqno_hi, key.seqno_hi), hi->second));
		if (include)
			list.push_back(last);
	}

	// Find partial matches and split existing ranges
	for (it = lo; it != last;)
	{
		range curr(it->first);
		rangedata& data = it->second;

		if (key.seqno_lo <= curr.seqno_lo && key.seqno_hi >= curr.seqno_hi)
		{
			// Existing range:  |-----|
			// New range:       |-----| 
			inserted = it++;
		}

		else if (key.seqno_lo > curr.seqno_lo && key.seqno_hi < curr.seqno_hi)
		{
			// Existing range: |-----|
			// New range:        |-| 
			ranges.insert(it, range_map::value_type(range(curr.seqno_lo, key.seqno_lo), data));
			inserted = ranges.insert(it, range_map::value_type(range(key.seqno_lo, key.seqno_hi), data));
			ranges.insert(it, range_map::value_type(range(key.seqno_hi, curr.seqno_hi), data));
			ranges.erase(it++);
		}

		else if (key.seqno_lo > it->first.seqno_lo)
		{
			// Existing range: |-----|
			// New range:        |---|
			ranges.insert(it, range_map::value_type(range(curr.seqno_lo, key.seqno_lo), data));
			inserted = ranges.insert(it, range_map::value_type(range(key.seqno_lo, curr.seqno_hi), data));
			ranges.erase(it++);
		}

		else if (key.seqno_hi < it->first.seqno_hi)
		{
			// Existing range: |-----|
			// New range:      |---|
			inserted = ranges.insert(it, range_map::value_type(range(curr.seqno_lo, key.seqno_hi), data));
			ranges.insert(it, range_map::value_type(range(key.seqno_hi, curr.seqno_hi), data));
			ranges.erase(it++);
		}

		else
		{
			// This really shouldn't happen!
			assert(false);
		}

		list.push_back(inserted);
	}

	return;
}



void flowdata::register_sent(uint32_t start, uint32_t end, const timeval& ts)
{
	range key(adjust(start), adjust(end));
	range_list ranges;
	get_ranges(ranges, key, false);

	// TODO: update last_segment if ts > last_segment

	if (ranges.empty())
	{
		// We have a completely new range
		this->ranges.insert(std::pair<range,rangedata>(key, ts));
	}
	else
	{
		// Update existing ranges
		for (range_list::iterator i = ranges.begin(); i != ranges.end(); ++i)
		{
			(*i)->second.sent.push_back(ts);
		}
	}
}



void flowdata::register_ack(uint32_t ackno, const timeval& ts)
{
}



flowdata::flowdata(uint32_t first_seqno, const timeval& first_ts)
	: first_seqno(first_seqno), highest_ackd(first_seqno), rtt(-1)
	, first_ts(first_ts), last_ts(first_ts)
{
}



flowdata& flowdata::operator=(const flowdata& rhs)
{
	first_seqno = rhs.first_seqno;
	highest_ackd = rhs.highest_ackd;
	rtt = rhs.rtt;
	first_ts = rhs.first_ts;
	last_ts = rhs.last_ts;

	// TODO: Only copy the members that needs to be copied, we should avoid deep-copies anyway
}