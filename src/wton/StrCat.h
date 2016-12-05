// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

// strCat("ab", 17, "c", ...) concatenates the char* representation of any number of parameters -> "ab17c"

// strBuf is storage for temporary strings
char* strBuf();
void finishStrBuf(char*);

// copies src to dst
// up to end, inclusive
// returns the terminal character
// doesn't guarantee null terminal
char* _strCpy(char* dst, const char* src, const char* end);

const char* toString(const WtonString& s);
const char* toString(const char* s);
const char* toString(const int& i);
const char* strCat();

// TODO should probably use variadic templates here
template <class T1>
inline const char* strCat(const T1& t1)
{
   const char* s1 = toString(t1);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2>
inline const char* strCat(const T1& t1, const T2& t2)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);
   const char* s7 = toString(t7);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   dst = _strCpy(dst, s7, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);
   const char* s7 = toString(t7);
   const char* s8 = toString(t8);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   dst = _strCpy(dst, s7, end);
   dst = _strCpy(dst, s8, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);
   const char* s7 = toString(t7);
   const char* s8 = toString(t8);
   const char* s9 = toString(t9);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   dst = _strCpy(dst, s7, end);
   dst = _strCpy(dst, s8, end);
   dst = _strCpy(dst, s9, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);
   const char* s7 = toString(t7);
   const char* s8 = toString(t8);
   const char* s9 = toString(t9);
   const char* s10 = toString(t10);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   dst = _strCpy(dst, s7, end);
   dst = _strCpy(dst, s8, end);
   dst = _strCpy(dst, s9, end);
   dst = _strCpy(dst, s10, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11>
inline const char* strCat(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10, const T11& t11)
{
   const char* s1 = toString(t1);
   const char* s2 = toString(t2);
   const char* s3 = toString(t3);
   const char* s4 = toString(t4);
   const char* s5 = toString(t5);
   const char* s6 = toString(t6);
   const char* s7 = toString(t7);
   const char* s8 = toString(t8);
   const char* s9 = toString(t9);
   const char* s10 = toString(t10);
   const char* s11 = toString(t11);

   char *buf, *dst, *end;
   dst = buf = strBuf();
   end = buf + kMaxStrCatLen - 1;

   dst = _strCpy(dst, s1, end);
   dst = _strCpy(dst, s2, end);
   dst = _strCpy(dst, s3, end);
   dst = _strCpy(dst, s4, end);
   dst = _strCpy(dst, s5, end);
   dst = _strCpy(dst, s6, end);
   dst = _strCpy(dst, s7, end);
   dst = _strCpy(dst, s8, end);
   dst = _strCpy(dst, s9, end);
   dst = _strCpy(dst, s10, end);
   dst = _strCpy(dst, s11, end);
   *end = 0;

   finishStrBuf(buf);
   return buf;
}
