union mmxVal
{
	int8	i8[8];
	int16	i16[4];
	int32	i32[2];
	int64	i64[1];
  
	uint8	u8[8];
	uint16	u16[4];
	uint32	u32[2];
  uint64	u64[1];
  
  float	r32[2];
  float	r64[1];
}

union xmmVal
{
	int8	i8[16];
	int16	i16[8];
	int32	i32[4];
	int64	i64[2];
  
	uint8	u8[16];
	uint16	u16[8];
	uint32	u32[4];
  uint64	u64[2];
  
  float	r32[4];
  float	r64[2];
  
  __mm128 sses;
  __mm128d ssed;
  __mm128i ssei;
  
  
  char* ToString_i8(char* s, size_t len);
	char* ToString_i16(char* s, size_t len);
	char* ToString_i32(char* s, size_t len);
	char* ToString_i64(char* s, size_t len);

	char* ToString_u8(char* s, size_t len);
	char* ToString_u16(char* s, size_t len);
	char* ToString_u32(char* s, size_t len);
	char* ToString_u64(char* s, size_t len);

	char* ToString_x8(char* s, size_t len);
	char* ToString_x16(char* s, size_t len);
	char* ToString_x32(char* s, size_t len);
	char* ToString_x64(char* s, size_t len);

	char* ToString_r32(char* s, size_t len);
char* ToString_r64(char* s, size_t len);
}

/*

char* xmmVal::ToString_i8(char* s, size_t len)
{
	snprintf(s, len, "%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
		i8[0], i8[1], i8[2], i8[3], i8[4], i8[5], i8[6], i8[7], i8[8], i8[9], i8[10], i8[11], i8[12], i8[13], i8[14], i8[15]);
	return s;
}

char* xmmVal::ToString_i16(char* s, size_t len)
{
	snprintf(s, len, "%8d %8d %8d %8d %8d %8d %8d %8d",
		i16[0], i16[1], i16[2], i16[3], i16[4], i16[5], i16[6], i16[7]);
	return s;
}

char* xmmVal::ToString_i32(char* s, size_t len)
{
	snprintf(s, len, "%12d %12d %12d %12d",
		i32[0], i32[1], i32[2], i32[3]);
	return s;
}

char* xmmVal::ToString_i64(char* s, size_t len)
{
	snprintf(s, len, "%16lld %16lld",
		i64[0], i64[1]);
	return s;
}

char* xmmVal::ToString_u8(char* s, size_t len)
{
	snprintf(s, len, "%4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u %4u",
		i8[0], i8[1], i8[2], i8[3], i8[4], i8[5], i8[6], i8[7],
		i8[8], i8[9], i8[10], i8[11], i8[12], i8[13], i8[14], i8[15]);
	return s;
}

char* xmmVal::ToString_u16(char* s, size_t len)
{
	snprintf(s, len, "%8u %8u %8u %8u %8u %8u %8u %8u",
		i16[0], i16[1], i16[2], i16[3], i16[4], i16[5], i16[6], i16[7]);
	return s;
}

char* xmmVal::ToString_u32(char* s, size_t len)
{
	snprintf(s, len, "%12u %12u %12u %12u",
		i32[0], i32[1], i32[2], i32[3]);
	return s;
}

char* xmmVal::ToString_u64(char* s, size_t len)
{
	snprintf(s, len, "%16llu %16llu",
		i64[0], i64[1]);
	return s;
}

char* xmmVal::ToString_x8(char* s, size_t len)
{
	snprintf(s, len, "%4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x %4x",
		i8[0], i8[1], i8[2], i8[3], i8[4], i8[5], i8[6], i8[7],
		i8[8], i8[9], i8[10], i8[11], i8[12], i8[13], i8[14], i8[15]);
	return s;
}

char* xmmVal::ToString_x16(char* s, size_t len)
{
	snprintf(s, len, "%8x %8x %8x %8x %8x %8x %8x %8x",
		i16[0], i16[1], i16[2], i16[3], i16[4], i16[5], i16[6], i16[7]);
	return s;
}

char* xmmVal::ToString_x32(char* s, size_t len)
{
	snprintf(s, len, "%16x %16x %16x %16x",
		i32[0], i32[1], i32[2], i32[3]);
	return s;
}

char* xmmVal::ToString_x64(char* s, size_t len)
{
	snprintf(s, len, "%16llx %16llx",
		i64[0], i64[1]);
	return s;
}

char* xmmVal::ToString_r32(char* s, size_t len)
{
	snprintf(s, len, "%12.6f %12.6f | %12.6f %12.6f",
		r32[0], r32[1], r32[2], r32[3]);
	return s;
}

char* xmmVal::ToString_r64(char* s, size_t len)
{
	snprintf(s, len, "%24.12f | %24.12f",
		r64[0], r64[1]);
	return s;
}
 */
