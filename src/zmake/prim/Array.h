// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

template <typename T>
struct Array : public std::vector<T>
{
   template <typename U> 
   int findFirst(const U& val) const
   {
      for (int i = 0; i < len(); ++i)
      {
         if (datum(i) == val)
         {
            return i;
         }
      }
      return -1;
   }

   template <typename U> 
   bool contains(const U& rhs) const
   {
      return findFirst(rhs) != -1;
   }

   void fastRemoveIdx(int idx)
   {
      int last = len() - 1;
      datum(idx) = datum(last);
      std::vector<T>::pop_back();
   }

   bool fastRemoveValue(const T& val)
   {
      int pos = findFirst(val);
      if (pos != -1)
      {
         fastRemoveIdx(pos);
         return true;
      }
      else
      {
         return false;
      }
   }

   void fastRemoveValues(const Array<T>& rhs)
   {
      for (int i = 0; i < rhs.len(); ++i)
      {
         fastRemoveValue(rhs[i]);
      }
   }

   Array<T>& operator+=(const Array<T>& rhs)
   {
      std::vector<T>::insert(std::vector<T>::end(), rhs.begin(), rhs.end());
      return *this;
   }

   Array<T>& operator+=(const T& rhs)
   {
      std::vector<T>::push_back(rhs);
      return *this;
   }

   int len() const
   {
      return (int)std::vector<T>::size();
   }

   bool empty() const
   {
      return len() == 0;
   }

   bool filled() const
   {
      return !empty();
   }

   T& datum(int i)
   {
      return std::vector<T>::operator[](i);
   }

   const T& datum(int i) const
   {
      return std::vector<T>::operator[](i);
   }
};
