#pragma once

#ifndef uint64_t
typedef unsigned long long uint64_t;
#endif

#ifndef int64_t
typedef long long int64_t;
#endif

//union FloatToIntConvertor
//{
//    float float_value;
//    int int_value;
//};

//#TODO: quaternion packing and delta values
// https://gist.github.com/gafferongames/bb7e593ba1b05da35ab6#file-delta_compression-cpp-L613
class BitPacker
{
public:
							BitPacker(void* data, int bytes) { InitWrite(data, bytes); } // operates on pre-allocated data only
							BitPacker(const void* data, int bytes) { InitRead(data, bytes); } // operates on pre-allocated data only

	void					InitWrite(void* data, int bytes);
	void					InitRead(const void* data, int bytes);
	void					Reset(); // keeps data, resets cursor
	const unsigned char*	GetData() const { return m_pData; }
	int						GetBytesWritten() const { return (m_cursor + (m_bitCursor != 0)); }
	int						GetBytesRead() const { return m_cursor; }

	// writing
	void					WriteBits(int value, int numBits);
	void					WriteChar(char value);
	void					WriteUChar(unsigned char value);
	void					WriteShort(short value);
	void					WriteUShort(unsigned short value);
	void					WriteInt(int value);
	void					WriteUInt(unsigned int value);
	void					WriteFloat(float value);
	void					WriteFloatCompressed(float value, float min, float max, float resolution); // "resolution" is the desired accuracy (0.01 / 0.1 / 0.5 etc.)
	void					WriteBytes(const void* data, int bytes);
	void					WriteString(const char* str);
	//void					WriteVector3(const Vector3& v);
	//void					WriteUnitVectorCompressed(const Vector3& v);
	void					WriteAlign();

	// reading
	int						ReadBits(int numBits) const;
	char					ReadChar() const;
	unsigned char			ReadUChar() const;
	short					ReadShort() const;
	unsigned short			ReadUShort() const;
	int						ReadInt() const;
	unsigned int			ReadUInt() const;
	float					ReadFloat() const;
	float					ReadFloatCompressed(float min, float max, float resolution) const;
	int						ReadBytes(void* data, int bytes) const;
	int						ReadString(char* str, int maxStrSize) const; // returns the length of the read string, including the null character
	//void					ReadVector3(Vector3& v) const;
	//void					ReadUnitVectorCompressed(Vector3& v) const;
	void					ReadAlign() const;
	int						GetReadRemaining() const { return (m_dataSize - m_cursor); }

private:
	// TODO split into read/write buffers
	unsigned char*			m_pData; // read/write
	int						m_dataSize; // buffer size in bytes

	mutable int				m_cursor; // current read/write size in bytes. mutable because we also use it for reading (const functions)
	mutable int				m_bitCursor; // how many bits we wrote to or read from the last byte
	uint64_t				m_accumulator; // only used when writing to accumulate byte fractions
};

inline void BitPacker::InitRead(const void* data, int bytes)
{
	m_pData = (unsigned char*)data;
	m_dataSize = bytes;

	m_cursor = 0;
	m_bitCursor = 0;
	m_accumulator = 0;
}

inline void BitPacker::InitWrite(void* data, int bytes)
{
	m_pData = (unsigned char*)data;
	m_dataSize = bytes;

	m_cursor = 0;
	m_bitCursor = 0;
	m_accumulator = 0;
}

inline void BitPacker::Reset()
{
	m_cursor = 0;
	m_bitCursor = 0;
	m_accumulator = 0;
}

inline void BitPacker::WriteAlign()
{
	m_cursor += m_bitCursor != 0;
	m_bitCursor = 0;
	m_accumulator = 0;
}

inline void BitPacker::ReadAlign() const
{
	m_cursor += m_bitCursor != 0;
	m_bitCursor = 0;
}

inline char BitPacker::ReadChar() const
{
	return (char)ReadBits(8);
}

inline unsigned char BitPacker::ReadUChar() const
{
	return (unsigned char)ReadBits(8);
}

inline short BitPacker::ReadShort() const
{
	return (short)ReadBits(16);
}

inline unsigned short BitPacker::ReadUShort() const
{
	return (unsigned short)ReadBits(16);
}

inline int BitPacker::ReadInt() const
{
	return ReadBits(32);
}

inline unsigned int BitPacker::ReadUInt() const
{
	return (unsigned int)ReadBits(32);
}

inline float BitPacker::ReadFloat() const
{
	float value;

	// WARNING this breaks strict aliasing, bugs out on GCC if -fstrict-aliasing. Use -fno-strict-aliasing
	*(int*)&value = ReadBits(32);

	//FloatToIntConvertor v;
	//v.int_value = ReadBits(32);
	//value = v.float_value;

	return value;
}
/*
inline void BitPacker::ReadVector3(Vector3& v) const
{
	v.x = ReadFloat();
	v.y = ReadFloat();
	v.z = ReadFloat();
}
*/
inline void BitPacker::WriteChar(char value)
{
	WriteBits(value, 8);
}

inline void BitPacker::WriteUChar(unsigned char value)
{
	WriteBits(value, 8);
}

inline void BitPacker::WriteShort(short value)
{
	WriteBits(value, 16);
}

inline void BitPacker::WriteUShort(unsigned short value)
{
	WriteBits(value, 16);
}

inline void BitPacker::WriteInt(int value)
{
	WriteBits(value, 32);
}

inline void BitPacker::WriteUInt(unsigned int value)
{
	WriteBits(value, 32);
}

inline void BitPacker::WriteFloat(float value)
{
	// WARNING this breaks strict aliasing, bugs out on GCC if -fstrict-aliasing. Use -fno-strict-aliasing
	WriteBits(*(int *)&value, 32);

	//FloatToIntConvertor v;
	//v.float_value = value;
	//WriteBits(v.int_value, 32);
}
/*
inline void BitPacker::WriteVector3(const Vector3& v)
{
	WriteFloat(v.x);
	WriteFloat(v.y);
	WriteFloat(v.z);
}
*/
