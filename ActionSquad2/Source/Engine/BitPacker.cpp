#include "dxstdafx.h"

#include <stdio.h>
#include "BitPacker.h"

// ((uint64_t(1) << numBits) - 1) with numBits[0, 32]
static uint64_t g_64BitValueMask[33] = {
	0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,
	0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff,
	0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
	0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff, 
};

void BitPacker::WriteBits(int value, int numBits)
{
	if (numBits <= 0 || numBits > 32)
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBits() invalid numBits=%d\n", numBits);
		return;
	}

	if (value < 0 && (numBits < 8 || !MATH_IsPowerOfTwo((UINT32)numBits))) // signed values only for 8/16/32 bits (for now)
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBits() trying to write negative value %d with less than 32 bits (%d)\n", value, numBits);
		return;
	}

	if (numBits > (m_dataSize * 8 - (m_cursor * 8 + m_bitCursor)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBits() overflowing max size of %d bytes!\n", m_dataSize);
		return;
	}

	if (numBits != 32)
	{
		// not really OK. this is redundant when WriteBits() is called by one of WriteChar/Short/etc
		// also, if "value" is positive, there is guarantee that we want an unsigned value
		if (value > 0)
		{
			const int maxValue = (1 << numBits) - 1;
			if (value > maxValue)
			{
				ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBits() overflow of value %d when using only %d bits\n", value, numBits);
				return;
			}
		}
		else
		{
			const int maxValue = 1 << (numBits - 1);
			if (value < -maxValue)
			{
				ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBits() overflow of value %d when using only %d bits\n", value, numBits);
				return;
			}
		}
	}

	// 1. zero out the part of the value we don't need
	// 2. accumulate value to the 64bit buffer
	m_accumulator |= ((int64_t)value & g_64BitValueMask[numBits]) << m_bitCursor; //((int64_t)value & ((uint64_t(1) << numBits) - 1)) << m_bitCursor;
	m_bitCursor += numBits;

	// write to byte array if needed
	while (m_bitCursor >= 8)
	{
		m_pData[m_cursor++] = (unsigned char)(m_accumulator & 0xff);
		m_bitCursor -= 8;
		m_accumulator >>= 8;
	}

	// write remaining but don't advance cursor. in case this is the last call, we don't need to call EndWrite. Otherwise we will just overwrite the value on the next call.
	if (m_bitCursor > 0)
	{
		m_pData[m_cursor] = (unsigned char)(m_accumulator & 0xff);
	}
}

int BitPacker::ReadBits(int numBits) const
{
	if (numBits <= 0 || numBits > 32)
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::ReadBits() invalid numBits=%d\n", numBits);
		return 0;
	}

	if (numBits > (m_dataSize * 8 - (m_cursor * 8 + m_bitCursor)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::ReadBits() overflowing max size of %d bytes!\n", m_dataSize);
		return 0;
	}

	int value = 0;
	int readBits = 0;

	while (readBits < numBits)
	{
		const int numBitsToRead = min(numBits - readBits, 8 - m_bitCursor);
		int temp = (m_pData[m_cursor] >> m_bitCursor) & ((1 << numBitsToRead) - 1);

		m_bitCursor += numBitsToRead;
		if (m_bitCursor >= 8)
		{
			m_bitCursor -= 8;
			++m_cursor;
		}

		// accumulate value
		value |= temp << readBits;
		readBits += numBitsToRead;
	}

	return value;
}

void BitPacker::WriteBytes(const void* data, int bytes)
{
	if (bytes > (m_dataSize - GetBytesWritten()))
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::WriteBytes() overflowing max size of %d bytes while trying to write %d bytes\n", m_dataSize, bytes);
		return;
	}

	// make sure we start aligned on a byte boundary
	WriteAlign();

	// bulk write
	memcpy(m_pData + m_cursor, data, bytes);
	m_cursor += bytes;
}

int	BitPacker::ReadBytes(void* data, int bytes) const
{
	if (bytes > GetReadRemaining())
	{
		ErrorBox(K_ERR_CRITICAL, L"[Error] BitPacker::ReadBytes() overflowing max size of %d bytes while trying to read %d bytes\n", m_dataSize, bytes);
		return 0;
	}

	// make sure we start aligned on a byte boundary
	ReadAlign();

	// bulk read
	memcpy(data, m_pData + m_cursor, bytes);
	m_cursor += bytes;
	return bytes;
}

void BitPacker::WriteString(const char* str)
{
	if (str)
	{
		int len = strlen(str) + 1;
		WriteBytes(str, len); // also write the terminating character, so that we know when to stop when reading
	}
	else
	{
		char nullc = '\0';
		WriteBytes(&nullc, 1);
	}
}

int BitPacker::ReadString(char* str, int maxStrSize) const
{
	// make sure we start aligned on a byte boundary
	ReadAlign();

	// stop when hitting null character
	int len = 0;
	for (;;)
	{
		char c = m_pData[m_cursor++];
		if (c == '\0')
			break;

		if (len < (maxStrSize - 1))
			str[len++] = c;
	}
	str[len] = '\0';
	return len + 1;
}

void BitPacker::WriteFloatCompressed(float value, float min, float max, float resolution)
{
	// ex. writing 4.5f (meters) in the [-50,50] range with 0.01 resolution (centimeters)
	// delta = 100
	// numPossibleValues = 10,000
	// bitsRequired = 14
	// valueRatio = 0.545
	// iValue = 5450

	const float delta = max - min;
	const float numPossibleValues = delta / resolution;
	const int iNumPossibleValues = (int)ceilf(numPossibleValues);
	const int bitsRequired = MATH_GetBitsNeededForValue(iNumPossibleValues);
	const float valueRatio = LIMIT((value - min) / delta, 0.0f, 1.0f);
	const int iValue = (int)floorf(valueRatio * iNumPossibleValues + 0.5f);
	WriteBits(iValue, bitsRequired);
}

float BitPacker::ReadFloatCompressed(float min, float max, float resolution) const
{
	const float delta = max - min;
	const float numPossibleValues = delta / resolution;
	const int iNumPossibleValues = (int)ceilf(numPossibleValues);
	const int bitsRequired = MATH_GetBitsNeededForValue(iNumPossibleValues);
	const int iValue = ReadBits(bitsRequired);
	const float valueRatio = iValue / (float)iNumPossibleValues;
	return (valueRatio * delta + min);
}

// TODO: can be much better, look it up
//void BitPacker::WriteUnitVectorCompressed(const Vector3& v)
//{
//	// write the signs of the Z component and write only two of them as 2-byte values
//	WriteBits(v.z < 0.0f ? 1 : 0, 1);
//	WriteShort((short)(v.x * 32765.0f));
//	WriteShort((short)(v.y * 32765.0f));
//}
//
//void BitPacker::ReadUnitVectorCompressed(Vector3& v) const
//{
//	int signZ = ReadBits(1);
//	v.x = ReadShort() / 32765.0f;
//	v.y = ReadShort() / 32765.0f;
//	v.z = sqrtf(1.0f - v.x * v.x - v.y * v.y);
//	if (signZ)
//		v.z = -v.z;
//}
