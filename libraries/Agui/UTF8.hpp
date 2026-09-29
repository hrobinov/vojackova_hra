#pragma once
#include <stdlib.h>
#include <string>
#include <string_view>
#include <cassert>

namespace agui
{
  /** Set of useful UTF8 methods.
   * Most methods are template so this class is not DLL exported. It is inline. */
  class UTF8 {
  public:
    /** Moves the iterator to next unicode character in the string.
     * @return Number of bytes skipped. */
    template<typename Iterator1, typename Iterator2>
    inline static size_t bringToNextUnichar(Iterator1& it,
                                            const Iterator2& last)
    {
      if (it == last)
        return 0;
      unsigned char c;
      size_t res = 1;
      for (++it; last != it; ++it, ++res)
      {
        c = *it;
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0))
          break;
      }

      return res;
    }

    inline static size_t bringToNextUnichar(size_t& index, const std::string& str)
    {
      return bringToNextUnichar(index, std::string_view(str));
    }

    /** Moves the iterator to next unicode character in the string.
     * @return Number of bytes skipped. */
    inline static size_t bringToNextUnichar(size_t& index,
                                            std::string_view str)
    {
      if (index >= str.length())
        return 0;
      unsigned char c;
      size_t res = 1;
      for (++index; index < str.length(); ++index, ++res)
      {
        c = str[index];
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0))
          break;
      }

      return res;
    }

    /** Moves the iterator to previous unicode character in the string.
     * @return Number of bytes skipped. */
    template<typename Iterator1, typename Iterator2>
    inline static size_t bringToPrevUnichar(Iterator1& it,
                                            const Iterator2& last)
    {
      if (it == last)
        return 0;
      unsigned char c;
      size_t res = 1;
      for (--it; last != it; --it, ++res)
      {
        c = *it;
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0))
          break;
      }

      return res;
    }

    inline static size_t bringToPrevUnichar(size_t& index, const std::string& str)
    {
      return bringToPrevUnichar(index, std::string_view(str));
    }

    /** Moves the iterator to previous unicode character in the string.
     * @return Number of bytes skipped. */
    inline static size_t bringToPrevUnichar(size_t& index,
                                            std::string_view str)
    {
      if (index == 0)
        return 0;
      unsigned char c;
      size_t res = 1;
      for (--index; index > 0; --index, ++res)
      {
        c = str[index];
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0))
          break;
      }

      return res;
    }

    /** Moves the iterator forward by count UTF8 characters.
     * @return Number of bytes skipped. */
    template<typename Iterator>
    inline static size_t multIncUtf8StringIterator(Iterator& it,
                                                   const Iterator& last, size_t count)
    {
      size_t res = 0;
      for (size_t i = 0; i < count; i++)
      {
        if (it == last)
          break;
        res += bringToNextUnichar(it, last);
      }

      return res;
    }

    /** Moves the iterator to the first UTF8 character.
     * @return Number of bytes skipped. */
    template<typename Iterator1, typename Iterator2>
    inline static size_t _decUtf8StringIterator(Iterator1& it,
                                                const Iterator2& first)
    {
      if (it == first)
        return 0;
      size_t res = 1;
      unsigned char c;
      --it;
      for (; first != it; --it, ++res)
      {
        c = *it;
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0))
          break;
      }

      return res;
    }
    /** Moves the iterator forward by count UTF8 characters.
     * @return Iterator. */
    template<typename Iterator>
    inline static Iterator getMultIncUtf8StringIterator(Iterator it,
                                                        const Iterator& last, size_t count)
    {
      multIncUtf8StringIterator(it, last, count);
      return it;
    }

    /** Moves the iterator to the specified position.
     * @return Iterator. */
    inline static std::string::iterator positionToIterator(std::string& str, size_t pos)
    {
      std::string::iterator res = str.begin();
      multIncUtf8StringIterator(res, str.end(), pos);
      return res;
    }

    /** Moves the iterator to the specified position.
     * @return Iterator. */
    inline static std::string_view::iterator positionToIterator(std::string_view str, size_t pos)
    {
      std::string_view::iterator res = str.begin();
      multIncUtf8StringIterator(res, str.end(), pos);
      return res;
    }

    /** @return The number of UTF8 characters in this string. */
    inline static size_t length(const std::string_view str)
    {
      size_t res = 0;
      auto it = str.begin();
      auto end = str.end();
      for (; it != end; bringToNextUnichar(it, end))
        res++;

      return res;
    }

    /** @return The UTF8 sub string. All values are in UTF8 characters.
     * The returned substring is from start to start + n UTF8 characters. */
    inline static std::string subStr(std::string_view str, size_t start,
                                     size_t n = (size_t)-1)
    {
      if (n == (size_t)-1)
        return std::string(positionToIterator(str, start), str.end());
      else
        return std::string(
          positionToIterator(str, start),
          positionToIterator(str, start + n));
    }

    /** @return The UTF8 sub string. All values are in UTF8 characters.
     * The returned substring is from start to start + n UTF8 characters. */
    inline static std::string_view stringViewSubStr(std::string_view str, size_t start,
                                                    size_t n = (size_t)-1)
    {
      auto startIt = positionToIterator(str, start);
      auto endIt = str.end();

      if (n != (size_t)-1)
        endIt = positionToIterator(str, start + n);

      return std::string_view(str.data() + std::distance(str.begin(), startIt), std::distance(startIt, endIt));
    }

    /** Erases just like a normal string but values are in UTF8 characters, not bytes. */
    inline static void erase(std::string& str, size_t start, size_t n = (size_t)-1)
    {
      std::string::iterator startIt = positionToIterator(str, start);
      std::string::iterator endIt = getMultIncUtf8StringIterator(startIt, str.end(), n);

      str.erase(startIt, endIt);
    }
    /** Inserts just like a normal string but values are in UTF8 characters, not bytes. */
    inline static void insert(std::string& str, size_t start, const std::string& s)
    {
      std::string::iterator it = positionToIterator(str, start);
      str.insert(it, s.begin(), s.end());
    }

    /** @return The number of bytes the UTF32 encoded character 'c' will occupy in UTF8 form. */
    inline static size_t getUnicharLength(int c)
    {

      size_t uc = c;

      if (uc <= 0x7f)
        return 1;
      if (uc <= 0x7ff)
        return 2;
      if (uc <= 0xffff)
        return 3;
      if (uc <= 0x10ffff)
        return 4;
      /* The rest are illegal. */
      return 0;
    }
    /** @return The number of bytes written to outputChars.
     * outputChars should have enough room for the number of bytes + nullptr.
     * Usually 5 bytes is enough. */
    inline static size_t encodeUtf8(char outputChars[], int inputUnichar)
    {
      size_t uc = inputUnichar;

      if (uc <= 0x7f)
      {
        outputChars[0] = static_cast<char>(uc);
        return 1;
      }

      if (uc <= 0x7ff)
      {
        outputChars[0] = 0xC0 | ((uc >> 6) & 0x1F);
        outputChars[1] = 0x80 | (uc & 0x3F);
        return 2;
      }

      if (uc <= 0xffff)
      {
        outputChars[0] = 0xE0 | ((uc >> 12) & 0x0F);
        outputChars[1] = 0x80 | ((uc >> 6) & 0x3F);
        outputChars[2] = 0x80 | (uc & 0x3F);
        return 3;
      }

      if (uc <= 0x10ffff)
      {
        outputChars[0] = 0xF0 | ((uc >> 18) & 0x07);
        outputChars[1] = 0x80 | ((uc >> 12) & 0x3F);
        outputChars[2] = 0x80 | ((uc >> 6) & 0x3F);
        outputChars[3] = 0x80 | (uc & 0x3F);
        return 4;
      }

      /* Otherwise is illegal. */
      return 0;
    }

    inline static size_t getByteIndexFromCharIndex(size_t charIndex, const std::string& text)
    {
      size_t byteIndex = 0;
      for (size_t currentChar = 0; currentChar < charIndex; currentChar++)
      {
        [[maybe_unused]] size_t charLength = UTF8::bringToNextUnichar(byteIndex, text);
        assert(charLength >= 1);
      }
      return byteIndex;
    }

    inline static size_t getCharIndexFromByteIndex(size_t byteIndex, const std::string& text)
    {
      size_t charIndex = 0;
      size_t currentByteIndex = 0;
      while (currentByteIndex < byteIndex)
      {
        [[maybe_unused]] size_t charLength = UTF8::bringToNextUnichar(currentByteIndex, text);
        assert(charLength >= 1);
        charIndex++;
      }
      return charIndex;
    }
  };
}
