// mqueue.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "mqueue.h"

NAMESPACE_BEGIN(CryptoPP)

MessageQueue::MessageQueue(unsigned int nodeSize)
	: m_queue(nodeSize), m_lengths(1, 0U), m_messageCounts(1, 0U)
{
}

unsigned int MessageQueue::CopyRangeTo2(BufferedTransformation &target, unsigned long &begin, unsigned long end, const std::string &channel, bool blocking) const
{
	if (begin >= MaxRetrievable())
		return 0;

	return m_queue.CopyRangeTo2(target, begin, STDMIN(MaxRetrievable(), end), channel, blocking);
}

unsigned int MessageQueue::TransferTo2(BufferedTransformation &target, unsigned long &transferBytes, const std::string &channel, bool blocking)
{
	transferBytes = STDMIN(MaxRetrievable(), transferBytes);
	unsigned int blockedBytes = m_queue.TransferTo2(target, transferBytes, channel, blocking);
	m_lengths.front() -= transferBytes;
	return blockedBytes;
}

bool MessageQueue::GetNextMessage()
{
	if (NumberOfMessages() > 0 && !AnyRetrievable())
	{
		m_lengths.pop_front();
		if (m_messageCounts[0] == 0 && m_messageCounts.size() > 1)
			m_messageCounts.pop_front();
		return true;
	}
	else
		return false;
}

unsigned int MessageQueue::CopyMessagesTo(BufferedTransformation &target, unsigned int count, const std::string &channel) const
{
	ByteQueue::Walker walker(m_queue);
	std::deque<unsigned long>::const_iterator it = m_lengths.begin();
	unsigned int i;
	for (i=0; i<count && it != --m_lengths.end(); ++i, ++it)
	{
		walker.TransferTo(target, *it, channel);
		if (GetAutoSignalPropagation())
			target.ChannelMessageEnd(channel, GetAutoSignalPropagation()-1);
	}
	return i;
}

void MessageQueue::swap(MessageQueue &rhs)
{
	m_queue.swap(rhs.m_queue);
	m_lengths.swap(rhs.m_lengths);
}

const byte * MessageQueue::Spy(unsigned int &contiguousSize) const
{
	const byte *result = m_queue.Spy(contiguousSize);
	contiguousSize = (unsigned int)STDMIN((unsigned long)contiguousSize, MaxRetrievable());
	return result;
}

// *************************************************************

NAMESPACE_END
